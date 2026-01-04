const int LDR_SENSOR_PIN = A0;

const unsigned long LDR_SENSOR_INTERVAL = 500;
const float LDR_IIR_FILTER_ALPHA = 0.15f;

const uint16_t GLARE_THRESHOLD = 165;
const uint16_t GLARE_HYSTERESIS = 20;
const uint8_t TILT_OPEN = 90;
const uint8_t TILT_GLARE = 30;

// LDR-Sensor (PhotoResistor)
uint16_t filtered_lightLevel_int;

// command for servo
uint8_t targetPos = TILT_OPEN;
uint8_t lastTargetPos = TILT_OPEN;

// constrains for servo movement
const uint8_t SERVO_MIN = 0;
const uint8_t SERVO_MAX = 180;

void setup() {
  Serial.begin(115200);

  pinMode(LDR_SENSOR_PIN, INPUT);
}

void loop() {
  unsigned long now = millis();

  readSensors(now);
  checkGlareProtection(filtered_lightLevel_int);
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

  if (!glareActive && lightLevel > GLARE_THRESHOLD + GLARE_HYSTERESIS) {
    glareActive = true;
  }
  else if (glareActive && lightLevel < GLARE_THRESHOLD - GLARE_HYSTERESIS) {
    glareActive = false;
  }

  targetPos = glareActive ? TILT_GLARE : TILT_OPEN;
  targetPos = constrain(targetPos, SERVO_MIN, SERVO_MAX); // ensure the output is within valid range

  // for testing the servo command in a console
  if(targetPos != lastTargetPos){ // event-based logging

    Serial.print("[GLARE] state=");
    Serial.print(glareActive ? "ON" : "OFF");
    Serial.print(" targetPos=");
    Serial.println(targetPos);

    lastTargetPos = targetPos;
  }
}