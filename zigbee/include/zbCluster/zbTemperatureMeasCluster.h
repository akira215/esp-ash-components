/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"

class ZbTemperatureMeasCluster : public ZbCluster
{

public:
    ZbTemperatureMeasCluster( bool isClient = false,
                int16_t min_value = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_INVALID,
                int16_t max_value = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_INVALID,
                int16_t measured_value = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_UNKNOWN
                ) 
    {
        esp_zb_temperature_meas_cluster_cfg_s cfg;
        cfg.measured_value  = measured_value;
        cfg.min_value = min_value;                    
        cfg.max_value = max_value;                                                                   
        _attr_list = esp_zb_temperature_meas_cluster_create(&cfg);

        _init(0x0402, isClient);
    }

    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        ESP_ERROR_CHECK(esp_zb_temperature_meas_cluster_add_attr(_attr_list, 
                            attr_id, value));
    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK(esp_zb_cluster_list_add_temperature_meas_cluster(cluster_list, _attr_list, 
            isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    }
    
};