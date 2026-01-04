#include <Servo.h>

const unsigned long LDR_SENSOR_INTERVAL = 500;
const int LDR_SENSOR_PIN = A0;


// LDR-Sensor (PhotoResistor)
uint16_t filtered_lightLevel_int;


void setup() {
  Serial.begin(115200);

  pinMode(LDR_SENSOR_PIN, INPUT);
}

void loop() {
  unsigned long now = millis();

  readSensors(now);
}


void readSensors(unsigned long now){
  static unsigned long lastRead_LDR_Sensor = 0;
  static float filtered_lightLevel = analogRead(LDR_SENSOR_PIN);

  // LDR_Sensor
  if (now - lastRead_LDR_Sensor >= LDR_SENSOR_INTERVAL) {
    lastRead_LDR_Sensor = now;
    filtered_lightLevel = filtered_lightLevel * 0.85 + analogRead(LDR_SENSOR_PIN) * 0.15; //Exponential Moving Average (EMA) filter (alpha = 0.15 for a time constant of 3s at 500ms sample rate)
    filtered_lightLevel_int = (uint16_t)(filtered_lightLevel + 0.5f);
    Serial.println(filtered_lightLevel_int);
  }
}