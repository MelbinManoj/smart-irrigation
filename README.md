# 💧 Smart Irrigation System

> Automatic plant watering with an Arduino Uno: a soil-moisture sensor decides when to run the pump, with safety limits so it never floods or runs dry.

![badge](https://img.shields.io/badge/Arduino%20Uno-00878F?logo=arduino&logoColor=white&style=flat) ![badge](https://img.shields.io/badge/C%2B%2B-00599C?logo=cplusplus&logoColor=white&style=flat) ![badge](https://img.shields.io/badge/IoT-00C2A8?style=flat)
![tests](https://github.com/MelbinManoj/smart-irrigation/actions/workflows/tests.yml/badge.svg)

> 📝 **About this code:** I built this project during my B.Tech, but the original source code was lost. The code here is a **rebuilt version (2026)** based on the original design. It compiles for the Arduino Uno, and its logic is checked by automated tests that run on simulated sensor data. It has not yet been re-tested on the physical hardware.

## 📖 Overview

Farm and garden irrigation is often done by hand, which wastes water and labour. This system measures soil moisture and switches a water pump through a relay **only when the soil is actually dry**, then stops once it is wet enough. That saves water and fertiliser and needs no one to supervise it.

## ✨ Features

- Capacitive soil-moisture sensing, calibrated to 0–100 %, averaged over 10 samples to reduce noise
- **Hysteresis**: starts below 30 % and stops at 60 %, so the pump doesn't rapidly switch on and off
- **Safety limit**: at most 60 s per watering cycle, then a 5 min cooldown, so a broken sensor can't flood the field
- **Sensor-fault detection**: a disconnected or shorted sensor switches the pump off immediately
- Handles the `millis()` overflow after ~49 days correctly
- Live status over the Serial Monitor (moisture %, state)

**States:** `IDLE` → `WATERING` → `IDLE` (normal)  ·  `WATERING` → `COOLDOWN` (max run time hit)  ·  any → `SENSOR_FAULT`

## 🧰 Components

| Component | Qty |
|---|:---:|
| Arduino Uno | 1 |
| Capacitive soil-moisture sensor v1.2 | 1 |
| 5 V relay module | 1 |
| Mini submersible water pump (3–6 V) + tubing | 1 |
| Separate pump power supply | 1 |
| Jumper wires | — |

## 🔌 Wiring

| Connection | Arduino pin |
|---|---|
| Sensor AOUT | A0 |
| Sensor VCC / GND | 5V / GND |
| Relay IN | D7 (active LOW) |
| Relay VCC / GND | 5V / GND |
| Pump | Relay COM/NO, from its own supply |
| Status LED | D13 (on-board) |

## 🚀 Getting Started

1. Open [`smart_irrigation/smart_irrigation.ino`](./smart_irrigation/smart_irrigation.ino) in the Arduino IDE. The `irrigation_controller.h` file in the same folder opens with it.
2. Select **Arduino Uno** (or Nano) and upload. No extra libraries are needed.
3. Open the Serial Monitor to watch the live readings.
4. **Calibrate:** Open the Serial Monitor, note the `raw` value with the sensor in dry air and in a glass of water, and put them into `dryRaw` / `wetRaw`.

## 🧪 Tests

All decision logic lives in [`irrigation_controller.h`](./smart_irrigation/irrigation_controller.h), with no Arduino dependencies, so it can be tested on a PC with simulated sensor data:

```bash
cd test
g++ -std=c++11 -I../smart_irrigation test_irrigation_controller.cpp -o test && ./test
```

What's tested:

- ✅ Moisture % conversion and clamping
- ✅ Wet soil keeps the pump off
- ✅ Dry soil starts watering; stops at 60 % and does not restart inside the band
- ✅ Pump is cut after 60 s if the soil never gets wet, then retries after the cooldown
- ✅ Unplugged or shorted sensor switches the pump off at once
- ✅ Correct timing across the `millis()` overflow

These tests run automatically on every push with GitHub Actions.

## 📂 Project Structure

```text
smart-irrigation/
├── smart_irrigation/
│   ├── smart_irrigation.ino      # hardware layer: pins, sensors, outputs
│   └── irrigation_controller.h # decision logic (unit-tested)
├── test/
│   └── test_irrigation_controller.cpp
└── README.md
```

## 👤 Author

**Melbin Manoj** — [GitHub](https://github.com/MelbinManoj) · [LinkedIn](https://www.linkedin.com/in/melbin-manoj-8ba512269/)
