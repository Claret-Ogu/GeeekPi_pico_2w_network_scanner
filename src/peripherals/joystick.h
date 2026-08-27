#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include <stdbool.h>

// Joystick pins
#define JOYSTICK_X_PIN 26 // ADC0
#define JOYSTICK_Y_PIN 27 // ADC1

// Joystick thresholds
#define JOYSTICK_DEADZONE 100
#define JOYSTICK_MAX 4095

// Joystick event
typedef struct {
    int16_t x;  // -1000 to 1000
    int16_t y;  // -1000 to 1000
    bool pressed;
} joystick_event_t;

// Function prototypes
void joystick_init(void);
bool joystick_get_event(joystick_event_t* event);
int16_t joystick_get_x(void);
int16_t joystick_get_y(void);
bool joystick_is_pressed(void);

#endif