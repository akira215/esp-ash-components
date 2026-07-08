/*
  ws2812
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "ws2812.h"

#include <esp_log.h> // todel
#include <cmath>
#include <algorithm>

#include "freertos/FreeRTOS.h"
#include "driver/rmt_encoder.h"
//#include "esp_heap_caps.h"
//#include <string.h>

#define TAG "WS2812_Driver"

Ws2812::Ws2812(const gpio_num_t pin, uint32_t led_count): _led_count(led_count)
{                         
   
    _led_buffer = new ws2812_rgb_t[led_count];
    _led_states.resize(_led_count);

    if (!_led_buffer) {
        ESP_LOGE(TAG, "Failed to allocate memory for LED buffer");
        return;
    }


    rmt_tx_channel_config_t tx_config{};
    tx_config.gpio_num = pin;
    tx_config.clk_src = RMT_CLK_SRC_DEFAULT;
    tx_config.resolution_hz = RMT_RESOLUTION_HZ; // 10MHz
    tx_config.mem_block_symbols = 64;
    tx_config.trans_queue_depth = 4;
    tx_config.flags.with_dma = false; 


    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_config, &_rmt));

    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .duration0 = T0H, .level0 = 1, 
            .duration1 = T0L, .level1 = 0, 
        },
        .bit1 = {
            .duration0 = T1H, .level0 = 1,
            .duration1 = T1L, .level1 = 0,
        },
        .flags = { .msb_first = 1 } // Le WS2812 attend le bit de poids fort en premier
    };

    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytes_encoder_config, &_bytes_encoder));

    ESP_ERROR_CHECK(rmt_enable(_rmt));
    ESP_LOGD(TAG, "RMT initialisé avec succès.");
}

Ws2812::~Ws2812()
{
    ESP_ERROR_CHECK(rmt_del_channel(_rmt));
    if (_led_buffer) {
        delete[] _led_buffer;
        _led_buffer = nullptr;
    }

}

void Ws2812::setOn(uint16_t index) 
{
    if (index >= _led_count) {
        ESP_LOGE(TAG, "led_on - LED index out of bounds: %d / %d", index, _led_count);
        return;
    }
    
    _led_states[index].is_on = true;  // Mark as on
    refreshRgbBuffer(index);        // Restores the previous color state perfectly
}

void Ws2812::setOff(uint16_t index) 
{
    if (index >= _led_count) {
        ESP_LOGE(TAG, "led_off - LED index out of bounds: %d / %d", index, _led_count);
        return;
    }
    
    _led_states[index].is_on = false; // Mark as off
    refreshRgbBuffer(index);        // Clears the hardware buffer safely
}


void Ws2812::toggle(uint16_t index) 
{
    if (index >= _led_count) {
        ESP_LOGE(TAG, "led_toggle - LED index out of bounds: %d / %d", index, _led_count);
        return;
    }
    
    _led_states[index].is_on = !_led_states[index].is_on;
    refreshRgbBuffer(index);
}



void Ws2812::refreshRgbBuffer(uint16_t index) {
    if (index >= _led_count || _led_buffer == nullptr) {
        ESP_LOGE(TAG, "refreshRgbBuffer - Invalid buffer or LED index out of bounds: %d / %d", index, _led_count);
        return;
    }


    hsv_state_t& state = _led_states[index];

    // If power flag is off
    if (!state.is_on) {
        _led_buffer[index].red = 0;
        _led_buffer[index].green = 0;
        _led_buffer[index].blue = 0;
        return; // Exit early; don't calculate anything else
    }


    // Branch COLOR_TEMPERATURE: Output temperature logic
    if (state.mode == LightMode::COLOR_TEMPERATURE) {
        float tmp_internal = state.kelvin / 100.0f;
        float r = 0, g = 0, b = 0;

        if (tmp_internal <= 66.0f) r = 255.0f;
        else { r = tmp_internal - 60.0f; r = 329.698727446f * std::pow(r, -0.1332047592f); }

        if (tmp_internal <= 66.0f) { g = tmp_internal; g = 99.4708025861f * std::log(g) - 161.1195681661f; }
        else { g = tmp_internal - 60.0f; g = 288.1221695283f * std::pow(g, -0.0755148492f); }

        if (tmp_internal >= 66.0f) b = 255.0f;
        else if (tmp_internal <= 19.0f) b = 0.0f;
        else { b = tmp_internal - 10.0f; b = 138.5177312231f * std::log(b) - 305.0447927307f; }

        r = std::clamp(r, 0.0f, 255.0f); g = std::clamp(g, 0.0f, 255.0f); b = std::clamp(b, 0.0f, 255.0f);

        // Uses the central value parameter as brightness scale regardless of active mode
        _led_buffer[index].red   = static_cast<uint8_t>((r * state.value) / 255.0f);
        _led_buffer[index].green = static_cast<uint8_t>((g * state.value) / 255.0f);
        _led_buffer[index].blue  = static_cast<uint8_t>((b * state.value) / 255.0f);
        return;
    }

    // Branch COLOR_HSV: Classic HSV Translation Logic
    float h = (state.hue >= 360) ? 0.0f : state.hue / 60.0f;
    float s = state.saturation / 255.0f;
    float v = state.value / 255.0f;

    int i = static_cast<int>(std::floor(h));
    float f = h - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    float r = 0, g = 0, b = 0;

    switch (i) {
        case 0:  r = v; g = t; b = p; break;
        case 1:  r = q; g = v; b = p; break;
        case 2:  r = p; g = v; b = t; break;
        case 3:  r = p; g = q; b = v; break;
        case 4:  r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }

    _led_buffer[index].red   = static_cast<uint8_t>(r * 255.0f);
    _led_buffer[index].green = static_cast<uint8_t>(g * 255.0f);
    _led_buffer[index].blue  = static_cast<uint8_t>(b * 255.0f);
}


void Ws2812::setPixelRgb(uint8_t r, uint8_t g, uint8_t b, uint16_t index)
{
    
    if (index >= _led_count || _led_buffer == nullptr) {
        ESP_LOGE(TAG, "set_pixel_rgb - Invalid buffer or LED index out of bounds: %d / %d", index, _led_count);
        return;
    }

    _led_buffer[index].red = r;
    _led_buffer[index].green = g;
    _led_buffer[index].blue = b;

}

void Ws2812::setPixelBrightness(uint8_t brightness, uint16_t index) 
{
    if (index >= _led_count || _led_buffer == nullptr) {
        ESP_LOGE(TAG, "set_brightness - Invalid buffer or LED index out of bounds: %d / %d", index, _led_count);
        return;
    }
    
    _led_states[index].value = brightness;
    refreshRgbBuffer(index);
}


void Ws2812::setPixelHue(uint16_t hue, uint16_t index) 
{
    if (index >= _led_count || _led_buffer == nullptr) {
        ESP_LOGE(TAG, "set_hue - Invalid buffer or LED index out of bounds: %d / %d", index, _led_count);
        return;
    }

    _led_states[index].mode = LightMode::COLOR_HSV; // Swap mode implicitly on call
    _led_states[index].hue = hue;
    refreshRgbBuffer(index); // Re-bake changes to hardware map
}
    

void Ws2812::setPixelSaturation(uint8_t saturation, uint16_t index) 
{
    if (index >= _led_count || _led_buffer == nullptr) {
        ESP_LOGE(TAG, "set_saturation - Invalid buffer or LED index out of bounds: %d / %d", index, _led_count);
        return;
    }
    
    _led_states[index].saturation = saturation;
    refreshRgbBuffer(index);
}


void Ws2812::setPixelTemperature(uint32_t kelvin, uint16_t index) 
{
    if (index >= _led_count || _led_buffer == nullptr) {
        ESP_LOGE(TAG, "set_pixel_temperature - Invalid buffer or LED index out of bounds: %d / %d", index, _led_count);
        return;
    }

    _led_states[index].mode = LightMode::COLOR_TEMPERATURE; // Swap mode implicitly on call
    _led_states[index].kelvin = kelvin;
    refreshRgbBuffer(index);
}


void Ws2812::send()
{
    if (!(_rmt)) {
        ESP_LOGE(TAG, "RMT channel not initialized");
        return;
    }

    if (!(_bytes_encoder)) {
        ESP_LOGE(TAG, "Bytes encoder not initialized");
        return;
    }
    
    rmt_transmit_config_t tx_config = {
        .loop_count = 0 // unique transmission
    };

    
    // Check if TX is ongoing
    esp_err_t err = rmt_tx_wait_all_done(_rmt, 0);

    if (err == ESP_ERR_TIMEOUT) {
        // Transmission is ONGOING

        // Wait if previous transmission is still ongoing
        esp_err_t err = rmt_tx_wait_all_done(_rmt, portMAX_DELAY);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to wait for previous transmission to complete: %s", esp_err_to_name(err));
            return;
        }
        // RESET > 50 µs
        esp_rom_delay_us(60);
    }
    
    err = rmt_transmit(_rmt, _bytes_encoder, _led_buffer, sizeof(_led_buffer), &tx_config);
    if (err != ESP_OK)
        ESP_LOGE(TAG, "Error during transmission: %s", esp_err_to_name(err));


}

