# ws2812
driver for WS2812 NeoPixel led using DMA streaming on RMT

## Requirements

Only ESP-IDF framework is required, with RMT component

## Build Instructions

Add all the esp-ash-components as a git submodule of your project

## Current Status

Still work in progress, but is completely running. All bugs 
could be reported to try to improve it.

## Performance



## Caveats

### To be developped



## Usage and Examples

```h
#pragma once

#include "ws2812.h" // to del

// Main class used for testing only
class Main final
{
    Ws2812      _neoLed{static_cast<gpio_num_t>(8)};
public:
    Main();
    void run(void);
    void setup(void);
    void print_hardware(void);

}; // Main Class
```

```cpp


#include "main.h"
#include <freertos/FreeRTOS.h>
#include <esp_log.h>

Main App;

Main::Main()
{
    // Setting the log level for each module
    esp_log_level_set("Main_app", ESP_LOG_VERBOSE);  // Put verbose to check available stack
    esp_log_level_set("WS2812_Driver", ESP_LOG_VERBOSE);  // Put verbose to check available stack
}

void Main::setup(void)
{
    ESP_LOGI(TAG,"---------- Setup ----------");

    _neoLed.set_pixel(41, 103, 196); // blue
    _neoLed.send();
}

void Main::run(void)
{

    uint8_t red = 255;
    uint8_t green = 0;
    uint8_t blue = 50;

    for (int i = 0; i < 256; i++) {
        _neoLed.set_pixel(red, green, blue);
        _neoLed.send();
        
        // Gradually increase the red value
        red = 255 -i;
        green = i;
        blue = 10;
        
        vTaskDelay(pdMS_TO_TICKS(10)); // Adjust this delay to control the speed of color change
    }

    for (int i = 0; i < 256; i++) {
        _neoLed.set_pixel(red, green, blue);
        _neoLed.send();
        
        // Gradually increase the red value
        red = i;
        green = 255 - i;
        blue = 10;
        
        vTaskDelay(pdMS_TO_TICKS(10)); // Adjust this delay to control the speed of color change
    }

}


extern "C" void app_main(void)
{
    ESP_LOGI(TAG,"---------- Starting App ----------");
    App.setup();

    while (true)
    {
        App.run();
    }    

    //should not reach here
}


```

## Changes

### New in version 0.0.0


## Credits


