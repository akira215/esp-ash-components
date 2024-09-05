/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"
#include "zcl\esp_zigbee_zcl_identify.h"

class ZbIdentifyCluster : public ZbCluster
{

public:
    ZbIdentifyCluster( bool isClient = false,
                uint16_t  identify_time = ESP_ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE) 
    {
        esp_zb_identify_cluster_cfg_s cfg;
        cfg.identify_time  = identify_time;                                                                     
        _attr_list = esp_zb_identify_cluster_create(&cfg);

        _init(ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY , isClient);
    }

    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        ESP_ERROR_CHECK(esp_zb_identify_cluster_add_attr(_attr_list, 
                            attr_id, value));
    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK(esp_zb_cluster_list_add_identify_cluster(cluster_list, _attr_list, 
            isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    }
    
};