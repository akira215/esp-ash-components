/*
  cppads1115
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "cppi2c.h"
#include "esp_event.h"
#include "cppgpio.h"

#include <esp_err.h>

/* ESP32 Specifics ------------------------------- */
#define ADS1115_DEBUG_LEVEL ESP_LOG_DEBUG


class Ads1115
{
    public:

        /// @brief Hardware Address configuration of ADS1115 device
        typedef enum : uint16_t
        {
            Addr_Gnd = 0x48,    ///< ADDR pin to GND
            Addr_Vcc = 0x49,    ///< ADDR pin to VCC
            Addr_Sda = 0x4a,    ///< ADDR pin to SDA
            Addr_Scl = 0x4b     ///< ADDR pin to SCL
        } addr_t;

        /// @brief Registers Adress of ADS1115 device
        typedef enum : uint8_t
        {
            reg_conversion      = 0x00, ///< Conversion register
            reg_configuration   = 0x01, ///< Configuration register
            reg_lo_thresh       = 0x02, ///< Low Threshold register
            reg_hi_thresh       = 0x03  ///< High Threshold register
        } reg_addr_t;

        /// @brief Multiplexer configuration. This bit determine the input that will be converted
        typedef enum { // multiplex options
            MUX_0_1 = 0,    ///< + AIN0 | - AIN1 | Default
            MUX_0_3,        ///< + AIN0 | - AIN3 
            MUX_1_3,        ///< + AIN1 | - AIN3 
            MUX_2_3,        ///< + AIN2 | - AIN3 
            MUX_0_GND,      ///< + AIN0 | - GND
            MUX_1_GND,      ///< + AIN1 | - GND
            MUX_2_GND,      ///< + AIN2 | - GND
            MUX_3_GND,      ///< + AIN3 | - GND
        } mux_t;

        /// @brief Programmable Gain Amplifier configuration
        typedef enum { // full-scale resolution options
            FSR_6_144 = 0,          ///< resolution = 0.1875mV/bit
            FSR_4_096,              ///< resolution = 0.125mV/bit  
            FSR_2_048,              ///< resolution = 0.0625mV/bit | Default
            FSR_1_024,              ///< resolution = 0.03125mV/bit
            FSR_0_512,              ///< resolution = 0.015625mV/bit
            FSR_0_256,              ///< resolution = 0.0078125mV/bit
        } fsr_t;

        /// @brief Operating Mode
        typedef enum {
            MODE_CONTINUOUS = 0,
            MODE_SINGLE // default
        } mode_t;

        /// @brief Data rate, expressed in Samples per Second
        typedef enum { // samples per second
            SPS_8 = 0,
            SPS_16,
            SPS_32,
            SPS_64,
            SPS_128, ///< Default
            SPS_250,
            SPS_475,
            SPS_860
        } sps_t;

        /// @brief 2 bytes register value
        typedef union { 
            struct {
                uint8_t LSB;
                uint8_t MSB;
            };
            uint16_t reg;
        } reg2Bytes_t;

        /// @brief Configuration register value
        typedef union { // configuration register
            struct {
                uint16_t COMP_QUE:2;  ///< bits 0..  1  Comparator queue and disable
                uint16_t COMP_LAT:1;  ///< bit  2       Latching Comparator
                uint16_t COMP_POL:1;  ///< bit  3       Comparator Polarity
                uint16_t COMP_MODE:1; ///< bit  4       Comparator Mode
                uint16_t DR:3;        ///< bits 5..  7  Data rate
                uint16_t MODE:1;      ///< bit  8       Device operating mode
                uint16_t PGA:3;       ///< bits 9..  11 Programmable gain amplifier configuration
                uint16_t MUX:3;       ///< bits 12.. 14 Input multiplexer configuration
                uint16_t OS:1;        ///< bit  15      Operational status or single-shot conversion start
            } bit;
            reg2Bytes_t reg;
        } Cfg_reg;

        /// @brief handler type for interrupt on READY pin
        typedef void (*ads_handler_t) (uint16_t, double) ; 

        /// @brief Constructor
        /// @param i2c_master : a pointer to an initilized I2c instance
        /// @param dev_address : ADS1115 Address set by hardware configuration
        /// @param clk_speed : speed of i2c bus. default value is 400kHz
        Ads1115(I2c* i2c_master, addr_t dev_address, uint32_t clk_speed = 400000);
        ~Ads1115();

        /// @brief return the current configuration. This will not read the device.
        const Cfg_reg& getConfig();

        /// @brief set the current configuration. This will not write to the device 
        /// until a new conversion is triggered
        void setConfig(const Cfg_reg& config);

        /// @brief Write to the device register
        /// @param reg : the device register to write in
        /// @param data : a 2 bytes data as all the register of the device are 2B
        esp_err_t writeRegister(reg_addr_t reg, reg2Bytes_t data);

        /// @brief Read to the device register
        /// @param reg : the device register to be read
        /// @return 2 bytes data as all the register of the device are 2B
        reg2Bytes_t readRegister(reg_addr_t reg);

        /// @brief Set the multiplexer for the next conversion
        /// @param mux : multiplexer to be set
        void setMux(mux_t mux);

        /// @brief Set the resolution for the next conversion
        /// @param fsr : resolution to be set
        void setPga(fsr_t fsr);

        /// @brief Set the mode taht will be triggered with the next conversion
        /// @param mode : mode to be set (i.e. one shot or continuous)
        void setMode(mode_t mode);

        /// @brief Set the Sample rate for the next conversion
        /// @param sps : Rate (sample per second)
        void setSps(sps_t sps);

        /// @brief This method trigger a conversion with current config
        /// @param inputs : the multiplexer setting for this conversion
        /// @return the conversion register if Ready pin is not set, otherwise 0
        uint16_t    getRaw(mux_t inputs);

        /// @brief This method trigger a conversion with current config
        /// @param inputs : the multiplexer setting for this conversion
        /// @return the voltage in Volt, if Ready pin is not set, otherwise 0
        double      getVoltage(mux_t inputs);

        /// @brief Test if the device is busy, reading the OS bit of the 
        /// configuration register on the device
        bool isBusy();

        /// @brief configure the device to trigger an output on a pin when conversion is ready
        /// @param gpio gpio on which the ALERT/RDY pin of the ads device is connect to the ESP
        /// @param callback a function that will be called with the result of the conversion.
        /// The call back shall have signature void(uint16_t input, double value)
        void setReadyPin(const gpio_num_t gpio, ads_handler_t callback);
        
        /// @brief remove the ready pin configuration on the device 
        /// Interrupt will also be cleared on the pin, callback will never been called back again
        void removeReadyPin();

    private:
        typedef struct {
            Cfg_reg*                    config;
            bool                        voltage {false};
            i2c_master_dev_handle_t     dev_handle {nullptr};
            I2c*                        i2c_master {nullptr};
            ads_handler_t               callback {nullptr};
        } intrArgs;
        
        double getVoltageFromRaw(uint16_t value);

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

