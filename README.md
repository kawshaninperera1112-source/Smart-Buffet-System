# 🍽️ Smart IoT Buffet Tray Monitoring System

An ESP32-based IoT project that weighs buffet trays in real time, shows their status on a local LCD, sounds a buzzer when food is running low, and sends a phone notification through the **Blynk IoT** platform so staff can refill trays before they run out.

<!-- Add a photo of the finished prototype and a wiring diagram here, for example:
![Prototype](images/prototype.jpg)
![Wiring Diagram](images/wiring.png) -->

---

## 📌 Overview

In a buffet, empty trays hurt the guest experience and staff usually notice too late. This system uses load cells under two food trays to measure how much food is left. When a tray falls below a set weight, the system alerts staff locally (buzzer and LCD) and remotely (Blynk push notification).

## ✨ Features

- **Two independent trays:** dual HX711 load-cell amplifiers, each weighing its own tray.
- **Local display:** 16x2 I2C LCD shows each tray's weight and a status of `LOW`, `MID` or `HIGH`.
- **Audible alert:** buzzer sounds while any tray is `LOW`.
- **Cloud monitoring:** live weights streamed to Blynk (virtual pins `V0` and `V1`).
- **Smart notifications:** one "refill" event per low-food episode (no repeated spam while the tray stays low).
- **Fault tolerance:** detects a missing LCD or load cell at start-up and keeps running with what is available (`NO SENSOR` is shown for a failed tray).
- **Warm-up period:** 10-second initialisation countdown before readings are used.

## 🔩 Hardware

| Component | Quantity | Notes |
|---|---|---|
| ESP32 development board | 1 | Wi-Fi enabled microcontroller |
| Load cell + HX711 amplifier | 2 | One per tray |
| 16x2 LCD with I2C backpack | 1 | I2C address `0x27` |
| Active buzzer | 1 | Low-level alert |

### Pin Mapping (ESP32)

| Device | Signal | GPIO |
|---|---|---|
| Load Cell 1 (Tray 1) | DOUT | 4 |
| Load Cell 1 (Tray 1) | SCK | 5 |
| Load Cell 2 (Tray 2) | DOUT | 18 |
| Load Cell 2 (Tray 2) | SCK | 19 |
| Buzzer | Signal | 23 |
| LCD (I2C) | SDA | 21 |
| LCD (I2C) | SCL | 22 |

Power (VCC/GND) connections for each module are not listed here.

## ⚙️ How It Works

1. Both load cells are read (average of 3 samples) and converted to weight using a calibration factor.
2. Each tray gets a status from its weight:

| Status | Condition |
|---|---|
| `LOW` | below 30 |
| `MID` | 30 up to 800 |
| `HIGH` | 800 and above |

3. The LCD shows the weight and status for both trays.
4. If either tray is `LOW`, the buzzer turns on and a Blynk event (`tray1_low` or `tray2_low`) is sent once.
5. The notification flag resets when the tray is refilled, so the next low episode triggers a new alert.
6. Weights are also pushed to Blynk every 500 ms for live monitoring.

Weights are in the units set by your calibration (grams when calibrated with gram weights).

## 🚀 Setup

### 1. Software

- [Arduino IDE](https://www.arduino.cc/en/software) with the **ESP32 board package** installed
- Libraries (Library Manager):
  - `Blynk`
  - `LiquidCrystal_I2C`
  - `HX711` (by Bogdan Necula / bogde)

### 2. Blynk configuration

1. Create a template named **Buffet** in the Blynk console.
2. Add two numeric datastreams: **V0** (Tray 1 weight) and **V1** (Tray 2 weight).
3. Add two events: **`tray1_low`** and **`tray2_low`** (enable notifications for them).
4. Create a device from the template and copy its **Template ID** and **Auth Token**.

### 3. Credentials (never commit these)

Inside the sketch folder, copy `secrets.h.example` to `secrets.h`, then fill in your own values:

```cpp
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Buffet"
#define BLYNK_AUTH_TOKEN    "YOUR_BLYNK_AUTH_TOKEN"

char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";
```

`secrets.h` is listed in `.gitignore`, so it stays out of the repository.

### 4. Calibrate the load cells

The calibration factors `cal1` and `cal2` in the sketch are specific to the load cells used in this prototype. For your own hardware:

1. Run the calibration example that ships with the HX711 library.
2. Place a known weight on the tray and note the raw reading.
3. Divide the raw reading by the known weight to get the factor, and set `cal1` / `cal2`.

You can also adjust `LOW_TH` and `HIGH_TH` to match your tray sizes and portions.

### 5. Upload

Select your ESP32 board and port, then upload the sketch. Open the Serial Monitor at **115200 baud** to see live readings (`T1:... | T2:...`).

## ⚠️ Known Limitations & Future Work

- The sketch uses blocking `delay()` calls in the main loop and the blocking `Blynk.begin()`, so the display may not start normally when Wi-Fi is unavailable. A non-blocking design (`millis()` timing, `Blynk.config()` with a connection timeout) would keep the local display and buzzer working offline.
- No hysteresis: a reading that hovers around the `LOW` threshold can flip status repeatedly.
- The buzzer cannot be muted from the device.
- Only two trays are supported; more trays would need more HX711 modules or a multiplexing approach.
- Future ideas: a dashboard with weight history, a mute button, and estimating remaining serving portions.

## 📂 Project Structure

```
Smart-Buffet-System/
├── smart_buffet_tray_monitor/
│   ├── smart_buffet_tray_monitor.ino   # Main firmware sketch
│   └── secrets.h.example               # Credentials template (copy to secrets.h)
├── .gitignore                          # Ignores secrets.h
└── README.md
```

(The Arduino IDE requires the sketch folder and `.ino` file to share the same name.)

## 🛠️ Tech Stack

- **Hardware:** ESP32, HX711 load cell amplifiers, I2C LCD, buzzer
- **Firmware:** C++ (Arduino framework)
- **IoT platform:** Blynk

## 👩‍💻 Author

**Kawshani Perera**
BICT (Hons) undergraduate, Rajarata University of Sri Lanka
[GitHub](https://github.com/kawshaninperera1112-source)
