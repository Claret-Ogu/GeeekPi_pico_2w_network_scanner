#include "screens.h"
#include "display_driver.h"
#include "network_scanner.h"
#include "wifi_manager.h"
#include "device_db.h"
#include "joystick.h"
#include "touch.h"
#include <string.h>
#include <stdio.h>

// Global state for main screen
static uint32_t selected_device = 0;
static uint32_t scroll_offset = 0;
static uint32_t visible_devices = 0;
static bool scan_in_progress = false;

// Device detail state
static uint32_t detail_device_index = 0;
static network_device_t detail_device;

// Settings state
static char settings_ssid[32] = WIFI_SSID;
static char settings_password[32] = WIFI_PASSWORD;
static uint32_t scan_interval = 60000;
static bool auto_scan = true;
static uint8_t settings_selection = 0;

// Helper function to get device icon
static const char* get_device_icon(uint8_t type) {
    switch (type) {
        case 1: return "💻";
        case 2: return "📱";
        case 3: return "📱";
        case 4: return "⚡";
        case 5: return "🔌";
        default: return "❓";
    }
}

// Helper function to render a device entry
static void render_device_entry(uint16_t x, uint16_t y, uint32_t index, bool selected) {
    network_device_t device;
    if (!network_scan_get_device(index, &device)) return;
    
    // Background
    uint16_t bg = selected ? COLOR_BLUE : COLOR_BLACK;
    display_fill_rect(x, y, 300, 50, bg);
    
    // Border
    display_draw_rect(x, y, 300, 50, COLOR_WHITE);
    
    // Icon
    display_draw_string(x + 5, y + 5, get_device_icon(device.device_type), COLOR_WHITE, bg);
    
    // Device name or hostname
    char name[32];
    if (strlen(device.hostname) > 0) {
        strncpy(name, device.hostname, 31);
    } else {
        snprintf(name, 31, "%d.%d.%d.%d",
                 ip4_addr1(&device.ip), ip4_addr2(&device.ip),
                 ip4_addr3(&device.ip), ip4_addr4(&device.ip));
    }
    display_draw_string(x + 30, y + 5, name, COLOR_WHITE, bg);
    
    // Vendor
    display_draw_string(x + 30, y + 20, device.vendor, COLOR_CYAN, bg);
    
    // IP and status
    char info[48];
    snprintf(info, 47, "%d.%d.%d.%d  %s",
             ip4_addr1(&device.ip), ip4_addr2(&device.ip),
             ip4_addr3(&device.ip), ip4_addr4(&device.ip),
             device.online ? "●" : "○");
    display_draw_string(x + 150, y + 5, info, 
                       device.online ? COLOR_GREEN : COLOR_RED, bg);
    
    // Latency
    if (device.online) {
        char latency[16];
        snprintf(latency, 15, "%dms", device.latency_ms);
        display_draw_string(x + 150, y + 20, latency, COLOR_YELLOW, bg);
    }
}

// Main Screen Implementation
static void main_screen_init_cb(void) {
    selected_device = 0;
    scroll_offset = 0;
    visible_devices = network_scan_get_device_count();
    scan_in_progress = false;
}

static void main_screen_render_cb(void) {
    // Clear screen
    display_clear(COLOR_BLACK);
    
    // Header
    display_fill_rect(0, 0, 320, 40, COLOR_DARK);
    display_draw_string(5, 5, "⚡ NetScan Pro", COLOR_WHITE, COLOR_DARK);
    
    // Network info
    if (wifi_is_connected()) {
        char net_info[32];
        snprintf(net_info, 31, "📶 %s", wifi_get_ip());
        display_draw_string(5, 20, net_info, COLOR_CYAN, COLOR_DARK);
    } else {
        display_draw_string(5, 20, "❌ Not Connected", COLOR_RED, COLOR_DARK);
    }
    
    // Signal strength bar
    int signal = wifi_get_signal_strength();
    int bars = 0;
    if (signal > -50) bars = 4;
    else if (signal > -60) bars = 3;
    else if (signal > -70) bars = 2;
    else if (signal > -80) bars = 1;
    
    char signal_str[16];
    snprintf(signal_str, 15, "▮%d", bars);
    display_draw_string(250, 5, signal_str, COLOR_GREEN, COLOR_DARK);
    
    // Stats
    uint32_t total = network_scan_get_device_count();
    uint32_t online = 0;
    for (uint32_t i = 0; i < total; i++) {
        network_device_t dev;
        if (network_scan_get_device(i, &dev) && dev.online) {
            online++;
        }
    }
    
    char stats[48];
    snprintf(stats, 47, "📡 %d Active | 🆕 %d | ⚠️ 0", total, total - online);
    display_fill_rect(0, 40, 320, 20, COLOR_BLACK);
    display_draw_string(5, 42, stats, COLOR_WHITE, COLOR_BLACK);
    
    // Device list
    uint32_t max_display = 8; // 8 devices fit on screen
    uint32_t start_idx = scroll_offset;
    uint32_t end_idx = start_idx + max_display;
    if (end_idx > total) end_idx = total;
    
    for (uint32_t i = start_idx; i < end_idx; i++) {
        uint16_t y_pos = 60 + (i - start_idx) * 52;
        render_device_entry(10, y_pos, i, i == selected_device);
    }
    
    // Footer
    display_fill_rect(0, 440, 320, 40, COLOR_DARK);
    display_draw_string(5, 450, "🔄 Refresh  ⚙️ Settings  🔍 Scan", COLOR_WHITE, COLOR_DARK);
    display_draw_string(5, 460, "▲▼ Select  ▶ View Details", COLOR_CYAN, COLOR_DARK);
    
    // Update display
    display_update();
}

static void main_screen_update_cb(void) {
    // Check if scan is in progress
    // Update would handle animation if needed
}

static void main_screen_handle_event_cb(ui_event_t* event) {
    if (!event) return;
    
    switch (event->type) {
        case UI_EVENT_JOYSTICK:
            // Handle joystick navigation
            if (event->joy_y < -50) {
                // Up - scroll up
                if (selected_device > 0) {
                    selected_device--;
                    if (selected_device < scroll_offset) {
                        scroll_offset = selected_device;
                    }
                    main_screen_render_cb();
                }
            } else if (event->joy_y > 50) {
                // Down - scroll down
                uint32_t total = network_scan_get_device_count();
                if (selected_device < total - 1) {
                    selected_device++;
                    if (selected_device >= scroll_offset + 8) {
                        scroll_offset++;
                    }
                    main_screen_render_cb();
                }
            } else if (event->joy_x > 50) {
                // Right - view details
                if (selected_device < network_scan_get_device_count()) {
                    show_device_detail_screen(selected_device);
                }
            }
            break;
            
        case UI_EVENT_TOUCH:
            // Handle touch selection
            // Check if touch is on a device entry
            if (event->y >= 60 && event->y < 440) {
                uint32_t idx = (event->y - 60) / 52;
                if (idx < 8) {
                    uint32_t device_idx = scroll_offset + idx;
                    if (device_idx < network_scan_get_device_count()) {
                        selected_device = device_idx;
                        show_device_detail_screen(device_idx);
                    }
                }
            }
            break;
            
        case UI_EVENT_SCAN_COMPLETE:
            scan_in_progress = false;
            main_screen_render_cb();
            break;
            
        default:
            break;
    }
}

// Settings Screen Implementation
static void settings_screen_init_cb(void) {
    settings_selection = 0;
}

static void settings_screen_render_cb(void) {
    display_clear(COLOR_BLACK);
    
    // Header
    display_fill_rect(0, 0, 320, 40, COLOR_DARK);
    display_draw_string(5, 5, "⚙️ Settings", COLOR_WHITE, COLOR_DARK);
    display_draw_string(5, 20, "← Back", COLOR_CYAN, COLOR_DARK);
    
    // Settings options
    const char* options[] = {
        "Wi-Fi SSID",
        "Wi-Fi Password",
        "Scan Interval",
        "Auto Scan",
        "Save Settings",
        "Reset Device DB",
        "Factory Reset"
    };
    
    for (int i = 0; i < 7; i++) {
        uint16_t y_pos = 50 + i * 40;
        uint16_t color = (i == settings_selection) ? COLOR_BLUE : COLOR_BLACK;
        display_fill_rect(10, y_pos, 300, 30, color);
        
        char buffer[64];
        switch (i) {
            case 0:
                snprintf(buffer, 63, "SSID: %s", settings_ssid);
                break;
            case 1:
                snprintf(buffer, 63, "Password: %s", settings_password);
                break;
            case 2:
                snprintf(buffer, 63, "Interval: %d ms", scan_interval);
                break;
            case 3:
                snprintf(buffer, 63, "Auto Scan: %s", auto_scan ? "ON" : "OFF");
                break;
            default:
                strncpy(buffer, options[i], 63);
        }
        display_draw_string(15, y_pos + 5, buffer, COLOR_WHITE, color);
    }
    
    display_update();
}

static void settings_screen_handle_event_cb(ui_event_t* event) {
    if (!event) return;
    
    switch (event->type) {
        case UI_EVENT_JOYSTICK:
            if (event->joy_y < -50 && settings_selection > 0) {
                settings_selection--;
                settings_screen_render_cb();
            } else if (event->joy_y > 50 && settings_selection < 6) {
                settings_selection++;
                settings_screen_render_cb();
            } else if (event->joy_x < -50) {
                // Back to main
                ui_pop_screen();
                show_main_screen();
            }
            break;
            
        case UI_EVENT_TOUCH:
            // Handle touch on settings items
            if (event->y >= 50 && event->y < 330) {
                int idx = (event->y - 50) / 40;
                if (idx < 7) {
                    settings_selection = idx;
                    settings_screen_render_cb();
                    
                    // Handle selection
                    switch (idx) {
                        case 4: // Save
                            // Save settings to flash
                            settings_screen_render_cb();
                            break;
                        case 6: // Factory reset
                            // Reset everything
                            break;
                    }
                }
            }
            break;
            
        default:
            break;
    }
}

// Device Detail Screen Implementation
static void detail_screen_init_cb(void) {
    memset(&detail_device, 0, sizeof(detail_device));
    network_scan_get_device(detail_device_index, &detail_device);
}

static void detail_screen_render_cb(void) {
    display_clear(COLOR_BLACK);
    
    // Header
    display_fill_rect(0, 0, 320, 40, COLOR_DARK);
    display_draw_string(5, 5, "← Back", COLOR_CYAN, COLOR_DARK);
    display_draw_string(250, 5, "📱", COLOR_WHITE, COLOR_DARK);
    
    // Device info
    char info[64];
    
    // Name
    snprintf(info, 63, "%s %s", get_device_icon(detail_device.device_type), 
             detail_device.hostname);
    display_draw_string(10, 50, info, COLOR_WHITE, COLOR_BLACK);
    
    // Details
    display_draw_string(10, 80, "IP Address:", COLOR_CYAN, COLOR_BLACK);
    snprintf(info, 63, "  %d.%d.%d.%d",
             ip4_addr1(&detail_device.ip), ip4_addr2(&detail_device.ip),
             ip4_addr3(&detail_device.ip), ip4_addr4(&detail_device.ip));
    display_draw_string(10, 95, info, COLOR_WHITE, COLOR_BLACK);
    
    display_draw_string(10, 115, "MAC Address:", COLOR_CYAN, COLOR_BLACK);
    snprintf(info, 63, "  %02X:%02X:%02X:%02X:%02X:%02X",
             detail_device.mac[0], detail_device.mac[1],
             detail_device.mac[2], detail_device.mac[3],
             detail_device.mac[4], detail_device.mac[5]);
    display_draw_string(10, 130, info, COLOR_WHITE, COLOR_BLACK);
    
    display_draw_string(10, 150, "Vendor:", COLOR_CYAN, COLOR_BLACK);
    snprintf(info, 63, "  %s", detail_device.vendor);
    display_draw_string(10, 165, info, COLOR_WHITE, COLOR_BLACK);
    
    display_draw_string(10, 185, "Status:", COLOR_CYAN, COLOR_BLACK);
    const char* status = detail_device.online ? "● Online" : "○ Offline";
    uint16_t color = detail_device.online ? COLOR_GREEN : COLOR_RED;
    display_draw_string(10, 200, status, color, COLOR_BLACK);
    
    if (detail_device.online) {
        snprintf(info, 63, "  Latency: %d ms", detail_device.latency_ms);
        display_draw_string(10, 215, info, COLOR_YELLOW, COLOR_BLACK);
    }
    
    display_draw_string(10, 235, "First Seen:", COLOR_CYAN, COLOR_BLACK);
    // Format time - simplified
    snprintf(info, 63, "  %lu", detail_device.first_seen);
    display_draw_string(10, 250, info, COLOR_WHITE, COLOR_BLACK);
    
    display_draw_string(10, 270, "Last Seen:", COLOR_CYAN, COLOR_BLACK);
    snprintf(info, 63, "  %lu", detail_device.last_seen);
    display_draw_string(10, 285, info, COLOR_WHITE, COLOR_BLACK);
    
    // Actions
    display_fill_rect(10, 400, 100, 40, COLOR_DARK);
    display_draw_rect(10, 400, 100, 40, COLOR_WHITE);
    display_draw_string(20, 412, "📝 Rename", COLOR_WHITE, COLOR_DARK);
    
    display_fill_rect(120, 400, 100, 40, COLOR_DARK);
    display_draw_rect(120, 400, 100, 40, COLOR_WHITE);
    display_draw_string(130, 412, "⭐ Fav", COLOR_WHITE, COLOR_DARK);
    
    display_fill_rect(230, 400, 80, 40, COLOR_DARK);
    display_draw_rect(230, 400, 80, 40, COLOR_WHITE);
    display_draw_string(240, 412, "🔒", COLOR_WHITE, COLOR_DARK);
    
    display_update();
}

static void detail_screen_handle_event_cb(ui_event_t* event) {
    if (!event) return;
    
    switch (event->type) {
        case UI_EVENT_JOYSTICK:
            if (event->joy_x < -50) {
                // Back to main
                ui_pop_screen();
                show_main_screen();
            }
            break;
            
        case UI_EVENT_TOUCH:
            if (event->y < 40 && event->x < 100) {
                // Back button
                ui_pop_screen();
                show_main_screen();
            }
            break;
            
        default:
            break;
    }
}

// Screen definitions
const screen_t main_screen = {
    .init = main_screen_init_cb,
    .update = main_screen_update_cb,
    .handle_event = main_screen_handle_event_cb,
    .render = main_screen_render_cb,
    .name = "Main"
};

const screen_t settings_screen = {
    .init = settings_screen_init_cb,
    .update = NULL,
    .handle_event = settings_screen_handle_event_cb,
    .render = settings_screen_render_cb,
    .name = "Settings"
};

const screen_t detail_screen = {
    .init = detail_screen_init_cb,
    .update = NULL,
    .handle_event = detail_screen_handle_event_cb,
    .render = detail_screen_render_cb,
    .name = "Detail"
};

// Public functions
void show_main_screen(void) {
    ui_switch_screen(&main_screen);
}

void show_settings_screen(void) {
    ui_push_screen(&settings_screen);
}

void show_device_detail_screen(uint32_t device_index) {
    detail_device_index = device_index;
    ui_push_screen(&detail_screen);
}

void show_scanning_screen(void) {
    // Simple scanning indicator
    display_clear(COLOR_BLACK);
    display_draw_string(100, 220, "🔍 Scanning...", COLOR_WHITE, COLOR_BLACK);
    display_update();
}