/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"
#include "zcl/esp_zigbee_zcl_power_config.h"

class ZbPowerCfgCluster : public ZbCluster
{

public:
    /// @brief @brief Power Config Cluster 0x0001
    /// @param isClient 
    /// @param main_voltage unit is 100mV
    /// @param main_freq = 0.5 x measured frequency in Hz. shall be between 1 and 0xfh (2hz to 506Hz) 0 for DC
    /// @param main_alarm_mask 
    /// @param main_voltage_min 
    /// @param main_voltage_max 
    /// @param main_voltage_dwell 
   ZbPowerCfgCluster( bool isClient = false,
                    uint16_t    main_voltage = 0,
                    uint8_t     main_freq = 0,
                    uint8_t     main_alarm_mask = ESP_ZB_ZCL_POWER_CONFIG_MAINS_ALARM_MASK_DEFAULT_VALUE, 
                    uint16_t    main_voltage_min = ESP_ZB_ZCL_POWER_CONFIG_MAINS_VOLTAGE_MIN_THRESHOLD_DEFAULT_VALUE,
                    uint16_t    main_voltage_max = ESP_ZB_ZCL_POWER_CONFIG_MAINS_VOLTAGE_MAX_THRESHOLD_DEFAULT_VALUE,
                    uint16_t    main_voltage_dwell = ESP_ZB_ZCL_POWER_CONFIG_MAINS_DWELL_TRIP_POINT_DEFAULT_VALUE
                    ) 
    {
        esp_zb_power_config_cluster_cfg_t cfg;
        cfg.main_voltage  = main_voltage;
        cfg.main_freq = main_freq;
        cfg.main_alarm_mask = main_alarm_mask;
        cfg.main_voltage_min = main_voltage_min;
        cfg.main_voltage_max =  main_voltage_max;
        cfg.main_voltage_dwell =  main_voltage_dwell;

        _attr_list = esp_zb_power_config_cluster_create(&cfg);

        _init(ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG, isClient);
    }

    ///@brief Copy constructor
    ZbPowerCfgCluster(const ZbPowerCfgCluster& other)
    {
        esp_zb_power_config_cluster_cfg_t cfg;
        cfg.main_voltage  = *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_VOLTAGE_ID)->data_p));
        cfg.main_freq =  *((uint8_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_FREQUENCY_ID)->data_p));
        cfg.main_alarm_mask = *((uint8_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_ALARM_MASK_ID)->data_p));
        cfg.main_voltage_min = *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_VOLTAGE_MIN_THRESHOLD)->data_p));
        cfg.main_voltage_max =  *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_VOLTAGE_MAX_THRESHOLD)->data_p));
        cfg.main_voltage_dwell =  *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_POWER_CONFIG_MAINS_DWELL_TRIP_POINT )->data_p));
        
        
        _attr_list = esp_zb_power_config_cluster_create(&cfg);

        _copyAttributes(other);

        _init(ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG, other.isClient());
    }


    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        ESP_ERROR_CHECK(esp_zb_power_config_cluster_add_attr(_attr_list, 
                            attr_id, value));
    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK(esp_zb_cluster_list_add_power_config_cluster(cluster_list, _attr_list, 
            isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    }

   
};