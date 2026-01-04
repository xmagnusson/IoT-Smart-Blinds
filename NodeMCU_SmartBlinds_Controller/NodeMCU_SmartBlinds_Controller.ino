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

// LDR-Sensor (PhotoResistor)
uint16_t filtered_lightLevel_int;

// Servo state
bool isServoBlinds1Attached = false;
uint8_t currentPos = TILT_OPEN;
uint8_t targetPos = TILT_OPEN;

// Timing
const unsigned long idleDetachTime = 1000; // miliseconds

// Movement step
const unsigned long moveInterval = 10; // ms between steps

// constrains for servo movement
const uint8_t SERVO_MIN = 0;
const uint8_t SERVO_MAX = 180;


void setup() {
  Serial.begin(115200);

  pinMode(LDR_SENSOR_PIN, INPUT);

  // Servo 1 Init
  servoBlinds1.attach(SERVO_PIN_BLINDS_1); 
  isServoBlinds1Attached = true;
  servoBlinds1.write(TILT_OPEN);
}

void loop() {
  unsigned long now = millis();

  readSensors(now);
  checkGlareProtection(filtered_lightLevel_int);
  moveBlinds(now);
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
  static uint8_t lastTargetPos = TILT_OPEN;

  if (!glareActive && lightLevel > GLARE_THRESHOLD + GLARE_HYSTERESIS) {
    glareActive = true;
  }
  else if (glareActive && lightLevel < GLARE_THRESHOLD - GLARE_HYSTERESIS) {
    glareActive = false;
  }

  targetPos = glareActive ? TILT_GLARE : TILT_OPEN;

  // for testing the servo command in a console
  if(targetPos != lastTargetPos){ // event-based logging

    Serial.print("[GLARE] state=");
    Serial.print(glareActive ? "ON" : "OFF");
    Serial.print(" targetPos=");
    Serial.println(targetPos);

    lastTargetPos = targetPos;
  }
}

// Function to move blinds slowly to a desired target position
void moveBlinds(unsigned long now){ 
  static unsigned long lastStepTime_servoBlinds1 = 0;
  static unsigned long lastMoveTime_servoBlinds1 = 0;

  // Servo controling Blinds 1
  if (currentPos != targetPos && now - lastStepTime_servoBlinds1 >= moveInterval) {

    lastStepTime_servoBlinds1 = now;

    if (!isServoBlinds1Attached) {
      servoBlinds1.attach(SERVO_PIN_BLINDS_1); 
      isServoBlinds1Attached = true;
    }

    // Step toward target
    if (currentPos < targetPos) currentPos += 1;
    if (currentPos > targetPos) currentPos -= 1;

    servoBlinds1.write(currentPos);
    lastMoveTime_servoBlinds1 = now; // reset idle timer
  }

  // Detach if idle for > 1s (to stop servo from buzzing when stationary)
  if (isServoBlinds1Attached && now - lastMoveTime_servoBlinds1 > idleDetachTime) {
    servoBlinds1.detach();
    isServoBlinds1Attached = false;
  }

}