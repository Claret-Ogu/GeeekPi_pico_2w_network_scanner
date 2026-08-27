#ifndef CONFIG_H
#define CONFIG_H

// Wi-Fi Configuration
#define WIFI_SSID     "YourNetworkName"
#define WIFI_PASSWORD "YourNetworkPassword"

// Display Configuration
#define DISPLAY_ROTATION 0
#define DISPLAY_BRIGHTNESS 255

// Scan Configuration
#define SCAN_INTERVAL_MS 60000
#define SCAN_TIMEOUT_MS 2000
#define MAX_SCAN_DEVICES 64

// UI Configuration
#define UI_THEME_DARK 1
#define UI_ANIMATIONS_ENABLED 1

// Pin Definitions
#define PIN_BUTTON1 15
#define PIN_BUTTON2 14
#define PIN_LED_RGB 12
#define PIN_BUZZER 13
#define PIN_LED_D1 16
#define PIN_LED_D2 17

#endif