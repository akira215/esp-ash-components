/*
  cppads1115
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include <stdio.h>

#include <esp_log.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cppads1115.h"

#include <iostream> // todel

static const char *ADS_TAG = "ADS1115";

void Ads1115::event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    // target pointer address has been copied by the post function
    intrArgs** args = static_cast<intrArgs**>(event_data); 
    
    Cfg_reg* config = (*args)->config;
    uint16_t nMux = static_cast<uint16_t>(config->bit.MUX);
    i2c_master_dev_handle_t dev_handle = (*args)->dev_handle;
    I2c* i2c_master = (*args)->i2c_master;

    // Call the client callback with the values
    double value = (double)((int16_t)(i2c_master->WriteReadWord(dev_handle, reg_conversion)));

    const double fsr[] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};
    const int16_t bits = (1L<<15)-1;

    if ((*args)->voltage)
    {
        value = value * fsr[config->bit.PGA] / (double)bits;
        (*args)->voltage = false;
    }

    (*args)->callback(nMux,value);
}

Ads1115::Ads1115(I2c* i2c_master, Ads1115::addr_t dev_address,uint32_t clk_speed)
{
    esp_log_level_set(ADS_TAG, ADS1115_DEBUG_LEVEL);
    _i2c_master = i2c_master;
    _dev_handle = _i2c_master->addDevice(dev_address, clk_speed);

    _config.bit.OS = 1; // always start conversion
    _config.bit.MUX = MUX_0_1;
    _config.bit.PGA = FSR_2_048;
    _config.bit.MODE = MODE_SINGLE;
    _config.bit.DR = SPS_128;
    _config.bit.COMP_MODE = 0;
    _config.bit.COMP_POL = 0;
    _config.bit.COMP_LAT = 0;
    _config.bit.COMP_QUE = 0b11;

    _useReadyPin = false;
    _readyGpio = GPIO_NUM_NC;

    _cfg_changed = true;
    _intArgs = new intrArgs;
}

Ads1115::~Ads1115()
{
    _i2c_master->removeDevice(_dev_handle);
    delete _intArgs;
}

const Ads1115::Cfg_reg& Ads1115::getConfig()
{
    return _config;
}

void Ads1115::setConfig(const Ads1115::Cfg_reg& config)
{
    _config.reg = config.reg;
    _cfg_changed = true;
}

void Ads1115::setMux(Ads1115::mux_t mux) {
  _config.bit.MUX = mux;
  _cfg_changed = true;
}

void Ads1115::setPga(Ads1115::fsr_t fsr) {
  _config.bit.PGA = fsr;
  _cfg_changed = true;
}

void Ads1115::setMode(Ads1115::mode_t mode) {
  _config.bit.MODE = mode;
  _cfg_changed = true;
}

void Ads1115::setSps(Ads1115::sps_t sps) {
  _config.bit.DR = sps;
  _cfg_changed = true;
}

esp_err_t Ads1115::writeRegister(Ads1115::reg_addr_t reg, Ads1115::reg2Bytes_t data)
{
    esp_err_t err;
    uint8_t write[3]= { reg, data.MSB, data.LSB };

    err = _i2c_master->WriteRegister(_dev_handle, write, 3); 
    return err;
}

Ads1115::reg2Bytes_t Ads1115::readRegister(Ads1115::reg_addr_t reg)
{
    Ads1115::reg2Bytes_t data;

    data.reg = _i2c_master->WriteReadWord(_dev_handle, reg);

    return data;
}

void Ads1115::setReadyPin(const gpio_num_t gpio, ads_handler_t callback) {
    
    esp_err_t err;

    if (_useReadyPin)
        return;         // exit if already configured

    _readyGpio.init(gpio);
    //_readyGpio.disablePulldown();
    _readyGpio.enablePullup();
    _readyGpio.enableInterrupt(GPIO_INTR_NEGEDGE);
   
    _intArgs->callback = callback;
    _intArgs->dev_handle = _dev_handle;
    _intArgs->i2c_master = _i2c_master;

    esp_event_loop_create_default();    // Create System Event Loop
    
    _readyGpio.setEventHandler(&event_handler , &_intArgs); // should send address of pointer, as it will be copied
    
    _useReadyPin = true;
    _config.bit.COMP_QUE = 0b00; // assert after one conversion
    _cfg_changed = true;


    Ads1115::reg2Bytes_t value;
    value.reg = 0x0000;
    err = writeRegister(Ads1115::reg_lo_thresh, value); // set lo threshold to minimum
    if(err) ESP_LOGE(ADS_TAG,"setReadyPin - could not set low threshold: %s",esp_err_to_name(err));
    
    value.reg = 0xFFFF;
    err = writeRegister(Ads1115::reg_hi_thresh, value); // set hi threshold to maximum
    if(err) ESP_LOGE(ADS_TAG,"setReadyPin - could not set high threshold: %s",esp_err_to_name(err));
}

void Ads1115::removeReadyPin() {

    esp_err_t err;

    if (!_useReadyPin)
        return;         // exit if already configured
    
    _intArgs->voltage = false;  // Required to avoid triggering voltage if ready pin is set back again
    _readyGpio.disableInterrupt();
    _intArgs->callback = nullptr;
    _intArgs->dev_handle = nullptr;
    _intArgs->i2c_master = nullptr;

    _readyGpio.clearEventHandlers();
    
    _useReadyPin = false;
    _config.bit.COMP_QUE = 0b00; // assert after one conversion
    _cfg_changed = true;

    // Set back config register to the default value
    Ads1115::reg2Bytes_t value;
    value.reg = 0x8000;
    err = writeRegister(Ads1115::reg_lo_thresh, value); // set lo threshold to minimum
    if(err) ESP_LOGE(ADS_TAG,"setReadyPin - could not set low threshold: %s",esp_err_to_name(err));
    
    value.reg = 0x7FFF;
    err = writeRegister(Ads1115::reg_hi_thresh, value); // set hi threshold to maximum
    if(err) ESP_LOGE(ADS_TAG,"setReadyPin - could not set high threshold: %s",esp_err_to_name(err));
}

uint16_t Ads1115::getRaw(Ads1115::mux_t inputs) {
    const static uint16_t sps[] = {8,16,32,64,128,250,475,860};
    esp_err_t err;
    
    _intArgs->config = &_config;

    _config.bit.MUX = inputs;
    err = writeRegister(reg_configuration , _config.reg); // This trigger the conversion

    if(err) {
        ESP_LOGE(ADS_TAG,"could not write to device: %s",esp_err_to_name(err));
        return 0;
    }

    if(!_useReadyPin) {
        // wait for 1 ms longer than the sampling rate, plus a little bit for rounding
        vTaskDelay((((1000/sps[_config.bit.DR]) + 1) / portTICK_PERIOD_MS)+1);
        
        while(isBusy()){ // Block until read the answer
            ESP_LOGE(ADS_TAG,"Device is busy - retrying in 1ms");
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        reg2Bytes_t res = readRegister(reg_conversion);
        return res.reg;
    }

    return 0;
}

double Ads1115::getVoltage(Ads1115::mux_t inputs) {
    const double fsr[] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};
    const int16_t bits = (1L<<15)-1;
    int16_t raw;

    if(_useReadyPin)
       _intArgs->voltage = true;

    raw = getRaw(inputs);
    return (double)raw * fsr[_config.bit.PGA] / (double)bits;
}


bool Ads1115::isBusy(){
    Cfg_reg cfg_reg;
    cfg_reg.reg = Ads1115::readRegister(reg_configuration);
    if(cfg_reg.bit.OS == 1)
        return false;
    
    return true;
}

