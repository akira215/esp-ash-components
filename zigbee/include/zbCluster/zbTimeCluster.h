/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"
#include "zcl\esp_zigbee_zcl_time.h"


class ZbTimeCluster : public ZbCluster
{

public:
    ZbTimeCluster( bool isClient = false,
                    uint16_t time = ESP_ZB_ZCL_TIME_TIME_DEFAULT_VALUE,
                    uint16_t time_status = ESP_ZB_ZCL_TIME_TIME_STATUS_DEFAULT_VALUE 
                    ) 
    {
        esp_zb_time_cluster_cfg_t cfg;
        cfg.time  = time;
        cfg.time_status = time_status;                                                                       
        _attr_list = esp_zb_time_cluster_create(&cfg);

        _init(ESP_ZB_ZCL_CLUSTER_ID_TIME, isClient);
    }

    ///@brief Copy constructor
    ZbTimeCluster(const ZbTimeCluster& other)
    {
        esp_zb_time_cluster_cfg_t cfg;
        cfg.time = *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_TIME_TIME_ID)->data_p));
        cfg.time_status = *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_TIME_TIME_STATUS_ID)->data_p));
        
        
        _attr_list = esp_zb_time_cluster_create(&cfg);

        _copyAttributes(other);

        _init(ESP_ZB_ZCL_CLUSTER_ID_TIME, other.isClient());
    }

    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        ESP_ERROR_CHECK(esp_zb_time_cluster_add_attr(_attr_list, 
                            attr_id, value));
    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK(esp_zb_cluster_list_add_time_cluster(cluster_list, _attr_list, 
            isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    }

   
};