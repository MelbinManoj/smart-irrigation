// Smart Irrigation System - Arduino Uno
// Reads a capacitive soil-moisture sensor and switches a water pump through
// a relay, with hysteresis, a maximum run time and sensor-fault protection.
//
// Wiring:
//   Soil moisture sensor AOUT -> A0, VCC -> 5V, GND -> GND
//   Relay module IN -> D7 (active LOW), VCC -> 5V, GND -> GND
//   Pump powered through the relay's COM/NO contacts from its own supply
//   Status LED -> D13 (on-board)

#include "irrigation_controller.h"

const uint8_t SENSOR_PIN = A0;
const uint8_t RELAY_PIN = 7;
const uint8_t LED_PIN = 13;
const bool RELAY_ACTIVE_LOW = true;

// Calibrate dryRaw/wetRaw for your sensor: note A0 in dry air and in a glass of water.
IrrigationConfig config = {
  520,              // dryRaw
  260,              // wetRaw
  30,               // start watering below 30 %
  60,               // stop watering at 60 %
  60000UL,          // max 60 s per cycle
  300000UL,         // 5 min cooldown after a max-length cycle
  50,               // fault if reading < 50 (shorted)
  1000              // fault if reading > 1000 (disconnected)
};

IrrigationController controller(config);
unsigned long lastPrint = 0;

int readMoistureRaw() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {   // average 10 samples to reduce noise
    sum += analogRead(SENSOR_PIN);
    delay(5);
  }
  return (int)(sum / 10);
}

void setPump(bool on) {
  digitalWrite(RELAY_PIN, (on ^ RELAY_ACTIVE_LOW) ? HIGH : LOW);
  digitalWrite(LED_PIN, on ? HIGH : LOW);
}

const char* stateName(IrrigationState s) {
  switch (s) {
    case IDLE: return "IDLE";
    case WATERING: return "WATERING";
    case COOLDOWN: return "COOLDOWN";
    default: return "SENSOR_FAULT";
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  setPump(false);
  Serial.println(F("Smart Irrigation System started"));
}

void loop() {
  int raw = readMoistureRaw();
  bool pumpOn = controller.update(raw, millis());
  setPump(pumpOn);

  if (millis() - lastPrint >= 2000) {
    lastPrint = millis();
    Serial.print(F("raw="));
    Serial.print(raw);
    Serial.print(F(" moisture="));
    Serial.print(controller.moisturePercent());
    Serial.print(F("% state="));
    Serial.println(stateName(controller.getState()));
  }
  delay(200);
}
