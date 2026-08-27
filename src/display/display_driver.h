#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"

// Display pins
#define PIN_CS       5
#define PIN_DC       6
#define PIN_RST      7
#define PIN_CLK      2
#define PIN_DIN      3

// Display dimensions
#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 480

// Colors (RGB565)
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_DARK      0x1082

// Display commands
#define ST7796_SLPOUT   0x11
#define ST7796_DISPON   0x29
#define ST7796_CASET    0x2A
#define ST7796_RASET    0x2B
#define ST7796_RAMWR    0x2C
#define ST7796_MADCTL   0x36

void display_init(void);
void display_clear(uint16_t color);
void display_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void display_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void display_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void display_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void display_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg);
void display_draw_string(uint16_t x, uint16_t y, const char* str, uint16_t fg, uint16_t bg);
void display_update(void);

#endif