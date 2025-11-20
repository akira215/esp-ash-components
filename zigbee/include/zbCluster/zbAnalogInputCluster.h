/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"
#include "zcl\esp_zigbee_zcl_analog_input.h"


class ZbAnalogInputCluster : public ZbCluster
{

public:
    ZbAnalogInputCluster( bool isClient = false,
                    bool out_of_service = ESP_ZB_ZCL_ANALOG_INPUT_OUT_OF_SERVICE_DEFAULT_VALUE,
                    float_t present_value = 0,
                    uint8_t status_flags = ESP_ZB_ZCL_ANALOG_INPUT_STATUS_FLAG_DEFAULT_VALUE 
                    ) 
    {
        esp_zb_analog_input_cluster_cfg_t cfg;
        cfg.out_of_service  = out_of_service;
        cfg.present_value = present_value;
        cfg.status_flags = status_flags; 

        _attr_list = esp_zb_analog_input_cluster_create(&cfg);

        _init(ESP_ZB_ZCL_CLUSTER_ID_ANALOG_INPUT, isClient);
    }

    ///@brief Copy constructor
    ZbAnalogInputCluster(const ZbAnalogInputCluster& other)
    {
        esp_zb_analog_input_cluster_cfg_t cfg;
        cfg.out_of_service = *((bool*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_ANALOG_VALUE_OUT_OF_SERVICE_ID)->data_p));
        cfg.present_value = *((float_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_ANALOG_VALUE_PRESENT_VALUE_ID)->data_p));
        cfg.status_flags = *((uint8_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_ANALOG_VALUE_STATUS_FLAGS_ID)->data_p));
        
        _attr_list = esp_zb_analog_input_cluster_create(&cfg);

        _copyAttributes(other);

        _init(ESP_ZB_ZCL_CLUSTER_ID_ANALOG_INPUT, other.isClient());
    }

    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        ESP_ERROR_CHECK(esp_zb_analog_input_cluster_add_attr(_attr_list, 
                            attr_id, value));
    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK(esp_zb_cluster_list_add_analog_input_cluster(cluster_list, _attr_list, 
            isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    }
   
};