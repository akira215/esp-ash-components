/*
  periodicTask
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "periodicTask.h"


//! \brief Interrupt Service Routine.
//! Satic Method
//! @param
bool IRAM_ATTR PeriodicTask::timer_alarm_callback(gptimer_handle_t timer, 
                const gptimer_alarm_event_data_t *edata, void *user_data)
{
    // Call the virtual method of the instance
    PeriodicTask* instance = static_cast<PeriodicTask*>(user_data);
    return instance->timerCallback();
}


PeriodicTask::PeriodicTask(uint64_t delay_ms)
{
    // Configure the timer
    gptimer_config_t timer_cfg;
    timer_cfg.clk_src = GPTIMER_CLK_SRC_DEFAULT,
    timer_cfg.direction = GPTIMER_COUNT_DOWN,
    timer_cfg.resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
    timer_cfg.intr_priority = 0; // low priority

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_cfg, &_timer));

    setDelay(delay_ms);

    // configure the call back.
    gptimer_event_callbacks_t timerCallback;
    timerCallback.on_alarm = timer_alarm_callback;

    gptimer_register_event_callbacks(_timer, &timerCallback, (void*)this);
    //ESP_ERROR_CHECK(gptimer_set_raw_count(_timer, delay_ms));
}

PeriodicTask::~PeriodicTask()
{
    gptimer_del_timer(_timer);
}


void PeriodicTask::setDelay(uint64_t delay_ms)
{
    // Configure the alarm
    gptimer_alarm_config_t alarm_cfg;
    alarm_cfg.alarm_count = 0;
    alarm_cfg.reload_count = delay_ms * 1000;
    alarm_cfg.flags.auto_reload_on_alarm = true;
    ESP_ERROR_CHECK(gptimer_set_alarm_action(_timer, &alarm_cfg));

}

void PeriodicTask::start()
{
    gptimer_enable(_timer);
    gptimer_start(_timer);
}

void PeriodicTask::stop()
{
    gptimer_stop(_timer);
    gptimer_disable(_timer);
}

