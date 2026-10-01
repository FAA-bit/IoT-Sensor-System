# Testprotokoll

## Syfte

Syftet med testningen är att kontrollera att hela IoT-systemet
fungerar från den fysiska sensorn till REST API:t.

Testerna är end-to-end-tester där jag kontrollerar de viktigaste
delarna av systemet.

---

## Testmiljö

### Hårdvara

- ESP32-C6
- DHT11
- USB-anslutning till dator

### Mjukvara

- ESP-IDF
- Mosquitto MQTT
- Python
- Paho MQTT
- Flask
- PowerShell

### Protokoll

- Wi-Fi
- MQTT
- HTTP
- JSON

---

## Test 1 – DHT11 sensor

**Syfte:** Kontrollera att den fysiska DHT11-sensorn ger temperatur- och luftfuktighetsvärden.

**Test:** Starta ESP32-C6 och öppna ESP-IDF monitor.

**Förväntat resultat:** Temperatur och luftfuktighet visas i terminalen.

**Resultat:** Temperature: 25.4 C | Humidity: 27.0 %

**Status:** PASS

## Test 2 – Wi-Fi

**Syfte:** Kontrollera att ESP32-C6 ansluter till Wi-Fi.

**Test:** Starta ESP32-C6 och kontrollera loggen.

**Förväntat resultat:** ESP32-C6 ansluter och får en IP-adress.

**Resultat:** Wi-Fi connected! IP address: .....

**Status:** PASS

## Test 3 – MQTT-anslutning

**Syfte:** Kontrollera att ESP32-C6 kan ansluta till MQTT-brokern.

**Test:** Starta Mosquitto och ESP32-C6.

**Förväntat resultat:** ESP32-C6 ansluter till MQTT-brokern.

**Resultat:** MQTT connected to broker.....

**Status:** PASS

## Test 4 – MQTT-publicering

**Syfte:** Kontrollera att ESP32-C6 skickar sensorvärden till rätt MQTT-topic.

**MQTT-topic:** iot/esp32/dht11

**Förväntat resultat:** ESP32 publicerar temperatur och luftfuktighet som JSON.

**Exempel på resultat:**
{
    "temperature": 25.4,
    "humidity": 27.0
}

**Status:** PASS

## Test 5 – Python tar emot MQTT-data

**Syfte:** Kontrollera att Python-applikationen tar emot data från MQTT-brokern.

**Test:** Starta Python API och låt ESP32 skicka sensorvärden.

**Förväntat resultat:** Python visar mottagna MQTT-meddelanden.

**Exempel:** MQTT message received: iot/esp32/dht11 -> {"temperature":25.5,"humidity":41.0}
Sensor data updated successfully.

**Status:** PASS

## Test 6 – JSON-validering

**Syfte:** Kontrollera att API:t accepterar giltig JSON och kontrollerar
sensorvärdena.

**Test:** Skicka eller ta emot ett giltigt JSON-meddelande från ESP32.

**Förväntat resultat:** Sensorvärdet accepteras och sparas.

**Resultat:** Sensor data updated successfully.

**Status:** PASS

## Test 7 – REST API /api/sensor

**Syfte:** Kontrollera att den senaste sensordatan kan hämtas genom REST API:t.

**Anrop:** Invoke-RestMethod http://localhost:5000/api/sensor | ConvertTo-Json

**Förväntat resultat:** API:t returnerar temperatur och luftfuktighet som JSON.

**Exempel:**
{
    "temperature": 24.9,
    "humidity": 26.0
}

**Status:** PASS

## Test 8 – Health endpoint

**Syfte:** Kontrollera att API:t är igång och kan visa systemets status.

**Anrop:** Invoke-RestMethod http://localhost:5000/api/health | ConvertTo-Json

**Förväntat resultat:** API:t returnerar status, antal mottagna meddelanden och senaste
sensorvärden.

**Exempel:**
{
    "status": "ok",
    "messages_received": 50,
    "latest_temperature": 24.0,
    "latest_humidity": 26.0
}

**Status:** PASS

## Test 9 – Monitoring

**Syfte:** Kontrollera att antalet mottagna MQTT-meddelanden ökar.

**Test:** Låt ESP32 vara igång och kontrollera /api/health flera gånger.

**Förväntat resultat:** messages_received ökar när nya MQTT-meddelanden tas emot.

**Resultat:** messages_received: 50

**Status:** PASS

## Test 10 – Felhantering
### Test 10.1 – Felaktig MQTT brokeradress

**Test:** MQTT broker-adressen ändrades till en felaktig IP-adress.

**Förväntat resultat:** API:t ska inte kunna ansluta och ska försöka igen.

**Resultat:** 
MQTT connection error: timed out
Retrying MQTT connection in 5 seconds.....

**Status:** PASS

Felet dokumenteras mer detaljerat i docs/felsokning.md.

### Test 10.2 – Felaktig MQTT-topic

**Test:** MQTT-topic ändrades till en felaktig topic.

**Förväntat resultat:** MQTT-anslutningen ska fungera, men API:t ska inte få några
sensorvärden.

**Resultat:** 
{
    "latest_humidity": null,
    "latest_temperature": null,
    "messages_received": 0,
    "status": "ok"
}

**Status:** PASS

Felet dokumenteras mer detaljerat i docs/felsokning.md.

## Sammanfattning

De viktigaste delarna av systemet testades från den fysiska DHT11
sensorn till REST API:t.

Testerna visade att:
- DHT11 ger verkliga sensorvärden.
- ESP32-C6 ansluter till Wi-Fi.
- ESP32-C6 ansluter till MQTT-brokern.
- Sensorvärden skickas med MQTT.
- Python-applikationen tar emot MQTT-data.
-  JSON-data valideras.
- REST API:t returnerar sensordata.
- Health-endpointen fungerar.
- Monitoring med messages_received fungerar.
- Kommunikationsfel kan upptäckas och åtgärdas.

**Sammanlagt:** PASS