/*
  thread
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include <string>
#include <functional>
#include "esp_event.h"
#include "esp_openthread.h"
#include "esp_openthread_types.h"
#include "openthread/instance.h"
#include "openthread/thread.h"


#pragma once


class Thread
{
public:
    /// @brief attribute update callback type 
    using threadCallback_t = std::function<void(esp_openthread_event_t)>;      // event_id

private:
    otInstance* _instance    = nullptr;

    std::vector<threadCallback_t>   _eventHandlers;

public:
    Thread();
    
    void init();

    /// @brief register attribute update handler for this attribute.
    /// Update handler shall be type eventCallback_t : 
    /// void(int msg_id, std::string topic, std::string data)
    /// @param func pointer to the method ex: &Main::eventHandler
    /// @param instance instance of the object for this handler (ex: this)
    template<typename C, typename... Args>
    void registerHandler(void (C::* func)(Args...), C* instance){
        _eventHandlers.push_back([instance, func](Args&&... args) {
            (instance->*func)(std::forward<Args>(args)...);
        });
    }

    void callEventHandlers(esp_openthread_event_t event_id) {
        for (const auto& handler : _eventHandlers) {
            handler(event_id);
        }
    }

private:
    static void thread_event_handler(void* arg, esp_event_base_t event_base, 
                                                    int32_t event_id, void* event_data);
    
    static size_t hex_string_to_bytes(const char *hex, uint8_t *bytes, size_t max_len);

    static void thread_loop_task(void *aContext);

}; // Thread Class
