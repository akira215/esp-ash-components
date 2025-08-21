# Ads1115

Ads1115 is a C++ class to interface Texas Instrument ADS1115 device to ESP devices, using ESP-IDF framework.

## Requirements

Apart from ESP-IDF framework, the following components are required:
 - I2c class (cppi2c)
 - GpioInput class (cppgpio)

## Build Instructions

The most efficient way is to put a symlink in the components folder of your project point to the cppads1115 directory. Take care that the component mentionned in the Requirements shall as well be integrated as component in your project


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
#include "cppi2c.h"
#include "ads1115.h"

// Main class used for testing only
class Main final
{
public:
    Main();
    void run(void);
    void setup(void);

    // Event handler when conversion is received
    static void ads1115_event_handler(uint16_t input, int16_t value);
    
private:
    I2c i2c_master;
    Ads1115 ads;

}; // Main Class
```

```cpp
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_mac.h>
#include <esp_log.h>


#include "driver/i2c_master.h"
#include "driver/gpio.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Main.h"

#include <iostream>

static const char *TAG = "Main_app";

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      //!< GPIO number used for I2C master clock 
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      //!< GPIO number used for I2C master data  
#define I2C_MASTER_NUM              I2C_NUM_0                  //!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip 
#define ADS_I2C_FREQ_HZ             400000                     //!< I2C master clock frequency 
#define ADS_I2C_TIMEOUT_MS          1000
#define GPIO_INPUT_IO_READY         CONFIG_ADS1115_READY_INT   //!< GPIO number connect to the ready pin of the converter

Main App;

Main::Main(): 
        i2c_master(I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, true),
        ads(&i2c_master,Ads1115::Addr_Gnd, ADS_I2C_FREQ_HZ)
{
    
}

void Main::run(void)
{
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "Conversion : %f", ads.getVoltage(Ads1115::MUX_2_GND));
}

void Main::setup(void)
{
    Ads1115::Cfg_reg cfg = ads.getConfig();
    ESP_LOGI(TAG, "Config: %d", cfg.reg.reg);
    ESP_LOGI(TAG, "COMP QUE:    %x",cfg.bit.COMP_QUE);
    ESP_LOGI(TAG, "COMP LAT:    %x",cfg.bit.COMP_LAT);
    ESP_LOGI(TAG, "COMP POL:    %x",cfg.bit.COMP_POL);
    ESP_LOGI(TAG, "COMP MODE:   %x",cfg.bit.COMP_MODE);
    ESP_LOGI(TAG, "DataRate:    %x",cfg.bit.DR);
    ESP_LOGI(TAG, "MODE:        %x",cfg.bit.MODE);
    ESP_LOGI(TAG, "PGA:         %x",cfg.bit.PGA);
    ESP_LOGI(TAG, "MUX:         %x",cfg.bit.MUX);
    ESP_LOGI(TAG, "OS:          %x",cfg.bit.OS);

    Ads1115::reg2Bytes_t regData;
    regData = ads.readRegister(Ads1115::reg_lo_thresh);
    ESP_LOGI(TAG, "Reg Lo Thresh : %x", regData.reg);
    ESP_LOGI(TAG, "Reg Lo MSB : %x", regData.MSB);
    ESP_LOGI(TAG, "Reg Lo LSB : %x", regData.LSB);

    regData = ads.readRegister(Ads1115::reg_hi_thresh);
    ESP_LOGI(TAG, "Reg Hi Thresh : %x", regData.reg);
    ESP_LOGI(TAG, "Reg Hi MSB : %x", regData.MSB);
    ESP_LOGI(TAG, "Reg Hi LSB : %x", regData.LSB);
    
    ESP_LOGI(TAG, "Changing config --------------");
    regData.MSB = 0x01; 
    //ads.writeRegister(Ads1115::reg_hi_thresh,regData);

    regData = ads.readRegister(Ads1115::reg_hi_thresh);
    ESP_LOGI(TAG, "Reg Hi Thresh : %x", regData.reg);

    ESP_LOGI(TAG, "Changing config --------------");
    regData.MSB = 0x01; 
    //ads.writeRegister(Ads1115::reg_lo_thresh,regData);

    regData = ads.readRegister(Ads1115::reg_lo_thresh);
    ESP_LOGI(TAG, "Reg Lo Thresh : %x", regData.reg);


    regData = ads.readRegister(Ads1115::reg_configuration);
    ESP_LOGI(TAG, "Configuration : %x", regData.reg);
    ESP_LOGI(TAG, "Cfg MSB : %x", regData.MSB);
    ESP_LOGI(TAG, "Cfg LSB : %x", regData.LSB);

    ESP_LOGI(TAG, "Starting --------------");
 

    ads.setPga(Ads1115::FSR_4_096); // Setting range for PGA optimized to 3.3V Power supply
    ads.setSps(Ads1115::SPS_8); // Setting range for PGA optimized to 3.3V Power supply

    // event handler shall have signature void(uint16_t input, int16_t value)
    ads.setReadyPin(GPIO_NUM_3, &ads1115_event_handler);

    regData = ads.readRegister(Ads1115::reg_configuration);
    ESP_LOGI(TAG, "Configuration : %x", regData.reg);

    regData = ads.readRegister(Ads1115::reg_lo_thresh);
    ESP_LOGI(TAG, "Reg Lo Thresh : %x", regData.reg);

    regData = ads.readRegister(Ads1115::reg_hi_thresh);
    ESP_LOGI(TAG, "Reg Hi Thresh : %x", regData.reg);
}

void Main::ads1115_event_handler(uint16_t input, int16_t value)
{
    ESP_LOGI(TAG, "Callback Main Ads1115 input : %d", input);
    ESP_LOGI(TAG, "Callback Main Ads1115 value : %d", value);
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


