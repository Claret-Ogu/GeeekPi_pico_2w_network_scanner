#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>
#include <stdbool.h>

// Touch pins
#define TOUCH_SDA_PIN 8
#define TOUCH_SCL_PIN 9
#define TOUCH_RST_PIN 10
#define TOUCH_INT_PIN 11

// Touch screen dimensions
#define TOUCH_WIDTH 320
#define TOUCH_HEIGHT 480

// Touch event
typedef struct {
    uint16_t x;
    uint16_t y;
    bool touched;
} touch_event_t;

// Function prototypes
void touch_init(void);
bool touch_get_event(touch_event_t* event);
bool touch_is_touched(void);
void touch_get_position(uint16_t* x, uint16_t* y);

#endif