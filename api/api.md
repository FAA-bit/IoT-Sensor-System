# REST API Documentation

## Overview

The Flask REST API provides access to the latest valid temperature and
humidity readings received from the ESP32-C6 through MQTT.

```text
DHT11 → ESP32-C6 → Wi-Fi → MQTT broker → Python/Paho MQTT client → Flask REST API
```

The API stores the latest valid reading in memory. Sensor history is not
stored.

## Running the API

Install the Python dependencies and start the application from the `api`
directory:

```powershell
pip install -r requirements.txt
python app.py
```

The Flask server listens on port `5000` and binds to `0.0.0.0`. It can be
accessed locally at:

```text
http://localhost:5000
```

## Configuration

The API reads the MQTT broker host and optional credentials from
environment variables:

| Variable | Required | Description |
| --- | --- | --- |
| `MQTT_BROKER` | Yes | Hostname or IP address of the MQTT broker |
| `MQTT_USERNAME` | No | MQTT username |
| `MQTT_PASSWORD` | No | MQTT password |

The MQTT port is `1883`, and the subscribed topic is
`iot/esp32/dht11`; both are currently defined in `app.py`.

The API uses MQTT username/password authentication only when both
`MQTT_USERNAME` and `MQTT_PASSWORD` are set. Configure credentials
through environment variables rather than adding real credentials to
source control.

## Endpoints

### `GET /api/sensor`

Returns the latest valid sensor reading.

Example request:

```text
GET http://localhost:5000/api/sensor
```

Example response (`200 OK`):

```json
{
  "temperature": 25.5,
  "humidity": 37.0
}
```

If no valid sensor reading has been received since the API started, the
endpoint returns `503 Service Unavailable`:

```json
{
  "error": "No sensor data available"
}
```

### `GET /api/health`

Returns the API response status, the number of valid MQTT messages
received since startup, and the latest sensor values.

Example request:

```text
GET http://localhost:5000/api/health
```

Example response (`200 OK`):

```json
{
  "status": "ok",
  "messages_received": 50,
  "latest_temperature": 24.0,
  "latest_humidity": 26.0
}
```

Before the first valid sensor reading, the latest values are `null` and
`messages_received` is `0`. A `"status": "ok"` response means the API
endpoint is responding; it does not confirm that the MQTT broker is
connected or that sensor data is arriving.

## MQTT Integration

The Python application subscribes to the MQTT topic
`iot/esp32/dht11` on the configured broker at port `1883`. It expects
messages in this JSON format:

```json
{
  "temperature": 25.4,
  "humidity": 27.0
}
```

When a message is valid, the API updates its in-memory reading and
increments `messages_received`.

## Input Validation

Before storing a message, the application checks that:

- The payload is valid JSON.
- The JSON contains both `temperature` and `humidity`.
- Both values can be converted to numbers.
- Temperature is between `-40` and `80` °C, inclusive.
- Humidity is between `0` and `100` %, inclusive.

Invalid messages are discarded and an explanation is printed to the API
log. They do not update the latest reading or increment
`messages_received`.

## Error and Connection Handling

| Situation | Behavior |
| --- | --- |
| No valid reading received yet | `/api/sensor` returns `503`; `/api/health` returns `null` latest values |
| Invalid MQTT JSON or sensor values | Message is discarded and an error is logged |
| MQTT broker connection fails | The application logs the error and retries after 5 seconds |
| API process is stopped | API endpoints are unavailable |
