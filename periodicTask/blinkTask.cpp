/*
  blinkTask
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "blinkTask.h"

BlinkTask::BlinkTask(const gpio_num_t pin, uint64_t delay_ms) : PeriodicTask(delay_ms)
{
    ESP_ERROR_CHECK(ledPin.init(pin));
    start();
}

void BlinkTask::setBlinkPeriod(uint64_t delay_ms)
{
    setDelay(delay_ms);
}

bool BlinkTask::timerCallback()
{
    ledPin.toggle();
    return false;
}
   
