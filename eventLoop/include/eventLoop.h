/*
  eventLoop
  based on https://habr.com/en/articles/665730/
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <condition_variable>
#include <functional>
#include <future>
#include <vector>

class EventLoop
{
public:
	using callable_t = std::function<void()>;
	
	/// @brief Constructor of the event loop 
	/// @param usStackDepth as per xTaskCreate
	/// @param uxPriority as per xTaskCreate
	EventLoop(const char* name = "EventLoop",uint32_t usStackDepth = 4096, 
									UBaseType_t uxPriority = 5);
	EventLoop(const EventLoop&) = delete;
	EventLoop(EventLoop&&) noexcept = delete;
	~EventLoop() noexcept;

	EventLoop& operator= (const EventLoop&) = delete;
	EventLoop& operator= (EventLoop&&) noexcept = delete;

    void enqueue(callable_t&& callable) noexcept;
	
private:
	std::vector<callable_t> _writeBuffer;
	std::mutex _mutex;
	std::condition_variable _condVar;
	bool _running{ true };
    TaskHandle_t _task = nullptr;

    static void taskFunc(void *pvParameters) noexcept;

}; // EventLoop

