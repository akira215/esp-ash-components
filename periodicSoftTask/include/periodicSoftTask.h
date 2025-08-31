/*
  periodicSoftTask
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "esp_timer.h"
#include <functional>

/// @brief Task that will be trigger after a timeout
class PeriodicSoftTask
{
    typedef std::function<void(void)> taskCallback_t;
    taskCallback_t _func;
    esp_timer_handle_t _timer;

    static void timerCallback(void* arg);
    void initAndStartTimer(uint64_t delay_ms, const char* name);
public:
    
    /// @brief Construct the task scheduler object 
    /// that will be destroyed after running the task
    /// to pass args to the function, use std::bind
    /// @param func pointer to the method ex: &Main::task
    /// @param instance instance of the object for this handler (ex: this)
    /// @param delay_ms delay for the periodic task
    /// @param name name of the task that will be generated
    template<typename C>
    PeriodicSoftTask(void (C::* func)(), C* instance, 
                    uint64_t delay_ms, const char* name = "periodicSoftTask") {
        _func = std::bind(func,std::ref(*instance));
        initAndStartTimer(delay_ms, name);
    }
    ~PeriodicSoftTask();
};