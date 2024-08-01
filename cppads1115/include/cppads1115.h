#pragma once

#include "cppi2c.h"
//#include "driver/gpio.h"
#include "esp_event.h"
#include "cppgpio.h"

#include <esp_err.h>

/* ESP32 Specifics ------------------------------- */
#define ADS1115_DEBUG_LEVEL ESP_LOG_DEBUG

/* Device Address -------------------------------- */
//#define ADS1115_DEF_DEV_ADR 0b1001000 >> 1 // 0x48 
/*
#define ADS111X_ADDR_GND      0x48 //!< I2C device address with ADDR pin connected to ground
#define ADS111X_ADDR_VCC      0x49 //!< I2C device address with ADDR pin connected to VCC
#define ADS111X_ADDR_SDA      0x4a //!< I2C device address with ADDR pin connected to SDA
#define ADS111X_ADDR_SCL      0x4b //!< I2C device address with ADDR pin connected to SCL
*/
/* Address Pointer Register ---------------------- */
/*
#define ADS1115_REG_CONV                  0x00
#define ADS1115_REG_CFG                   0x01
#define ADS1115_REG_LO_THRESH             0x02
#define ADS1115_REG_HI_THRESH             0x03
*/
/* Conversion Register --------------------------- */
#define ADS1115_CONV_RESULT_MASK          0xFFF0
#define ADS1115_CONV_RESULT               0x0000

/* Config Register ------------------------------- */
/*
// Most Significant Byte //
#define ADS1115_CFG_MS_OS_ACTIVE          0x8000  // when reading, 1 is busy, 0 is ready. When writing, starts a covnersion from powerdown state
#define ADS1115_CFG_MS_MUX_OMASK          0x8F00  // mask out

// inline functions pass these defintions, so are shifted instead of an uint16 argument
#define ADS1115_CFG_MS_MUX_DIFF_AIN0_AIN1 0x00    // default
#define ADS1115_CFG_MS_MUX_DIFF_AIN0_AIN3 0x10 
#define ADS1115_CFG_MS_MUX_DIFF_AIN1_AIN3 0x20  
#define ADS1115_CFG_MS_MUX_DIFF_AIN2_AIN3 0x30  
#define ADS1115_CFG_MS_MUX_SNGL_AIN0_GND  0x40  
#define ADS1115_CFG_MS_MUX_SNGL_AIN1_GND  0x50  
#define ADS1115_CFG_MS_MUX_SNGL_AIN2_GND  0x60  
#define ADS1115_CFG_MS_MUX_SNGL_AIN3_GND  0x70 

#define ADS1115_CFG_MS_PGA_FSR_6_144V     0x0000   // only expresses full-scale range of ADC scaling. Do not apply more than VDD + 0.3 V to the analog inputs
#define ADS1115_CFG_MS_PGA_FSR_4_096V     0x0200   // only expresses full-scale range of ADC scaling. Do not apply more than VDD + 0.3 V to the analog inputs
#define ADS1115_CFG_MS_PGA_FSR_2_048V     0x0600   // default
#define ADS1115_CFG_MS_PGA_FSR_1_024V     0x0800  
#define ADS1115_CFG_MS_PGA_FSR_0_512V     0x0C00 
#define ADS1115_CFG_MS_PGA_FSR_0_256V     0x0E00  
#define ADS1115_CFG_MS_MODE_CON           0x0000   // Continuous Mode
#define ADS1115_CFG_MS_MODE_SS            0x0100   // Single shot Mode

// Least Significant Byte //
#define ADS1115_CFG_LS_DR_128SPS          0x00
#define ADS1115_CFG_LS_DR_250SPS          0x20
#define ADS1115_CFG_LS_DR_490SPS          0x40
#define ADS1115_CFG_LS_DR_920SPS          0x60
#define ADS1115_CFG_LS_DR_1600SPS         0x80   // default
#define ADS1115_CFG_LS_DR_2400SPS         0xA0
#define ADS1115_CFG_LS_DR_3300SPS         0xC0

#define ADS1115_CFG_LS_COMP_MODE_TRAD     0x00   // default
#define ADS1115_CFG_LS_COMP_MODE_WIN      0x10
#define ADS1115_CFG_LS_COMP_POL_LOW       0x00   // default
#define ADS1115_CFG_LS_COMP_POL_HIGH      0x08
#define ADS1115_CFG_LS_COMP_LAT_NON       0x00   // default
#define ADS1115_CFG_LS_COMP_LAT_LATCH     0x04
#define ADS1115_CFG_LS_COMP_QUE_ONE       0x00   // assert after x conversion..
#define ADS1115_CFG_LS_COMP_QUE_TWO       0x01
#define ADS1115_CFG_LS_COMP_QUE_FOUR      0x02
#define ADS1115_CFG_LS_COMP_QUE_DIS       0x03   // default; disable comparator and set ALERT/RDY pin to high-impedance
*/
/* Lo_tresh / Hi_thresh Register ---------------- */
#define ADS1115_THRESH_VALUE_MASK         0xFFF0
#define ADS1115_RDY_HI_THRESH_VALUE       0x8000 // Set the most-significant bit of the Hi_thresh register to 1
#define ADS1115_RDY_LO_THRESH_VALUE       0x0000 // the most-significant bit of Lo_thresh register to 0 to enable the pin as a conversion ready pin

#define BYTES_INT(A,B) (((A << 8) & 0xFF00) | B)

//ESP_EVENT_DECLARE_BASE(INPUT_EVENTS);

class Ads1115
{
    public:
        typedef enum : uint16_t
        {
            Addr_Gnd = 0x48,
            Addr_Vcc = 0x49,
            Addr_Sda = 0x4a,
            Addr_Scl = 0x4b
        } addr_t;

        typedef enum : uint8_t
        {
            reg_conversion      = 0x00,
            reg_configuration   = 0x01,
            reg_lo_thresh       = 0x02,
            reg_hi_thresh       = 0x03
        } reg_addr_t;

        typedef enum { // multiplex options
            MUX_0_1 = 0, // default
            MUX_0_3,
            MUX_1_3,
            MUX_2_3,
            MUX_0_GND,
            MUX_1_GND,
            MUX_2_GND,
            MUX_3_GND,
        } mux_t;

        typedef enum { // full-scale resolution options
            FSR_6_144 = 0,          // resolution = 0.1875mV/bit
            FSR_4_096,              // resolution = 0.125mV/bit  
            FSR_2_048, // default   // resolution = 0.0625mV/bit
            FSR_1_024,              // resolution = 0.03125mV/bit
            FSR_0_512,              // resolution = 0.015625mV/bit
            FSR_0_256,              // resolution = 0.0078125mV/bit
        } fsr_t;

        typedef enum {
            MODE_CONTINUOUS = 0,
            MODE_SINGLE // default
        } mode_t;

        typedef enum { // samples per second
            SPS_8 = 0,
            SPS_16,
            SPS_32,
            SPS_64,
            SPS_128, // default
            SPS_250,
            SPS_475,
            SPS_860
        } sps_t;

        typedef union { 
            struct {
                uint8_t LSB;
                uint8_t MSB;
            };
            uint16_t reg;
        } reg2Bytes_t;

        typedef union { // configuration register
            struct {
                uint16_t COMP_QUE:2;  // bits 0..  1  Comparator queue and disable
                uint16_t COMP_LAT:1;  // bit  2       Latching Comparator
                uint16_t COMP_POL:1;  // bit  3       Comparator Polarity
                uint16_t COMP_MODE:1; // bit  4       Comparator Mode
                uint16_t DR:3;        // bits 5..  7  Data rate
                uint16_t MODE:1;      // bit  8       Device operating mode
                uint16_t PGA:3;       // bits 9..  11 Programmable gain amplifier configuration
                uint16_t MUX:3;       // bits 12.. 14 Input multiplexer configuration
                uint16_t OS:1;        // bit  15      Operational status or single-shot conversion start
            } bit;
            //uint16_t reg;
            reg2Bytes_t reg;
        } Cfg_reg;


        Ads1115(I2c* i2c_master, addr_t dev_address);
        ~Ads1115();

        const Cfg_reg& getConfig();
        void setConfig(const Cfg_reg& config);

        esp_err_t writeRegister(reg_addr_t reg, reg2Bytes_t data);
        reg2Bytes_t readRegister(reg_addr_t reg);

        void setMux(mux_t mux);
        void setPga(fsr_t fsr);
        void setMode(mode_t mode);
        void setSps(sps_t sps);

        uint16_t    getRaw();
        uint16_t    getRaw(mux_t inputs);
        double      getVoltage(mux_t inputs);

        bool isBusy();
        void setReadyPin(const gpio_num_t gpio, esp_event_handler_t callback);
        void removeReadyPin();

        static void IRAM_ATTR isr_handler(void* arg);

        esp_event_loop_handle_t _loop_handle {}; // check to move private

    private:
        i2c_master_dev_handle_t _dev_handle;
        I2c*                    _i2c_master;
        Cfg_reg                 _config;
        bool                    _cfg_changed;
        bool                    _useReadyPin;
        mux_t                   _inputs;
        GpioInput               _readyGpio;
};


/*
esp_err_t ADS1115_initiate(uint8_t dev_addr, uint16_t reg_cfg);
esp_err_t ADS1115_set_config(uint8_t dev_addr, uint16_t reg_cfg);

int16_t ADS1115_get_conversion();
bool ADS1115_get_conversion_state();

esp_err_t ADS1115_request_by_definition(uint8_t def);
esp_err_t ADS1115_set_thresh_by_definition(uint8_t thresh, uint16_t val);

esp_err_t ADS1115_set_ready_pin(); // Configure the ALERT/RDY pin as Ready

static inline esp_err_t ADS1115_set_lo_thresh(uint16_t value) { return ADS1115_set_thresh_by_definition(ADS1115_REG_LO_THRESH, value); };
static inline esp_err_t ADS1115_set_hi_thresh(uint16_t value) { return ADS1115_set_thresh_by_definition(ADS1115_REG_HI_THRESH, value); };

static inline esp_err_t ADS1115_request_single_ended_AIN0() { return ADS1115_request_by_definition(ADS1115_CFG_MS_MUX_SNGL_AIN0_GND); };
static inline esp_err_t ADS1115_request_single_ended_AIN1() { return ADS1115_request_by_definition(ADS1115_CFG_MS_MUX_SNGL_AIN1_GND); };
static inline esp_err_t ADS1115_request_single_ended_AIN2() { return ADS1115_request_by_definition(ADS1115_CFG_MS_MUX_SNGL_AIN2_GND); };
static inline esp_err_t ADS1115_request_single_ended_AIN3() { return ADS1115_request_by_definition(ADS1115_CFG_MS_MUX_SNGL_AIN3_GND); };

static inline esp_err_t ADS1115_request_diff_AIN0_AIN1() { return ADS1115_request_by_definition(ADS1115_CFG_MS_MUX_DIFF_AIN0_AIN1); };
static inline esp_err_t ADS1115_request_diff_AIN0_AIN3() { return ADS1115_request_by_definition(ADS1115_CFG_MS_MUX_DIFF_AIN0_AIN3); };
static inline esp_err_t ADS1115_request_diff_AIN1_AIN3() { return ADS1115_request_by_definition(ADS1115_CFG_MS_MUX_DIFF_AIN1_AIN3); };
static inline esp_err_t ADS1115_request_diff_AIN2_AIN3() { return ADS1115_request_by_definition(ADS1115_CFG_MS_MUX_DIFF_AIN2_AIN3); };
*/