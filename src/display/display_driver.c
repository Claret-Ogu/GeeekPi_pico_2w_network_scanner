#include "display_driver.h"
#include <string.h>
#include <math.h>

// Static frame buffer (partial)
static uint16_t framebuffer[DISPLAY_WIDTH * 240]; // Half screen buffer
static bool dma_busy = false;
static uint8_t dma_channel;

// SPI and display state
static spi_inst_t* spi = spi0;
static uint8_t display_rotation = 0;

// Function prototypes
static void display_write_command(uint8_t cmd);
static void display_write_data(uint8_t data);
static void display_write_data16(uint16_t data);

// Font data (5x7)
static const uint8_t font[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x4F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x3E, 0x14, 0x3E, 0x14}, // #
    // ... (full font would be here)
};

static void display_write_command(uint8_t cmd) {
    gpio_put(PIN_DC, 0);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(spi, &cmd, 1);
    gpio_put(PIN_CS, 1);
}

static void display_write_data(uint8_t data) {
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(spi, &data, 1);
    gpio_put(PIN_CS, 1);
}

static void display_write_data16(uint16_t data) {
    uint8_t buf[2];
    buf[0] = data >> 8;
    buf[1] = data & 0xFF;
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(spi, buf, 2);
    gpio_put(PIN_CS, 1);
}

static void display_write_data16_bulk(const uint16_t* data, uint32_t count) {
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    
    // Use DMA for bulk transfers if available
    if (count > 100 && !dma_busy) {
        dma_busy = true;
        // DMA transfer setup
        // dma_channel = dma_claim_unused_channel(true);
        // ... DMA code
        dma_busy = false;
    } else {
        // Manual transfer
        for (uint32_t i = 0; i < count; i++) {
            uint8_t buf[2];
            buf[0] = data[i] >> 8;
            buf[1] = data[i] & 0xFF;
            spi_write_blocking(spi, buf, 2);
        }
    }
    gpio_put(PIN_CS, 1);
}

void display_init(void) {
    // Initialize SPI
    spi_init(spi, 60000000); // 60MHz
    spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    
    // Initialize pins
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    
    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_put(PIN_DC, 1);
    
    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_put(PIN_RST, 1);
    
    gpio_init(PIN_CLK);
    gpio_set_function(PIN_CLK, GPIO_FUNC_SPI);
    gpio_init(PIN_DIN);
    gpio_set_function(PIN_DIN, GPIO_FUNC_SPI);
    
    // Hardware reset
    gpio_put(PIN_RST, 0);
    sleep_ms(50);
    gpio_put(PIN_RST, 1);
    sleep_ms(120);
    
    // Initialization sequence
    display_write_command(0x01); // SWRESET
    sleep_ms(120);
    
    display_write_command(0x11); // SLPOUT
    sleep_ms(120);
    
    display_write_command(0x36); // MADCTL
    display_write_data(0x08);
    
    display_write_command(0x3A); // COLMOD
    display_write_data(0x55); // 16-bit color
    
    display_write_command(0xB0); // RMCTRL
    display_write_data(0x00);
    
    display_write_command(0xB1); // FRCTRL
    display_write_data(0xC0);
    display_write_data(0x1B);
    
    display_write_command(0xB4); // GAMCTRL
    display_write_data(0x01);
    
    display_write_command(0xB6); // DISSET
    display_write_data(0x00);
    display_write_data(0x02);
    
    display_write_command(0xE8); // TEON
    display_write_data(0x09);
    
    display_write_command(0xC0); // PWRCTRL
    display_write_data(0x1C);
    display_write_data(0x1C);
    
    display_write_command(0xC1); // VCOMCTRL
    display_write_data(0x01);
    
    display_write_command(0xC5); // VCOMOFF
    display_write_data(0x80);
    
    display_write_command(0xF0); // ECH
    display_write_data(0x55);
    display_write_data(0xAA);
    display_write_data(0x55);
    display_write_data(0xAA);
    
    display_write_command(0xF6); // ECH2
    display_write_data(0x05);
    display_write_data(0x10);
    display_write_data(0x07);
    
    display_write_command(0xFC); // PMCTRL
    display_write_data(0x00);
    display_write_data(0x55);
    
    display_write_command(0x29); // DISPON
    sleep_ms(120);
    
    display_write_command(0x2C); // RAMWR
    
    // Clear display
    display_clear(COLOR_BLACK);
}

void display_clear(uint16_t color) {
    uint16_t buffer[100];
    for (int i = 0; i < 100; i++) buffer[i] = color;
    
    display_set_window(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x += 100) {
            int count = (x + 100 > DISPLAY_WIDTH) ? DISPLAY_WIDTH - x : 100;
            display_write_data16_bulk(buffer, count);
        }
    }
}

void display_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    display_write_command(ST7796_CASET);
    display_write_data16(x1);
    display_write_data16(x2);
    
    display_write_command(ST7796_RASET);
    display_write_data16(y1);
    display_write_data16(y2);
    
    display_write_command(ST7796_RAMWR);
}

void display_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    
    display_set_window(x, y, x, y);
    display_write_data16(color);
}

void display_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    if (x + w > DISPLAY_WIDTH) w = DISPLAY_WIDTH - x;
    if (y + h > DISPLAY_HEIGHT) h = DISPLAY_HEIGHT - y;
    
    uint16_t buffer[100];
    for (int i = 0; i < 100; i++) buffer[i] = color;
    
    display_set_window(x, y, x + w - 1, y + h - 1);
    
    uint32_t total_pixels = w * h;
    while (total_pixels > 0) {
        uint32_t count = (total_pixels > 100) ? 100 : total_pixels;
        display_write_data16_bulk(buffer, count);
        total_pixels -= count;
    }
}

void display_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        display_draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void display_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    display_draw_line(x, y, x + w - 1, y, color);
    display_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color);
    display_draw_line(x + w - 1, y + h - 1, x, y + h - 1, color);
    display_draw_line(x, y + h - 1, x, y, color);
}

void display_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
    int x = r;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        display_draw_pixel(x0 + x, y0 + y, color);
        display_draw_pixel(x0 + y, y0 + x, color);
        display_draw_pixel(x0 - y, y0 + x, color);
        display_draw_pixel(x0 - x, y0 + y, color);
        display_draw_pixel(x0 - x, y0 - y, color);
        display_draw_pixel(x0 - y, y0 - x, color);
        display_draw_pixel(x0 + y, y0 - x, color);
        display_draw_pixel(x0 + x, y0 - y, color);
        
        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void display_draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg) {
    if (c < 32 || c > 127) return;
    c -= 32;
    
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (font[c][col] & (1 << row)) {
                display_draw_pixel(x + col, y + row, fg);
            } else {
                display_draw_pixel(x + col, y + row, bg);
            }
        }
    }
}

void display_draw_string(uint16_t x, uint16_t y, const char* str, uint16_t fg, uint16_t bg) {
    while (*str) {
        display_draw_char(x, y, *str++, fg, bg);
        x += 6; // 5 pixels wide + 1 spacing
    }
}

void display_update(void) {
    // Main update function - currently nothing needed for partial buffer
    // This would handle double buffering if implemented
}