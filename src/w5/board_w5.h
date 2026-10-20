#pragma once

// Waveshare ESP32-S3-Touch-LCD-5, from its schematic and Waveshare's board
// config (github.com/waveshareteam/ESP32-S3-Touch-LCD-5).

// Shared I2C bus: touch (GT911), IO expander (CH422G), RTC (PCF85063), and
// the screw terminal's SDA/SCL through a level shifter.
const int W5_I2C_SDA = 8;
const int W5_I2C_SCL = 9;

const int W5_TOUCH_INT = 4;

// Onboard TJA1051T/3 CAN transceiver
const int W5_CAN_TX = 15;
const int W5_CAN_RX = 16;

// MicroSD on SPI; its chip select is CH422G EXIO4
const int W5_SD_MOSI = 11;
const int W5_SD_SCK = 12;
const int W5_SD_MISO = 13;

const int W5_WIDTH = 800;
const int W5_HEIGHT = 480;
