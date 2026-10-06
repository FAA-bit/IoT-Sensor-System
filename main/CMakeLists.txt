idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES dht11 esp_wifi esp_event nvs_flash mqtt
    EMBED_TXTFILES "certs/ca.crt"
)