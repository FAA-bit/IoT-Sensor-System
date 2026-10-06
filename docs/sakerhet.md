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

## Risks and Limitations

### MQTT traffic is not encrypted

The current broker listener uses MQTT on port `1883` without TLS.
Username and password authentication does not encrypt the connection;
credentials and messages may be exposed to parties able to observe the
network traffic.

For deployment beyond a trusted development network, configure MQTT over
TLS, validate the broker certificate on clients, and use unique,
strongly protected credentials.

### Broker listens on all interfaces

The Mosquitto configuration listens on `0.0.0.0:1883`. This makes the
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

## Deployment Recommendations

- Use TLS for MQTT and HTTPS for the API.
- Keep broker and API ports restricted to trusted networks with firewall
  rules.
- Use unique, strong credentials and rotate them when exposure is
  suspected.
- Grant MQTT clients only the topic permissions they need.
- Do not commit credentials, generated firmware containing real
  credentials, or broker password files.
- Replace the Flask development server with a production-grade WSGI
  server before deployment.
