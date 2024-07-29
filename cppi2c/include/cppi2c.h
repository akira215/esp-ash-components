#pragma once

//#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "esp_intr_alloc.h"
#include <vector>
#include <string>

class I2c
{
private:
    //uint16_t _slaveAddr{};
    i2c_port_t _port{};
    i2c_mode_t _mode{};
    size_t _slv_rx_buf_len{};
    size_t _slv_tx_buf_len{};
    int _intr_alloc_flags{};
    i2c_master_bus_handle_t _bus_handle{};
    std::vector<i2c_master_dev_handle_t> _devices; // vector of devices

public:
    //I2c(i2c_port_t port, size_t slv_rx_buf_len = 0, size_t slv_tx_buf_len = 0, int intr_alloc_flags = 0);
    I2c(i2c_port_t port, 
        uint8_t sda_io_num,
        uint8_t scl_io_num,
        bool pullup_en = false,
        i2c_clock_source_t clk_source = I2C_CLK_SRC_DEFAULT,
        uint8_t glitch_period = 7,
        int intr_priority = 0,
        size_t queue_depth = 0); // Depth of internal transfer queue. Only valid in asynchronous transaction
    ~I2c();

    i2c_master_dev_handle_t addDevice(uint16_t device_address, 
                                    uint32_t clk_speed_hz = 100000,
                                    i2c_addr_bit_len_t dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                    uint32_t timeout_us = 0,
                                    bool ack_check = true);
    esp_err_t removeDevice(i2c_master_dev_handle_t dev_handle);

    std::string ReadRegister(i2c_master_dev_handle_t i2c_dev, size_t read_size, int xfer_timeout_ms = -1); 
    std::string WriteReadRegister(i2c_master_dev_handle_t i2c_dev,std::string writeData, size_t read_size, int xfer_timeout_ms = -1);

    uint8_t WriteReadByte(i2c_master_dev_handle_t i2c_dev,uint8_t writeData, int xfer_timeout_ms = -1);
    uint16_t WriteReadWord(i2c_master_dev_handle_t i2c_dev,uint8_t writeData, int xfer_timeout_ms = -1);
    
    esp_err_t WriteRegister(i2c_master_dev_handle_t i2c_dev, std::string writeData, int xfer_timeout_ms = -1);

}; // class I2c