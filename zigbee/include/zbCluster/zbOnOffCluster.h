/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"
#include "zcl\esp_zigbee_zcl_on_off.h"

class ZbOnOffCluster : public ZbCluster
{

public:
    ZbOnOffCluster( bool isClient = false,
                    bool on_off = ESP_ZB_ZCL_ON_OFF_ON_OFF_DEFAULT_VALUE
                    ) 
    {
        esp_zb_on_off_cluster_cfg_t cfg;
        cfg.on_off  = on_off;                                                                    
        _attr_list = esp_zb_on_off_cluster_create(&cfg);

        _init(ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, isClient);
    }

    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        ESP_ERROR_CHECK(esp_zb_on_off_cluster_add_attr(_attr_list, 
                            attr_id, value));
    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK(esp_zb_cluster_list_add_on_off_cluster(cluster_list, _attr_list, 
            isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    }

   
};