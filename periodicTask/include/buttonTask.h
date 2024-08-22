/*
  buttonTask
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "periodicTask.h"
#include "cppgpio.h"
#include "esp_event.h"

static const uint64_t FAST_POLL     = 5;    // 5 ms
static const uint64_t STANDARD_POLL = 10;   // 10 ms
static const uint64_t SLOW_POLL     = 20;   // 20 ms

static const uint8_t DEBOUNCE_MASK  = 0b11000111;   // we will check only the 1 bit of the mask, the 0 are don't care bit

/// @brief button is polled periodically and send different signal (long press, ...)
class ButtonTask : public PeriodicTask
{
    GpioInput   _buttonPin;
    uint32_t    _idleCounter;
    uint32_t    _longPress; // the number of tick to reach a long press (depend on polling period)
    bool        _shortPressHandlerSet;
    bool        _longPressHandlerSet;

public:
    ButtonTask(const gpio_num_t pin, 
                uint64_t delay_ms = STANDARD_POLL,
                uint64_t longPress = 2000);
    ButtonTask(GpioInput& inputGpio, 
                uint64_t delay_ms = STANDARD_POLL,
                uint64_t longPress = 2000);

    void setPollPeriod(uint64_t delay_ms);
    void setLongPress(uint64_t delay_ms);

    esp_err_t setShortPressHandler(esp_event_handler_t Gpio_e_h);
    esp_err_t setLongPressHandler(esp_event_handler_t Gpio_e_h);
    esp_err_t clearShortPressHandler();
    esp_err_t clearLongPressHandler();
    bool canSleep();

protected:
    virtual bool timerCallback();
};