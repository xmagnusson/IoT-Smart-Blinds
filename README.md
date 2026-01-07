# IoT Smart Blinds

Smart Blinds is an IoT proof-of-concept (PoC) project that enables manual and automatic
control of window blinds via a web interface. The system demonstrates end-to-end
communication between an embedded edge device and a Spring Boot backend using MQTT,
with real-time UI updates via WebSockets.


## Architecture Overview

#### The system has 4 main parts:
- ESP8266 edge device (controls the blinds, glare protection logic - AUTO mode)
- Mosquitto (MQTT Broker)
- Spring Boot backend (Web application logic)
- Web UI (User interface in the browser - WebSockets for live data)

#### Communication protocols:
-	MQTT (edge device ↔ MQTT Broker ↔ Spring Boot backend)
-	REST (UI → Spring Boot backend)
-	WebSocket (Spring Boot backend → UI)

(All components communicate over a local Wi-Fi network using TCP/IP.)

<br/>
<div align="center">
  <img src="doc/diagrams/Architecture.png" alt="Architecture diagram" width="800">
  <div>System architecture diagram</div>
</div>
<br/>

The architecture is scalable. It is possible to add more devices without big changes. The devices and UI are not tightly coupled because they communicate through an MQTT Broker making the design more robust. Live UI updates via WebSockets ensure a responsive user experience.


## Screenshots 

### Web Application - UI
<div align="center">
  <img src="doc/images/IoT_SmartBlinds_i1.png" alt="Smart Blinds Web UI" width="800">
  <div>This is the main dashboard that a user interacts with. It shows Telemetry data and device controls.</div>
</div>
<br/>
<br/>

<div align="center">
  <img src="doc/images/IoT_SmartBlinds_i2.png" alt="Smart Blinds Web UI - Responsive" width="600">
  <div>The UI was designed with responsiveness in mind for good UX on mobile platforms as well.</div>
</div>
<br/>

### Edge Device for Smart Blinds Control (PoC)
<div align="center">
  <img src="doc/images/Edge_device_esp8266.jpeg" alt="ESP8266 Smart Blinds Device" width="600">
  <div>Edge device ESP8266 with light sensor and servo.</div>
</div>
<br/>
As mentioned this is just a PoC at the moment and is not yet controlling real blinds. However the PoC works and in the future for controlling real blinds a stronger servo motor (or potentially a stepper motor) could be used.


## Wiring Diagram
<div align="center">
  <img src="doc/diagrams/Edge_device_wiring_diagram.png" alt="Smart Blinds Edge Device wiring diagram" width="600">
  <div>Smart Blinds Edge Device - wiring diagram.</div>
</div>


## Implementation Details
<div align="justify">
ESP8266 is in the role of an edge device in the system. Its main task is to control the blinds with a servo (in this case of a PoC). It is designed to be controlled remotely from a web UI, but also to be able to run autonomously (e.g. when the connection is lost). It has two modes AUTO and MANUAL. In the AUTO mode the device runs logic locally to implement glare protection. Using a light sensor (LDR – Light Dependent Resistor) it can sense the light level and adjust the tilt of the blinds with the servo, once the level passes the threshold (Indicating normal or strong sunlight). The sensor has a software IIR Low-pass filter implemented. The time constant is set to 3 seconds. This ensures a stable reading with good response time, because the weather doesn’t change fast. To ensure stable operation, hysteresis is implemented for the on and off thresholds. The device uses a non-blocking code and implements a state machine for clear and predictable behaviour. It subscribes to commands and publishes its state and availability to MQTT Broker. By specifying Last Will & Testament (”offline”), the MQTT Broker publishes it when connection with the device is lost.
<br/>
<br/>
The MQTT Broker runs in a Docker container. I implemented authentication with username and password for basic security. For enhanced security I would configure Access Control List (ACL) to restrict device access to other topics and TLS to encrypt the communication between the MQTT Broker and its clients (devices). For now, as this is just a PoC running only on a local private network, full security was not configured.
<br/>
<br/>
Spring Boot web application serves as a backend that connects to the MQTT Broker. It parses the messages and pushes fresh data directly to the UI through WebSockets. The backend also exposes REST API endpoints to post commands from the UI. When a command is posted by the user, it publishes the command to the MQTT Broker. In the future it can run other logic (e.g. scheduling tasks).
<br/>
<br/>
The UI is responsive and designed with a good UX and reliability in mind. In AUTO mode it is not possible to set tilt because the UI part for this is disabled. Before switching the mode in the UI after user action, the UI shows “Loading ...” at first. It waits until it receives confirmation from the edge device that the mode has changed. When it switches to MANUAL mode after the confirmation from edge device, the user is able to set tilt in the UI.
</div>


## Technologies

#### Backend
- Java 21
- Spring Boot
- Spring Integration MQTT
- WebSockets (STOMP)
- Jackson (JSON processing)

#### Edge Device
- ESP8266
- Arduino framework
- Servo motor
- Light sensor

#### Communication
- MQTT (Mosquitto)

#### Infrastructure
- Docker (running Mosquitto and later can run also the Spring Boot app)

#### Frontend
- HTML | CSS | JavaScript
- Bootstrap
- Thymeleaf


## MQTT Topic Structure
The topic structure and the app is designed with multiple rooms and devices in mind.

```text
home/bedroom/blinds/{deviceId}/state
home/bedroom/blinds/{deviceId}/availability

home/bedroom/blinds/{deviceId}/cmd/mode
home/bedroom/blinds/{deviceId}/cmd/position
```


## Plans for the next version
- **Scheduled automation** - using Spring Boot Task Scheduler, the backend will be able to automatically close the blinds according to user set schedule. This could run every day to close and open the blinds automatically or even based on the sunset and sunrise calculations.
