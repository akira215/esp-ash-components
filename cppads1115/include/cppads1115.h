#pragma once

#include "cppi2c.h"
#include "esp_event.h"
#include "cppgpio.h"

#include <esp_err.h>

/* ESP32 Specifics ------------------------------- */
#define ADS1115_DEBUG_LEVEL ESP_LOG_DEBUG


//#define BYTES_INT(A,B) (((A << 8) & 0xFF00) | B)

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

        typedef void (*ads_handler_t) (uint16_t, int16_t) ; 

        typedef struct {
            mux_t                       mux {MUX_0_1};
            i2c_master_dev_handle_t     dev_handle {nullptr};
            I2c*                        i2c_master {nullptr};
            ads_handler_t               callback {nullptr};
        } intrArgs;


        Ads1115(I2c* i2c_master, addr_t dev_address, uint32_t clk_speed = 400000);
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

        ///
        /// @brief configure the device to trigger an output on a pin when conversion is ready
        /// @param gpio gpio on which the ALERT/RDY pin of the ads device is connect to the ESP
        /// @param callback a function that will be called with the result of the conversion.
        /// The call back shall have signature void(uint16_t input, int16_t value)
        void setReadyPin(const gpio_num_t gpio, ads_handler_t callback);

        void removeReadyPin();

    private:
        static void event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);    

    private:
        i2c_master_dev_handle_t _dev_handle;
        I2c*                    _i2c_master;
        Cfg_reg                 _config;
        bool                    _cfg_changed;
        bool                    _useReadyPin;
        GpioInput               _readyGpio;
        intrArgs*               _intArgs;
};

