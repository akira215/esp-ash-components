#include <stdio.h>
//#include <driver/i2c.h>
#include <esp_log.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cppads1115.h"

static const char *ADS_TAG = "ADS1115";

//ESP_EVENT_DEFINE_BASE(INPUT_EVENTS);

void IRAM_ATTR Ads1115::isr_handler(void* arg) {

    int32_t pin = 0;
    // = static_cast<int32_t>((static_cast<intrArgs *>(arg))->readyPin);
    //mux_t inputs = (static_cast<intrArgs *>(arg))->inputs;

    esp_event_isr_post(INPUT_EVENTS, pin, /*(void*) inputs*/nullptr, /*sizeof(inputs)*/0, nullptr);
}

Ads1115::Ads1115(I2c* i2c_master, Ads1115::addr_t dev_address)
{
    esp_log_level_set(ADS_TAG, ADS1115_DEBUG_LEVEL);
    _i2c_master = i2c_master;
    _dev_handle = _i2c_master->addDevice(dev_address,400000);

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
}

Ads1115::~Ads1115()
{
    _i2c_master->removeDevice(_dev_handle);
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

uint16_t Ads1115::getRaw(Ads1115::mux_t inputs) {
    const static uint16_t sps[] = {8,16,32,64,128,250,475,860};
    esp_err_t err;
    
    _inputs = inputs;

    if(_useReadyPin) {
        //err = esp_event_handler_instance_register(INPUT_EVENTS, _intrArgs.readyPin, _intrArgs.callback, /*(void*)&_intrArgs.inputs**/0, nullptr);
        //gpio_isr_handler_add(_intrArgs.readyPin, isr_handler, (void*)&_intrArgs);
    }
    
    
    _config.bit.MUX = inputs;
    err = writeRegister(reg_configuration , _config.reg); // This trigger the conversion

    if(err) {
        ESP_LOGE(ADS_TAG,"could not write to device: %s",esp_err_to_name(err));
        if(_useReadyPin) {
            //gpio_isr_handler_remove(_intrArgs.readyPin);
            
        }
        return 0;
    }

    if(_useReadyPin) {
        /*
        xQueueReceive(ads->rdy_pin.gpio_evt_queue, &tmp, portMAX_DELAY);
        gpio_isr_handler_remove(ads->rdy_pin.pin);
        */
    }
    else {
        // wait for 1 ms longer than the sampling rate, plus a little bit for rounding
        vTaskDelay((((1000/sps[_config.bit.DR]) + 1) / portTICK_PERIOD_MS)+1);
        bool test = isBusy();
        if(test)
            ESP_LOGE(ADS_TAG,"Device is busy");
        reg2Bytes_t res = readRegister(reg_conversion);
        return res.reg;
    }

    return 0;
}

double Ads1115::getVoltage(Ads1115::mux_t inputs) {
  const double fsr[] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256};
  const int16_t bits = (1L<<15)-1;
  int16_t raw;

  raw = getRaw(inputs);
  return (double)raw * fsr[_config.bit.PGA] / (double)bits;
}


uint16_t Ads1115::getRaw() {
  const static uint16_t sps[] = {8,16,32,64,128,250,475,860};
  const static uint8_t len = 2;
  uint8_t data[2];
  esp_err_t err;
  bool tmp; // temporary bool for reading from queue
/*
  if(ads->rdy_pin.in_use) {
    gpio_isr_handler_add(ads->rdy_pin.pin, gpio_isr_handler, (void*)ads->rdy_pin.gpio_evt_queue);
    xQueueReset(ads->rdy_pin.gpio_evt_queue);
  }
  */

 
    // see if we need to send configuration data
    if((_config.bit.MODE==MODE_SINGLE) || (_cfg_changed)) { // if it's single-ended or a setting changed
        err = writeRegister(reg_configuration , _config.reg); 
        if(err) {
            ESP_LOGE(ADS_TAG,"could not write to device: %s",esp_err_to_name(err));
            /*
            if(ads->rdy_pin.in_use) {
                gpio_isr_handler_remove(ads->rdy_pin.pin);
                xQueueReset(ads->rdy_pin.gpio_evt_queue);
            }
            */
            return 0;
        }
        _cfg_changed = false; // say that the data is unchanged now
    }

    if(false/*ads->rdy_pin.in_use*/) {
        /*
        xQueueReceive(ads->rdy_pin.gpio_evt_queue, &tmp, portMAX_DELAY);
        gpio_isr_handler_remove(ads->rdy_pin.pin);
        */
    }
    else {
        // wait for 1 ms longer than the sampling rate, plus a little bit for rounding
        vTaskDelay((((1000/sps[_config.bit.DR]) + 1) / portTICK_PERIOD_MS)+1);
        bool test = isBusy();
        if(test)
            ESP_LOGE(ADS_TAG,"Device is busy");
    }

    reg2Bytes_t res = readRegister(reg_conversion);
    return res.reg;
}

bool Ads1115::isBusy(){
    Cfg_reg cfg_reg;
    cfg_reg.reg = Ads1115::readRegister(reg_configuration);
    if(cfg_reg.bit.OS == 1)
        return false;
    
    return true;
}

void Ads1115::setReadyPin(const gpio_num_t gpio, esp_event_handler_t callback) {
    //gpio_config_t io_conf;
    esp_err_t err;

    if (_useReadyPin)
        return;         // exit if already configured

    //_readyGpio.enablePullup();
    _readyGpio.init(gpio);
    //_readyGpio.disablePulldown();
    _readyGpio.enablePullup();
    _readyGpio.enableInterrupt(GPIO_INTR_NEGEDGE);

    esp_event_loop_create_default();    // Create System Event Loop
    _readyGpio.setEventHandler(callback,&_inputs,sizeof(_inputs));

    
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
    /*
    
    gpio_config_t io_conf;
    esp_err_t err;

    if (_useReadyPin)
        return;         // exit if already configured

    io_conf.intr_type = GPIO_INTR_NEGEDGE; // positive to negative (pulled down)
    io_conf.pin_bit_mask =  1ULL << gpio;
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf); // set gpio configuration

    gpio_set_pull_mode(gpio, GPIO_PULLUP_ONLY);

    //ads->rdy_pin.gpio_evt_queue = xQueueCreate(1, sizeof(bool));
    gpio_install_isr_service(0);

    _useReadyPin = true;
    _readyPin = gpio;
    _config.bit.COMP_QUE = 0b00; // assert after one conversion
    _cfg_changed = true;
    
    Ads1115::reg2Bytes_t value;
    value.reg = 0x0000;
    err = writeRegister(Ads1115::reg_lo_thresh, value); // set lo threshold to minimum
    if(err) ESP_LOGE(ADS_TAG,"setReadyPin - could not set low threshold: %s",esp_err_to_name(err));
    
    value.reg = 0xFFFF;
    err = writeRegister(Ads1115::reg_hi_thresh, value); // set hi threshold to maximum
    if(err) ESP_LOGE(ADS_TAG,"setReadyPin - could not set high threshold: %s",esp_err_to_name(err));
*/

}

/*

esp_err_t ADS1115_set_ready_pin()
{
    esp_err_t r = ESP_OK;
    r +=ADS1115_set_lo_thresh(ADS1115_RDY_LO_THRESH_VALUE); 
    r +=ADS1115_set_hi_thresh(ADS1115_RDY_HI_THRESH_VALUE);

    ads_cfg.reg_cfg &= 0xFFFC; //Ensure that COMP_QUE is not 0b11 to keep alive RDY pin setting it to 0b00

    return r;
}

*/
