# Troubleshooting

This guide covers build configuration and communication issues observed
while testing the IoT system.

## CMake/Kconfig Fails to Generate Project Configuration

### Symptom

Running `idf.py set-target esp32c6` fails with a CMake or Kconfig error.

### Diagnosis

The ESP-IDF log showed a path character incorrectly encoded as `NÃ¤`
instead of `ä`.

### Cause

The project path contained a non-ASCII character (`ä`), which caused
issues when the build tools processed the path.

### Resolution

Move the project to a path that does not contain special or non-ASCII
characters.

### Verification

Run `idf.py set-target esp32c6` again and confirm that the same
CMake/Kconfig error no longer occurs.

## Fault 1 – Incorrect MQTT Broker Address

### Symptom

The Python API started, but it could not connect to the MQTT broker. The
log showed a connection timeout, followed by a retry every five seconds.

### Diagnosis

During this test, the API was configured with the incorrect broker
address `172.16.217.99:1883`. The broker was reachable at
`172.16.217.22:1883`.

### Cause

The API was using an incorrect MQTT broker host. The API reads the broker
host from the `MQTT_BROKER` environment variable; it is not hard-coded
in `api/app.py`.

### Resolution

Set `MQTT_BROKER` to the current hostname or IP address of the broker,
then restart the API. For example, in PowerShell:

```powershell
cd api
$env:MQTT_BROKER = "172.16.217.22"
python app.py
```

Set this variable in the same PowerShell session used to start the API.
The address above is the broker address recorded during this test; use
the address for your current network.

### Verification

After correcting the address and restarting the API, the log showed:

```text
MQTT connected successfully.
Subscribed to: iot/esp32/dht11
```

The API began receiving sensor data, and the message counter increased
from 1 to 7.

## Fault 2 – Incorrect MQTT Topic

### Symptom

The Python API connected to the MQTT broker, but no sensor data arrived.
The API log showed a successful broker connection, but no MQTT sensor
messages appeared.

### Diagnosis

During this test, the API subscribed to `iot/esp32/faid`, while the
ESP32-C6 published to `iot/esp32/dht11`. The topics did not match.
The health endpoint showed that the API was responding but had not
received sensor messages:

```powershell
Invoke-RestMethod http://localhost:5000/api/health | ConvertTo-Json
```

Example response:

```json
{
  "latest_humidity": null,
  "latest_temperature": null,
  "messages_received": 0,
  "status": "ok"
}
```

### Cause

The API topic in `api/app.py` had intentionally been changed to the
incorrect topic `iot/esp32/faid`. The ESP32-C6 continued publishing to
`iot/esp32/dht11`.

### Resolution

Set the API topic in `api/app.py` to `iot/esp32/dht11`, matching the
topic configured in the ESP32 firmware, and restart the API.

### Verification

The API log showed that it subscribed to `iot/esp32/dht11`. Sensor
messages then arrived, the message counter increased, and the latest
temperature and humidity values were updated.
