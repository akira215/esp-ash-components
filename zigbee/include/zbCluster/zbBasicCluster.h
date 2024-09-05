/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"
#include "zcl\esp_zigbee_zcl_basic.h"

class ZbBasicCluster : public ZbCluster
{

public:
    ZbBasicCluster( bool isClient = false,
                    uint8_t zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE,
                    uint8_t power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_DEFAULT_VALUE 
                    ) 
    {
        esp_zb_basic_cluster_cfg_s cfg;
        cfg.zcl_version  = zcl_version;
        cfg.power_source = power_source;                                                                       
        _attr_list = esp_zb_basic_cluster_create(&cfg);

        _init(ESP_ZB_ZCL_CLUSTER_ID_BASIC, isClient);
    }

    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(_attr_list, 
                            attr_id, value));
    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK(esp_zb_cluster_list_add_basic_cluster(cluster_list, _attr_list, 
            isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    }

   
};