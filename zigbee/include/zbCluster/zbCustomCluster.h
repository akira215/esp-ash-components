/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"
//#include "zcl\esp_zigbee_zcl_basic.h"

class ZbCustomCluster : public ZbCluster
{

public:
    /// @brief Custom Cluster constructor
    /// @param isClient 
    /// @param clusterId shall be > 0x8000
    ZbCustomCluster( bool isClient = false, uint16_t clusterId ) 
    {                                                                     
        _attr_list = esp_zb_zcl_attr_list_create(clusterId);

        _init(clusterId, isClient);
    }

    ///@brief Copy constructor
    ZbCustomCluster(const ZbCustomCluster& other)
    {
        _attr_list = esp_zb_zcl_attr_list_create(other.getId());

        _copyAttributes(other);

        _init(other.getId(), other.isClient());
    }

    // Implementation only to avoid Pure virtual 
    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        addCustomAttribute(attr_id, value, 
                ESP_ZB_ZCL_ATTR_TYPE_NULL, ESP_ZB_ZCL_ATTR_ACCESS_INTERNAL);
    }

    virtual void addCustomAttribute(uint16_t attr_id, void* value, 
                uint8_t attr_type, uint8_t attr_access, uint16_t manuf_code = 0)
    {
        ESP_ERROR_CHECK(esp_zb_custom_cluster_add_custom_attr(_attr_list, 
                            attr_id, attr_type, attr_access, value));

    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK(esp_zb_cluster_list_add_custom_cluster(cluster_list, _attr_list, 
            isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    }

   
};