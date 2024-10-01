# GpioInput - GpioOutput

GpioInput & GpioOutput are C++ class to access to Gpio pin of ESP devices, using ESP-IDF framework. It is a kind of wrapper

## Requirements

Only ESP-IDF framework is required

## Build Instructions

Add all the esp-ash-components as a git submodule of your project

## Current Status

Still work in progress, but is completely running. All bugs 
could be reported to try to improve it.

## Performance



## Caveats

### To be developped



## Usage and Examples

```h
#pragma once

#include <iostream>

#include "cppgpio.h"

#define GPIO_LED                    CONFIG_LED_GPIO  //!< GPIO number connect to the LED

// Main class used for testing only
class Main final
{
public:
    Main();
    void run(void);
    void setup(void);
    void print_hardware(void);

    // LED pin on Board
    GpioOutput cppLed { static_cast<gpio_num_t>(GPIO_LED) };
    // Repurpose the BOOT button to work as an input
    GpioInput cppButton { GPIO_NUM_9 };
    // A second input to demonstrate Event_ID different event handlers

    esp_event_loop_handle_t gpio_loop_handle {};
    
    // Event Handler for cppButton
    static void button_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
    
    // Event Handler for custom loop
    static void task_custom_event(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
    // Handle for the queue
    static QueueHandle_t button_evt_queue;
    // Prototype for the task
    static void gpio_task_example(void *arg);
private:

}; // Main Class
```

```cpp

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_mac.h>
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Main.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include <iostream>

static const char *TAG = "Main_app";


QueueHandle_t Main::button_evt_queue {nullptr};

Main App;

Main::Main(): 
{
    
}

void Main::run(void)
{
    vTaskDelay(pdMS_TO_TICKS(2000));

    cppLed.setLevel(cppButton.read()); 
}

void Main::setup(void)
{
    
    cppButton.enablePullup();
    cppButton.enableInterrupt(GPIO_INTR_NEGEDGE);

    // System Event Loop
    esp_event_loop_create_default();    // Create System Event Loop
    // Get a pointer of pointer to retrieve the instance in the event loop
    Main** ptr = new Main*(this);
    cppButton.setEventHandler(&button_event_handler, ptr);

    // Custom event loop
    esp_event_loop_args_t gpio_loop_args;
    gpio_loop_args.queue_size = 5;
    gpio_loop_args.task_name = "loop_task"; // Task will be created
    gpio_loop_args.task_priority = uxTaskPriorityGet(NULL);
    gpio_loop_args.task_stack_size = 2048;
    gpio_loop_args.task_core_id = tskNO_AFFINITY;
    esp_event_loop_create(&gpio_loop_args, &gpio_loop_handle); // Create Custom Event Loop
    cppButton.setEventHandler(gpio_loop_handle, &task_custom_event);

    // Queue Handle
    button_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);
    cppButton.setQueueHandle(button_evt_queue);

    // Print Hardware infos
    print_hardware();
}

void Main::button_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    Main* instance = *(static_cast<Main**>(event_data));
    instance->cppLed.toggle();
    std::cout << "Button triggered interrupt with ID: " << id << '\n';
}

void Main::task_custom_event(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    std::cout << "Button triggered interrupt with ID: " << id << " With Custom Loop\n";
}

void Main::gpio_task_example(void *arg)
{
   uint32_t io_num;
    for (;;)
    {
        if (xQueueReceive(TankMonitor::button_evt_queue, &io_num, portMAX_DELAY))
        {
            std::cout << "Interrupt triggered from pin " << (int)io_num << " and send to queue\n";
        }
    }
}

void Main::print_hardware (void)
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }
    

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

}




extern "C" void app_main(void)
{
    App.setup();

    while (true)
    {
        App.run();
    }    

    //should not reach here
    //ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
    //ESP_ERROR_CHECK(i2c_del_master_bus(i2c_handle));
    //ESP_LOGD(TAG, "I2C de-initialized successfully");
}


```

## Changes

### New in version 0.0.0


## Credits


