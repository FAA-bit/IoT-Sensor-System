# Arkitekturbeskrivning

## Översikt

Projektet är ett IoT-system där en fysisk DHT11-sensor är ansluten till en ESP32-C6.
ESP32-C6 läser av temperatur och luftfuktighet och skickar värdena via Wi-Fi och MQTT till en Mosquitto MQTT-broker.
Python-applikationen tar emot MQTT-meddelandena och gör sedan datan tillgänglig genom ett REST API med Flask.

Dataflödet är:

          DHT11
            ↓
         ESP32-C6
            ↓ Wi-Fi / MQTT
        Mosquitto MQTT-broker
            ↓
        Python / Paho MQTT
            ↓ HTTP / REST
         Flask API

## Systemets komponenter

### 1. DHT11

DHT11 är den fysiska sensorn i projektet.
Den mäter:
- Temperatur
- Luftfuktighet
Sensorn är ansluten till ESP32-C6 och mätvärdena läses regelbundet.

### 2. ESP32-C6

ESP32-C6 fungerar som IoT-enheten.

Dess uppgifter är att:
- Ansluta till Wi-Fi
- Läsa temperatur och luftfuktighet från DHT11
- Kontrollera att sensorvärdena är rimliga
- Skapa ett JSON-meddelande
- Publicera data till MQTT-brokern
- Försöka ansluta igen om Wi-Fi eller MQTT kopplas bort
ESP32 använder MQTT-topic: iot/esp32/dht11


### 3. Mosquitto MQTT-broker

Mosquitto används som MQTT-broker.
Brokern tar emot meddelanden från ESP32 och skickar dem vidare till klienter som prenumererar på rätt topic.

MQTT-port: 1883
MQTT-topic: iot/esp32/dht11

Kommunikationen använder MQTT:s publish/subscribe-modell.
ESP32 publicerar data och Python-applikationen prenumererar på samma topic.

### 4. Python-applikation

Python-applikationen använder Paho MQTT för att ansluta till Mosquitto och ta emot sensorvärden.
Applikationen:
- Prenumererar på MQTT-topic
- Tar emot JSON-data
- Kontrollerar JSON-formatet
- Kontrollerar temperatur och luftfuktighet
- Sparar det senaste giltiga värdet
- Räknar antal mottagna meddelanden
- Loggar kommunikationsfel

Om MQTT-anslutningen försvinner försöker applikationen ansluta igen efter fem sekunder.

### 5. Flask REST API

Flask används för att göra sensordatan tillgänglig genom HTTP.
API-port: 5000
API:t har bland annat dessa endpoints: GET /api/sensor
Returnerar det senaste giltiga temperatur- och luftfuktighetsvärdet.
GET /api/health
Returnerar systemets status, antal mottagna MQTT-meddelanden och det senaste sensorvärdet.

## Kommunikation

### a. ESP32-C6 → Mosquitto

Protokoll: MQTT
Port: 1883
Topic: iot/esp32/dht11
Dataformat: JSON
ESP32-publicerar ett nytt sensorvärde ungefär varannan sekund.

### b. Mosquitto → Python

Protokoll: MQTT
Port: 1883
Topic: iot/esp32/dht11
Python-applikationen prenumererar på topicen och tar emot
sensorvärdena.

### c. Klient → Flask API

Protokoll: HTTP
Port: 5000
Exempel: GET http://localhost:5000/api/sensor
Dataformat: JSON

### d. Kommunikationmodell

MQTT använder en publish/subscribe-modell.
ESP32 är en MQTT publisher och skickar data till: iot/esp32/dht11
Python-applikationen är en MQTT subscriber och lyssnar på samma topic.
Mosquitto fungerar som broker mellan dem.
REST API:t använder istället HTTP request/response-modellen.
En klient skickar exempelvis en GET-request till /api/sensor och Flask returnerar den senaste sensorinformationen.

### e. IP-adresser

IP-adresserna kan ändras beroende på vilket nätverk jag använder.
Under testningen kördes MQTT-brokern på datorns lokala IP-adress.
Exempel:
MQTT-broker: 172.16.217.22
Port: 1883
Flask API: 172.16.217.22
Port: 5000

ESP32 får sin IP-adress från Wi-Fi-nätverket genom DHCP.
Eftersom IP-adresserna kan ändras mellan olika nätverk behöver MQTT-brokeradressen uppdateras när nätverket ändras.

### f. Dataformat

Sensorinformationen skickas som JSON.
Exempel: 
{
    "temperature": 25.5,
    "humidity": 41.0
}
Temperatur anges i grader Celsius.
Luftfuktighet anges i procent.
Python-applikationen validerar värdena innan de sparas.
Temperatur måste vara mellan -40 °C och 80 °C.
Luftfuktighet måste vara mellan 0 % och 100 %.

## Varför MQTT?

Jag valde MQTT eftersom det passar bra för IoT-kommunikation.
ESP32 behöver bara publicera små meddelanden till en topic och behöver inte kommunicera direkt med Python-applikationen.
Mosquitto fungerar som en mellanhand och gör att flera klienter kan ta emot samma sensorinformation.
MQTT har också stöd för återanslutning och passar bra för små sensorvärden som skickas regelbundet.

## Felhantering

ESP32 loggar om DHT11-läsningen misslyckas.
Om MQTT inte är anslutet skickas inte sensorvärdet förrän anslutningen fungerar igen.
Python-applikationen försöker ansluta till MQTT-brokern igen efter fem sekunder om anslutningen misslyckas.
Python-applikationen kontrollerar också inkommande JSON och sensorvärden innan de används.

## Säkerhet

MQTT-brokern använder autentisering med användarnamn och lösenord.
Wi-Fi- och MQTT-lösenord lagras inte direkt i GitHub-repot.
De hanteras lokalt genom secrets.h och miljövariabler.
Den nuvarande MQTT-kommunikationen använder port 1883 utan TLS.
Detta är en känd begränsning i projektet.