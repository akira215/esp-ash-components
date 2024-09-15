/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  Zigbee End Point Class
*/

#include "zbEndpoint.h"


#include "ha/esp_zigbee_ha_standard.h" //TODEL

#include <iostream>

ZbEndPoint::ZbEndPoint(uint8_t id, uint16_t device_id,
                    uint16_t profile_id, uint32_t device_version)
{

    _cluster_list = esp_zb_zcl_cluster_list_create();


    _endpoint_config.endpoint = id;
    _endpoint_config.app_device_id = device_id;
    _endpoint_config.app_profile_id = profile_id;
    _endpoint_config.app_device_version = device_version;


}


ZbEndPoint::~ZbEndPoint()
{

}

void ZbEndPoint::initZbCluster()
{
    /*
    ZbCluster basicTest(0x0000,false);
    basicTest.addAttribute(0x0000,ESP_ZB_ZCL_ATTR_TYPE_U8,
                        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, ZbData<uint8_t>(0x08));
    basicTest.addAttribute(0x0007,ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM,
                        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, ZbData<uint8_t>(0x01));
                        
    basicTest.addAttribute(0x0004,ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,
                        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, ZbData<const char*>(MANUFACTURER_NAME));
    basicTest.addAttribute(0x0005,ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,
                        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, ZbData<const char*>(MODEL_IDENTIFIER));

    ZbCluster identifyServer(0x0003,false);
    identifyServer.addAttribute(0x0000,ESP_ZB_ZCL_ATTR_TYPE_U16,
                        ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, 
                        ZbData<uint16_t>(ESP_ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE));

    ZbCluster identifyClient(0x0003,true);


    addCluster(&basicTest);
    addCluster(&identifyServer);
    addCluster(&identifyClient);


    std::cout << std::endl << std::endl << std::endl; 
    std::cout << " ++++ Zb Implementation Clusters ++++" << std::endl;



    ZbDebug::printClusterList(&_clusterList.front());             
*/
}



esp_zb_cluster_list_t* ZbEndPoint::getClusterList()
{
    /*
    std::cout << "---------- Clusters ZBC++ ----------" << std::endl;
    std::cout << "Nb Clusters " << _clusterList.size() << std::endl;
    std::cout << "Cluster list addr" << &_clusterList.front() << std::endl;
    std::cout << "attr sizeof" << sizeof(esp_zb_attribute_list_t) << std::endl;
    //ZbDebug::printClusterList(&_clusterList.front());
*/
/*
    std::cout << "---------- Clusters ZBC++ ----------" << std::endl;
    ZbDebug::printClusterList(_cluster_list);
    */
   /*
    std::cout << "---------- Clusters SWITCH ----------" << std::endl;
    esp_zb_on_off_switch_cfg_t switch_cfg = ESP_ZB_DEFAULT_ON_OFF_SWITCH_CONFIG();
    esp_zb_ep_list_t *esp_zb_on_off_switch_ep = esp_zb_on_off_switch_ep_create(2, &switch_cfg);
    std::cout << "Endpoint Id : " << esp_zb_on_off_switch_ep->next->endpoint.ep_id << std::endl;
    std::cout << "Cluster 0 : " << esp_zb_on_off_switch_ep->next->endpoint.cluster_list->cluster.cluster_id << std::endl;
    //std::cout << "Cluster 0 : " << esp_zb_on_off_switch_ep->endpoint.cluster_list->cluster.cluster_id << std::endl;
    ZbDebug::printClusterList(esp_zb_on_off_switch_ep->next->endpoint.cluster_list);
*/
    
    return _cluster_list;
    //return (&_clusterList.front());
}

esp_zb_endpoint_config_t ZbEndPoint::getConfig()
{
    return _endpoint_config;
}

uint8_t ZbEndPoint::getId()
{
    return _endpoint_config.endpoint;
}


void ZbEndPoint::addCluster(ZbCluster* cluster)
{
    cluster->addToList(_cluster_list);
    cluster->setEndPoint(this);
    
    if(cluster->isServer())
        _serverClusterMap[cluster->getId()] = cluster;
    else
        _clientClusterMap[cluster->getId()] = cluster;
}

ZbCluster* ZbEndPoint::getCluster(uint16_t id, bool isClient)
{
    if (isClient){
        auto it = _clientClusterMap.find(id);
        if (it == _clientClusterMap.end())
            return nullptr;
        return it->second;
    }

    auto it = _serverClusterMap.find(id);
    if (it == _serverClusterMap.end())
        return nullptr;
    
    return it->second;
}