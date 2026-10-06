# Test Report

## Purpose and Scope

This report documents end-to-end testing of the IoT system, from the
DHT11 sensor to the REST API. The tests cover sensor readings, Wi-Fi and
MQTT communication, data validation, API responses, and communication
failure handling.

## Test Environment

### Hardware

- ESP32-C6
- DHT11 sensor
- USB connection to a computer

### Software

- ESP-IDF
- Mosquitto MQTT broker
- Python with Paho MQTT and Flask
- PowerShell
- Mosquitto TLS certificates and client CA certificates

### Protocols and Data Format

- Wi-Fi
- MQTT
- TLS
- HTTP
- JSON

MQTT topic: `iot/esp32/dht11`

Example MQTT payload:

```json
{
  "temperature": 25.4,
  "humidity": 27.0
}
```

## Test Results

### Test 1 – DHT11 Sensor

**Purpose:** Verify that the sensor provides temperature and humidity
readings.

**Procedure:** Start the ESP32-C6 and open the ESP-IDF monitor.

**Expected result:** Temperature and humidity readings are displayed in
the terminal.

**Result:**

```text
Temperature: 25.4 C | Humidity: 27.0 %
```

**Status:** PASS

### Test 2 – Wi-Fi Connection

**Purpose:** Verify that the ESP32-C6 connects to Wi-Fi.

**Procedure:** Start the ESP32-C6 and check the device log.

**Expected result:** The ESP32-C6 connects and obtains an IP address.

**Result:** The log showed `Wi-Fi connected! IP address: ...`.

**Status:** PASS

### Test 3 – MQTT/TLS Connection

**Purpose:** Verify that the ESP32-C6 can establish an authenticated,
TLS-protected connection to the MQTT broker.

**Procedure:** Start the Mosquitto TLS listener and the ESP32-C6 with
the trusted CA certificate configured.

**Expected result:** The ESP32-C6 validates the broker certificate and
connects to the MQTT broker on port `8883`.

**Result:** The log showed `MQTT connected to broker`. The firmware is
configured to use an `mqtts://` broker URI and the trusted CA
certificate.

**Status:** PASS

### Test 4 – MQTT Publishing

**Purpose:** Verify that the ESP32-C6 publishes sensor readings to the
correct MQTT topic.

**Topic:** `iot/esp32/dht11`

**Expected result:** The ESP32 publishes temperature and humidity as
JSON.

**Example published message:**

```json
{
  "temperature": 25.4,
  "humidity": 27.0
}
```

**Status:** PASS

### Test 5 – Python Receives MQTT Data

**Purpose:** Verify that the Python application receives messages from
the MQTT broker.

**Procedure:** Start the Python API and allow the ESP32-C6 to publish
sensor readings.

**Expected result:** The Python log displays received MQTT messages.

**Example log output:**

```text
MQTT message received: iot/esp32/dht11 -> {"temperature":25.5,"humidity":41.0}
Sensor data updated successfully.
```

**Status:** PASS

### Test 6 – Valid JSON Validation

**Purpose:** Verify that the API accepts a valid JSON message containing
sensor readings.

**Procedure:** Send or receive a valid JSON message from the ESP32-C6.

**Expected result:** The sensor readings are accepted and stored.

**Result:** The Python log showed `Sensor data updated successfully`.

**Status:** PASS

### Test 7 – REST API: `/api/sensor`

**Purpose:** Verify that the latest sensor readings can be retrieved
from the REST API.

**Request:**

```powershell
Invoke-RestMethod http://localhost:5000/api/sensor | ConvertTo-Json
```

**Expected result:** The API returns the temperature and humidity as
JSON.

**Example response:**

```json
{
  "temperature": 24.9,
  "humidity": 26.0
}
```

**Status:** PASS

### Test 8 – Health Endpoint

**Purpose:** Verify that the API responds and reports its status and
monitoring data.

**Request:**

```powershell
Invoke-RestMethod http://localhost:5000/api/health | ConvertTo-Json
```

**Expected result:** The API returns its status, the number of received
messages, and the latest sensor readings.

**Example response:**

```json
{
  "status": "ok",
  "messages_received": 50,
  "latest_temperature": 24.0,
  "latest_humidity": 26.0
}
```

**Status:** PASS

### Test 9 – Received Message Monitoring

**Purpose:** Verify that the received MQTT message counter increases.

**Procedure:** Leave the ESP32-C6 running and check `/api/health`
multiple times.

**Expected result:** `messages_received` increases as valid MQTT
messages arrive.

**Result:** The counter showed `messages_received: 50`.

**Status:** PASS

## Error Handling Tests

### Test 10.1 – Incorrect MQTT Broker Address

**Purpose:** Check how the API handles an unreachable broker.

**Procedure:** Change the MQTT broker address to an incorrect IP address.

**Expected result:** The API cannot connect and retries the connection.

**Result:**

```text
MQTT connection error: timed out
Retrying MQTT connection in 5 seconds...
```

**Status:** PASS

The failure and its troubleshooting steps are also described in the
[troubleshooting guide](../docs/felsokning.md).

### Test 10.2 – Incorrect MQTT Topic

**Purpose:** Verify that missing data can be identified when the API
subscribes to the wrong topic.

**Procedure:** Change the topic so that the API subscribes to an
incorrect topic.

**Expected result:** The MQTT connection succeeds, but the API receives
no sensor messages.

**Result:**

```json
{
  "latest_humidity": null,
  "latest_temperature": null,
  "messages_received": 0,
  "status": "ok"
}
```

**Status:** PASS

`status: "ok"` indicates that the API is responding. The zero message
count and `null` values indicate that no sensor readings have arrived.

The failure and its troubleshooting steps are also described in the
[troubleshooting guide](../docs/felsokning.md).

## Summary

The documented tests checked the system from the DHT11 sensor to the
REST API. All test cases in this report are marked PASS. The tests showed
that:

- The DHT11 sensor provides temperature and humidity readings.
- The ESP32-C6 connects to Wi-Fi and the MQTT broker.
- The ESP32-C6 uses MQTT over TLS and a trusted CA certificate to
  connect to the broker.
- Sensor readings are published over MQTT and received by the Python
  application.
- A valid JSON message containing sensor readings is accepted.
- The REST API returns sensor data and monitoring information.
- An incorrect broker address and an incorrect topic produce different,
  observable communication failures.
