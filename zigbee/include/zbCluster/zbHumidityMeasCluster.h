/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"
#include "zcl\esp_zigbee_zcl_humidity_meas.h"

#include <esp_log.h> // TODEL development purpose

class ZbHumidityMeasCluster : public ZbCluster
{
        // ********************************************
        // Typo iussues in ESP_ZB_ZCL_PATTR_RESSURE_MEASUREMENT_MIN_VALUE_DEFAULT_VALUE 
        // and MAX. Shall be corrected in next release
        //  ********************************************
public:
    ZbHumidityMeasCluster( bool isClient = false,
                    int16_t measured_value = ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MEASURED_VALUE_DEFAULT,
                    int16_t min_value = ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MIN_MEASURED_VALUE_DEFAULT,
                    int16_t max_value = ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MAX_MEASURED_VALUE_DEFAULT
                    ) 
    {
        esp_zb_humidity_meas_cluster_cfg_t cfg;
        cfg.measured_value  = measured_value;
        cfg.min_value = min_value;
        cfg.max_value = max_value;     
                                                             
        _attr_list = esp_zb_humidity_meas_cluster_create(&cfg);

        _init(ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, isClient);
    }

    ///@brief Copy constructor
    ZbHumidityMeasCluster(const ZbHumidityMeasCluster& other)
    {
        esp_zb_humidity_meas_cluster_cfg_t cfg;

        cfg.measured_value  = 
                *((uint16_t*)(other.getAttribute((uint16_t)
                    ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID)->data_p));
        cfg.min_value = *((uint16_t*)(other.getAttribute((uint16_t)
                    ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_ID)->data_p));
        cfg.max_value = *((uint16_t*)(other.getAttribute((uint16_t)
                    ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MAX_VALUE_ID)->data_p));   

        _attr_list = esp_zb_humidity_meas_cluster_create(&cfg);

        _copyAttributes(other);

        _init(ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, other.isClient());
    }

    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        //attrType_t attr =  getAttrTypeAndAccess(attr_id);
        //addCustomAttribute(attr_id, value, attr.type, attr.access);

        ESP_ERROR_CHECK(esp_zb_humidity_meas_cluster_add_attr(_attr_list, 
                            attr_id, value));

    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK( esp_zb_cluster_list_add_humidity_meas_cluster(cluster_list, _attr_list, 
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
            ESP_LOGW("ZbHumidityMeasCluster",
                "Attribute desc not defined : attr id = %d", attr_id); // TODEL development purpose
            break;
        }
        return attr;
    }


};