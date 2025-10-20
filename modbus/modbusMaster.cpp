/*
  modbus
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>
#include <esp_err.h>
#include "modbusMaster.h"

static const char *MODBUS_TAG = "Modbus";


#ifdef CONFIG_MB_UART_DEBUG
    #define MB_CUST_DATA_LEN 100 // The length of debug command buffer
    static char my_custom_data[MB_CUST_DATA_LEN] = {0}; // the debug data buffer

    // static function to output Modbus message
    // This is the custom function handler for the command.
    // The handler is executed from the context of modbus controller event task and should be as simple as possible.
    // Parameters: frame_ptr - the pointer to the incoming ADU frame from slave starting from function code,
    // len - the pointer to length of the frame. After return from the handler the modbus object will 
    // handle the end of transaction according to the exception returned.
    mb_exception_t ModbusMaster::debug_handler(void *inst, uint8_t *frame_ptr, uint16_t *len)
    {
        MB_RETURN_ON_FALSE((frame_ptr && len && *len && *len < (MB_CUST_DATA_LEN - 1)), MB_EX_ILLEGAL_DATA_VALUE, MODBUS_TAG,
                                "incorrect custom frame buffer");
        ESP_LOGD(MODBUS_TAG, "Custom handler, Frame ptr: %p, len: %u", frame_ptr, *len);
        strncpy((char *)&my_custom_data[0], (char *)&frame_ptr[1], MB_CUST_DATA_LEN);
        ESP_LOG_BUFFER_HEXDUMP("CUSTOM_DATA", &my_custom_data[0], (*len - 1), ESP_LOG_WARN);
        return MB_EX_NONE;
    }
#endif

ModbusMaster::ModbusMaster(mb_comm_mode_t mode,
                            uart_port_t port,
                            int tx_pin,
                            int rx_pin,
                            int rts_pin,
                            int cts_pin,
                            uint32_t baudrate,
                            uart_word_length_t data_bits,
                            uart_parity_t parity,
                            uart_stop_bits_t stop_bits,
                            uint32_t timeout,
                            uart_mode_t uart_mode)
{
    // Initialize Modbus controller
    mb_communication_info_t comm;

    comm.ser_opts.mode = mode;
    comm.ser_opts.port = port;
    comm.ser_opts.baudrate = baudrate;
    comm.ser_opts.data_bits = data_bits;
    comm.ser_opts.parity = parity;
    comm.ser_opts.stop_bits = stop_bits;
    comm.ser_opts.response_tout_ms = timeout;
    comm.ser_opts.uid = 0; // dummy port for master


    esp_err_t err = mbc_master_create_serial(&comm, &_master_handle);
    if (_master_handle == nullptr)
        ESP_LOGE(MODBUS_TAG,"mb controller initialization fail, null handle.");
    
    if (err != ESP_OK)
        ESP_LOGE(MODBUS_TAG,"mb controller initialization fail, returns(0x%x).", (int)err);

    #ifdef CONFIG_MB_UART_DEBUG
        // override handler for output Modbus message
        const uint8_t override_command = 0x41;
        // Delete the handler for specified command, if available
        err = mbc_delete_handler(_master_handle, override_command);
        if (err != ESP_OK)
            ESP_LOGE(MODBUS_TAG,"could not override handler, returned (0x%x).", (int)err);
    
        err = mbc_set_handler(_master_handle, override_command, debug_handler);
        if (err != ESP_OK)
            ESP_LOGE(MODBUS_TAG,"could not override handler, returned (0x%x).", (int)err);

        mb_fn_handler_fp handler = NULL;
        err = mbc_get_handler(_master_handle, override_command, &handler);
        if ((err != ESP_OK) || (handler != debug_handler))
            ESP_LOGE(MODBUS_TAG,"could not get handler for command %d, returned (0x%x).", (int)override_command, (int)err);
    
    #endif

    // Set UART pin numbers
    err = uart_set_pin(port, tx_pin, rx_pin,
                              rts_pin, cts_pin);
    if (err != ESP_OK)
        ESP_LOGE(MODBUS_TAG,"mb serial set pin failure, uart_set_pin() returned (0x%x).", (int)err);                     
   
    // Set a dump descriptor as it is required to start master mb
    err = mbc_master_set_descriptor(_master_handle,_descriptor,2);
    if (err != ESP_OK)
        ESP_LOGE(MODBUS_TAG,"mb controller set empty descriptor fail, returned (0x%x).", (int)err);


    err = mbc_master_start(_master_handle);
    if (err != ESP_OK)
        ESP_LOGE(MODBUS_TAG,"mb controller start fail, returned (0x%x).", (int)err);    
   
    // Set driver mode to Half Duplex
    err = uart_set_mode(port, uart_mode);
    if (err != ESP_OK)
        ESP_LOGE(MODBUS_TAG,"mb serial set mode failure, uart_set_mode() returned (0x%x).", (int)err);

    ESP_LOGI(MODBUS_TAG, "Modbus master stack initialized...");

}

ModbusMaster::~ModbusMaster()
{
    ESP_ERROR_CHECK(mbc_master_delete(_master_handle));
    _master_handle = nullptr;
}

mb_data ModbusMaster::getRequest(uint8_t slave_addr, 
                                uint8_t cmd, 
                                uint16_t reg_start, 
                                uint16_t reg_size)
{
    mb_data data(reg_size*2); // Modbus length is 1 word = 2 bytes

    mb_param_request_t req = {
        .slave_addr = slave_addr,              // the slave UID to send the request
        .command = cmd,                            // read holding 0x04 read input,
        .reg_start = reg_start,                             // unused,
        .reg_size = reg_size   // length of the data to receive (registers)
    };

    esp_err_t err = mbc_master_send_request(_master_handle, &req, data.buffer());

    if (err == ESP_OK) {
        ESP_LOGD(MODBUS_TAG, "sendRequest read successful @ 0x%02x", reg_start);
            for (int i=0; i < data.getSize();++i)
                ESP_LOGV(MODBUS_TAG, "0x%02x", data.getByte(i));
        return data;
    } else {
        ESP_LOGE(MODBUS_TAG, "Request read fail, err = 0x%x (%s).",                      
                            (int)err,                                                             
                            (char*)esp_err_to_name(err));     
    }

    return mb_data();
}


mb_data ModbusMaster::readRegisters(uint8_t slave_addr, 
                                    uint16_t reg_start, 
                                    uint16_t reg_size)
{
    return getRequest(slave_addr, CMD_READ_HOLDING_REGISTER, reg_start, reg_size);
}

void ModbusMaster::setRequest(uint8_t slave_addr, 
                                uint8_t cmd, 
                                uint16_t reg_start, 
                                mb_data data)
{
    uint16_t reg_size = (uint16_t)(data.getSize()/2);
    //mb_data data(reg_size*2); // Modbus length is 1 word = 2 bytes

    mb_param_request_t req = {
        .slave_addr = slave_addr,              // the slave UID to send the request
        .command = cmd,                            // read holding 0x04 read input,
        .reg_start = reg_start,                             // unused,
        .reg_size = reg_size   // length of the data to receive (registers)
    };

    esp_err_t err = mbc_master_send_request(_master_handle, &req, data.buffer());

    if (err == ESP_OK) {
        ESP_LOGD(MODBUS_TAG, "sendRequest write successful @ 0x%02x", reg_start);
            for (int i=0; i < data.getSize();++i)
                ESP_LOGV(MODBUS_TAG, "0x%02x", data.getByte(i));

    } else {
        ESP_LOGE(MODBUS_TAG, "Request write fail, err = 0x%x (%s).",                      
                            (int)err,                                                             
                            (char*)esp_err_to_name(err));     
    }

}

void ModbusMaster::writeRegisters(uint8_t slave_addr, 
                                uint16_t reg_start, 
                                mb_data data)
{
    setRequest(slave_addr, CMD_WRITE_MULTIPLE_REGISTERS, reg_start, data);
}


