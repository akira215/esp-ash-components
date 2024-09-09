/*
  cppi2c
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "cppi2c.h"
#include <stdexcept>
#include <algorithm>

I2c::I2c(i2c_port_t port, 
        uint8_t sda_io_num,
        uint8_t scl_io_num,
        bool pullup_en,
        i2c_clock_source_t clk_source,
        uint8_t glitch_period,
        int intr_priority,
        size_t queue_depth)
{
    _port = port;
    
    i2c_master_bus_config_t i2c_mst_config = i2c_master_bus_config_t();

    i2c_mst_config.i2c_port = _port;
    i2c_mst_config.sda_io_num = static_cast<gpio_num_t>(sda_io_num);
    i2c_mst_config.scl_io_num = static_cast<gpio_num_t>(scl_io_num);
    i2c_mst_config.clk_source = clk_source; // default or i2c_clock_source_t::I2C_CLK_SRC_APB
    i2c_mst_config.glitch_ignore_cnt = glitch_period;
    i2c_mst_config.intr_priority = intr_priority;
    i2c_mst_config.trans_queue_depth = queue_depth;
    i2c_mst_config.flags.enable_internal_pullup = pullup_en;

    //Create the new master bus;
    //esp_err_t status = ESP_OK;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &_bus_handle));
    /*
    if (status == ESP_ERR_NOT_FOUND ) {
        throw std::out_of_range( "Error creating master I2C bus, no more port available" );
    }
    */
  
}

I2c::~I2c()
{
    for(i2c_master_dev_handle_t& d : _devices) 
        ESP_ERROR_CHECK(i2c_master_bus_rm_device(d));

    _devices.clear();
    
    i2c_del_master_bus(_bus_handle);
}


i2c_master_dev_handle_t I2c::addDevice(uint16_t device_address, 
                                    uint32_t clk_speed_hz,
                                    i2c_addr_bit_len_t dev_addr_length,
                                    uint32_t timeout_us,
                                    bool ack_check)
{
    uint32_t disable_ack_check = 1;
    if (ack_check)
        disable_ack_check = 0;

    i2c_device_config_t dev_cfg;
    dev_cfg.dev_addr_length = dev_addr_length;
    dev_addr_length = dev_addr_length;
    dev_cfg.device_address = device_address;
    dev_cfg.scl_speed_hz = clk_speed_hz;
    dev_cfg.scl_wait_us = timeout_us;
    dev_cfg.flags.disable_ack_check = disable_ack_check;



    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(_bus_handle, &dev_cfg, &dev_handle));

    _devices.emplace_back(dev_handle);
    return dev_handle;
}

esp_err_t I2c::removeDevice(i2c_master_dev_handle_t dev_handle)
{
    esp_err_t status = ESP_OK;
    status = i2c_master_bus_rm_device(dev_handle);

    _devices.erase(std::remove(_devices.begin(), _devices.end(), dev_handle), _devices.end());
    return status;
}

std::string I2c::ReadRegister(i2c_master_dev_handle_t i2c_dev,size_t read_size, int xfer_timeout_ms)
{
    uint8_t* rxBuf = new uint8_t[read_size];

    ESP_ERROR_CHECK(i2c_master_receive(i2c_dev, rxBuf, read_size, xfer_timeout_ms));

    std::string rxData(reinterpret_cast<char const*>(rxBuf), read_size);

    return rxData;
}

std::string I2c::WriteReadRegister(i2c_master_dev_handle_t i2c_dev,std::string writeData,size_t read_size, int xfer_timeout_ms)
{
    uint8_t* rxBuf = new uint8_t[read_size];
    const uint8_t* write = reinterpret_cast<const uint8_t*>(writeData.c_str());


    ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_dev, write, writeData.length(), rxBuf, read_size, xfer_timeout_ms));

    std::string rxData(reinterpret_cast<char const*>(rxBuf), read_size);

    return rxData;
}

uint8_t I2c::WriteReadByte(i2c_master_dev_handle_t i2c_dev, uint8_t writeData, int xfer_timeout_ms)
{
    uint8_t rxBuf;
    
    ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_dev, &writeData, 1, &rxBuf, 1, xfer_timeout_ms));

    return rxBuf;
}

uint16_t I2c::WriteReadWord(i2c_master_dev_handle_t i2c_dev, uint8_t writeData, int xfer_timeout_ms)
{
    uint8_t rxBuf[2];
    
    ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_dev, &writeData, 1, rxBuf, 2, xfer_timeout_ms));

    return (rxBuf[0] << 8) | rxBuf[1];
}


esp_err_t I2c::WriteRegister(i2c_master_dev_handle_t i2c_dev,std::string writeData, int xfer_timeout_ms)
{
    //uint8_t* rxBuf = new uint8_t[read_size];
    const uint8_t* write = reinterpret_cast<const uint8_t*>(writeData.c_str());

    return i2c_master_transmit(i2c_dev, write, writeData.length(), xfer_timeout_ms);
}

esp_err_t I2c::WriteRegister(i2c_master_dev_handle_t i2c_dev, uint8_t* writeData, size_t data_size, int xfer_timeout_ms)
{
    return i2c_master_transmit(i2c_dev, writeData, data_size, xfer_timeout_ms);
}

esp_err_t I2c::WriteByte(i2c_master_dev_handle_t i2c_dev, uint8_t writeData, int xfer_timeout_ms)
{
    return i2c_master_transmit(i2c_dev, &writeData, 1, xfer_timeout_ms);
}
esp_err_t I2c::WriteWord(i2c_master_dev_handle_t i2c_dev, uint16_t writeData, int xfer_timeout_ms)
{
    const uint8_t* write = reinterpret_cast<const uint8_t*>(&writeData);
    return i2c_master_transmit(i2c_dev, write, 2, xfer_timeout_ms);
}
