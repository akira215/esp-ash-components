/*
  ws2812
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "freertos/FreeRTOS.h"
//#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"


#define RMT_RESOLUTION_HZ      10000000 // 10 MHz (1 tick = 0.1 microseconde)

// timing for WS2812 (ticks for 10MHz)
// T0H : ~0.35 us -> 4 ticks | T0L : ~0.80 us -> 8 ticks
// T1H : ~0.70 us -> 7 ticks | T1L : ~0.60 us -> 6 ticks
#define T0H       4
#define T0L       8
#define T1H       7
#define T1L       6


class Ws2812
{
   
    rmt_channel_handle_t _rmt {nullptr};
    rmt_encoder_handle_t _bytes_encoder {nullptr};
    uint16_t _led_count;

    typedef struct {
        uint8_t green = 0;
        uint8_t red = 0;
        uint8_t blue = 0;
    } ws2812_rgb_t; // Format GRB for WS2812

    ws2812_rgb_t* _led_buffer {nullptr};


public:
    /// @brief Constructor of Ws2812
    /// @param pin GPIO pin connected to the WS2812 data line
    Ws2812(const gpio_num_t pin, uint32_t led_count = 1); 
    ~Ws2812();

    void set_pixel(uint8_t r, uint8_t g, uint8_t b, uint16_t index = 0);
    void send();


}; // Ws2812 Class
