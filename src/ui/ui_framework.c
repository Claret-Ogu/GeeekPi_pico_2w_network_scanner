#include "ui_framework.h"
#include "joystick.h"
#include "touch.h"
#include <string.h>

// Screen stack
#define MAX_SCREEN_STACK 8
static const screen_t* screen_stack[MAX_SCREEN_STACK];
static int screen_stack_ptr = 0;
static const screen_t* current_screen = NULL;

// Event queue
#define MAX_EVENTS 16
static ui_event_t event_queue[MAX_EVENTS];
static int event_queue_head = 0;
static int event_queue_tail = 0;

void ui_init(void) {
    // Initialize event queue
    memset(event_queue, 0, sizeof(event_queue));
    
    // Initialize peripherals
    touch_init();
    joystick_init();
}

void ui_process_events(void) {
    // Process touch events
    touch_event_t touch_event;
    if (touch_get_event(&touch_event)) {
        ui_event_t ui_event;
        ui_event.type = UI_EVENT_TOUCH;
        ui_event.x = touch_event.x;
        ui_event.y = touch_event.y;
        ui_send_event(&ui_event);
    }
    
    // Process joystick events
    joystick_event_t joy_event;
    if (joystick_get_event(&joy_event)) {
        ui_event_t ui_event;
        ui_event.type = UI_EVENT_JOYSTICK;
        ui_event.joy_x = joy_event.x;
        ui_event.joy_y = joy_event.y;
        ui_send_event(&ui_event);
    }
    
    // Process queued events
    while (event_queue_head != event_queue_tail) {
        ui_event_t event = event_queue[event_queue_head];
        event_queue_head = (event_queue_head + 1) % MAX_EVENTS;
        
        if (current_screen && current_screen->handle_event) {
            current_screen->handle_event(&event);
        }
    }
}

void ui_update(void) {
    if (current_screen && current_screen->update) {
        current_screen->update();
    }
}

void ui_switch_screen(const screen_t* screen) {
    if (!screen) return;
    
    current_screen = screen;
    screen_stack[0] = screen;
    screen_stack_ptr = 0;
    
    if (current_screen->init) {
        current_screen->init();
    }
    
    if (current_screen->render) {
        current_screen->render();
    }
}

void ui_push_screen(const screen_t* screen) {
    if (!screen || screen_stack_ptr >= MAX_SCREEN_STACK - 1) return;
    
    screen_stack[++screen_stack_ptr] = screen;
    current_screen = screen;
    
    if (current_screen->init) {
        current_screen->init();
    }
    
    if (current_screen->render) {
        current_screen->render();
    }
}

void ui_pop_screen(void) {
    if (screen_stack_ptr <= 0) return;
    
    screen_stack_ptr--;
    current_screen = screen_stack[screen_stack_ptr];
    
    if (current_screen->render) {
        current_screen->render();
    }
}

void ui_send_event(ui_event_t* event) {
    if (!event) return;
    
    int next_tail = (event_queue_tail + 1) % MAX_EVENTS;
    if (next_tail != event_queue_head) {
        event_queue[event_queue_tail] = *event;
        event_queue_tail = next_tail;
    }
}