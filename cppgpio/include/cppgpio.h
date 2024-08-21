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
    public:
        virtual gpio_num_t getPin() = 0;
        int32_t getPinNum() { return static_cast<int32_t>(getPin()); }
    protected:
        bool _active_low;
    }; // GpioBase Class

class GpioInput : public GpioBase
    {
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

        esp_err_t setEventHandler(esp_event_handler_t Gpio_e_h, void* data = nullptr);
        esp_err_t setEventHandler(esp_event_loop_handle_t Gpio_e_l, esp_event_handler_t Gpio_e_h);
        void setQueueHandle(QueueHandle_t Gpio_e_q);
        esp_err_t clearEventHandlers();
        
        static void IRAM_ATTR gpio_isr_callback(void* arg);
        virtual gpio_num_t getPin() { return _interrupt_args.pin; }
    private:
        esp_event_handler_t _event_handle = nullptr;
        static portMUX_TYPE _eventChangeMutex;
        static bool _interrupt_service_installed;

        struct interrupt_args
        {
            bool                    event_handler_set = false;
            bool                    custom_event_handler_set = false;
            bool                    queue_enabled = false;
            gpio_num_t              pin {GPIO_NUM_NC};
            esp_event_loop_handle_t custom_event_loop_handle {nullptr};
            QueueHandle_t           queue_handle {nullptr};
            void*                   event_data {nullptr};
        } _interrupt_args;


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
        virtual gpio_num_t getPin() {   return _pin;    }
    private:
        gpio_num_t _pin;
    }; // GpioOutput Class