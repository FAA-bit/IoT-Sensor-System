# System Architecture

## Overview

This IoT system reads temperature and humidity from a DHT11 sensor
connected to an ESP32-C6. The device publishes readings over Wi-Fi and
MQTT to a Mosquitto broker. A Python application subscribes to the
messages and makes the latest valid reading available through a Flask
REST API.

## Data Flow

```text
DHT11 sensor
    │ sensor readings
    ▼
ESP32-C6 ── Wi-Fi / MQTT publish ──► Mosquitto broker
                                         │
                                         │ MQTT subscribe
                                         ▼
                                  Python / Paho client
                                         │
                                         │ in-memory latest reading
                                         ▼
                                   Flask REST API
                                         │ HTTP / JSON
                                         ▼
                                      API client
```

The ESP32-C6 publishes a new reading approximately every two seconds.
The Python API validates incoming messages and keeps the latest valid
reading in memory; it does not store historical data.

## Components

### DHT11 Sensor

The DHT11 measures temperature and relative humidity. It is connected
to the ESP32-C6, which reads the sensor periodically.

### ESP32-C6

The ESP32-C6 acts as the sensor device. Its firmware:

- Connects to the configured Wi-Fi network.
- Reads temperature and humidity from the DHT11.
- Checks that the readings fall within the expected ranges.
- Formats valid readings as JSON.
- Publishes readings to the MQTT broker on topic
  `iot/esp32/dht11`.
- Logs sensor, Wi-Fi, and MQTT events.

Wi-Fi and MQTT settings are provided locally through
`main/secrets.h`. The firmware retries Wi-Fi connection a limited number
of times during startup; MQTT reconnection is handled by the ESP-MQTT
client.

### Mosquitto MQTT Broker

Mosquitto receives MQTT publications from the ESP32-C6 and forwards
them to clients subscribed to the matching topic.

- Protocol: MQTT over TLS
- Port: `8883`
- Topic: `iot/esp32/dht11`

The ESP32-C6 publishes to this topic, and the Python application
subscribes to it. Both clients use the trusted CA certificate to verify
the broker certificate.

### Python MQTT Application

The Python application uses Paho MQTT to connect to Mosquitto and
subscribe to `iot/esp32/dht11`. It:

- Parses incoming JSON messages.
- Checks for `temperature` and `humidity` fields.
- Converts and validates the sensor values.
- Stores the latest valid reading in memory.
- Counts accepted messages and logs communication or validation errors.

Temperature must be between `-40` and `80` °C, inclusive. Humidity must
be between `0` and `100` %, inclusive. If the broker connection fails,
the application logs the error and retries after five seconds.

### Flask REST API

Flask exposes the latest sensor reading over HTTP on port `5000`.

- `GET /api/sensor` returns the latest valid temperature and humidity.
- `GET /api/health` returns the API response status, number of accepted
  MQTT messages, and latest sensor values.

The health endpoint indicates that the API is responding. It does not
independently confirm that the MQTT broker is connected or sensor data
is arriving.

## Communication and Data Format

### ESP32-C6 to Mosquitto

- Protocol: MQTT over TLS over Wi-Fi
- Port: `8883`
- Topic: `iot/esp32/dht11`
- Payload: JSON
- Publishing interval: approximately two seconds

### Mosquitto to Python

- Protocol: MQTT over TLS
- Port: `8883`
- Topic: `iot/esp32/dht11`

The Python application subscribes to the same topic used by the
ESP32-C6.

### API Client to Flask

- Protocol: HTTP
- Port: `5000`
- Data format: JSON

For example, a client can request `GET http://localhost:5000/api/sensor`.

### Example MQTT Payload

```json
{
  "temperature": 25.5,
  "humidity": 41.0
}
```

Temperature is expressed in degrees Celsius and humidity as a
percentage.

## Addressing

The broker address depends on the network where the system is running.
Configure the broker host for the ESP32-C6 in the local firmware
configuration and in `api/app.py`. The broker certificate must be valid
for the address the clients use. The ESP32-C6 obtains its own Wi-Fi
address through DHCP.

When the network changes, verify that both MQTT clients can reach the
broker at its current address. The Flask API listens on all interfaces
and can be reached at `http://localhost:5000` from the host computer or
at the host computer's network address from another device on the
network.

## Why MQTT?

MQTT is suited to this project because it is a lightweight
publish/subscribe protocol for small messages. The ESP32-C6 publishes
readings to a topic without needing a direct connection to the Python
application. Mosquitto acts as an intermediary and can deliver each
publication to multiple subscribers.

## Error Handling

- The ESP32-C6 logs failed DHT11 reads.
- The firmware does not publish a reading while MQTT is disconnected.
- The ESP-MQTT client manages MQTT reconnection.
- The Python application retries broker connections after five seconds
  when a connection attempt fails.
- The Python application rejects invalid JSON and sensor values before
  updating the latest reading.

## Security Considerations

The Mosquitto broker requires username/password authentication. The
firmware credentials are stored in the local, Git-ignored
`main/secrets.h` file, and the Python API reads credentials from
environment variables. Both MQTT clients use TLS on port `8883` and
validate the broker certificate using a trusted CA certificate. The
Flask API itself still uses plain HTTP without authentication. See the
[security notes](sakerhet.md) for remaining risks and recommendations.
