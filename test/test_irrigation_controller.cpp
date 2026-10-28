// Host-side unit tests for the irrigation logic.
// Build & run:  g++ -std=c++11 -I../smart_irrigation test_irrigation_controller.cpp -o t && ./t
#include <cstdio>
#include "irrigation_controller.h"

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static IrrigationConfig cfg() { return {520, 260, 30, 60, 60000UL, 300000UL, 50, 1000}; }
static int rawFor(int percent) { return 520 - (520 - 260) * percent / 100; }

int main() {
  // percent conversion and clamping
  { IrrigationController c(cfg());
    CHECK(c.toPercent(520) == 0); CHECK(c.toPercent(260) == 100);
    CHECK(c.toPercent(390) == 50); CHECK(c.toPercent(700) == 0); CHECK(c.toPercent(100) == 100); }

  // wet soil: pump stays off
  { IrrigationController c(cfg());
    CHECK(!c.update(rawFor(70), 0)); CHECK(c.getState() == IDLE); }

  // dry soil: pump turns on, keeps running in the hysteresis band, stops when wet
  { IrrigationController c(cfg());
    CHECK(c.update(rawFor(20), 0));
    CHECK(c.update(rawFor(45), 5000));   // between 30 % and 60 %: keep watering
    CHECK(!c.update(rawFor(61), 9000));  // reached 60 %: stop
    CHECK(c.getState() == IDLE);
    CHECK(!c.update(rawFor(45), 10000)); // band again: must NOT restart
  }

  // safety limit: soil never gets wet -> stop after 60 s, cooldown 5 min, then retry
  { IrrigationController c(cfg());
    uint32_t t = 0;
    CHECK(c.update(rawFor(10), t));
    t = 59000; CHECK(c.update(rawFor(10), t));
    t = 60000; CHECK(!c.update(rawFor(10), t)); CHECK(c.getState() == COOLDOWN);
    t = 200000; CHECK(!c.update(rawFor(10), t));
    t = 360000; CHECK(!c.update(rawFor(10), t)); CHECK(c.getState() == IDLE);
    t = 360200; CHECK(c.update(rawFor(10), t));  // tries again
  }

  // sensor fault (wire unplugged) while watering -> pump off immediately
  { IrrigationController c(cfg());
    CHECK(c.update(rawFor(10), 0));
    CHECK(!c.update(1023, 1000)); CHECK(c.getState() == SENSOR_FAULT);
    CHECK(!c.update(20, 2000));   // shorted
    CHECK(c.update(rawFor(10), 3000));  // sensor back and dry -> waters again
  }

  // millis() rollover (~49 days) does not break the run-time limit
  { IrrigationController c(cfg());
    uint32_t start = 0xFFFFFFFFUL - 1000;
    CHECK(c.update(rawFor(10), start));
    CHECK(c.update(rawFor(10), start + 30000));   // wrapped past 0
    CHECK(!c.update(rawFor(10), start + 60000)); CHECK(c.getState() == COOLDOWN);
  }

  if (failures == 0) printf("All irrigation controller tests passed.\n");
  return failures == 0 ? 0 : 1;
}
