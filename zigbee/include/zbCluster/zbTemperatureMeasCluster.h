/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"
#include "zcl\esp_zigbee_zcl_temperature_meas.h"

class ZbTemperatureMeasCluster : public ZbCluster
{

public:
    ZbTemperatureMeasCluster( bool isClient = false,
                int16_t min_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MIN_MEASURED_VALUE_DEFAULT,
                int16_t max_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MAX_MEASURED_VALUE_DEFAULT,
                int16_t measured_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MEASURED_VALUE_DEFAULT
                ) 
    {
        esp_zb_temperature_meas_cluster_cfg_t cfg;
        cfg.measured_value  = measured_value;
        cfg.min_value = min_value;                    
        cfg.max_value = max_value;                                                                   
        _attr_list = esp_zb_temperature_meas_cluster_create(&cfg);

        _init(ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, isClient);
    }

    ///@brief Copy constructor
     ZbTemperatureMeasCluster(const ZbTemperatureMeasCluster& other)
    {
        esp_zb_temperature_meas_cluster_cfg_t cfg;
        cfg.measured_value = *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID)->data_p));
        cfg.min_value = *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_ID)->data_p));
        cfg.max_value = *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_ID)->data_p));
        
        
        _attr_list = esp_zb_temperature_meas_cluster_create(&cfg);

        _copyAttributes(other);

        _init(ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, other.isClient());
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