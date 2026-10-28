// Pure decision logic for the smart irrigation system.
// No Arduino dependencies, so it can be unit-tested on a PC.
#ifndef IRRIGATION_CONTROLLER_H
#define IRRIGATION_CONTROLLER_H

#include <stdint.h>

struct IrrigationConfig {
  int dryRaw;                     // ADC reading in completely dry soil
  int wetRaw;                     // ADC reading in fully wet soil
  int startBelowPercent;          // start watering below this moisture
  int stopAtPercent;              // stop watering at/above this moisture
  uint32_t maxRunMs;         // safety limit for one watering cycle
  uint32_t cooldownMs;       // pause after hitting the safety limit
  int faultLowRaw;                // readings outside [faultLowRaw, faultHighRaw]
  int faultHighRaw;               //   mean the sensor is disconnected/shorted
};

enum IrrigationState { IDLE, WATERING, COOLDOWN, SENSOR_FAULT };

class IrrigationController {
 public:
  explicit IrrigationController(const IrrigationConfig& c)
      : cfg(c), state(IDLE), stateSince(0), lastPercent(0) {}

  // Convert a raw ADC value to 0-100 % moisture (capacitive sensors read
  // lower when wetter, so dryRaw > wetRaw).
  int toPercent(int raw) const {
    long span = (long)cfg.dryRaw - cfg.wetRaw;
    if (span == 0) return 0;
    long p = ((long)cfg.dryRaw - raw) * 100L / span;
    if (p < 0) p = 0;
    if (p > 100) p = 100;
    return (int)p;
  }

  // Call regularly with the latest sensor reading and the current time.
  // Returns true if the pump should be ON.
  bool update(int raw, uint32_t nowMs) {
    if (raw < cfg.faultLowRaw || raw > cfg.faultHighRaw) {
      enter(SENSOR_FAULT, nowMs);
      return false;
    }
    lastPercent = toPercent(raw);

    switch (state) {
      case SENSOR_FAULT:            // sensor is back: resume normally
      case IDLE:
        if (lastPercent < cfg.startBelowPercent) enter(WATERING, nowMs);
        else enter(IDLE, nowMs);
        break;
      case WATERING:
        if (lastPercent >= cfg.stopAtPercent) enter(IDLE, nowMs);
        else if (nowMs - stateSince >= cfg.maxRunMs) enter(COOLDOWN, nowMs);
        break;
      case COOLDOWN:
        if (nowMs - stateSince >= cfg.cooldownMs) enter(IDLE, nowMs);
        break;
    }
    return state == WATERING;
  }

  IrrigationState getState() const { return state; }
  int moisturePercent() const { return lastPercent; }

 private:
  void enter(IrrigationState s, uint32_t nowMs) {
    if (s != state) { state = s; stateSince = nowMs; }
  }
  IrrigationConfig cfg;
  IrrigationState state;
  uint32_t stateSince;
  int lastPercent;
};

#endif
