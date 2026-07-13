/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "matterEndpoint.h"
#include <matterNode.h>

static const char *TAG = "MatterEndpoint";

MatterEndpoint::MatterEndpoint(MatterNode* node, esp_matter::endpoint_t* endpoint ) : _node(node), _endpoint(endpoint)
{
    populateCluster();
}

MatterEndpoint::~MatterEndpoint()
{
    for (auto &item : _clustersMap) {
        delete item.second;
    }
    _clustersMap.clear();
}

void MatterEndpoint::populateCluster()
{
    esp_matter::cluster_t* cluster = esp_matter::cluster::get_first(_endpoint);

    while (cluster != nullptr) {

        MatterCluster* matter_cluster = new MatterCluster(this, cluster);

        _clustersMap[matter_cluster->getClusterId()] = matter_cluster;

        ESP_LOGD(TAG, "Added cluster with ID %u to endpoint %d", matter_cluster->getClusterId(), getEndpointId());
        
        // Move to the next cluster structure
        cluster = esp_matter::cluster::get_next(cluster);
    }
}

MatterCluster* MatterEndpoint::getCluster(uint32_t clusterId)
{
    if (_clustersMap.contains(clusterId)) {
        return _clustersMap[clusterId];
    }
    return nullptr;
}
