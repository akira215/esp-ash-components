# Persisted value

Storing value in the NVM

## Requirements

Only ESP-IDF framework is required

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

#include "persistedValue.h" 

#define HA_ESP_SENSOR_ENDPOINT          10      /* esp temperature sensor device endpoint, used for temperature measurement */
// Main class used for testing only
class Main final
{
public:
    Main();
    void run(void);
    void setup(void);
private:
    PersistedValue<int8_t> testNVM;

}; // Main Class
```

```cpp

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "main.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>


static const char *TAG = "Main_app";

Main App;

Main::Main():testNVM("test")
{

}

void Main::setup(void)
{
    ESP_LOGI(TAG,"App is starting");
   
    ESP_LOGI(TAG,"testNVS %d",(uint8_t) testNVM);

    testNVM = 67;
   
    ESP_LOGI(TAG,"after setting testNVS %d",(uint8_t) testNVM);

    testNVM.save();

}

void Main::run(void)
{
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG,"App is running");
    
}


extern "C" void app_main(void)
{
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


