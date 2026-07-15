#define BLYNK_TEMPLATE_ID "TMPL6gYOgJf_x"      // Define Blynk template ID
#define BLYNK_TEMPLATE_NAME "Buffet"           // Define Blynk project name
#define BLYNK_AUTH_TOKEN "k-H7XG9p86UTb3WNRbkou9fvrdpOIRH_"  // Fix: Removed trailing spaces

#include <WiFi.h>                             // Include WiFi library for ESP32
#include <BlynkSimpleEsp32.h>                 // Include Blynk library for ESP32
#include <Wire.h>                             // Include I2C communication library
#include <LiquidCrystal_I2C.h>                // Include LCD I2C library
#include "HX711.h"                             // Include HX711 load cell amplifier library

// WiFi Credentials
char ssid[] = "YOUR_WIFI_SSID";                // Change this before flashing, but keep placeholders for GitHub
char pass[] = "YOUR_WIFI_PASSWORD";            // Change this before flashing, but keep placeholders for GitHub

#define DOUT1 4                              // Load cell 1 data pin
#define SCK1  5                              // Load cell 1 clock pin
#define DOUT2 18                              // Load cell 2 data pin
#define SCK2  19                              // Load cell 2 clock pin
#define BUZ   23                              // Buzzer pin

HX711 scale1;                                // Create HX711 object for tray 1
HX711 scale2;                                // Create HX711 object for tray 2
LiquidCrystal_I2C lcd(0x27, 16, 2);         // Create LCD object at I2C address 0x27, 16x2
BlynkTimer timer;                            // Create a Blynk timer object

bool lcd_ok = false;                         // Flag to check if LCD is connected
bool lc1_ok = false;                         // Flag to check if load cell 1 is working
bool lc2_ok = false;                         // Flag to check if load cell 2 is working

bool notified1 = false;                      // Flag to check if tray 1 LOW event sent
bool notified2 = false;                      // Flag to check if tray 2 LOW event sent

unsigned long startTime;                     // Variable to store system start time
const unsigned long START_DELAY = 10000;    // Delay before system becomes ready (10 seconds)
bool systemReady = false;                    // Flag to check if system is ready

float cal1 = -449.596;                 // Calibration factor for load cell 1
float cal2 = -470.305;                 // Calibration factor for load cell 2

float LOW_TH = 30;                          // Weight threshold for LOW level
float HIGH_TH = 800;                         // Weight threshold for HIGH level

String getStatus(float w) {                  // Function to determine tray status
    if (w < LOW_TH) return "LOW";            // Return LOW if weight is below LOW_TH
    else if (w < HIGH_TH) return "MID";     // Return MID if weight is between LOW_TH and HIGH_TH
    else return "HIGH";                      // Return HIGH if weight is above HIGH_TH
}

bool checkI2C(byte addr) {                   // Function to check if I2C device exists
    Wire.beginTransmission(addr);            // Start I2C communication with given address
    return (Wire.endTransmission() == 0);    // Return true if device responds
}

void showLCDMsg(String line1, String line2, int d = 1500) {  // Function to display message on LCD
    if (!lcd_ok) return;                     // Exit if LCD not connected
    lcd.clear();                             // Clear LCD screen
    lcd.setCursor(0, 0);                     // Set cursor to first line
    lcd.print(line1);                         // Print first line
    lcd.setCursor(0, 1);                     // Set cursor to second line
    lcd.print(line2);                         // Print second line
    delay(d);                                // Wait for specified duration (default 1500ms)
}

void sendToBlynk() {                         // Function to send weight data to Blynk
    if (lc1_ok) {                             // If load cell 1 is ready
        float w1 = scale1.get_units(3);      // Read weight (average 3 readings)
        Blynk.virtualWrite(V0, w1);          // Send weight to Blynk virtual pin V0
    }
    if (lc2_ok) {                             // If load cell 2 is ready
        float w2 = scale2.get_units(3);      // Read weight (average 3 readings)
        Blynk.virtualWrite(V1, w2);          // Send weight to Blynk virtual pin V1
    }
}

void setup() {                                
    Serial.begin(115200);                      // Start serial communication at 115200 baud
    delay(1000);                              // Wait 1 second

    pinMode(BUZ, OUTPUT);                      // Set buzzer pin as output

    Wire.begin();                             // Initialize I2C communication
    Wire.setTimeOut(50);                      // Set I2C timeout to 50ms

    Serial.println("System Booting...");      // Print boot message

    if (checkI2C(0x27)) {                     // Check if LCD is connected at address 0x27
        lcd.init();                            // Initialize LCD
        lcd.backlight();                       // Turn on backlight
        lcd_ok = true;                         // Mark LCD as OK
        Serial.println("LCD Found");           // Print message
        showLCDMsg("Connecting WiFi", ssid);   // Show connecting WiFi message
    } else {
        Serial.println("LCD NOT FOUND → running without display");  // Print if LCD missing
    }

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Connect to Blynk server

    if (WiFi.status() == WL_CONNECTED) {      // Check if WiFi connected
        showLCDMsg("WiFi Connected", "");     // Show WiFi connected message
    } else {
        showLCDMsg("WiFi Failed", "");        // Show WiFi failed message
    }

    showLCDMsg("Blynk Connecting...", "");    // Show Blynk connecting message

    if (Blynk.connected()) {                  // Check if Blynk connected
        showLCDMsg("Blynk Connected", "");    // Show Blynk connected message
    } else {
        showLCDMsg("Blynk Offline", "");      // Show Blynk offline message
    }

    scale1.begin(DOUT1, SCK1);                // Initialize load cell 1
    delay(1000);                              // Wait 1 second
    if (scale1.is_ready()) {                  // Check if load cell 1 is ready
        scale1.set_scale(cal1);               // Set calibration factor
        scale1.tare();                        // Reset to zero
        lc1_ok = true;                        // Mark as OK
        Serial.println("Load Cell 1 OK");     // Print message
    } else {
        Serial.println("Load Cell 1 NOT FOUND");  // Print if not found
    }

    scale2.begin(DOUT2, SCK2);                // Initialize load cell 2
    delay(1000);                              // Wait 1 second
    if (scale2.is_ready()) {                  // Check if load cell 2 is ready
        scale2.set_scale(cal2);               // Set calibration factor
        scale2.tare();                        // Reset to zero
        lc2_ok = true;                        // Mark as OK
        Serial.println("Load Cell 2 OK");     // Print message
    } else {
        Serial.println("Load Cell 2 NOT FOUND");  // Print if not found
    }

    timer.setInterval(500L, sendToBlynk);     // Set Blynk data sending interval 500ms

    Serial.println("System Running...");      // Print system running
    lcd.clear();                               // Clear LCD
    startTime = millis();                      // Store system start time
}

void loop() {                                  
    if (!systemReady) {                        // If system not ready yet
        if (millis() - startTime >= START_DELAY) {  // Check if delay passed
            systemReady = true;                // Mark system ready
            showLCDMsg("System Ready", "");    // Show system ready message
        } else {
            int remaining = (START_DELAY - (millis() - startTime)) / 1000;  // Remaining seconds
            showLCDMsg("Initializing...", String(remaining) + " sec", 1000); // Show countdown
            return;                             // Skip rest of loop
        }
        lcd.clear();                             // Clear LCD after ready
    }

    Blynk.run();                                 // Run Blynk tasks
    timer.run();                                 // Run timer tasks

    float w1 = 0;                                // Weight variable for tray 1
    float w2 = 0;                                // Weight variable for tray 2

    if (lc1_ok) w1 = scale1.get_units(3);       // Read weight from tray 1
    if (lc2_ok) w2 = scale2.get_units(3);       // Read weight from tray 2

    String s1 = getStatus(w1);                  // Determine status for tray 1
    String s2 = getStatus(w2);                  // Determine status for tray 2

    Serial.print("T1:");                        // Print tray 1 label
    if (lc1_ok) Serial.print(w1, 1);            // Print weight if ready
    else Serial.print("FAIL");                   // Print FAIL if not ready

    Serial.print(" | T2:");                     // Print tray 2 label
    if (lc2_ok) Serial.print(w2, 1);            // Print weight if ready
    else Serial.print("FAIL");                   // Print FAIL if not ready

    Serial.println();                            // New line

    if (lcd_ok) {                                // If LCD connected
        lcd.setCursor(0, 0);                     // Set cursor to first line
        if (lc1_ok) {                             // If tray 1 ready
            lcd.print("Tray 1:");                // Print tray label
            lcd.print(w1, 0);                    // Print weight
            lcd.print("  ");
            lcd.setCursor(12, 0);                // Move cursor for status
            lcd.print(s1);                        // Print status
            lcd.print("   ");
        } else {
            lcd.print("Tray 1:NO SENSOR   ");     // Print if tray 1 not ready
        }

        lcd.setCursor(0, 1);                     // Set cursor to second line
        if (lc2_ok) {                             // If tray 2 ready
            lcd.print("Tray 2:");                // Print tray label
            lcd.print(w2, 0);                    // Print weight
            lcd.print("  ");
            lcd.setCursor(12, 1);                // Move cursor for status
            lcd.print(s2);                        // Print status
            lcd.print("   ");
        } else {
            lcd.print("Tray 2:NO SENSOR   ");     // Print if tray 2 not ready
        }
    }

    if ((lc1_ok && s1 == "LOW") || (lc2_ok && s2 == "LOW")) {  // If any tray LOW
        digitalWrite(BUZ, HIGH);
        delay(500);                  // Turn on buzzer
    } else {
        digitalWrite(BUZ, LOW);  
           delay(500);              // Turn off buzzer
    }

    if (lc1_ok && s1 == "LOW") {                 // Tray 1 LOW notification
        if (!notified1) {                        // If not already notified
            Blynk.logEvent("tray1_low", "Tray 1 is LOW! Please refill."); // Send event
            notified1 = true;                    // Mark notified
        }
    } else {
        notified1 = false;                        // Reset notification flag
    }

    if (lc2_ok && s2 == "LOW") {                 // Tray 2 LOW notification
        if (!notified2) {                        // If not already notified
            Blynk.logEvent("tray2_low", "Tray 2 is LOW! Please refill."); // Send event
            notified2 = true;                    // Mark notified
        }
    } else {
        notified2 = false;                        // Reset notification flag
    }

    delay(200);                                   // Small delay to stabilize loop
}