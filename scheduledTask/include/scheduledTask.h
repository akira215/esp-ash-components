/*
  scheduledTask
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "esp_timer.h"
#include <functional>
#include <string>

/// @brief Task that will be trigger after a timeout
class ScheduledTask
{
    typedef std::function<void(void)> taskCallback_t;
    taskCallback_t _func;
    esp_timer_handle_t _timer;

    std::string _name;
    bool _autoDelete;

    static void timerCallback(void* arg);

public:
    
    /// @brief Construct the task scheduler object 
    /// that will be destroyed after running the task
    /// to pass args to the function, use std::bind
    /// @param func pointer to the method ex: &Main::task
    /// @param instance instance of the object for this handler (ex: this)
    template<typename C, typename... Args>
    ScheduledTask(void (C::* func)(Args... args), C* instance,
                    uint64_t delay_ms, 
                    std::string name = std::string("scheduledTask"),
                    bool autoDelete = true, Args... args) :
                        _name(name), _autoDelete(autoDelete) {
        _func = std::bind(func,std::ref(*instance), args...);
        startTimer(delay_ms);
    }
    ~ScheduledTask();
    void startTimer(uint64_t delay_ms);
};
