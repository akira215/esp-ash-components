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
                        uint64_t longPress) : 
                PeriodicTask(delay_ms)
{
    _buttonPin.init(pin);
    _idleCounter = 0;    
    setLongPress(longPress);
    _shortPressHandlerSet = false;
    start();
}

ButtonTask::ButtonTask(GpioInput& inputGpio, uint64_t delay_ms,
                        uint64_t longPress) : 
                PeriodicTask(delay_ms), _buttonPin(inputGpio)
{
    _idleCounter = 0;  
    setLongPress(longPress);
    _shortPressHandlerSet = false;
    start();
}


void ButtonTask::setPollPeriod(uint64_t delay_ms)
{
    // Need to recompute longpress tick
    uint32_t longPress_ms = _longPress * _period_ms; 
    setDelay(delay_ms);
    setLongPress(longPress_ms);
}


void ButtonTask::setLongPress(uint64_t delay_ms)
{
    _longPress = (uint32_t)(delay_ms / _period_ms);
}

/// @brief executed in ISR context
/// @return Whether a high priority task has been waken up by this function
bool ButtonTask::timerCallback()
{
    // Debouncing strategy consist of checking the 8 bit button history,
    // using a mask to hide transition (mask is 0b11000111, all the 0 are
    // don't care bit). We compare to rising an falling pattern
    // A long press is detected only once. Button shall be release prior
    // to take in account any other press (short or long) detection
    static uint8_t button_history = 0;
    static uint32_t longPressCounter = 0;
    static bool pressed = false;
    uint8_t btnPressed = (uint8_t)_buttonPin.read();

    button_history = button_history << 1;
    button_history |= (uint8_t)_buttonPin.read();

    if(btnPressed)
        _idleCounter = 0;
    
    if ((button_history & DEBOUNCE_MASK) == 0b00000111){ 
        // button is pressed debounced
        pressed = true;
        button_history = 0b11111111;
    } else if((button_history & DEBOUNCE_MASK) == 0b11000000){
        // button is released debounced
        if(pressed && _shortPressHandlerSet)
            esp_event_isr_post_to(_loop_handle, SHORT_PRESS, 
                        _buttonPin.getPinNum(), nullptr, 0, nullptr);

        pressed = false;
        button_history = 0b00000000;
    } else if (btnPressed && (longPressCounter >_longPress)) {
        // long press detected 
        if (_longPressHandlerSet)
            esp_event_isr_post_to(_loop_handle, LONG_PRESS, 
                        _buttonPin.getPinNum(), nullptr, 0, nullptr);
        pressed = false;
        longPressCounter = 0;
    } else if (pressed && btnPressed) {
        // button still pressed but we didn't reach the longpress until now
        longPressCounter++;
    } else{
        longPressCounter = 0;
        _idleCounter++;
    }

    return false;
}

bool ButtonTask::canSleep()
{
    return _idleCounter > (uint32_t)(5000 / _period_ms);; // 5000 ms = 5 sec
}



// System event loop
esp_err_t ButtonTask::setShortPressHandler(esp_event_handler_t handler, void *handler_arg)
{
    esp_err_t status{ESP_OK};

    if (_shortPressHandlerSet)
        status = clearShortPressHandler();

    stop();
    //esp_event_loop_create_default();    // Create System Event Loop
    createCustomEventLoop();
    //status = esp_event_handler_instance_register(SHORT_PRESS, _buttonPin.getPinNum(), handler, handler_arg, nullptr);
    status = esp_event_handler_instance_register_with(_loop_handle, SHORT_PRESS, 
                            _buttonPin.getPinNum(), handler, handler_arg, nullptr);
    
    _shortPressHandlerSet = true;
    
    start();
    return status;
}

esp_err_t ButtonTask::setLongPressHandler(esp_event_handler_t handler, void *handler_arg)
{
    esp_err_t status{ESP_OK};

    if (_longPressHandlerSet)
        status = clearLongPressHandler();

    stop();
    createCustomEventLoop();
    status = esp_event_handler_instance_register_with(_loop_handle, LONG_PRESS, 
                            _buttonPin.getPinNum(), handler, handler_arg, nullptr);
    //esp_event_loop_create_default();    // Create System Event Loop
    //status = esp_event_handler_instance_register(LONG_PRESS, _buttonPin.getPinNum(), handler, handler_arg, nullptr);
    _longPressHandlerSet = true;
    
    start();
    
    return status;
}

esp_err_t ButtonTask::clearShortPressHandler()
{
    esp_err_t status {ESP_OK};
    stop();
    if (_shortPressHandlerSet){
        esp_event_handler_instance_unregister_with(_loop_handle, SHORT_PRESS, 
                                                _buttonPin.getPinNum(), nullptr);
        _shortPressHandlerSet = false;
        if(!_longPressHandlerSet)
            clearCustomEventLoop();
    }
    start();

    return status;
}

esp_err_t ButtonTask::clearLongPressHandler()
{
    esp_err_t status {ESP_OK};
    stop();
    if (_longPressHandlerSet){
        esp_event_handler_instance_unregister_with(_loop_handle, LONG_PRESS, 
                                                _buttonPin.getPinNum(), nullptr);
        _longPressHandlerSet = false;
        if(!_shortPressHandlerSet)
            clearCustomEventLoop();
    }
    start();

    return status;
}

void ButtonTask::createCustomEventLoop()
{
    if(_evtLoopCreated)
        return;

    // Custom event loop
    esp_event_loop_args_t loop_args;
    loop_args.queue_size = 5;
    loop_args.task_name = "button_task"; // Task will be created
    loop_args.task_priority = uxTaskPriorityGet(NULL);
    loop_args.task_stack_size = 4096;
    loop_args.task_core_id = tskNO_AFFINITY;
    esp_event_loop_create(&loop_args, &_loop_handle); // Create Custom Event Loop

    _evtLoopCreated = true;
    
}

void ButtonTask::clearCustomEventLoop()
{
    if(!_evtLoopCreated)
        return;

    esp_event_loop_delete(_loop_handle);
    
    _evtLoopCreated = false;

}
