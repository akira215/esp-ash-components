/*
  periodicTask
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "freertos/FreeRTOS.h"
#include "driver/gptimer.h"
#include <esp_log.h>

class PeriodicTask
{
    gptimer_handle_t    _timer;
protected:
    uint64_t            _period_ms;

public:
    /// @brief Constructor of a periodic task
    /// @param delay in ms for task execution (default = 1s)
    PeriodicTask(uint64_t delay_ms = 1000); 
    virtual ~PeriodicTask();

    void setDelay(uint64_t delay_ms);
    void start();
    void stop();

protected:
    /// @brief timerCallback shall be implemented is derived class
    /// it will be executed in ISR context.
    virtual bool timerCallback() = 0;

private:
    static bool IRAM_ATTR timer_alarm_callback(gptimer_handle_t timer, 
                                const gptimer_alarm_event_data_t *edata, 
                                void *user_data);
}; // PeriodicTask Class

