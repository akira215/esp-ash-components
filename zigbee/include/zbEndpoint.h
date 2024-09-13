/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once
#include "zbDebug.h" // TODEL

#include "esp_zigbee_core.h"
#include "zbCluster.h"
#include <map>

/* Attribute values in ZCL string format
 * The string should be started with the length of its own.
 */
#define MANUFACTURER_NAME               "\x05""AKIRA" //TODO del
#define MODEL_IDENTIFIER                "\x08""ASH-Tank"  //TODO del


class ZbEndPoint
{
        /// @brief only server clusters are stored here
        std::map<uint16_t,ZbCluster*> _serverClusterMap;
        std::map<uint16_t,ZbCluster*> _clientClusterMap;
        
        esp_zb_cluster_list_t*      _cluster_list;
        esp_zb_attribute_list_t*    _basic_cluster; //TODEL Debug only
        
        esp_zb_endpoint_config_t    _endpoint_config;
        esp_zb_identify_cluster_cfg_t _identify_cfg; //TODEL
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


        esp_zb_cluster_list_t* getClusterList();
        esp_zb_endpoint_config_t getConfig();
        
        /// @brief Get Id of the endpoint
        /// @return the id
        uint8_t getId();

        void addCluster(ZbCluster* cluster);

        /// @brief retrieve a pointer to a cluser
        /// @param id id of the cluster
        /// @return pointer on the cluster
        ZbCluster* getCluster(uint16_t id, bool isClient);

    private:

        void initZbCluster(); //TODO del debug only
        //void printAttr(esp_zb_attribute_list_t* attrList); //TODO del debug only


};
