/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include <vector>
#include <any>
#include "esp_zigbee_core.h"

#define MANUFACTURER_NAME               "\x09""ESPRESSIF" // todel

class ZbCluster
{
    public:
        /// @brief Constructor
        /// @param id Id of the endpoint
        ZbCluster(uint16_t id, bool isClient);
        ~ZbCluster();

        void addAttribute(uint16_t attr_id,
                        esp_zb_zcl_attr_type_t type, 
                        esp_zb_zcl_attr_access_t access,
                        std::any value);
        void getAttribute(uint16_t attr_id);


    private:
        void _init();

        

    private:
        esp_zb_zcl_cluster_t cluster;
        std::vector<esp_zb_attribute_list_t> _vecAttrList;
        std::vector<std::any> _attribute_datas;
        //esp_zb_attribute_list_t* _attr_list;
        //esp_zb_attribute_list_t _attr_list;
};
