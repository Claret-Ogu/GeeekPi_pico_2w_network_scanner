#ifndef UI_FRAMEWORK_H
#define UI_FRAMEWORK_H

#include <stdint.h>
#include <stdbool.h>
#include "display_driver.h"

// UI Events
typedef enum {
    UI_EVENT_NONE,
    UI_EVENT_TOUCH,
    UI_EVENT_BUTTON,
    UI_EVENT_JOYSTICK,
    UI_EVENT_TIMER,
    UI_EVENT_SCAN_COMPLETE
} ui_event_type_t;

// UI Event structure
typedef struct {
    ui_event_type_t type;
    uint16_t x;
    uint16_t y;
    uint8_t button;
    int8_t joy_x;
    int8_t joy_y;
    void* data;
} ui_event_t;

// Screen structure
typedef struct {
    void (*init)(void);
    void (*update)(void);
    void (*handle_event)(ui_event_t* event);
    void (*render)(void);
    const char* name;
} screen_t;

// Function prototypes
void ui_init(void);
void ui_process_events(void);
void ui_update(void);
void ui_switch_screen(const screen_t* screen);
void ui_push_screen(const screen_t* screen);
void ui_pop_screen(void);
void ui_send_event(ui_event_t* event);

// Globals
extern const screen_t main_screen;
extern const screen_t settings_screen;
extern const screen_t detail_screen;

#endif