/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  Zigbee End Point Class
*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>

#include "zbEndpoint.h"

static const char *ZEP_TAG = "ZbEndpoint";

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
    // TODO delete all the ZbCluster objects 
}

esp_zb_cluster_list_t* ZbEndPoint::getClusterList()
{
    return _cluster_list;
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
    ESP_LOGV(ZEP_TAG, "%d - Adding Cluster Id: %d", getId(), cluster->getId());

    cluster->addToList(_cluster_list);
    
    if(cluster->isServer())
        _serverClusterMap[cluster->getId()] = cluster;
    else
        _clientClusterMap[cluster->getId()] = cluster;

    cluster->setEndPoint(this);
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