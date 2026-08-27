#include "touch.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// GT911 I2C address
#define GT911_ADDR 0x5D

// GT911 registers
#define GT911_READ_XY 0x814E
#define GT911_POINT_INFO 0x814E

static bool touched = false;
static uint16_t touch_x = 0;
static uint16_t touch_y = 0;

static void write_register(uint16_t reg, uint8_t value) {
    uint8_t buf[3];
    buf[0] = (reg >> 8) & 0xFF;
    buf[1] = reg & 0xFF;
    buf[2] = value;
    i2c_write_blocking(i2c0, GT911_ADDR, buf, 3, false);
}

static uint8_t read_register(uint16_t reg) {
    uint8_t buf[2];
    buf[0] = (reg >> 8) & 0xFF;
    buf[1] = reg & 0xFF;
    i2c_write_blocking(i2c0, GT911_ADDR, buf, 2, true);
    i2c_read_blocking(i2c0, GT911_ADDR, buf, 1, false);
    return buf[0];
}

static void read_xy(void) {
    uint8_t buf[8];
    uint8_t cmd[2];
    cmd[0] = (GT911_READ_XY >> 8) & 0xFF;
    cmd[1] = GT911_READ_XY & 0xFF;
    
    i2c_write_blocking(i2c0, GT911_ADDR, cmd, 2, true);
    i2c_read_blocking(i2c0, GT911_ADDR, buf, 8, false);
    
    // Parse touch data
    uint8_t points = buf[0] & 0x0F;
    touched = (points > 0);
    
    if (touched) {
        // First touch point
        touch_x = ((buf[1] & 0x0F) << 8) | buf[2];
        touch_y = ((buf[3] & 0x0F) << 8) | buf[4];
        
        // Map to display coordinates
        touch_x = touch_x * DISPLAY_WIDTH / TOUCH_WIDTH;
        touch_y = touch_y * DISPLAY_HEIGHT / TOUCH_HEIGHT;
    }
}

void touch_init(void) {
    // Initialize I2C
    i2c_init(i2c0, 400000);
    gpio_set_function(TOUCH_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(TOUCH_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(TOUCH_SDA_PIN);
    gpio_pull_up(TOUCH_SCL_PIN);
    
    // Reset touch controller
    gpio_init(TOUCH_RST_PIN);
    gpio_set_dir(TOUCH_RST_PIN, GPIO_OUT);
    gpio_put(TOUCH_RST_PIN, 0);
    sleep_ms(10);
    gpio_put(TOUCH_RST_PIN, 1);
    sleep_ms(50);
    
    // Initialize GT911
    write_register(0x8040, 0x00); // Config start
    sleep_ms(10);
}

bool touch_get_event(touch_event_t* event) {
    if (!event) return false;
    
    read_xy();
    
    event->x = touch_x;
    event->y = touch_y;
    event->touched = touched;
    
    return true;
}

bool touch_is_touched(void) {
    read_xy();
    return touched;
}

void touch_get_position(uint16_t* x, uint16_t* y) {
    if (x) *x = touch_x;
    if (y) *y = touch_y;
}