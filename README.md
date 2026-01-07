# IoT Smart Blinds

Smart Blinds is an IoT proof-of-concept (PoC) project that enables manual and automatic
control of window blinds via a web interface. The system demonstrates end-to-end
communication between an embedded edge device and a Spring Boot backend using MQTT,
with real-time UI updates via WebSockets.

## Overview

(to be added)

## Screenshots 

### Web Application - UI
<div align="center">
  <img src="doc/images/IoT_SmartBlinds_i1.png" alt="Smart Blinds Web UI" width="800">
  <div>The main dashboard that the user interacts with. It shows Telemetry data and device controls.</div>
</div>
<br/>
<br/>

<div align="center">
  <img src="doc/images/IoT_SmartBlinds_i2.png" alt="Smart Blinds Web UI - Responsive" width="600">
  <div>The UI was designed with responsiveness in mind for good UX on mobile platforms as well.</div>
</div>
<br/>

### Edge Device Smart blinds control (PoC)
<div align="center">
  <img src="doc/images/Edge_device_esp8266.jpeg" alt="ESP8266 Smart Blinds Device" width="600">
  <div>Edge device ESP8266 with light sensor and servo.</div>
</div>
<br/>
As mentioned this is just a PoC at the moment and is not yet controlling real blinds. However the PoC works and in the future for controlling real blinds a stronger servo motor (or potentially a stepper motor) could be used.

## Technologies

### Backend
- Java 21
- Spring Boot
- Spring Integration MQTT
- WebSockets (STOMP)
- Jackson (JSON processing)

### Edge Device
- ESP8266
- Arduino framework
- Servo motor
- Light sensor

### Communication
- MQTT (Mosquitto)

### Infrastructure
- Docker (running Mosquitto and later can run also the Springboot app)

### Frontend
- HTML / CSS / JavaScript
- Bootstrap


## MQTT Topic Structure
The topic structure and the app is designed with multiple rooms and devices in mind.

```text
home/bedroom/blinds/{deviceId}/state
home/bedroom/blinds/{deviceId}/availability

home/bedroom/blinds/{deviceId}/cmd/mode
home/bedroom/blinds/{deviceId}/cmd/position
```

## Plans for the next version
- **Scheduled automation** - using Springboot Task Schedular the backend will be able to automatically close the blinds according to user set schedule. This could run everyday to close and open the blinds automatically or even based on the sunset and sunrise calculations
