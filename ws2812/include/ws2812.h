/*
  ws2812
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include <vector>

//#include "driver/gpio.h"
#include "driver/rmt_tx.h"


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

    enum class LightMode { COLOR_HSV, COLOR_TEMPERATURE };

    typedef struct {
        uint8_t green = 0;
        uint8_t red = 0;
        uint8_t blue = 0;
    } ws2812_rgb_t; // Format GRB for WS2812

    ws2812_rgb_t* _led_buffer {nullptr};

    struct hsv_state_t {
        uint16_t hue = 0;          // 0 - 360
        uint8_t saturation = 255;  // 0 - 255
        uint8_t value = 255;       // 0 - 255 (This is your brightness level)
        bool is_on = true;   
    
        // Temperature Mode
        LightMode mode = LightMode::COLOR_HSV;
        uint32_t kelvin = 6500; // Default Daylight white balance
    };

    // Vector to preserve the independent properties of each physical pixel
    std::vector<hsv_state_t> _led_states;

    /// Internal helper function to bake the current HSV state down to the raw RGB buffer
    /// @brief Recalculates and flushes the HSV properties down to the raw transmission array.
    /// This isolates state logic from translation logic.
    void refreshRgbBuffer(uint16_t index);


public:
    /// @brief Constructor of Ws2812
    /// @param pin GPIO pin connected to the WS2812 data line
    Ws2812(const gpio_num_t pin, uint32_t led_count = 1); 
    ~Ws2812();

    void setOn(uint16_t index = 0);
    void setOff(uint16_t index = 0);
    void toggle(uint16_t index = 0);

    /// @brief set RGB for one pixel
    /// @param rgb respective level for each color
    /// @param index Zero-indexed position of the targeted LED.
    void setPixelRgb(uint8_t r, uint8_t g, uint8_t b, uint16_t index = 0);

    /// @brief Adjusts the overall brightness level of an existing pixel color.
    /// @param brightness Brightness scale from 0 (off) to 255 (maximum brightness).
    /// @param index Zero-indexed position of the targeted LED.
    void setPixelBrightness(uint8_t brightness, uint16_t index = 0);

   /// @brief Sets a pixel using Hue, Saturation, and Value color configurations.
   /// @param hue Color wheel position from 0 to 360 degrees.
   /// @param index Zero-indexed position of the targeted LED.
   void setPixelHue(uint16_t hue, uint16_t index = 0);

    // @brief Sets a pixel using Hue, Saturation, and Value color configurations.
   /// @param saturation Richness of color from 0 (grayscale/white) to 255 (pure color).
   /// @param index Zero-indexed position of the targeted LED.
   void setPixelSaturation(uint8_t saturation, uint16_t index = 0);


    /// @brief Sets a pixel color using Kelvin Correlated Color Temperature (CCT).
    /// @param kelvin Warmth level from 1000K (Candlelight) up to 40000K (Deep Blue Sky). Optimal: 1500-10000K.
    /// @param index Zero-indexed position of the targeted LED.
    void setPixelTemperature(uint32_t kelvin, uint16_t index = 0);

    void send();


}; // Ws2812 Class
