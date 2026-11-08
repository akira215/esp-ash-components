# Wifi
Provide Wifi (station mode for now) connection 

## Requirements

Only ESP-IDF framework is required

## Build Instructions

Provide in your main directory a `Kconfig.projbuild with` this minimum contain:

```
menu "Project Settings"
    
    orsource "$IDF_PATH/examples/common_components/env_caps/$IDF_TARGET/Kconfig.env_caps"

    rsource "./Kconfig.main"
    rsource "./../components/wifi/Kconfig.wifi"

    comment "Module Debugging"

    
endmenu

```

Main CMakelists.txt should look like this
```
set(SOURCES main.cpp)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS . 
                    REQUIRES wifi )
```



## Usage and Examples

To pass argument to callback if required, it shall be after all arg list

```h
#pragma once

#include "wifi.h"

// Main class used for testing only
class Main final
{

public:
    Main();
    void run(void);
    void setup(void);

private:
    Wifi* wifi = nullptr;

}; // Main Class
```


```cpp

#include "main.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MainApp";


Main App;


Main::Main()
{
    // Setting the log level for each module
    esp_log_level_set("Main_app", ESP_LOG_VERBOSE);  // Put verbose to check available stack
    esp_log_level_set("Wifi", ESP_LOG_VERBOSE);
}

void Main::setup(void)
{
    wifi = new Wifi();
}



void Main::run(void)
{
    //_hbs->run();
    vTaskDelay(pdMS_TO_TICKS(10)); // let time to reset watchdog
}


extern "C" void app_main(void)
{
    ESP_LOGI(TAG,"---------- Starting App ----------");
    App.setup();

}
```

## Changes

### New in version 0.0.0


## Credits


