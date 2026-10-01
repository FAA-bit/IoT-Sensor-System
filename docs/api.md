# API Documentation

## Description

The REST API provides access to the latest temperature and humidity data received from the ESP32-C6.

Data flow:

DHT11 → ESP32-C6 → MQTT → Mosquitto → Python/Paho → REST API

## API Server

The API is built with Python and Flask.

- Protocol: HTTP
- Host: localhost
- Port: 5000

## Endpoints

### GET /api/sensor

Returns the latest temperature and humidity measurement.

Example request: GET http://localhost:5000/api/sensor

Example response:
{
    "temperature": 25.5,
    "humidity": 37.0
}

### GET /api/health

Returns the current API status and monitoring information.

Example request:
{
    "status": "ok",
    "messages_received": 50,
    "latest_temperature": 24.0,
    "latest_humidity": 26.0
}

### MQTT Integration

The API receives sensor data from the MQTT broker.
- Broker: Mosquitto
- Port: 1883
- Topic: iot/esp32/dht11
- Protocol: MQTT

Example MQTT message:
{
    "temperature": 25.4,
    "humidity": 27.0
}

The Python application subscribes to the MQTT topic and stores the latest valid sensor measurement.

### Data Validation

The API validates incoming MQTT data before storing it.

Temperature:
- Minimum: -40 °C
- Maximum: 80 °C

Humidity:
- Minimum: 0 %
- Maximum: 100 %

The API also checks that the JSON contains:
- temperature
- humidity

Invalid JSON or invalid sensor values are rejected.

### Error Handling
**No sensor data**

If no valid sensor data has been received yet:
{
    "error": "No sensor data available"
}

The API returns HTTP status:503 Service Unavailable

**Invalid JSON**

If an MQTT message contains invalid JSON, the message is rejected and an error is written to the API log.

**Invalid sensor values**

If temperature or humidity is outside the allowed range, the message is rejected and an error is written to the API log.

**MQTT connection error**

If the connection to the MQTT broker fails, the application retries the connection after 5 seconds.
