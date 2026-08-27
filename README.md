# Network Scanner for GeeekPi Pico 2 W

[![Platform](https://img.shields.io/badge/platform-Raspberry%20Pi%20Pico%202%20W-blue)](https://www.raspberrypi.com/products/raspberry-pi-pico-2-w/)
[![Display](https://img.shields.io/badge/display-3.5%22%20Touchscreen-green)](https://www.geeekpi.com/)
[![License](https://img.shields.io/badge/license-MIT-yellow)](LICENSE)

A complete network scanning application for the Raspberry Pi Pico 2 W with GeeekPi 3.5" display module. Discover all devices on your network with an intuitive touch interface.

## Features

- 📶 **Wi-Fi Connection**: Connect to any WPA2 network
- 🔍 **Network Scanning**: Discover all active devices on your network
- 🏷️ **Device Identification**: MAC address, vendor, hostname, and device type
- 🎮 **Interactive UI**: Touch screen and joystick navigation
- 💾 **Device Database**: Persistent storage for known devices
- 📊 **Real-time Status**: Online/offline monitoring with latency
- 🔒 **Security Alerts**: Detect unknown or suspicious devices
- ⚡ **Optimized**: Fits in 520KB RAM with partial framebuffer

## Hardware Requirements

- [Raspberry Pi Pico 2 W](https://www.raspberrypi.com/products/raspberry-pi-pico-2-w/)
- [GeeekPi GPIO Expansion Module with 3.5" Screen](https://www.geeekpi.com/)
- Micro USB cable for power/programming

## Pin Connections

| Component | Pico Pin | Function |
|-----------|----------|----------|
| Display CLK | GP2 | SPI Clock |
| Display DIN | GP3 | SPI Data |
| Display CS | GP5 | Chip Select |
| Display DC | GP6 | Data/Command |
| Display RST | GP7 | Reset |
| Touch SDA | GP8 | I2C Data |
| Touch SCL | GP9 | I2C Clock |
| Touch RST | GP10 | Reset |
| Touch INT | GP11 | Interrupt |
| Joystick X | GP26 | ADC0 |
| Joystick Y | GP27 | ADC1 |
| RGB LED | GP12 | Programmable LED |
| Buzzer | GP13 | PWM Output |
| Button 1 | GP15 | User Input |
| Button 2 | GP14 | User Input |

## Quick Start

### Prerequisites

```bash
# Install required tools
sudo apt update
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential python3 git

# Clone with submodules
git clone --recursive https://github.com/yourusername/network-scanner-pico.git
cd network-scanner-pico

# Set up Pico SDK
git clone https://github.com/raspberrypi/pico-sdk.git --branch develop
export PICO_SDK_PATH=$(pwd)/pico-sdk

# Configure Wi-Fi
vim src/data/config.h  # Set WIFI_SSID and WIFI_PASSWORD

# Build
mkdir build && cd build
cmake ..
make -j4