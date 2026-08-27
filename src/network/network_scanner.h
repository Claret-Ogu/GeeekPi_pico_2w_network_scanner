#ifndef NETWORK_SCANNER_H
#define NETWORK_SCANNER_H

#include <stdint.h>
#include <stdbool.h>
#include "lwip/ip_addr.h"

// Device information structure
typedef struct {
    ip_addr_t ip;
    uint8_t mac[6];
    char hostname[32];
    char vendor[32];
    bool online;
    uint32_t last_seen;
    uint32_t first_seen;
    uint16_t latency_ms;
    uint8_t device_type; // 0=unknown, 1=computer, 2=phone, 3=tablet, 4=router, 5=IoT
} network_device_t;

// Scan settings
typedef struct {
    uint32_t scan_interval_ms;
    uint16_t ping_timeout_ms;
    bool auto_scan;
    uint8_t subnet_mask[4];
} scan_settings_t;

// Function prototypes
void network_scan_init(void);
void network_scan_devices(void);
void network_scan_set_settings(const scan_settings_t* settings);
void network_scan_get_settings(scan_settings_t* settings);
uint32_t network_scan_get_device_count(void);
bool network_scan_get_device(uint32_t index, network_device_t* device);
void network_scan_clear_devices(void);
void network_scan_trigger(void);

// Global variables
extern bool scanning_enabled;

#endif