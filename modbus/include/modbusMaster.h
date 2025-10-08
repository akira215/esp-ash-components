/*
  modbus
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include <esp_err.h>

class ModbusMaster
{
    public:
        /// @brief Constructor
        /// @param i2c_master : a pointer to an initilized I2c instance
        /// @param dev_address : ADS1115 Address set by hardware configuration
        /// @param clk_speed : speed of i2c bus. default value is 400kHz
        ModbusMaster(uint8_t pin);
        ~ModbusMaster();
};