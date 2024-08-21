/*
  blinkTask
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "periodicTask.h"
#include "cppgpio.h"

static const uint64_t FAST_BLINK        = 100;  // 100 ms
static const uint64_t NORMAL_BLINK      = 200;  // 200 ms
static const uint64_t STANDARD_BLINK    = 500;  // 500 ms
static const uint64_t SLOW_BLINK        = 1000; // 1 sec

class BlinkTask : public PeriodicTask
{
   GpioOutput ledPin;

public:
    BlinkTask(const gpio_num_t pin, uint64_t delay_ms = STANDARD_BLINK);

    void setBlinkPeriod(uint64_t delay_ms);
protected:
    virtual bool timerCallback();
};