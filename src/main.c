#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"

#include "display_driver.h"
#include "wifi_manager.h"
#include "network_scanner.h"
#include "ui_framework.h"
#include "screens.h"
#include "device_db.h"
#include "joystick.h"
#include "touch.h"
#include "config.h"

// Core 1 functions
void core1_entry(void) {
    // Initialize hardware for core 1
    adc_init();
    adc_gpio_init(26); // ADC0
    adc_gpio_init(27); // ADC1
    
    // Joystick initialization
    joystick_init();
    
    // Touch screen initialization
    touch_init();
    
    // Network scanning on core 1
    while (1) {
        if (wifi_is_connected() && scanning_enabled) {
            network_scan_devices();
        }
        sleep_ms(SCAN_INTERVAL_MS);
    }
}

int main() {
    stdio_init_all();
    
    // Initialize watchdog for recovery
    watchdog_enable(10000, 1);
    
    // Initialize Wi-Fi
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        while (1) {
            sleep_ms(1000);
        }
    }
    cyw43_arch_enable_sta_mode();
    
    // Connect to Wi-Fi
    if (!wifi_connect(WIFI_SSID, WIFI_PASSWORD)) {
        printf("Wi-Fi connection failed\n");
        // Still continue to show error on screen
    }
    
    // Initialize display
    display_init();
    display_clear(COLOR_BLACK);
    
    // Initialize UI framework
    ui_init();
    
    // Initialize device database
    device_db_init();
    
    // Launch core 1 for scanning
    multicore_launch_core1(core1_entry);
    
    // Initialize peripherals
    gpio_init(PIN_BUTTON1);
    gpio_set_dir(PIN_BUTTON1, GPIO_IN);
    gpio_pull_up(PIN_BUTTON1);
    
    gpio_init(PIN_BUTTON2);
    gpio_set_dir(PIN_BUTTON2, GPIO_IN);
    gpio_pull_up(PIN_BUTTON2);
    
    gpio_init(PIN_LED_RGB);
    gpio_set_dir(PIN_LED_RGB, GPIO_OUT);
    
    // Show main screen
    show_main_screen();
    
    // Main loop
    while (1) {
        watchdog_update();
        
        // Handle UI events
        ui_process_events();
        
        // Handle button presses
        if (!gpio_get(PIN_BUTTON1)) {
            // Refresh scan
            trigger_scan();
            show_main_screen();
            sleep_ms(200);
        }
        
        if (!gpio_get(PIN_BUTTON2)) {
            // Show settings
            show_settings_screen();
            sleep_ms(200);
        }
        
        // Update UI
        ui_update();
        
        // Sleep to save power
        sleep_ms(10);
    }
}