#include "joystick.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"

static int16_t last_x = 0;
static int16_t last_y = 0;
static bool last_pressed = false;

void joystick_init(void) {
    // ADC already initialized in main
    // Configure GPIO pins for ADC
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

bool joystick_get_event(joystick_event_t* event) {
    if (!event) return false;
    
    int16_t x = joystick_get_x();
    int16_t y = joystick_get_y();
    bool pressed = joystick_is_pressed();
    
    // Check if state changed
    bool changed = (x != last_x) || (y != last_y) || (pressed != last_pressed);
    
    if (changed) {
        event->x = x;
        event->y = y;
        event->pressed = pressed;
        
        last_x = x;
        last_y = y;
        last_pressed = pressed;
        
        return true;
    }
    
    return false;
}

int16_t joystick_get_x(void) {
    adc_select_input(0);
    uint16_t raw = adc_read();
    
    // Convert to -1000 to 1000 range
    int16_t value = ((int32_t)raw * 2000 / 4095) - 1000;
    
    // Apply deadzone
    if (abs(value) < JOYSTICK_DEADZONE) {
        value = 0;
    }
    
    return value;
}

int16_t joystick_get_y(void) {
    adc_select_input(1);
    uint16_t raw = adc_read();
    
    // Convert to -1000 to 1000 range
    int16_t value = ((int32_t)raw * 2000 / 4095) - 1000;
    
    // Apply deadzone
    if (abs(value) < JOYSTICK_DEADZONE) {
        value = 0;
    }
    
    return value;
}

bool joystick_is_pressed(void) {
    // Joystick press is not directly supported on this module
    // Use button 1 or 2 instead
    return false;
}