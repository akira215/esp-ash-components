/*
  buttonTask
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "buttonTask.h"

ESP_EVENT_DEFINE_BASE(SHORT_PRESS);
ESP_EVENT_DEFINE_BASE(LONG_PRESS);

ButtonTask::ButtonTask(const gpio_num_t pin, uint64_t delay_ms,
                    uint64_t shortPress, uint64_t longPress) : 
                PeriodicTask(delay_ms)
{
    _buttonPin.init(pin);
    _counter = 0;
    _idleCounter = 0;
    setShortPress(shortPress);    
    setLongPress(longPress);
    _shortPressHandlerSet = false;
    start();
}

ButtonTask::ButtonTask(GpioInput& inputGpio, uint64_t delay_ms,
                    uint64_t shortPress, uint64_t longPress) : 
                PeriodicTask(delay_ms), _buttonPin(inputGpio)
{
    _counter = 0;
    _idleCounter = 0;
    setShortPress(shortPress);    
    setLongPress(longPress);
    _shortPressHandlerSet = false;
    start();
}


void ButtonTask::setPollPeriod(uint64_t delay_ms)
{
    setDelay(delay_ms);
}

void ButtonTask::setShortPress(uint64_t delay_ms)
{
    _shortPress = (uint32_t)(delay_ms / _period_ms);
}

void ButtonTask::setLongPress(uint64_t delay_ms)
{
    _longPress = (uint32_t)(delay_ms / _period_ms);
}

/// @brief executed in ISR context
/// @return Whether a high priority task has been waken up by this function
bool ButtonTask::timerCallback()
{
    bool btnPressed = _buttonPin.read();
    
    if((btnPressed)&&(_counter >_longPress)){
        _idleCounter = 0;
        //Long press detected
        if (_longPressHandlerSet)
            esp_event_isr_post(LONG_PRESS, _buttonPin.getPinNum(), nullptr, 0, nullptr);
        _counter = 0; // to avoid triggerring shortpress when releasing 
    } else if (btnPressed){
        _counter ++;
        _idleCounter = 0;
    } else if (_counter > _shortPress){
        //Short press detected
        
        if (_shortPressHandlerSet)
            esp_event_isr_post(SHORT_PRESS, _buttonPin.getPinNum(), nullptr, 0, nullptr);

        _counter = 0;
    } else {
        _idleCounter++;
    }


    return false;
}

bool ButtonTask::canSleep()
{
    return _idleCounter > (uint32_t)(5000 / _period_ms);; // 5000 ms = 5 sec
}

// System event loop
esp_err_t ButtonTask::setShortPressHandler(esp_event_handler_t handler)
{
    esp_err_t status{ESP_OK};

    if (_shortPressHandlerSet)
        status = clearShortPressHandler();

    stop();
    esp_event_loop_create_default();    // Create System Event Loop
    status = esp_event_handler_instance_register(SHORT_PRESS, _buttonPin.getPinNum(), handler, 0, nullptr);
    _shortPressHandlerSet = true;
    
    start();
    return status;
}


esp_err_t ButtonTask::setLongPressHandler(esp_event_handler_t handler)
{
    esp_err_t status{ESP_OK};

    if (_longPressHandlerSet)
        status = clearLongPressHandler();

    stop();
    esp_event_loop_create_default();    // Create System Event Loop
    status = esp_event_handler_instance_register(LONG_PRESS, _buttonPin.getPinNum(), handler, 0, nullptr);
    _longPressHandlerSet = true;
    
    start();
    return status;
}

esp_err_t ButtonTask::clearShortPressHandler()
{
    esp_err_t status {ESP_OK};
    stop();
    if (_shortPressHandlerSet){
        esp_event_handler_instance_unregister(SHORT_PRESS, _buttonPin.getPinNum(), nullptr);
        _shortPressHandlerSet = false;
    }
    start();

    return status;
}

esp_err_t ButtonTask::clearLongPressHandler()
{
    esp_err_t status {ESP_OK};
    stop();
    if (_longPressHandlerSet){
        esp_event_handler_instance_unregister(LONG_PRESS, _buttonPin.getPinNum(), nullptr);
        _longPressHandlerSet = false;
    }
    start();

    return status;
}
