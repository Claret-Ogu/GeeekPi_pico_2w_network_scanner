#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include "pico/cyw43_arch.h"

// Wi-Fi configuration
#define WIFI_SSID     "YourNetworkName"
#define WIFI_PASSWORD "YourNetworkPassword"

// Function prototypes
bool wifi_connect(const char* ssid, const char* password);
void wifi_disconnect(void);
bool wifi_is_connected(void);
const char* wifi_get_ip(void);
int wifi_get_signal_strength(void);
void wifi_set_scan_enabled(bool enabled);
bool wifi_scan_enabled(void);

#endif