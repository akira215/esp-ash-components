/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"

#include <esp_log.h> // TODEL development purpose

class ZbCustomCluster : public ZbCluster
{

public:
    /// @brief Custom Cluster constructor
    /// @param isClient 
    /// @param clusterId shall be > 0x8000
    ZbCustomCluster( bool isClient = false, uint16_t clusterId = 0xfc00 ) 
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

    /// @brief Custom attribute 
    /// @param attr_id attribute id in the cluster
    /// @param attr_type refer to esp_zb_zcl_attr_type_t
    /// @param attr_access  refer to esp_zb_zcl_attr_access_t
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

private:
    /// @brief get the type and access according to ZCL specification
    /// @param attr_id the id of the attribute
    /// @return a struct with both data, to avoid 2 long switch testing
    attrType_t getAttrTypeAndAccess(uint16_t attr_id)
    {
        attrType_t attr = { ESP_ZB_ZCL_ATTR_TYPE_INVALID, 
                        ESP_ZB_ZCL_ATTR_ACCESS_INTERNAL };
        switch(attr_id)
        {
        default:
            ESP_LOGW("ZbPressureMeasCluster",
                "Attribute desc not defined : attr id = %d", attr_id); // TODEL development purpose
            break;
        }
        return attr;
    }
   
};