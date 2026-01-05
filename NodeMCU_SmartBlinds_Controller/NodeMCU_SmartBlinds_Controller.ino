#include <Servo.h>


const int LDR_SENSOR_PIN = A0; // NodeMCU analog input A0 (0-3.3V with internal voltage divider, scaled to 0-1023) , raw ESP8266 chip only accepts 0-1V on A0 !!!
const uint8_t SERVO_PIN_BLINDS_1 = D5; // GPIO14 (D5) on NodeMCU

Servo servoBlinds1;

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


void setup() {
  Serial.begin(115200);

  pinMode(LDR_SENSOR_PIN, INPUT);
}

void loop() {
  unsigned long now = millis();

  readSensors(now);
  updateStateMachine(now);
}


void updateStateMachine(unsigned long now) {
  static DeviceState lastState = STATE_INIT;

  if (deviceState != lastState) {
    onStateEnter(deviceState);
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