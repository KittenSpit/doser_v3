#pragma once
#include <Arduino.h>

// WiFi
//constexpr const char* WIFI_SSID = "PHD1 2.4";
//constexpr const char* WIFI_PASS = "Andrew1Laura2";

//const char* WIFI_SSID = "PHD1 2.4";
//const char* WIFI_PASS = "Andrew1Laura2";

constexpr const char* WIFI_SSID = "PHD1 2.4";
constexpr const char* WIFI_PASS = "Andrew1Laura2";

// OTA auth
constexpr const char* OTA_USER = "admin";
constexpr const char* OTA_PASS = "esp32pass";

// Web UI auth (separate from OTA)
constexpr const char* WEB_USER = "webadmin";
constexpr const char* WEB_PASS = "webpass";

// Pumps
constexpr int NUM_PUMPS = 3;
constexpr int PUMP_PWM_PINS[NUM_PUMPS] = { 25, 26, 27 }; // change to your wiring
constexpr int PUMP_DIR_PINS[NUM_PUMPS] = { 33, 32, 14 }; // change to your wiring

// PWM (LEDC)
constexpr int PWM_FREQ = 2000;
constexpr int PWM_RES_BITS = 8;
constexpr int PWM_MAX = (1<<PWM_RES_BITS)-1;

// OLED
constexpr int OLED_SDA = 21;
constexpr int OLED_SCL = 22;
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;

// Schedules
constexpr int MAX_SCHEDULES = 12;

// Logging
constexpr int LOG_CAPACITY = 50;

// Preferences namespace
constexpr const char* PREF_NS = "doser";
