#include "wifi_manager.h"
#include <string.h>
#include <stdio.h>

static bool connected = false;
static char ip_address[16] = {0};
static bool scan_enabled = true;
static int signal_strength = 0;

bool wifi_connect(const char* ssid, const char* password) {
    if (!ssid || !password) return false;
    
    printf("Connecting to Wi-Fi: %s\n", ssid);
    
    cyw43_arch_enable_sta_mode();
    
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Failed to connect to Wi-Fi\n");
        connected = false;
        return false;
    }
    
    connected = true;
    
    // Get IP address
    struct netif* netif = cyw43_arch_netif();
    ip4_addr_t ip = netif->ip_addr;
    sprintf(ip_address, "%d.%d.%d.%d", 
            ip4_addr1(&ip), ip4_addr2(&ip), 
            ip4_addr3(&ip), ip4_addr4(&ip));
    
    printf("Connected! IP: %s\n", ip_address);
    return true;
}

void wifi_disconnect(void) {
    cyw43_arch_wifi_leave();
    connected = false;
    memset(ip_address, 0, sizeof(ip_address));
}

bool wifi_is_connected(void) {
    if (!connected) return false;
    
    // Check actual connection state
    int state = cyw43_tcpip_link_status(cyw43_arch_netif());
    connected = (state == CYW43_LINK_UP);
    return connected;
}

const char* wifi_get_ip(void) {
    if (!connected) return "0.0.0.0";
    return ip_address;
}

int wifi_get_signal_strength(void) {
    if (!connected) return 0;
    
    // Get signal strength from cyw43
    int32_t rssi;
    if (cyw43_wifi_get_rssi(&cyw43_state, &rssi) == 0) {
        signal_strength = (int)rssi;
    }
    return signal_strength;
}

void wifi_set_scan_enabled(bool enabled) {
    scan_enabled = enabled;
}

bool wifi_scan_enabled(void) {
    return scan_enabled;
}