
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>

#include "eventLoop.h"

EventLoop::EventLoop(const char* name, uint32_t usStackDepth, UBaseType_t uxPriority)
{
    // TODO check if task name shall be unique for debugging purpose
    xTaskCreate(&EventLoop::taskFunc, name, usStackDepth,
                                         this, uxPriority, &_task);
}

EventLoop::~EventLoop() 
{
    enqueue([this]
    {
        _running = false;
    });

    if( _task )
        vTaskDelete( _task );
}

void EventLoop::enqueue(callable_t&& callable) noexcept
{
    {
        ESP_LOGV("EventLoop", "enqueue");
        std::lock_guard<std::mutex> guard(_mutex);
        _writeBuffer.emplace_back(std::move(callable));
    }
    _condVar.notify_one();
}

//Static
void EventLoop::taskFunc(void *pvParameters) noexcept
{
    EventLoop* e = static_cast<EventLoop*>(pvParameters);
    std::vector<callable_t> readBuffer;
    
    while (e->_running)
    {
        {
            std::unique_lock<std::mutex> lock(e->_mutex);
            e->_condVar.wait(lock, [e]
            {
                return !e->_writeBuffer.empty();
            });
            std::swap(readBuffer, e->_writeBuffer);
        }
        
        for (callable_t& func : readBuffer)
        {
            ESP_LOGV("EventLoop", "Call func");
            func();
        }
        
        readBuffer.clear();
    }
}