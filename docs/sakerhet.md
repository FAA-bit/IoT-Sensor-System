# Security Notes

## Scope

This document describes the security measures and limitations of the
current development setup. It is not a security certification. The
system should be treated as a local development project, not as a
production-ready service.

## Current Measures

### MQTT Authentication

The Mosquitto configuration disables anonymous access and uses a
password file. MQTT clients must authenticate with broker credentials.
The ESP32 credentials are configured locally in `main/secrets.h`. The
Python API reads `MQTT_USERNAME` and `MQTT_PASSWORD` from environment
variables and uses them when both are set.

### MQTT TLS

The Mosquitto broker accepts TLS connections on port `8883`. The ESP32-C6
and Python API use the trusted CA certificate to verify the broker
certificate. This encrypts MQTT traffic in transit and helps clients
confirm they are connecting to the expected broker.

### Credential Handling

`main/secrets.h` is listed in `.gitignore` and is not tracked by Git.
Create this file locally with credentials for your own environment, and
do not commit or share it. The API credentials should also be supplied
through environment variables rather than hard-coded in source files.

If credentials have ever been committed, exposed, or shared, removing
them from the current working tree is not sufficient: change or revoke
them at the Wi-Fi and MQTT broker, then update the local configuration.

### Input Validation

Before storing incoming MQTT readings, the API checks that the message
is valid JSON, includes `temperature` and `humidity`, and that both
values are numeric and within the configured ranges:

- Temperature: `-40` to `80` °C
- Humidity: `0` to `100` %

This helps reject malformed or implausible readings. It does not
authenticate message publishers or replace access control on the MQTT
broker.

## Security Limitations

- **TLS protects MQTT only.** The Flask API uses plain HTTP on port
  `5000`; requests and responses are not encrypted and the endpoints
  have no authentication.
- **No client-certificate authentication.** MQTT clients validate the
  broker certificate and authenticate with username and password. The
  broker does not require a separate client certificate.
- **No topic-level access policy is configured.** The current
  Mosquitto configuration uses a password file but does not define
  per-client topic permissions.
- **Firmware contains device credentials.** Wi-Fi and MQTT credentials
  are compiled into the ESP32-C6 firmware, so someone with access to a
  device or firmware image may be able to recover them.
- **No rate limiting or abuse protection.** The API has no rate limits,
  and the broker/API listeners are reachable on their configured
  network interfaces. Network access should be restricted with
  firewall rules.
- **No historical or durable monitoring.** Sensor values and the
  accepted-message counter are stored in memory and reset when the API
  restarts. The health endpoint does not independently confirm MQTT
  connectivity.
- **Input checks are basic validation, not a trust guarantee.** The API
  checks JSON fields and value ranges, but these checks do not prove
  that readings came from the physical sensor.

## Risk Details

### Protect private keys and certificates

The broker's server private key and CA private key must be kept secret
and must not be distributed to clients or committed to version control.
Clients need only the CA certificate to verify the broker certificate.

If a private key is exposed, replace the affected key and certificates.

### Broker listens on all interfaces

The Mosquitto configuration listens on `0.0.0.0:8883`. This makes the
broker reachable through all network interfaces, subject to firewall and
network controls. Restrict access to trusted devices and networks, and
avoid exposing the listener to the public internet.

### Flask API has no authentication or HTTPS

The Flask development server binds to `0.0.0.0` on port `5000`. The API
endpoints do not require authentication and use plain HTTP. Anyone who
can reach the server may query the readings and health information.
Keep the API on a trusted development network. A production deployment
should use an authenticated production-grade server and HTTPS.

### Secrets in firmware

ESP32 credentials are compiled into the firmware from `main/secrets.h`.
Keeping the header out of Git prevents accidental repository disclosure,
but does not protect credentials embedded in a firmware image or
extracted from a device. Use credentials with limited privileges and
rotate them if a device or firmware image is exposed.