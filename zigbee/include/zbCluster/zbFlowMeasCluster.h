/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"
#include "zcl\esp_zigbee_zcl_flow_meas.h"

#include <esp_log.h> // TODEL development purpose

class ZbFlowMeasCluster : public ZbCluster
{
public:
    ZbFlowMeasCluster( bool isClient = false,
                    uint16_t measured_value = ESP_ZB_ZCL_FLOW_MEASUREMENT_MEASURED_VALUE_DEFAULT,
                    uint16_t min_value = ESP_ZB_ZCL_FLOW_MEASUREMENT_MIN_MEASURED_VALUE_DEFAULT,
                    uint16_t max_value = ESP_ZB_ZCL_FLOW_MEASUREMENT_MAX_MEASURED_VALUE_DEFAULT
                    ) 
    {
        esp_zb_flow_meas_cluster_cfg_t cfg;
        cfg.measured_value  = measured_value;
        cfg.min_value = min_value;
        cfg.max_value = max_value;     
                                                             
        _attr_list = esp_zb_flow_meas_cluster_create(&cfg);
    /*    
        // BUG of ZCL library, shall not be here
        ESP_ERROR_CHECK(esp_zb_flow_meas_cluster_add_attr(_attr_list, 
                            attr_id, value));
    */
        //TODEL
        std::cout << std::hex << "ZbFlowMeasCluster Constructor - attr_list " << _attr_list << std::endl;
        std::cout << std::hex << "ZbFlowMeasCluster Constructor - attr_list->next " << _attr_list->next << std::endl;

        _init(ESP_ZB_ZCL_CLUSTER_ID_FLOW_MEASUREMENT, isClient);
    }

    ///@brief Copy constructor
    ZbFlowMeasCluster(const ZbFlowMeasCluster& other)
    {
        esp_zb_flow_meas_cluster_cfg_t cfg;

        // ********************************************
        // In ZCL spec rev 8 attributes are all uint16_t, 
        // nevertheless int esp_zigbee_type.h v1.6.6, they are int16_t
        //  ********************************************
        cfg.measured_value  = 
                *((int16_t*)(other.getAttribute((uint16_t)
                    ESP_ZB_ZCL_ATTR_FLOW_MEASUREMENT_VALUE_ID)->data_p));
        cfg.min_value = *((int16_t*)(other.getAttribute((uint16_t)
                    ESP_ZB_ZCL_ATTR_FLOW_MEASUREMENT_MIN_VALUE_ID)->data_p));
        cfg.max_value = *((int16_t*)(other.getAttribute((uint16_t)
                    ESP_ZB_ZCL_ATTR_FLOW_MEASUREMENT_MAX_VALUE_ID)->data_p));   

        _attr_list = esp_zb_flow_meas_cluster_create(&cfg);

        _copyAttributes(other);

        _init(ESP_ZB_ZCL_CLUSTER_ID_FLOW_MEASUREMENT, other.isClient());
    }

    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        //attrType_t attr =  getAttrTypeAndAccess(attr_id);
        //addCustomAttribute(attr_id, value, attr.type, attr.access);

        ESP_ERROR_CHECK(esp_zb_flow_meas_cluster_add_attr(_attr_list, 
                            attr_id, value));

    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK( esp_zb_cluster_list_add_flow_meas_cluster(cluster_list, _attr_list, 
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
            ESP_LOGW("Zb FlowMeas Cluster",
                "Attribute desc not defined : attr id = %d", attr_id); // TODEL development purpose
            break;
        }
        return attr;
    }


};