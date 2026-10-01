# Felsökning

## CMake/Kconfig kunde inte skapa projektets konfigurationsfiler

### Symptom
- `idf.py set-target esp32c6` avslutades med ett CMake/Kconfig-fel.

### Identifiering
- ESP-IDF-loggen visade felaktigt kodade tecken i sökvägen: `NÃ¤` i stället för `ä`.

### Orsak
Projektets sökväg innehöll ett icke-ASCII-tecken (`ä`), vilket orsakade problem vid behandlingen av sökvägen.

### Åtgärd
Projektet flyttades till en sökväg utan specialtecken.

### Verifiering
Kommandot `idf.py set-target esp32c6` kördes igen utan samma Kconfig/CMake-fel.

---

## Fault 1 – Wrong MQTT broker IP

### Symptom

The Python API started, but it could not connect to the MQTT broker.
The log showed:
MQTT connection error: timed out
The API kept trying to reconnect every five seconds.

### Identification

I checked the API log and saw that it was trying to connect to an
incorrect MQTT broker address.
The configured address was: 172.16.217.99:1883
The MQTT broker was running on: 172.16.217.22:1883

### Cause

The MQTT broker IP address in `api/app.py` was incorrect.

### Fix

I changed the broker address in `api/app.py` from: 172.16.217.99
to: 172.16.217.22

### Verification

After restarting the API, the log showed:

MQTT connected successfully.
Subscribed to: iot/esp32/dht11

The API then started receiving sensor data again.

The message counter increased from 1 to 7, which confirmed that
the MQTT communication was working again.

---

## Fault 2 – Wrong MQTT topic

### Symptom

The Python API was running and connected successfully to the MQTT broker,
but no sensor data was received.
The API log showed: MQTT connected successfully.
However, no MQTT sensor messages appeared.

### Identification
I checked the MQTT topic used by the API.
The API was subscribed to: iot/esp32/faid
The ESP32 was publishing to: iot/esp32/dht11
The topics were therefore different.
I also checked the monitoring endpoint:
```powershell
Invoke-RestMethod http://localhost:5000/api/health | ConvertTo-Json
The result showed: 
´´´json 
{
    "latest_humidity": null,
    "latest_temperature": null,
    "messages_received": 0,
    "status": "ok"
}
This showed that the API was running, but it had not received any
sensor messages.

```
### Cause

The MQTT topic in api/app.py was intentionally changed to the wrong
topic.
The API was listening to: iot/esp32/faid
while the ESP32 was publishing to: iot/esp32/dht11

### Fix

I changed the MQTT topic in api/app.py back to: iot/esp32/dht11
I then restarted the API.

### Verification

After the fix, the API showed: MQTT connected successfully.
Subscribed to: iot/esp32/dht11
The API then started receiving sensor data again.
The message counter started increasing and the latest temperature and
humidity values were updated.
This confirmed that the MQTT communication was working again.