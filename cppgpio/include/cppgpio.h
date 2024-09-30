/*
  cppgpio
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(INPUT_EVENTS);

class GpioBase
{
protected:
    bool _active_low;
    gpio_num_t _pin {GPIO_NUM_NC};

public:
    gpio_num_t getPin() { return _pin; };
    //int32_t getPinNum() { return static_cast<int32_t>(getPin()); }
    int32_t getPinNum() { return static_cast<int32_t>(getPin()); }
}; // GpioBase Class



class GpioInput : public GpioBase
{
    
    static portMUX_TYPE     _eventChangeMutex;
    static bool             _interrupt_service_installed;

    bool                    _event_handler_set          = false;
    bool                    _custom_event_handler_set   = false;
    bool                    _queue_enabled              = false;

    esp_event_handler_t     _event_handle               {nullptr};
    esp_event_loop_handle_t _custom_event_loop_handle   {nullptr};
    QueueHandle_t           _queue_handle               {nullptr};
    void*                   _event_data                 {nullptr};

private:
    esp_err_t _init(const gpio_num_t pin, const bool activeLow);
    
public:
    GpioInput(const gpio_num_t pin, const bool activeLow = false);
    GpioInput(void);
    ~GpioInput();

    esp_err_t init(const gpio_num_t pin, const bool activeLow = false);
    int read(void);

    esp_err_t enablePullup(void);
    esp_err_t disablePullup(void);
    esp_err_t enablePulldown(void);
    esp_err_t disablePulldown(void);
    esp_err_t enablePullupPulldown(void);
    esp_err_t disablePullupPulldown(void);

    esp_err_t enableInterrupt(gpio_int_type_t int_type);
    esp_err_t disableInterrupt();

    /// @brief Set system loop event handler
    /// @param handler function to be called
    /// @param data pointer of pointer of data to be passed to the handler (copy)
    /// @return 
    esp_err_t setEventHandler(esp_event_handler_t handler, 
                                void* data = nullptr);

    /// @brief Set custom loop event handler
    /// @param event_loop event loop to post the event
    /// @param handler function to be called
    /// @return 
    esp_err_t setEventHandler(esp_event_loop_handle_t event_loop, 
                                esp_event_handler_t handler,
                                void* data = nullptr);
    
    void setQueueHandle(QueueHandle_t event_queue);
    esp_err_t clearEventHandlers();
    
    static void IRAM_ATTR gpio_isr_callback(void* arg);


}; // GpioInput Class

class GpioOutput : public GpioBase
{
private:
    int _level = 0;
    esp_err_t _init(const gpio_num_t pin, const bool activeLow);

public:
    GpioOutput(const gpio_num_t pin, const bool activeLow);
    GpioOutput(const gpio_num_t pin);
    GpioOutput(void);
    esp_err_t init(const gpio_num_t pin, const bool activeLow);
    esp_err_t init(const gpio_num_t pin);
    esp_err_t on(void);
    esp_err_t off(void);
    esp_err_t toggle(void);
    esp_err_t setLevel(int level);

}; // GpioOutput Class