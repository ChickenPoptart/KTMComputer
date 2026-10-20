#pragma once

// LovyanGFX setup for the 5" ST7262 RGB panel. Timings and pins are
// Waveshare's (BOARD_WAVESHARE_ESP32_S3_TOUCH_LCD_5.h). The frame buffer
// lives in PSRAM; drawing writes straight into it.

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include "board_w5.h"

class LGFX_W5 : public lgfx::LGFX_Device {
public:
    LGFX_W5() {
        {
            auto cfg = panel.config();
            cfg.memory_width = cfg.panel_width = W5_WIDTH;
            cfg.memory_height = cfg.panel_height = W5_HEIGHT;
            cfg.offset_x = cfg.offset_y = 0;
            panel.config(cfg);
        }
        {
            auto cfg = panel.config_detail();
            cfg.use_psram = 1;
            panel.config_detail(cfg);
        }
        {
            auto cfg = bus.config();
            cfg.panel = &panel;
            // RGB565: d0-d4 blue, d5-d10 green, d11-d15 red
            cfg.pin_d0 = 14;  cfg.pin_d1 = 38;  cfg.pin_d2 = 18;  cfg.pin_d3 = 17;
            cfg.pin_d4 = 10;  cfg.pin_d5 = 39;  cfg.pin_d6 = 0;   cfg.pin_d7 = 45;
            cfg.pin_d8 = 48;  cfg.pin_d9 = 47;  cfg.pin_d10 = 21; cfg.pin_d11 = 1;
            cfg.pin_d12 = 2;  cfg.pin_d13 = 42; cfg.pin_d14 = 41; cfg.pin_d15 = 40;
            cfg.pin_henable = 5;
            cfg.pin_vsync = 3;
            cfg.pin_hsync = 46;
            cfg.pin_pclk = 7;
            cfg.freq_write = 16000000;
            cfg.hsync_polarity = 0;
            cfg.hsync_front_porch = 8;
            cfg.hsync_pulse_width = 4;
            cfg.hsync_back_porch = 8;
            cfg.vsync_polarity = 0;
            cfg.vsync_front_porch = 8;
            cfg.vsync_pulse_width = 4;
            cfg.vsync_back_porch = 8;
            cfg.pclk_active_neg = 1;
            cfg.de_idle_high = 0;
            cfg.pclk_idle_high = 0;
            bus.config(cfg);
        }
        panel.setBus(&bus);
        setPanel(&panel);
    }

private:
    lgfx::Bus_RGB bus;
    lgfx::Panel_RGB panel;
};
