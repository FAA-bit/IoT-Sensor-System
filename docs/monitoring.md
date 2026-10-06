# Logging and Monitoring

## Application Logs

The system writes runtime information to the API console and ESP32-C6
serial monitor.

### Python API

The API logs:

- MQTT broker connection and subscription events
- Received MQTT messages
- Successfully stored sensor readings
- Invalid JSON and sensor values
- MQTT connection failures and retry attempts

### ESP32-C6 Firmware

The firmware logs:

- Wi-Fi connection events and assigned IP address
- MQTT connection and publishing events
- DHT11 readings and read errors

These logs help identify where communication or sensor data flow is
failing.

## Health Endpoint

The API exposes a monitoring endpoint at `GET /api/health`. Query it
from PowerShell with:

```powershell
Invoke-RestMethod http://localhost:5000/api/health | ConvertTo-Json
```

Example response:

```json
{
  "status": "ok",
  "messages_received": 50,
  "latest_temperature": 24.0,
  "latest_humidity": 26.0
}
```

The response includes:

- `status`: indicates that the health endpoint responded
- `messages_received`: number of valid MQTT readings accepted since the
  API started
- `latest_temperature`: temperature from the most recent valid reading,
  or `null` if no reading has arrived
- `latest_humidity`: humidity from the most recent valid reading, or
  `null` if no reading has arrived

The `status` value does not confirm that the MQTT broker is connected or
that new sensor data is arriving. Compare `messages_received` over time
to check whether valid readings continue to reach the API. If the count
does not increase while the ESP32-C6 is running, investigate the broker
connection, MQTT topic, and sensor publishing logs.

## Limitations

- The message counter and latest reading are held in memory and reset
  when the API process restarts.
- The system does not currently store historical readings or provide
  persistent monitoring and alerting.