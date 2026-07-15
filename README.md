# Smart IoT Buffet Tray Monitoring System

An ESP32-based IoT project designed to monitor the food levels (weight) of buffet trays in real-time. It displays the status locally on an LCD screen and sends notifications via Blynk IoT app when the food is low.

## Features
- Dual Load Cell (HX711) integration for weighing two trays independently.
- Local 16x2 I2C LCD Display for real-time weight and level status (LOW, MID, HIGH).
- Audio alert via a physical Buzzer when thresholds are violated.
- Smart Cloud notification logic via Blynk IoT Platform.

## Pin Mapping (ESP32)
- **Load Cell 1 (Tray 1):** DOUT -> GPIO 4, SCK -> GPIO 5
- **Load Cell 2 (Tray 2):** DOUT -> GPIO 18, SCK -> GPIO 19
- **Buzzer:** GPIO 23
- **LCD (I2C):** SDA -> GPIO 21, SCL -> GPIO 22
