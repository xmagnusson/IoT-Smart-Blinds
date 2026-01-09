#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include "Secrets.h" // create a file Secrets.h (see Example__Secrets.h)
#include <ArduinoJson.h>


const int LDR_SENSOR_PIN = A0; // NodeMCU analog input A0 (0-3.3V with internal voltage divider, scaled to 0-1023) , raw ESP8266 chip only accepts 0-1V on A0 !!!
const uint8_t SERVO_PIN_BLINDS_1 = D5; // GPIO14 (D5) on NodeMCU

const unsigned long LDR_SENSOR_INTERVAL = 500;
const float LDR_IIR_FILTER_ALPHA = 0.15f;

const uint16_t GLARE_THRESHOLD = 750;
const uint16_t GLARE_HYSTERESIS = 50;
const uint8_t TILT_OPEN = 90;
const uint8_t TILT_GLARE = 30;

struct ServoControl {
  uint8_t currentPos;
  uint8_t targetPos;
  bool isAttached;
  unsigned long lastMoveTime;
  unsigned long lastStepTime;
};

// LDR-Sensor (PhotoResistor)
uint16_t filtered_lightLevel_int;

// Servo
Servo servoBlinds1;

// Servo states
ServoControl blinds1 = {
  .currentPos = TILT_OPEN,
  .targetPos = TILT_OPEN,
  .isAttached = false,
  .lastMoveTime = 0,
  .lastStepTime = 0
};

// Timing
const unsigned long idleDetachTime = 1000; // miliseconds

// Movement step
const unsigned long moveInterval = 40; // ms between steps

// Constraints for servo movement
const uint8_t SERVO_MIN = 0;
const uint8_t SERVO_MAX = 180;

// Finite State Machine - States
enum DeviceState {
  STATE_INIT,
  STATE_IDLE,
  STATE_MOVING
};

DeviceState deviceState = STATE_INIT;

// Control modes
enum ControlMode {
  MODE_AUTO,
  MODE_MANUAL
};

ControlMode controlMode = MODE_AUTO;

// Wifi
WiFiClient espClient;

bool wifiConnected = false;
unsigned long lastWifiTry = 0;
const unsigned long WIFI_RETRY_INTERVAL = 5000; // in ms

// MQTT
PubSubClient mqttClient(espClient);

bool mqttConnected = false;
unsigned long lastMqttTry = 0;
const unsigned long MQTT_RETRY_INTERVAL = 20000; // in ms

const char* DEVICE_ID = "B1";

// MQTT Topics
const char* TOPIC_CMD_MODE = "home/bedroom/blinds/B1/cmd/mode";
const char* TOPIC_CMD_POSITION = "home/bedroom/blinds/B1/cmd/position";

const char* TOPIC_STATE = "home/bedroom/blinds/B1/state";
const char* TOPIC_AVAILABILITY = "home/bedroom/blinds/B1/availability";


void setup() {
  Serial.begin(115200);

  pinMode(LDR_SENSOR_PIN, INPUT);

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

void loop() {
  unsigned long now = millis();

  handleWifi(now);
  handleMQTT(now);
  readSensors(now);
  updateStateMachine(now);
}


void updateStateMachine(unsigned long now) {
  static DeviceState lastState = STATE_INIT;

  if (deviceState != lastState) {
    onStateEnter(deviceState);
    publishState();
    lastState = deviceState;
  }

  switch (deviceState) {
    case STATE_INIT:
      handleInitState();
      break;

    case STATE_IDLE:
      handleIdleState(now);
      break;

    case STATE_MOVING:
      handleMovingState(now);
      break;

  }
}

void readSensors(unsigned long now){
  static unsigned long lastRead_LDR_Sensor = 0;
  static float filtered_lightLevel = analogRead(LDR_SENSOR_PIN);

  // LDR_Sensor
  if (now - lastRead_LDR_Sensor >= LDR_SENSOR_INTERVAL) {
    lastRead_LDR_Sensor = now;
    filtered_lightLevel = filtered_lightLevel * (1.0f - LDR_IIR_FILTER_ALPHA) + analogRead(LDR_SENSOR_PIN) * LDR_IIR_FILTER_ALPHA; //Exponential Moving Average (EMA) filter (alpha = 0.15 for a time constant of ~3s at 500ms sample rate)
    filtered_lightLevel_int = (uint16_t)(filtered_lightLevel + 0.5f);
    Serial.println(filtered_lightLevel_int);
  }
}

void checkGlareProtection(uint16_t lightLevel){
  static bool glareActive = false;
  static uint8_t lastTargetPos_S1 = TILT_OPEN;

  if(controlMode != MODE_AUTO) return;  // ignores glare protection in manual mode


  if (!glareActive && lightLevel > GLARE_THRESHOLD + GLARE_HYSTERESIS) {
    glareActive = true;
  }
  else if (glareActive && lightLevel < GLARE_THRESHOLD - GLARE_HYSTERESIS) {
    glareActive = false;
  }

  blinds1.targetPos = glareActive ? TILT_GLARE : TILT_OPEN;

  // for testing the servo command in a console
  if(blinds1.targetPos != lastTargetPos_S1){ // event-based logging

    Serial.print("[GLARE] state=");
    Serial.print(glareActive ? "ON" : "OFF");
    Serial.print(" blinds1.targetPos=");
    Serial.println(blinds1.targetPos);

    lastTargetPos_S1 = blinds1.targetPos;
  }
}

// Function to move blinds slowly to a desired target position
void moveBlinds(unsigned long now){ 
  
  // Servo controling Blinds 1
  if (blinds1.currentPos != blinds1.targetPos && now - blinds1.lastStepTime >= moveInterval) {
    blinds1.lastStepTime = now;

    if (!blinds1.isAttached) {
      servoBlinds1.attach(SERVO_PIN_BLINDS_1); 
      blinds1.isAttached = true;
    }

    // Step toward target
    if (blinds1.currentPos < blinds1.targetPos) blinds1.currentPos += 1;
    if (blinds1.currentPos > blinds1.targetPos) blinds1.currentPos -= 1;

    servoBlinds1.write(blinds1.currentPos);
    blinds1.lastMoveTime = now; // reset idle timer
  }

}

void servoTimeoutDetach(unsigned long now){

  // Servo controling Blinds 1
  // Detach if idle for > 1s (to stop servo from buzzing when stationary)
  if (blinds1.isAttached && now - blinds1.lastMoveTime > idleDetachTime) {
    servoBlinds1.detach();
    blinds1.isAttached = false;
  }
}

//State Machine State Handlers

void onStateEnter(DeviceState state) {
  switch (state) {
    case STATE_INIT:
      Serial.println("[ENTER] INIT");
      break;
    case STATE_IDLE:
      Serial.println("[ENTER] IDLE");
      break;
    case STATE_MOVING:
      Serial.println("[ENTER] MOVING");
      break;
  }
}

// INIT
void handleInitState() {

  // set a init servo position
  servoBlinds1.attach(SERVO_PIN_BLINDS_1);
  blinds1.isAttached = true;
  servoBlinds1.write(TILT_OPEN);
  blinds1.lastMoveTime = millis(); // not to detach servo immediatelly rather after a set interval

  // Transition
  deviceState = STATE_IDLE;

  Serial.println("[STATE] INIT → IDLE");
}

// IDLE
void handleIdleState(unsigned long now) {

  checkGlareProtection(filtered_lightLevel_int);
  
  // Ensure servo is detached in idle after 1 second of inactivity
  servoTimeoutDetach(now);

  // Transition condition
  if (blinds1.currentPos != blinds1.targetPos) {
    deviceState = STATE_MOVING;
    Serial.println("[STATE] IDLE → MOVING");
  }
}

// MOVING
void handleMovingState(unsigned long now) {

  moveBlinds(now);

  // Transition when done
  if (blinds1.currentPos == blinds1.targetPos) {
    deviceState = STATE_IDLE;
    Serial.println("[STATE] MOVING → IDLE");
  }
}


// Control Modes Logic
void setControlMode(ControlMode mode) {
  if (controlMode == mode) return;

  controlMode = mode;

  Serial.print("[MODE] ");
  Serial.println(mode == MODE_AUTO ? "AUTO" : "MANUAL");
}

void setTargetPosManually(uint8_t position) {
  if (controlMode != MODE_MANUAL) return;

  position = constrain(position, SERVO_MIN, SERVO_MAX);

  blinds1.targetPos = position;

  Serial.print("[MANUAL] blinds1.targetPos=");
  Serial.println(position);
}


// Wifi handler
void handleWifi(unsigned long now) {
  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiConnected) {
      wifiConnected = true;
      Serial.println("[WIFI] Connected");
      Serial.print("[WIFI] IP: ");
      Serial.println(WiFi.localIP());
    }
    return;
  }

  wifiConnected = false;

  // retry connection periodically
  if (now - lastWifiTry > WIFI_RETRY_INTERVAL) {
    lastWifiTry = now;
    Serial.print("[WIFI] Connecting...");
    Serial.println(WiFi.status());
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

// MQTT handler
void handleMQTT(unsigned long now) {
  if (!wifiConnected) return; // only try MQTT if connected to Wifi

  // Detect disconnect
  if (!mqttClient.connected()) {
    mqttConnected = false;
  }

  if (!mqttConnected) {
    if (now - lastMqttTry > MQTT_RETRY_INTERVAL) {
      lastMqttTry = now;
      Serial.println("[MQTT] Connecting...");

      if (mqttClient.connect(DEVICE_ID, MQTT_USERNAME, MQTT_PASSWORD, TOPIC_AVAILABILITY, 1, true, "offline")) {  // id, username, password, LWT Topic, QoS, retain, Last Will and Testament
        mqttConnected = true;
        Serial.println("[MQTT] connected");
        mqttClient.publish(TOPIC_AVAILABILITY, "online", true);
        publishState();
        mqttClient.subscribe(TOPIC_CMD_MODE);
        mqttClient.subscribe(TOPIC_CMD_POSITION);
      }
    }
  }

  if (mqttConnected) {
    mqttClient.loop();
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0'; // terminate raw message with '\0' before turning to C string
  String msg = String((char*)payload);

  Serial.print("[MQTT] ");
  Serial.print(topic);
  Serial.print(" → ");
  Serial.println(msg);

  // ----- MODE COMMAND -----
  if (strcmp(topic, TOPIC_CMD_MODE) == 0) {
    if (msg == "AUTO") {
      setControlMode(MODE_AUTO);
    } else if (msg == "MANUAL") {
      setControlMode(MODE_MANUAL);
    }
  }

  // ----- MANUAL POSITION COMMAND -----
  else if (strcmp(topic, TOPIC_CMD_POSITION) == 0) {
    setTargetPosManually(msg.toInt());
  }
  
  publishState(); //report back new state
}

void publishState() {
  StaticJsonDocument<256> doc;

  doc["mode"] = (controlMode == MODE_AUTO) ? "AUTO" : "MANUAL";
  doc["state"] = (deviceState == STATE_INIT)   ? "INIT" : (deviceState == STATE_IDLE)   ? "IDLE" : "MOVING";
  doc["currentPos"] = blinds1.currentPos;
  doc["targetPos"]  = blinds1.targetPos;
  doc["attached"]   = blinds1.isAttached;
  doc["light"]      = filtered_lightLevel_int;

  char payload[256];
  serializeJson(doc, payload);

  mqttClient.publish(TOPIC_STATE, payload, true);
}
