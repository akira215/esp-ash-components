/*
  cppgpio
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "cppgpio.h"

#include <iostream>  // TODEL
 
// TODO Disable interrupt method
// TODO License
// TODO Documentation
// TODO Readme

bool GpioInput::_interrupt_service_installed{false};
portMUX_TYPE GpioInput::_eventChangeMutex = portMUX_INITIALIZER_UNLOCKED;

ESP_EVENT_DEFINE_BASE(INPUT_EVENTS);

//! \brief Interrupt Service Routine.
//! Satic Method
//! @param
void IRAM_ATTR GpioInput::gpio_isr_callback(void *args)
{

    int32_t pin = static_cast<int32_t>((static_cast<interrupt_args *>(args))->pin);
    bool custom_event_handler_set = (static_cast<interrupt_args *>(args))->custom_event_handler_set;
    bool event_handler_set = (static_cast<interrupt_args *>(args))->event_handler_set;
    bool queue_enabled = (static_cast<interrupt_args *>(args))->queue_enabled;
    esp_event_loop_handle_t custom_event_loop_handle = (static_cast<interrupt_args *>(args))->custom_event_loop_handle;
    QueueHandle_t queue_handle = (static_cast<interrupt_args *>(args))->queue_handle;
    
    void* data = (static_cast<interrupt_args *>(args))->event_data;

    if (queue_enabled)
    {
        xQueueSendFromISR(queue_handle, &pin, NULL);
    }
    else if (custom_event_handler_set)
    {
        esp_event_isr_post_to(custom_event_loop_handle, INPUT_EVENTS, pin, nullptr, 0, nullptr);
    }
    else if (event_handler_set)
    {
        esp_event_isr_post(INPUT_EVENTS, pin, data, sizeof(void*), nullptr);
    }
        
}

esp_err_t GpioInput::_init (const gpio_num_t pin, const bool activeLow)
{
    esp_err_t status{ESP_OK};

    _active_low = activeLow;
    _interrupt_args.pin = pin;

    gpio_config_t cfg;
    cfg.pin_bit_mask = 1ULL << pin;
    cfg.mode = GPIO_MODE_INPUT;
    cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_POSEDGE;

    status |= gpio_config(&cfg);

    return status;
}
    
GpioInput::GpioInput(const gpio_num_t pin, const bool activeLow)
{
    _init(pin, activeLow);
}

GpioInput::GpioInput(void)
{

}
GpioInput::~GpioInput()
{

}

esp_err_t GpioInput::init(const gpio_num_t pin, const bool activeLow)
{
    return _init(pin, activeLow);
}


int GpioInput::read(void)
{
    return _active_low ? !gpio_get_level(_interrupt_args.pin) : gpio_get_level(_interrupt_args.pin);
}

esp_err_t GpioInput::enablePullup(void)
{
    return gpio_set_pull_mode(_interrupt_args.pin, GPIO_PULLUP_ONLY);
}

esp_err_t GpioInput::disablePullup(void)
{
    return gpio_set_pull_mode(_interrupt_args.pin, GPIO_FLOATING);
}

esp_err_t GpioInput::enablePulldown(void)
{
    return gpio_set_pull_mode(_interrupt_args.pin, GPIO_PULLDOWN_ONLY);
}

esp_err_t GpioInput::disablePulldown(void)
{
    return gpio_set_pull_mode(_interrupt_args.pin, GPIO_FLOATING);
}

esp_err_t GpioInput::enablePullupPulldown(void)
{
    return gpio_set_pull_mode(_interrupt_args.pin, GPIO_PULLUP_PULLDOWN);
}

esp_err_t GpioInput::disablePullupPulldown(void)
{
    return gpio_set_pull_mode(_interrupt_args.pin, GPIO_FLOATING);
}

esp_err_t GpioInput::enableInterrupt(gpio_int_type_t int_type)
{
    esp_err_t status{ESP_OK};

    // Invert triggers if active low is enabled
    if (_active_low)
    {
        switch (int_type)
        {
        case GPIO_INTR_POSEDGE:
            int_type = GPIO_INTR_NEGEDGE;
            break;
        case GPIO_INTR_NEGEDGE:
            int_type = GPIO_INTR_POSEDGE;
            break;
        case GPIO_INTR_LOW_LEVEL:
            int_type = GPIO_INTR_HIGH_LEVEL;
            break;
        case GPIO_INTR_HIGH_LEVEL:
            int_type = GPIO_INTR_LOW_LEVEL;
            break;
        default:
            break;
        }
    }

    if (!_interrupt_service_installed)
    {
        status = gpio_install_isr_service(0);
        if (ESP_OK == status)
        {
            _interrupt_service_installed = true;
        }            
    }

    if (ESP_OK == status)
    {
        status = gpio_set_intr_type(_interrupt_args.pin, int_type);
    }

    if (ESP_OK == status)
    {
        status = gpio_isr_handler_add(_interrupt_args.pin, gpio_isr_callback, (void*) &_interrupt_args);
    }
    return status;
}

esp_err_t GpioInput::disableInterrupt()
{
    // gpio_intr_disable is called by gpio_isr_handler_remove
    return gpio_isr_handler_remove(_interrupt_args.pin);

}


// System event loop
esp_err_t GpioInput::setEventHandler(esp_event_handler_t Gpio_e_h, void* data)
{
    esp_err_t status{ESP_OK};

    taskENTER_CRITICAL(&_eventChangeMutex);

    status = clearEventHandlers();

    status = esp_event_handler_instance_register(INPUT_EVENTS, _interrupt_args.pin, Gpio_e_h, 0, nullptr);

    if (ESP_OK == status)
    {
        _interrupt_args.event_handler_set  = true;
        _interrupt_args.event_data         = data;
    }
    
    taskEXIT_CRITICAL(&_eventChangeMutex);

    return status;
}

// Custom event loop
esp_err_t GpioInput::setEventHandler(esp_event_loop_handle_t Gpio_e_l, esp_event_handler_t Gpio_e_h)
{
    esp_err_t status{ESP_OK};

    taskENTER_CRITICAL(&_eventChangeMutex);

    status = clearEventHandlers();

    status |= esp_event_handler_instance_register_with(Gpio_e_l, INPUT_EVENTS, _interrupt_args.pin, Gpio_e_h, 0, nullptr);

    if (ESP_OK == status)
    {
        _event_handle = Gpio_e_h;
        _interrupt_args.custom_event_loop_handle = Gpio_e_l;
        _interrupt_args.custom_event_handler_set = true;
    }

    taskEXIT_CRITICAL(&_eventChangeMutex);

    return status;
}

void GpioInput::setQueueHandle(QueueHandle_t Gpio_e_q)
{
    taskENTER_CRITICAL(&_eventChangeMutex);
    clearEventHandlers();
    _interrupt_args.queue_handle = Gpio_e_q;
    _interrupt_args.queue_enabled = true;
    taskEXIT_CRITICAL(&_eventChangeMutex);
}

esp_err_t GpioInput::clearEventHandlers()
{
    esp_err_t status {ESP_OK};

    if(_interrupt_args.custom_event_handler_set)
    {
        esp_event_handler_unregister_with(_interrupt_args.custom_event_loop_handle, INPUT_EVENTS, _interrupt_args.pin, _event_handle);
        _interrupt_args.custom_event_handler_set = false;
    }
    else if (_interrupt_args.event_handler_set)
    {
        esp_event_handler_instance_unregister(INPUT_EVENTS, _interrupt_args.pin, nullptr);
        _interrupt_args.event_handler_set = false;
    }

    _interrupt_args.queue_handle = nullptr;
    _interrupt_args.queue_enabled = false;

    return status;
}