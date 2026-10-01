# Logging and Monitoring

## Logging

The system logs important events while it is running.

The Python API logs:
- MQTT connection
- MQTT subscription
- Received sensor messages
- Sensor data updates
- Invalid JSON
- Invalid sensor values
- MQTT connection errors

The ESP32 also logs:
- Wi-Fi connection
- MQTT connection
- DHT11 readings
- DHT11 errors
- MQTT publishing
These logs are useful when checking if communication between the
different parts of the system is working.

## Monitoring

- The API has a simple monitoring endpoint: GET /api/health
- It can be tested with: Invoke-RestMethod http://localhost:5000/api/health

The endpoint shows:
- API status
- Number of MQTT messages received
- Latest temperature
- Latest humidity
Example:
{
    "status": "ok",
    "messages_received": 50,
    "latest_temperature": 24.0,
    "latest_humidity": 26.0
}
The messages_received value is used as a simple monitoring metric.
It shows whether the API is receiving sensor data from the MQTT broker.

If the number stops increasing while the ESP32 is running, it can
indicate a problem with the MQTT connection or sensor data flow.

## Current limitation

The message counter is stored only in memory. If the API is restarted,
the counter starts again from zero.
There is currently no database or long-term monitoring system.
