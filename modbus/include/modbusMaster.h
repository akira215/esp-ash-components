/*
  modbus
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "mb_data.h"
#include "esp_modbus_master.h"
//#include "mbcontroller.h"

//#include "mb_objects/include/mb_proto.h"

// Helper macros for dictionary


class ModbusMaster
{
    void *_master_handle = nullptr;

    enum mb_command
    {   
        CMD_NONE                        = (  0 ),
        CMD_READ_COILS                  = (  1 ),
        CMD_READ_DISCRETE_INPUTS        = (  2 ),
        CMD_WRITE_SINGLE_COIL           = (  5 ),
        CMD_WRITE_MULTIPLE_COILS        = ( 15 ),
        CMD_READ_HOLDING_REGISTER       = (  3 ),
        CMD_READ_INPUT_REGISTER         = (  4 ),
        CMD_WRITE_REGISTER              = (  6 ),
        CMD_WRITE_MULTIPLE_REGISTERS    = ( 16 ),
        CMD_READWRITE_MULTIPLE_REGISTERS= ( 23 ),
        CMD_DIAG_READ_EXCEPTION         = (  7 ),
        CMD_DIAG_DIAGNOSTIC             = (  8 ),
        CMD_DIAG_GET_COM_EVENT_CNT      = ( 11 ),
        CMD_DIAG_GET_COM_EVENT_LOG      = ( 12 ),
        CMD_OTHER_REPORT_SLAVEID        = ( 17 ),
        CMD_ERROR                       = ( 0x80 )
    };


    public:
        /// @brief Constructor
        /// @param mode: bus communication mode (MB_ASCII or MB_RTU)
        /// @param port: UART number of ESP device (depend of device)
        /// @param baudrate: transmission speed on RS485
        /// @param data_bits: transmission word length on RS485
        /// @param parity:  transmission parity on RS485
        /// @param stop_bits: transmission stop bits on RS485
        /// @param timeout: response timeout in milliseconds
        /// @param uart_mode: RS485 uart mode
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

        

        /// @brief send a get request to a slave
        /// @param slave_addr: slave adress on the bus taht request will be send to
        /// @param cmd: Modbus command
        /// @param reg_start: address of the register on the slave
        /// @param reg_size: length (in word i.e. 2 bytes or 16 bits) of slave register
        /// @param return:  Answer from slave
        mb_data getRequest(uint8_t slave_addr, 
                            uint8_t cmd, 
                            uint16_t reg_start, 
                            uint16_t reg_size);
        
        /// @brief read register from slave
        /// @param slave_addr: slave adress on the bus taht request will be send to
        /// @param reg_start: address of the register on the slave
        /// @param reg_size: length (in word i.e. 2 bytes or 16 bits) of slave register
        /// @param return:  bytes from slave
        mb_data readRegisters(uint8_t slave_addr, 
                                uint16_t reg_start, 
                                uint16_t reg_size);
        
        
        /// @brief send a get request to a slave
        /// @param slave_addr: slave adress on the bus taht request will be send to
        /// @param cmd: Modbus command
        /// @param reg_start: address of the register on the slave
        /// @param data: data to be written
        void setRequest(uint8_t slave_addr, 
                            uint8_t cmd, 
                            uint16_t reg_start, 
                            mb_data data);
        
        /// @brief read register from slave
        /// @param slave_addr: slave adress on the bus taht request will be send to
        /// @param reg_start: address of the register on the slave
        /// @param params: data to be written
        void writeRegisters(uint8_t slave_addr, 
                                uint16_t reg_start, 
                                mb_data data);

        void testRequest(); // TODEL

        #ifdef CONFIG_MB_UART_DEBUG
            /// @brief Debug handler to read message on Modbus line
            static mb_exception_t debug_handler(void *inst, uint8_t *frame_ptr, uint16_t *len);
        #endif

};