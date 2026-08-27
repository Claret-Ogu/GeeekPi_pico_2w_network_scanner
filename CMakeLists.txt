cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(network_scanner C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(network_scanner
    src/main.c
    src/display/display_driver.c
    src/network/wifi_manager.c
    src/network/network_scanner.c
    src/ui/ui_framework.c
    src/ui/screens.c
    src/peripherals/joystick.c
    src/peripherals/touch.c
    src/data/device_db.c
)

target_include_directories(network_scanner PRIVATE
    src
    src/display
    src/network
    src/ui
    src/peripherals
    src/data
    src/utils
)

target_link_libraries(network_scanner
    pico_stdlib
    pico_cyw43_arch
    pico_lwip
    hardware_adc
    hardware_pwm
    hardware_spi
    hardware_i2c
    hardware_dma
    hardware_irq
    hardware_flash
)

pico_enable_stdio_uart(network_scanner 1)
pico_enable_stdio_usb(network_scanner 0)

pico_add_extra_outputs(network_scanner)

target_compile_options(network_scanner PRIVATE
    -Os
    -flto
    -Wl,-gc-sections
    -DPICO_FLASH_SIZE_BYTES=4194304
    -DPICO_RP2350
)

target_link_options(network_scanner PRIVATE
    -Wl,-gc-sections
    -Wl,--print-memory-usage
)