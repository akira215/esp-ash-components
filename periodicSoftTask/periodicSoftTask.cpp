/*
  periodicTask
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "periodicSoftTask.h"
#include "esp_err.h"

//! \brief callback from imer
//! @param
void PeriodicSoftTask::timerCallback(void *arg)
{
    // Call the vcallback function
    PeriodicSoftTask* instance = static_cast<PeriodicSoftTask*>(arg);
    instance->_func();
}


void PeriodicSoftTask::initAndStartTimer(uint64_t delay_ms, const char* name)
{
    // Configure the timer
    esp_timer_create_args_t timer_args;

    timer_args.callback = &timerCallback,
	timer_args.arg      = (void*)this,
	timer_args.name     = name;
			
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &_timer));

    ESP_ERROR_CHECK(esp_timer_start_periodic(_timer, delay_ms * 1000));
}

PeriodicSoftTask::~PeriodicSoftTask()
{
    esp_timer_stop(_timer);
    esp_timer_delete(_timer);
}