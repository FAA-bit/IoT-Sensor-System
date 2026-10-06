# My work Log.

## Day 1

- Started working with the ESP32-C6 development board.
- Reviewed the project structure and prepared the development environment.
- Built the project to check that the initial setup was working correctly.
- Next step: connect a sensor and begin reading data from it.

## Day 2

- Configured the DHT11 temperature and humidity sensor.
- Connected the sensor to the ESP32-C6 using the selected GPIO pin.
- Prepared the code needed to communicate with the DHT11.
- Next step: read temperature and humidity values and verify the sensor data.

## Day 3 - 

- Started working with MQTT communication.
- Reviewed how the ESP32-C6 will publish sensor data to an MQTT broker.
- Began configuring the MQTT connection and message topics.
- Next step: connect to the broker and publish DHT11 readings.

## Day 4 - 
Today I completed a working end-to-end IoT flow for the ESP32-C6. I verified that the DHT11 sensor produced stable temperature and humidity readings, connected it to the board, and confirmed that the values were read correctly in the firmware.

I then configured the Wi-Fi connection and connected the device to the network successfully. After that, I set up the MQTT connection to the Mosquitto broker, defined the publish topic, and sent sensor data in JSON format. This confirmed that the full path from sensor → ESP32-C6 → Wi-Fi → MQTT broker was working correctly.

I also implemented basic MQTT security with username/password authentication and added proper error handling to handle failed connections and invalid configuration more safely.

## Day 5 - 
- Started working with api
- The api folder has two files app.py & requirements.txt
- requirements.txt = tells Python what libraries the project needs & the file contains: 
    - flask - Flask is a Python framework that makes it relatively easy to create an HTTP/REST API.
   - paho-mqtt - Lets Python receive MQTT messages

## Day 6 - 
Just finish working with api



--------------------------------------------------------------------------------------------

Stage 1 — DONE ✅
- Physical sensor : DHT11 → ESP32-C6

Stage 2 — DONE ✅
- Network : ESP32-C6 → Wi-Fi

Stage 3 — DONE ✅
- MQTT over TLS : ESP32-C6 → Mosquitto on port 8883
- When working with Mosquitto, use this command at the start each time: 
$env:Path += ";C:\Program Files\mosquitto"

Stage 4 — DONE ✅
- JSON data contract :
{
  "temperature": 27.8,
  "humidity": 42.0
}

Stage 5 — DONE ✅
MQTT security
- MQTT username/password authentication
- TLS certificate validation
- No anonymous access
- Keep credentials out of GitHub
- Proper error handling

Stage 6 — API 🌐 DONE ✅
Will create/add an API operation.

Stage 7 — Logging & monitoring 📊  DONE ✅ 
- Connection status
- Sensor readings
- MQTT publish status
- At least one monitoring metric

Stage 8 — Fault testing 🧪 DONE ✅ 
Will intentionally introduce two faults, for example:
- Wrong MQTT broker IP
- Wrong MQTT topic / disconnected broker
