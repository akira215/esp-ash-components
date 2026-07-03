/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once


#include <esp_matter_endpoint.h>
#include <matterAttribute.h>
#include <map>

class MatterEndpoint;

/// @brief Matter Cluster class
/// store maps of cluster and cluster list as per SDK requirements
class MatterCluster
{
    MatterEndpoint*                     _endpoint = nullptr;
    esp_matter::cluster_t*              _cluster = nullptr;
    std::map<uint32_t,MatterAttribute*> _attributesMap;
public:

    /// @brief Constructor create the end point
    MatterCluster(MatterEndpoint* enpoint, esp_matter::cluster_t* cluster);
    ~MatterCluster();

    uint32_t getClusterId() { return esp_matter::cluster::get_id(_cluster); }
    uint8_t  getFlags()     { return esp_matter::cluster::get_flags(_cluster);}
    bool     isClient()     { return (getFlags()  & esp_matter::CLUSTER_FLAG_CLIENT) != 0; }
    bool     isServer()     { return (getFlags()  & esp_matter::CLUSTER_FLAG_SERVER) != 0; }

    // Flags is 0x01 CLUSTER_FLAG_SERVER - 
    // 0x02 CLUSTER_FLAG_CLIENT - 
    // 0x04 CLUSTER_FLAG_SERVER_CLIENT - 
    // 0x08 CLUSTER_FLAG_GEN_CLUSTER - 
    // 0x10 CLUSTER_FLAG_SHUTDOWN_FUNCTION
    // 
    // bool is_server = (flags & esp_matter::CLUSTER_FLAG_SERVER);
    // bool is_client = (flags & esp_matter::CLUSTER_FLAG_CLIENT);
    
};

