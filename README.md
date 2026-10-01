# IoT Sensor System

## Description

An IoT system for collecting temperature and humidity data from a
physical DHT11 sensor connected to an ESP32-C6.

The sensor data is sent using MQTT to a Mosquitto broker. A Python
application receives the data and makes it available through a
Flask REST API.

## Architecture

DHT11 → ESP32-C6 → Wi-Fi → MQTT → Mosquitto → Python/Paho MQTT → REST API

## Requirements

- ESP32-C6
- DHT11
- Mosquitto MQTT broker
- ESP-IDF
- Python 3
- Flask
- Paho MQTT
- VS Code

## Installation

Install ESP-IDF and Python.

Install the Python dependencies:

powershell: cd api
pip install -r requirements.txt

Install Mosquitto and make sure it is available in the terminal.
The ESP32-C6 project uses ESP-IDF.

## Configuration

## API

## Testing

The system has been tested from the physical DHT11 sensor to the
REST API.

Tests include:
- DHT11 sensor readings
- ESP32-C6 Wi-Fi connection
- MQTT connection
- MQTT publishing
- Python MQTT reception
- JSON validation
- REST API
- Health endpoint
- Monitoring with messages_received
- MQTT connection fault
- MQTT topic fault

Detailed tests can be found in: tests/testprotokoll.md

## Troubleshooting

Two communication faults were intentionally introduced and tested.

### Wrong MQTT broker IP

The API could not connect to the MQTT broker and showed a timeout.
The problem was fixed by changing the broker IP address back to the
correct address.

### Wrong MQTT topic

The API connected to MQTT but did not receive sensor data because
it was subscribed to the wrong topic.
The problem was fixed by changing the topic back to: iot/esp32/dht11

More details can be found in: docs/felsokning.md

## Documentation

More information about the project can be found in:
- docs/arkitektur.md – system architecture
- docs/api.md – REST API documentation
- docs/sakerhet.md – security
- docs/felsokning.md – troubleshooting
- docs/monitoring.md – logging and monitoring
- tests/testprotokoll.md – testing
