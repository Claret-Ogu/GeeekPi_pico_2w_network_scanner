#ifndef SCREENS_H
#define SCREENS_H

#include "ui_framework.h"

// Function prototypes
void show_main_screen(void);
void show_settings_screen(void);
void show_device_detail_screen(uint32_t device_index);
void show_scanning_screen(void);

// Screen declarations
extern const screen_t main_screen;
extern const screen_t settings_screen;
extern const screen_t detail_screen;
extern const screen_t scanning_screen;

// Global functions for screens
void main_screen_init(void);
void main_screen_update(void);
void main_screen_render(void);
void main_screen_handle_event(ui_event_t* event);

void settings_screen_init(void);
void settings_screen_update(void);
void settings_screen_render(void);
void settings_screen_handle_event(ui_event_t* event);

void detail_screen_init(void);
void detail_screen_update(void);
void detail_screen_render(void);
void detail_screen_handle_event(ui_event_t* event);

void scanning_screen_init(void);
void scanning_screen_update(void);
void scanning_screen_render(void);
void scanning_screen_handle_event(ui_event_t* event);

#endif