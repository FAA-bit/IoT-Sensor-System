from flask import Flask, jsonify
import paho.mqtt.client as mqtt
import json
import os
import threading
import time


# =========================================================
# Configuration
# =========================================================

MQTT_BROKER = os.getenv("MQTT_BROKER")     ## Just an example, replace with your MQTT broker address.
MQTT_PORT = 1883
MQTT_TOPIC = "iot/esp32/dht11"

MQTT_USERNAME = os.getenv("MQTT_USERNAME")
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD")


# =========================================================
# Flask application
# =========================================================

app = Flask(__name__)


# =========================================================
# Latest sensor data
# =========================================================

latest_data = {
    "temperature": None,
    "humidity": None
}

messages_received = 0

data_lock = threading.Lock()


# =========================================================
# MQTT callbacks
# =========================================================

def on_connect(client, userdata, flags, reason_code, properties=None):

    if reason_code == 0:
        print("MQTT connected successfully.")

        client.subscribe(MQTT_TOPIC)

        print(f"Subscribed to: {MQTT_TOPIC}")

    else:
        print(f"MQTT connection failed: {reason_code}")


def on_message(client, userdata, message):

    global latest_data
    global messages_received

    try:
        payload = message.payload.decode("utf-8")

        print(
            f"MQTT message received: "
            f"{message.topic} -> {payload}"
        )

        data = json.loads(payload)

        # ---------------------------------------------
        # Validate JSON structure
        # ---------------------------------------------

        if "temperature" not in data:
            print("Validation error: temperature missing")
            return

        if "humidity" not in data:
            print("Validation error: humidity missing")
            return

        temperature = float(data["temperature"])
        humidity = float(data["humidity"])

        # ---------------------------------------------
        # Validate sensor ranges
        # ---------------------------------------------

        if temperature < -40 or temperature > 80:
            print(
                f"Validation error: "
                f"invalid temperature {temperature}"
            )
            return

        if humidity < 0 or humidity > 100:
            print(
                f"Validation error: "
                f"invalid humidity {humidity}"
            )
            return

        # ---------------------------------------------
        # Store latest valid reading
        # ---------------------------------------------

        with data_lock:

            latest_data = {
                "temperature": temperature,
                "humidity": humidity
            }
    
        messages_received += 1
        print(f"Sensor data updated successfully.")
        print(f"Total messages received: {messages_received}")  

    except json.JSONDecodeError:

        print("Validation error: invalid JSON")

    except (ValueError, TypeError):

        print("Validation error: invalid sensor values")

    except Exception as error:

        print(f"Error processing MQTT message: {error}")


# =========================================================
# MQTT client
# =========================================================

mqtt_client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2,
    client_id="iot-api-bridge"
)

mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message


if MQTT_USERNAME and MQTT_PASSWORD:

    mqtt_client.username_pw_set(
        MQTT_USERNAME,
        MQTT_PASSWORD
    )

else:

    print(
        "WARNING: MQTT_USERNAME or MQTT_PASSWORD "
        "is not configured."
    )


# =========================================================
# Start MQTT
# =========================================================

def start_mqtt():

    while True:

        try:

            print(
                f"Connecting to MQTT broker "
                f"{MQTT_BROKER}:{MQTT_PORT}..."
            )

            mqtt_client.connect(
                MQTT_BROKER,
                MQTT_PORT,
                60
            )

            mqtt_client.loop_forever()

        except Exception as error:

            print(
                f"MQTT connection error: {error}"
            )

            print(
                "Retrying MQTT connection in 5 seconds..."
            )

            time.sleep(5)


mqtt_thread = threading.Thread(
    target=start_mqtt,
    daemon=True
)

mqtt_thread.start()


# =========================================================
# REST API
# =========================================================

@app.route("/api/sensor", methods=["GET"])
def get_sensor_data():

    with data_lock:

        if latest_data["temperature"] is None:

            return jsonify({
                "error": "No sensor data available"
            }), 503

        return jsonify(latest_data), 200


# =========================================================
# Health endpoint
# =========================================================

@app.route("/api/health", methods=["GET"])
def health():

    return jsonify({
        "status": "ok",
        "messages_received": messages_received,
        "latest_temperature": latest_data["temperature"],
        "latest_humidity": latest_data["humidity"]
    }), 200


# =========================================================
# Start Flask server
# =========================================================

if __name__ == "__main__":

    print("Starting IoT REST API...")

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=False
    )

