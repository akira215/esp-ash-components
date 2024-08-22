# blinkTask value

Blinking task

## Requirements

Only ESP-IDF framework is required
cppgpio required

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
#include "buttonTask.h" 
#include "blinkTask.h" 

// Main class used for testing only
class Main final
{
    GpioInput _button {GPIO_NUM_9,true};
    ButtonTask* _buttonTask;
    static GpioOutput  _led; 
public:
    Main();
    void run(void);
    void setup(void);
    static void shortPressHandler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
    static void longPressHandler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
private:

}; // Main Class
```

```cpp
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "main.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>


Main App;

Main::Main()
{ }

// Shall be static
void Main::shortPressHandler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    ESP_LOGW(TAG,"Short Press detected %ld -",id);
    _led.toggle();
}

// Shall be static
void Main::longPressHandler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    Main* instance = static_cast<Main*>(handler_args);
    ESP_LOGW(TAG,"Long Press detected %ld -",id);
    if (instance->_ledBlinking){
        delete instance->_ledBlinking;
        instance->_ledBlinking = nullptr;
    } else {
        instance->_ledBlinking = new BlinkTask(GPIO_NUM_23, FAST_BLINK);
    }
}



void Main::setup(void)
{
    // Blink  task
    BlinkTask* ledBlinking = new BlinkTask(GPIO_NUM_23);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    ledBlinking->setDelay(FAST_BLINK);
    delete ledBlinking;

    BlinkTask* fastBlinking= new BlinkTask(GPIO_NUM_23,NORMAL_BLINK);

    // Button task
    _button.enablePullup();
    _buttonTask = new ButtonTask (_button);
    _buttonTask->setShortPressHandler(&shortPressHandler);
    _buttonTask->setLongPressHandler(&longPressHandler,(void*)this);
    
    ESP_LOGI(TAG,"Button task created");

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


