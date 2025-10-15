/*
  modbus
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "mbcontroller.h"

class ModbusMaster
{
    void *_master_handle = nullptr;  


    public:
        /// @brief Constructor
        /// @param mode: bus communication mode (MB_ASCII or MB_RTU)
        /// @param port: UART number of ESP device (depend of device)
        /// @param baudrate: transmission speed on RS485
        /// @param data_bits: transmission word length on RS485
        /// @param parity:  transmission parity on RS485
        /// @param stop_bits: transmission stop bits on RS485
        /// @param timeout: response timeout in milliseconds
        ModbusMaster(mb_comm_mode_t mode = MB_RTU,
                    uart_port_t port = UART_NUM_0,
                    uint32_t baudrate = 9600,
                    uart_word_length_t data_bits = UART_DATA_8_BITS,
                    uart_parity_t parity = UART_PARITY_DISABLE,
                    uart_stop_bits_t stop_bits = UART_STOP_BITS_1,
                    uint32_t timeout = 1000);
        ~ModbusMaster();
    
        #ifdef CONFIG_MB_UART_DEBUG
            /// @brief Debug handler to read message on Modbus line
            static mb_exception_t debug_handler(void *inst, uint8_t *frame_ptr, uint16_t *len);
        #endif

};