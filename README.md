# IoT Sensor System

A small IoT project that reads temperature and humidity data from a DHT11 sensor connected to an ESP32-C6, publishes the readings over MQTT, and exposes the latest measurements through a Python Flask REST API.

## Overview

This project demonstrates an end-to-end IoT data flow:

DHT11 sensor → ESP32-C6 → Wi-Fi → MQTT over TLS → broker → Python MQTT client → Flask REST API

The system is designed for a local lab or home environment and focuses on reliable sensor data acquisition, MQTT communication, and API access to the latest reading.

## Features

- DHT11 sensor reading on ESP32-C6
- Wi-Fi connectivity for the ESP32 device
- MQTT over TLS for encrypted JSON sensor-data publishing
- MQTT subscriber in Python
- Input validation for sensor values and JSON payloads
- Flask API with live sensor data
- Health endpoint for monitoring
- Built-in troubleshooting and test documentation

## Architecture

### Hardware

- ESP32-C6
- DHT11 sensor
- Wi-Fi network
- Mosquitto MQTT broker

### Software

- C firmware running on the ESP32
- Python API receiving MQTT messages
- Flask service exposing sensor data over HTTP

### MQTT topic

The system publishes and subscribes to:

- `iot/esp32/dht11`
- MQTT over TLS on port `8883`
- Clients validate the broker certificate using the trusted CA certificate

### Message format

```json
{
  "temperature": 22.4,
  "humidity": 41.2
}
```

## Project Structure

```text
.
├── api/                  # Python Flask API and MQTT consumer
│   ├── app.py
│   ├── api.md           # REST API documentation
│   ├── certs/
│   │   └── ca.crt       # CA certificate trusted by the API MQTT client
│   └── requirements.txt
├── components/          # Hardware component drivers
├── docs/                # Project documentation
├── main/                # ESP32 firmware source
│   ├── main.c
│   ├── secrets.h
│   ├── certs/
│   │   └── ca.crt       # CA certificate embedded in firmware
│   └── CMakeLists.txt
├── tests/               # Test documentation and protocol notes
├── CMakeLists.txt       # ESP-IDF project configuration
├── mqtt.conf            # Example MQTT broker configuration
├── certs/               # Mosquitto TLS certificates and private keys
├── README.md            # Project overview
├── sdkconfig            # ESP-IDF config
└── dependencies.lock    # Dependency lock file
```

## Requirements

- ESP32-C6 development board
- DHT11 sensor module
- Mosquitto MQTT broker
- ESP-IDF toolchain
- Python 3.x
- Flask
- Paho MQTT client
- VS Code or similar IDE

## Configuration

Before running the firmware, update the Wi-Fi and MQTT settings in
`main/secrets.h`. The broker URI should use `mqtts://` and port `8883`.
The firmware and API both need the CA certificate that signed the broker
certificate.

The file contains the following values that should be customized for your environment:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `MQTT_BROKER_URI`
- `MQTT_USERNAME`
- `MQTT_PASSWORD`

The API broker host and TLS port are configured in `api/app.py`; its
MQTT credentials are read from the `MQTT_USERNAME` and `MQTT_PASSWORD`
environment variables.

Important: do not commit real credentials, private keys, or generated
firmware containing credentials to version control. Keep private
certificate keys secure; only distribute the CA certificate to clients
that need to verify the broker.

## Setup and Installation

### 1. Install ESP-IDF

Follow the official ESP-IDF installation guide for your operating system.

### 2. Install Python dependencies

```powershell
cd api
pip install -r requirements.txt
```

### 3. Start Mosquitto

Make sure Mosquitto is configured with a TLS listener on port `8883`,
has access to its server certificate and private key, and is reachable
from both the ESP32 and the Python API server. The clients must have
the CA certificate used to verify the broker certificate. Update the
certificate paths in `mqtt.conf` for the machine running Mosquitto.

### 4. Build and flash the ESP32 firmware

From the project root:

```powershell
idf.py build
idf.py flash
```

If you want to monitor the device output:

```powershell
idf.py monitor
```

### 5. Start the REST API

```powershell
cd api
python app.py
```

The API runs on:

- `http://localhost:5000`

## API

### GET /api/sensor

Returns the latest valid sensor reading.

Example response:

```json
{
  "temperature": 22.4,
  "humidity": 41.2
}
```

### GET /api/health

Returns the current service status and monitoring information.

Example response:

```json
{
  "status": "ok",
  "messages_received": 10,
  "latest_temperature": 22.4,
  "latest_humidity": 41.2
}
```

If no valid sensor data has been received yet, the API returns:

```json
{
  "error": "No sensor data available"
}
```

with HTTP status `503`.

## Validation and Testing

The project has been tested end-to-end from hardware measurement to rest API output.

Included test areas:

- DHT11 sensor readings
- ESP32-C6 Wi-Fi connection
- MQTT connectivity
- MQTT TLS certificate verification
- MQTT publishing
- Python MQTT subscription
- JSON validation
- REST API responses
- Health endpoint monitoring
- MQTT connection failure handling
- Wrong-topic handling

Detailed testing notes can be found in:

- `tests/testprotokoll.md`

## Troubleshooting

### Wrong MQTT broker IP

If the API cannot reach the MQTT broker or times out, verify that the broker address is correct in the ESP32 configuration and in the Python application settings.

### Wrong MQTT topic

If the API subscribes to the wrong topic, it may appear connected but receive no data. The correct topic is:

- `iot/esp32/dht11`

More troubleshooting details are available in:

- `docs/felsokning.md`

## Documentation

Additional project documentation is available in:

- `docs/arkitektur.md` – system architecture
- `api/api.md` – REST API documentation
- `docs/sakerhet.md` – security notes
- `docs/felsokning.md` – troubleshooting guide
- `docs/monitoring.md` – logging and monitoring
- `docs/arbetslogg.md` – project work log
- `tests/testprotokoll.md` – testing report

## Summary

This project combines embedded development, communication protocols, and web services into a practical IoT example. It shows how sensor data can move from physical hardware to a live API and how validation and monitoring are handled in a real deployment scenario.
