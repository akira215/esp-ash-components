/*
  modbus
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "mbcontroller.h"


// Helper macros for dictionary
// The macro to get offset for parameter in the appropriate structure
#define HOLD_OFFSET(field) ((uint16_t)(offsetof(holding_reg_params_t, field) + 1))
#define INPUT_OFFSET(field) ((uint16_t)(offsetof(input_reg_params_t, field) + 1))
#define COIL_OFFSET(field) ((uint16_t)(offsetof(coil_reg_params_t, field) + 1))
// Discrete offset macro
#define DISCR_OFFSET(field) ((uint16_t)(offsetof(discrete_reg_params_t, field) + 1))

#define STR(fieldname) ((const char *)( fieldname ))
#define TEST_HOLD_REG_START(field) (HOLD_OFFSET(field) >> 1)
#define TEST_HOLD_REG_SIZE(field) (sizeof(((holding_reg_params_t *)0)->field) >> 1)

#define TEST_INPUT_REG_START(field) (INPUT_OFFSET(field) >> 1)
#define TEST_INPUT_REG_SIZE(field) (sizeof(((input_reg_params_t *)0)->field) >> 1)

// Options can be used as bit masks or parameter limits
#define OPTS(min_val, max_val, step_val) { .opt1 = min_val, .opt2 = max_val, .opt3 = step_val }

class ModbusMaster
{
    void *_master_handle = nullptr;
    mb_parameter_descriptor_t *_device_parameters = nullptr;


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
                    int tx_pin = UART_PIN_NO_CHANGE,
                    int rx_pin = UART_PIN_NO_CHANGE,
                    int rts_pin = UART_PIN_NO_CHANGE,
                    int cts_pin = UART_PIN_NO_CHANGE,
                    uint32_t baudrate = 9600,
                    uart_word_length_t data_bits = UART_DATA_8_BITS,
                    uart_parity_t parity = UART_PARITY_DISABLE,
                    uart_stop_bits_t stop_bits = UART_STOP_BITS_1,
                    uint32_t timeout = 1000,
                    uart_mode_t uart_mode = UART_MODE_RS485_HALF_DUPLEX);
        ~ModbusMaster();

        /// @brief Set dictionary for the devices on the bus
        /// @param dict: an array of 'mb_parameter_descriptor_t'
        /// @param n: number of items in the 'dict' array
        void setDictionary(const mb_parameter_descriptor_t *dict, uint16_t n);
        
        /// @brief read parameter from device
        /// @param cid: cid of the dictionary
        void getParameter(uint16_t cid);

        void sendRequest();
        
        #ifdef CONFIG_MB_UART_DEBUG
            /// @brief Debug handler to read message on Modbus line
            static mb_exception_t debug_handler(void *inst, uint8_t *frame_ptr, uint16_t *len);
        #endif

};