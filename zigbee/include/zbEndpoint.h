/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "esp_zigbee_core.h"
#include "zbCluster.h"
#include <map>

/// @brief Zigbee Endpoint class
/// store maps of cluster and cluster list as per SDK requirements
class ZbEndPoint
{
        std::map<uint16_t,ZbCluster*> _serverClusterMap;
        std::map<uint16_t,ZbCluster*> _clientClusterMap;
        
        esp_zb_cluster_list_t*      _cluster_list;  
        esp_zb_endpoint_config_t    _endpoint_config;
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

        /// @brief retrieve the cluster list as per sdk
        /// @return the cluster list
        esp_zb_cluster_list_t* getClusterList();

        /// @brief retrieve the endpoint config as per sdk
        /// @return the endpoint config
        esp_zb_endpoint_config_t getConfig();
        
        /// @brief Get Id of the endpoint
        /// @return the id
        uint8_t getId();

        /// @brief Add a cluster to the endpoint
        /// note that a cluster shall be only in one endpoint (App shall provide a copy if required)
        /// @param cluster cluster to be added
        void addCluster(ZbCluster* cluster);

        /// @brief retrieve a pointer to a cluser
        /// @param id id of the cluster
        /// @return pointer on the cluster
        ZbCluster* getCluster(uint16_t id, bool isClient);

    private:


};
