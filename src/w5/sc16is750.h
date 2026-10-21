#pragma once

#include <stddef.h>
#include <stdint.h>

// SC16IS750 I2C-to-UART bridge (MCU-750 board) on the shared I2C bus. It
// gives the GPS a serial port: the 5" board has no free UART pins.
// Wiring: I2C/SPI and RESET to VIN, A0 and A1 to GND (address 0x4D).

// Sets up the UART (8N1) and clears its FIFOs. False if the chip doesn't
// answer. Call after Wire is started (ch422gBegin()).
bool sc16Begin(uint32_t baud);

// Reads up to cap received bytes from its 64-byte FIFO. Never blocks.
size_t sc16Read(uint8_t* out, size_t cap);

// Queues bytes for sending (to the GPS's RX).
void sc16Write(const uint8_t* data, size_t len);
