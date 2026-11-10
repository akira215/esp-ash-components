# ESP-IDF MQTT C++

Modbus C++ 

## Requirements

- espressif/esp__mqtt

It will be installed by the `idf_component.yml` file located in the lib folder.



## Build Instructions

Provide in your main directory a `Kconfig.projbuild with` this minimum contain:

```
menu "Project Settings"
    
    orsource "$IDF_PATH/examples/common_components/env_caps/$IDF_TARGET/Kconfig.env_caps"

    rsource "./Kconfig.main"
    rsource "./../components/mqttClient/Kconfig.mqtt"

    comment "Module Debugging"

    
endmenu

```

Main `CMakelists.txt` should look like this
```
set(SOURCES main.cpp)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS . 
                    REQUIRES wifi mqttClient)
```


## Code example

```h
#pragma once

#include "wifi.h"
#include "mqttClient.h"

// Main class used for testing only
class Main final
{

public:
    Main();
    void run(void);
    void setup(void);
    void onMqttConnection(bool connected, int res);
    void onMqttData(int msg_id, const std::string& topic, const std::string& data);

private:
    Wifi*       _wifi = nullptr;
    MqttClient* _mqtt = nullptr;

}; // Main Class 

```

```cpp
include "main.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_random.h" //todel




static const char *TAG = "MainApp";

Main App;


Main::Main()
{
    // Setting the log level for each module
    esp_log_level_set("Main_app", ESP_LOG_VERBOSE);  // Put verbose to check available stack
    esp_log_level_set("Wifi", ESP_LOG_VERBOSE);
    esp_log_level_set("MqttClient", ESP_LOG_VERBOSE);
}

void Main::setup(void)
{
    _wifi = new Wifi();
    if (_wifi->isConnected()) {
        _mqtt = new MqttClient();
        _mqtt->start();
    }
    _mqtt->registerConnectionHandler(&Main::onMqttConnection, this);
    _mqtt->registerDataHandler(&Main::onMqttData, this);
}

void Main::onMqttConnection(bool connected, int res)
{
    if (connected) {
        ESP_LOGI(TAG, "MQTT Connected. Session present: %d", res);
        _mqtt->subscribeTopic("akira/test1",0);
    } else {
        ESP_LOGW(TAG, "MQTT Disconnected. Error type: %d", res);
    }
}

void Main::onMqttData(int msg_id, const std::string& topic, const std::string& data)
{
    ESP_LOGI(TAG, "MQTT Data received. Message ID: %d, Topic: %s, Data: %s", msg_id, topic.c_str(), data.c_str());
}

void Main::run(void)
{
    int val = esp_random()%100;

    _mqtt->sendData("akira/test1", std::to_string(val));
    vTaskDelay(pdMS_TO_TICKS(2000)); // let time to reset watchdog
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

## Current Status

Still work in progress, but is completely running. All bugs 
could be reported to try to improve it.

## Performance



## Caveats

### To be developped



## Usage and Examples



## Changes

### New in version 0.0.0


## Credits


