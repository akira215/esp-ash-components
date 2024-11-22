/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"
#include "zcl\esp_zigbee_zcl_metering.h"

#include <esp_log.h> // TODEL development purpose

class ZbMeteringCluster : public ZbCluster
{
public:
    ZbMeteringCluster( bool isClient = false,
                    esp_zb_uint48_t current_summation_delivered = (esp_zb_uint48_t)(0),
                    uint8_t status = ESP_ZB_ZCL_METERING_STATUS_DEFAULT_VALUE,
                    uint8_t uint_of_measure = ESP_ZB_ZCL_METERING_UNIT_OF_MEASURE_DEFAULT_VALUE,
                    uint8_t summation_formatting = ESP_ZB_ZCL_METERING_FORMATTING_SET(0, 8, 0),
                    uint8_t metering_device_type = ESP_ZB_ZCL_METERING_RESERVED
                    ) 
    {
        esp_zb_metering_cluster_cfg_t cfg;
        cfg.current_summation_delivered  = current_summation_delivered;
        cfg.status = status;
        cfg.uint_of_measure = uint_of_measure;     
        cfg.summation_formatting = summation_formatting;
        cfg.metering_device_type = metering_device_type;                                                                 
        _attr_list = esp_zb_metering_cluster_create(&cfg);

        _init(ESP_ZB_ZCL_CLUSTER_ID_METERING, isClient);
    }

    ///@brief Copy constructor
    ZbMeteringCluster(const ZbMeteringCluster& other)
    {
        esp_zb_metering_cluster_cfg_t cfg;

        cfg.current_summation_delivered  = 
                *((esp_zb_uint48_t*)(other.getAttribute((uint16_t)
                    ESP_ZB_ZCL_ATTR_METERING_CURRENT_SUMMATION_DELIVERED_ID)->data_p));
        cfg.status = *((uint8_t*)(other.getAttribute((uint16_t)
                    ESP_ZB_ZCL_ATTR_METERING_STATUS_ID)->data_p));
        cfg.uint_of_measure = *((uint8_t*)(other.getAttribute((uint16_t)
                    ESP_ZB_ZCL_ATTR_METERING_UNIT_OF_MEASURE_ID)->data_p));   
        cfg.summation_formatting = *((uint8_t*)(other.getAttribute((uint16_t)
                    ESP_ZB_ZCL_ATTR_METERING_SUMMATION_FORMATTING_ID)->data_p));  
        cfg.metering_device_type = *((uint8_t*)(other.getAttribute((uint16_t)
                    ESP_ZB_ZCL_ATTR_METERING_METERING_DEVICE_TYPE_ID)->data_p));                                                                  
        
        _attr_list = esp_zb_metering_cluster_create(&cfg);

        _copyAttributes(other);

        _init(ESP_ZB_ZCL_CLUSTER_ID_METERING, other.isClient());
    }

    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        attrType_t attr =  getAttrTypeAndAccess(attr_id);
        ESP_ERROR_CHECK(esp_zb_cluster_add_attr(_attr_list, ESP_ZB_ZCL_CLUSTER_ID_METERING,
                            attr_id, attr.type, attr.access, value));
    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK( esp_zb_cluster_list_add_metering_cluster(cluster_list, _attr_list, 
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
        case ESP_ZB_ZCL_ATTR_METERING_CURRENT_SUMMATION_RECEIVED_ID:
            attr.type = ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM;
            attr.access = ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY;
            break;
        case ESP_ZB_ZCL_ATTR_METERING_MULTIPLIER_ID:
            attr.type = ESP_ZB_ZCL_ATTR_TYPE_U24;
            attr.access = ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY;
            break;
        case ESP_ZB_ZCL_ATTR_METERING_DIVISOR_ID:
            attr.type = ESP_ZB_ZCL_ATTR_TYPE_U24;
            attr.access = ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY;
            break;
        case ESP_ZB_ZCL_ATTR_METERING_DEFAULT_UPDATE_PERIOD_ID:
            attr.type = ESP_ZB_ZCL_ATTR_TYPE_U8;
            attr.access = ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY;
            break;
        default:
            ESP_LOGW("Zb Metering Cluster",
                "Attribute desc not defined : attr id = %d", attr_id); // TODEL development purpose
            break;
        }
        return attr;
    }


};