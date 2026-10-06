#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "mqtt_client.h"

#include "dht11.h"
#include "secrets.h"

/* =========================================================
 * CA certificate declarations
 * ========================================================= */

extern const uint8_t ca_crt_start[] asm("_binary_ca_crt_start");
extern const uint8_t ca_crt_end[]   asm("_binary_ca_crt_end");

/* =========================================================
 * Configuration
 * ========================================================= */

#define DHT11_GPIO          4

#define MQTT_TOPIC          "iot/esp32/dht11"
#define MQTT_BROKER_PORT    8883


/* =========================================================
 * Wi-Fi
 * ========================================================= */

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

#define WIFI_MAX_RETRY      5


/* =========================================================
 * Logging
 * ========================================================= */

static const char *TAG = "MAIN";


/* =========================================================
 * Wi-Fi variables
 * ========================================================= */

static EventGroupHandle_t wifi_event_group;

static int wifi_retry_count = 0;


/* =========================================================
 * MQTT variables
 * ========================================================= */

static esp_mqtt_client_handle_t mqtt_client = NULL;

static bool mqtt_connected = false;


/* =========================================================
 * Wi-Fi event handler
 * ========================================================= */

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Wi-Fi starting...");

        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (wifi_retry_count < WIFI_MAX_RETRY)
        {
            esp_wifi_connect();

            wifi_retry_count++;

            ESP_LOGW(
                TAG,
                "Wi-Fi disconnected. Retry %d/%d",
                wifi_retry_count,
                WIFI_MAX_RETRY
            );
        }
        else
        {
            xEventGroupSetBits(
                wifi_event_group,
                WIFI_FAIL_BIT
            );

            ESP_LOGE(
                TAG,
                "Wi-Fi connection failed"
            );
        }
    }
    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(
            TAG,
            "Wi-Fi connected! IP address: " IPSTR,
            IP2STR(&event->ip_info.ip)
        );

        wifi_retry_count = 0;

        xEventGroupSetBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT
        );
    }
}


/* =========================================================
 * Wi-Fi initialization
 * ========================================================= */

static esp_err_t wifi_init(void)
{
    wifi_event_group =
        xEventGroupCreate();

    ESP_ERROR_CHECK(
        esp_netif_init()
    );

    ESP_ERROR_CHECK(
        esp_event_loop_create_default()
    );

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL
        )
    );

    wifi_config_t wifi_config = {0};

    strcpy(
        (char *)wifi_config.sta.ssid,
        WIFI_SSID
    );

    strcpy(
        (char *)wifi_config.sta.password,
        WIFI_PASSWORD
    );

    wifi_config.sta.threshold.authmode =
        WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA)
    );

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );

    ESP_ERROR_CHECK(
        esp_wifi_start()
    );

    ESP_LOGI(
        TAG,
        "Wi-Fi setup successful."
    );

    EventBits_t bits =
        xEventGroupWaitBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT |
            WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

    if (bits & WIFI_CONNECTED_BIT)
    {
        return ESP_OK;
    }

    if (bits & WIFI_FAIL_BIT)
    {
        return ESP_FAIL;
    }

    return ESP_FAIL;
}


/* =========================================================
 * MQTT event handler
 * ========================================================= */

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data)
{
    esp_mqtt_event_handle_t event =
        event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:

            mqtt_connected = true;

            ESP_LOGI(
                TAG,
                "MQTT connected to broker."
            );

            ESP_LOGI(
                TAG,
                "MQTT topic: %s",
                MQTT_TOPIC
            );

            break;


        case MQTT_EVENT_DISCONNECTED:

            mqtt_connected = false;

            ESP_LOGW(
                TAG,
                "MQTT disconnected."
            );

            break;


        case MQTT_EVENT_PUBLISHED:

            ESP_LOGI(
                TAG,
                "MQTT message published. ID=%d",
                event->msg_id
            );

            break;


        case MQTT_EVENT_ERROR:

            ESP_LOGE(
                TAG,
                "MQTT error occurred."
            );

            if (event->error_handle != NULL)
            {
                ESP_LOGE(
                    TAG,
                    "MQTT error type: %d",
                    event->error_handle->error_type
                );
            }

            break;


        default:

            break;
    }
}


/* =========================================================
 * MQTT initialization
 * ========================================================= */

static void mqtt_init(void)
{
    const esp_mqtt_client_config_t mqtt_cfg =
    {
        .broker.address.uri =
            MQTT_BROKER_URI,
        
        .broker.verification.certificate = 
            (const char *)ca_crt_start,

        .credentials.username =
            MQTT_USERNAME,

        .credentials.authentication.password =
            MQTT_PASSWORD
    };


    mqtt_client =
        esp_mqtt_client_init(&mqtt_cfg);


    if (mqtt_client == NULL)
    {
        ESP_LOGE(
            TAG,
            "Failed to initialize MQTT client."
        );

        return;
    }


    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            mqtt_client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL
        )
    );


    ESP_ERROR_CHECK(
        esp_mqtt_client_start(
            mqtt_client
        )
    );


    ESP_LOGI(
        TAG,
        "MQTT client started: %s",
        MQTT_BROKER_URI
    );
}


/* =========================================================
 * DHT11 + MQTT task
 * ========================================================= */

static void dht11_mqtt_task(void *pvParameters)
{
    dht11_data_t data;


    while (true)
    {
        esp_err_t result =
            dht11_read(&data);


        if (result == ESP_OK)
        {
            ESP_LOGI(
                TAG,
                "Temperature: %.1f C | Humidity: %.1f %%",
                data.temperature,
                data.humidity
            );


            /* ---------------------------------------------
             * Basic input validation
             * --------------------------------------------- */

            if (data.temperature < -40.0f ||
                data.temperature > 80.0f)
            {
                ESP_LOGW(
                    TAG,
                    "Invalid temperature value: %.1f",
                    data.temperature
                );

                vTaskDelay(
                    pdMS_TO_TICKS(2000)
                );

                continue;
            }


            if (data.humidity < 0.0f ||
                data.humidity > 100.0f)
            {
                ESP_LOGW(
                    TAG,
                    "Invalid humidity value: %.1f",
                    data.humidity
                );

                vTaskDelay(
                    pdMS_TO_TICKS(2000)
                );

                continue;
            }


            /* ---------------------------------------------
             * Create JSON payload
             * --------------------------------------------- */

            char json_payload[128];


            snprintf(
                json_payload,
                sizeof(json_payload),
                "{\"temperature\":%.1f,\"humidity\":%.1f}",
                data.temperature,
                data.humidity
            );


            /* ---------------------------------------------
             * Publish MQTT message
             * --------------------------------------------- */

            if (mqtt_client != NULL &&
                mqtt_connected)
            {
                int msg_id =
                    esp_mqtt_client_publish(
                        mqtt_client,
                        MQTT_TOPIC,
                        json_payload,
                        0,
                        1,
                        0
                    );


                ESP_LOGI(
                    TAG,
                    "MQTT publish: topic=%s payload=%s",
                    MQTT_TOPIC,
                    json_payload
                );


                if (msg_id >= 0)
                {
                    ESP_LOGI(
                        TAG,
                        "MQTT publish accepted. ID=%d",
                        msg_id
                    );
                }
                else
                {
                    ESP_LOGE(
                        TAG,
                        "MQTT publish failed."
                    );
                }
            }
            else
            {
                ESP_LOGW(
                    TAG,
                    "MQTT not connected. Message not published."
                );
            }
        }
        else
        {
            ESP_LOGW(
                TAG,
                "DHT11 read failed: %s",
                esp_err_to_name(result)
            );
        }


        /* DHT11 should not be read too frequently. */

        vTaskDelay(
            pdMS_TO_TICKS(2000)
        );
    }
}


/* =========================================================
 * Main
 * ========================================================= */

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Starting IoT system..."
    );


    /* ---------------------------------------------
     * Initialize NVS
     * --------------------------------------------- */

    esp_err_t ret =
        nvs_flash_init();


    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(
            nvs_flash_erase()
        );

        ret = nvs_flash_init();
    }


    ESP_ERROR_CHECK(ret);


    /* ---------------------------------------------
     * Initialize DHT11
     * --------------------------------------------- */

    ESP_ERROR_CHECK(
        dht11_init(DHT11_GPIO)
    );


    ESP_LOGI(
        TAG,
        "DHT11 initialized on GPIO %d",
        DHT11_GPIO
    );


    /* ---------------------------------------------
     * Connect to Wi-Fi
     * --------------------------------------------- */

    if (wifi_init() != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Wi-Fi setup failed."
        );

        return;
    }


    /* ---------------------------------------------
     * Start MQTT
     * --------------------------------------------- */

    mqtt_init();


    /* ---------------------------------------------
     * Start DHT11 + MQTT task
     * --------------------------------------------- */

    xTaskCreate(
        dht11_mqtt_task,
        "dht11_mqtt_task",
        4096,
        NULL,
        5,
        NULL
    );


    ESP_LOGI(
        TAG,
        "IoT system started successfully."
    );
}