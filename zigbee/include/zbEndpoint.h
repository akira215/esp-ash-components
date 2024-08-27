/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once
#include "esp_zigbee_core.h"
#include "cppzb_cluster.h"

/* Attribute values in ZCL string format
 * The string should be started with the length of its own.
 */
#define MANUFACTURER_NAME               "\x05""AKIRA" //TODO del
#define MODEL_IDENTIFIER                "\x08""ASH-Tank"  //TODO del


class ZbEndPoint
{
        esp_zb_cluster_list_t*      _cluster_list;
        esp_zb_attribute_list_t*    _basic_cluster;
        std::vector<ZbCluster*>     _vecCluster;
        esp_zb_endpoint_config_t    _endpoint_config;
        uint8_t                     _id;
    public:

        /// @brief Constructor create the end point
        /// @param id Id of the endpoint
        /// @param device_id Device Id refer to device purpose 
        /// @param profile_id Application Profile (defautl Home Automation)
        /// @param device_version Version of the device (defautl 0)
        ZbEndPoint(uint8_t id,
                    uint16_t device_id,
                    uint16_t profile_id = ESP_ZB_AF_HA_PROFILE_ID,
                    uint32_t device_version = 0);
        ~ZbEndPoint();
        void _init();

        esp_zb_cluster_list_t* getClusterList();
        esp_zb_endpoint_config_t getConfig();

        ZbCluster* createCluster();
        void addCluster(ZbCluster* cluster);

    private:
        //void _init();


};
