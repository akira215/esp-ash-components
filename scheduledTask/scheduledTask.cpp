/*
  periodicTask
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "scheduledTask.h"
#include "esp_err.h"

//! \brief callback from imer
//! @param
void ScheduledTask::timerCallback(void *arg)
{
    // Call the vcallback function
    ScheduledTask* instance = static_cast<ScheduledTask*>(arg);
    instance->_func();

    //delete the object as it is oneshot timer
    if(instance->_autoDelete)
        delete instance;
}


void ScheduledTask::startTimer(uint64_t delay_ms)
{
    // Configure the timer
    esp_timer_create_args_t oneshot_timer_args;

    oneshot_timer_args.callback = &timerCallback,
	oneshot_timer_args.arg      = (void*)this,
	oneshot_timer_args.name     = _name.c_str();
			
    ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &_timer));

    ESP_ERROR_CHECK(esp_timer_start_once(_timer, delay_ms * 1000));
}

ScheduledTask::~ScheduledTask()
{
    esp_timer_delete(_timer);
}