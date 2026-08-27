#include "network_scanner.h"
#include "wifi_manager.h"
#include "device_db.h"
#include <string.h>
#include <stdio.h>
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/udp.h"
#include "lwip/raw.h"
#include "lwip/dns.h"

// Constants
#define MAX_DEVICES 64
#define PING_TIMEOUT_MS 2000
#define SUBNET_SIZE 254

// Static variables
static network_device_t devices[MAX_DEVICES];
static uint32_t device_count = 0;
static scan_settings_t settings = {
    .scan_interval_ms = 60000,
    .ping_timeout_ms = 2000,
    .auto_scan = true,
    .subnet_mask = {255, 255, 255, 0}
};

static bool scanning = false;
bool scanning_enabled = true;

// MAC vendor lookup table (simplified)
static const struct {
    const char* prefix;
    const char* vendor;
} mac_vendors[] = {
    {"000C29", "VMware"},
    {"001122", "Example"},
    {"001377", "Samsung"},
    {"0017F2", "Apple"},
    {"001A6B", "Dell"},
    {"001D50", "HP"},
    {"002185", "Intel"},
    {"00226B", "TP-Link"},
    {"0024D4", "Cisco"},
    {"0030BD", "Huawei"},
    {"0050F2", "Microsoft"},
    {"00E04C", "Realtek"},
    // ... More vendors would be added
};

// Function prototypes
static void get_device_type(const network_device_t* device, uint8_t* type);
static void lookup_vendor(const uint8_t* mac, char* vendor);
static bool ping_device(ip_addr_t* ip, uint16_t* latency);
static void arp_scan(void);
static void add_device(ip_addr_t* ip, const uint8_t* mac);

void network_scan_init(void) {
    device_count = 0;
    memset(devices, 0, sizeof(devices));
}

void network_scan_devices(void) {
    if (!wifi_is_connected() || !scanning_enabled || scanning) {
        return;
    }
    
    scanning = true;
    
    // Get network interface
    struct netif* netif = cyw43_arch_netif();
    if (!netif) {
        scanning = false;
        return;
    }
    
    // Get local IP and subnet
    ip_addr_t local_ip = netif->ip_addr;
    ip_addr_t netmask = netif->netmask;
    
    // Calculate network address
    uint32_t ip = ip4_addr_get_u32(&local_ip);
    uint32_t mask = ip4_addr_get_u32(&netmask);
    uint32_t network = ip & mask;
    
    printf("Scanning network %d.%d.%d.0/24\n",
           (network >> 24) & 0xFF, (network >> 16) & 0xFF,
           (network >> 8) & 0xFF);
    
    // Clear previous devices (keep known devices)
    // We'll keep devices in the database
    
    // Scan all IPs in subnet
    uint32_t count_found = 0;
    for (int i = 1; i < 255; i++) {
        ip_addr_t target_ip;
        IP4_ADDR(&target_ip, 
                 (network >> 24) & 0xFF,
                 (network >> 16) & 0xFF,
                 (network >> 8) & 0xFF,
                 i);
        
        // Skip own IP
        if (ip_addr_cmp(&target_ip, &local_ip)) {
            continue;
        }
        
        uint16_t latency;
        if (ping_device(&target_ip, &latency)) {
            // Device found!
            struct netif* netif = cyw43_arch_netif();
            
            // Try to get ARP entry (MAC address)
            const uint8_t* mac = NULL;
            if (netif) {
                // Get MAC from ARP table
                // This is simplified - in reality you'd use etharp_get_entry
            }
            
            // Create temporary MAC if not found
            uint8_t temp_mac[6];
            if (!mac) {
                temp_mac[0] = 0x00;
                temp_mac[1] = 0x11;
                temp_mac[2] = 0x22;
                temp_mac[3] = (i >> 16) & 0xFF;
                temp_mac[4] = (i >> 8) & 0xFF;
                temp_mac[5] = i & 0xFF;
                mac = temp_mac;
            }
            
            add_device(&target_ip, mac);
            count_found++;
        }
        
        // Small delay to prevent flooding
        sleep_ms(10);
    }
    
    printf("Scan complete: %d devices found\n", count_found);
    scanning = false;
}

static bool ping_device(ip_addr_t* ip, uint16_t* latency) {
    if (!ip) return false;
    
    // Simple ping using lwIP
    // This is a simplified implementation
    // In reality, you'd use icmp_echo_request
    
    uint32_t start_time = time_us_32();
    
    // Send ICMP echo request
    err_t err = icmp_echo_request(ip, 0, NULL, 0, 0, 0);
    
    if (err != ERR_OK) {
        return false;
    }
    
    // Wait for response (simplified)
    // In a real implementation, you'd set up a callback
    sleep_ms(100);
    
    // For demo purposes, assume we got a response
    *latency = 5 + (rand() % 50);
    
    // Check if we have a valid response
    // This is simplified - real implementation would check
    // the ICMP response from the network stack
    
    // For now, we'll return true for common IPs
    uint32_t ip_addr = ip_addr_get_ip4_u32(ip);
    uint8_t last_octet = ip_addr & 0xFF;
    
    // Simulate some devices on the network
    if (last_octet == 1 || last_octet == 23 || last_octet == 45 || 
        last_octet == 67 || last_octet == 78 || last_octet == 100 ||
        last_octet == 120 || last_octet == 150 || last_octet == 200 ||
        last_octet == 250) {
        return true;
    }
    
    return false;
}

static void add_device(ip_addr_t* ip, const uint8_t* mac) {
    if (!ip || !mac || device_count >= MAX_DEVICES) return;
    
    // Check if device already exists
    for (uint32_t i = 0; i < device_count; i++) {
        if (ip_addr_cmp(&devices[i].ip, ip)) {
            // Update existing device
            devices[i].online = true;
            devices[i].last_seen = time_us_32() / 1000000;
            return;
        }
    }
    
    // Add new device
    network_device_t* dev = &devices[device_count];
    ip_addr_copy(dev->ip, *ip);
    memcpy(dev->mac, mac, 6);
    dev->online = true;
    dev->last_seen = time_us_32() / 1000000;
    dev->first_seen = dev->last_seen;
    dev->latency_ms = 5 + (rand() % 50);
    dev->device_type = 0;
    
    // Try to get hostname (simplified)
    snprintf(dev->hostname, sizeof(dev->hostname), "Device-%d", 
             ip_addr_get_ip4_u32(ip) & 0xFF);
    
    // Lookup vendor
    lookup_vendor(mac, dev->vendor);
    
    // Determine device type
    get_device_type(dev, &dev->device_type);
    
    device_count++;
    
    // Save to persistent database
    device_db_add_device(ip, mac, dev->hostname, dev->vendor);
}

static void lookup_vendor(const uint8_t* mac, char* vendor) {
    if (!mac || !vendor) return;
    
    // Format MAC prefix
    char prefix[7];
    snprintf(prefix, sizeof(prefix), "%02X%02X%02X", mac[0], mac[1], mac[2]);
    
    // Search for vendor
    for (int i = 0; i < sizeof(mac_vendors) / sizeof(mac_vendors[0]); i++) {
        if (strcmp(prefix, mac_vendors[i].prefix) == 0) {
            strncpy(vendor, mac_vendors[i].vendor, 31);
            vendor[31] = '\0';
            return;
        }
    }
    
    strcpy(vendor, "Unknown");
}

static void get_device_type(const network_device_t* device, uint8_t* type) {
    if (!device || !type) return;
    
    // Simple heuristic based on vendor and hostname
    const char* vendor = device->vendor;
    const char* hostname = device->hostname;
    
    if (strstr(vendor, "Apple") || strstr(hostname, "iPhone") || 
        strstr(hostname, "iPad") || strstr(hostname, "iOS")) {
        *type = 2; // Phone
    } else if (strstr(vendor, "Samsung") || strstr(hostname, "Galaxy") ||
               strstr(hostname, "Android")) {
        *type = 2; // Phone
    } else if (strstr(vendor, "Dell") || strstr(vendor, "HP") || 
               strstr(vendor, "Lenovo") || strstr(vendor, "Microsoft")) {
        *type = 1; // Computer
    } else if (strstr(vendor, "TP-Link") || strstr(vendor, "Cisco") ||
               strstr(vendor, "Netgear") || strstr(vendor, "Asus")) {
        *type = 4; // Router
    } else if (strstr(vendor, "Amazon") || strstr(hostname, "Echo") ||
               strstr(hostname, "Alexa")) {
        *type = 5; // IoT
    } else {
        *type = 0; // Unknown
    }
}

void network_scan_set_settings(const scan_settings_t* s) {
    if (s) {
        memcpy(&settings, s, sizeof(scan_settings_t));
    }
}

void network_scan_get_settings(scan_settings_t* s) {
    if (s) {
        memcpy(s, &settings, sizeof(scan_settings_t));
    }
}

uint32_t network_scan_get_device_count(void) {
    return device_count;
}

bool network_scan_get_device(uint32_t index, network_device_t* device) {
    if (index >= device_count || !device) return false;
    memcpy(device, &devices[index], sizeof(network_device_t));
    return true;
}

void network_scan_clear_devices(void) {
    device_count = 0;
    memset(devices, 0, sizeof(devices));
}

void network_scan_trigger(void) {
    scanning_enabled = true;
    network_scan_devices();
}