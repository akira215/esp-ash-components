/*
  ws2812
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "ws2812.h"

#include <esp_log.h> // todel
#include "esp_heap_caps.h"
#include <string.h>

#define TAG "WS2812_Driver"

Ws2812::Ws2812(const gpio_num_t pin, uint32_t led_count): _led_count(led_count)
{                         
   
    _led_buffer = new ws2812_rgb_t[led_count];

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

void Ws2812::set_pixel(uint8_t r, uint8_t g, uint8_t b, uint16_t index)
{
    
    if (!(_led_buffer)) {
        ESP_LOGE(TAG, "LED buffer not allocated");
        return;
    }
    if (index >= _led_count) {
        ESP_LOGW(TAG, "LED index out of bounds: %d / %d", index, _led_count);
        return;
    }

    _led_buffer[index].red = r;
    _led_buffer[index].green = g;
    _led_buffer[index].blue = b;

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

