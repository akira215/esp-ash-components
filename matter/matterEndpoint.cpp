/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "matterEndpoint.h"
#include <matterNode.h>

static const char *TAG = "MatterEndpoint";

MatterEndpoint::MatterEndpoint(MatterNode* node, esp_matter::endpoint_t* endpoint ) : 
                                                    _node(node), _endpoint(endpoint)
{
    populateCluster();
}

MatterEndpoint::~MatterEndpoint()
{
    _clustersMap.clear();
}

void MatterEndpoint::populateCluster()
{
    uint32_t size = getClusterCount();
    if (size > _clustersMap.size())
        _clustersMap.reserve(size);

    esp_matter::cluster_t* cluster = esp_matter::cluster::get_first(_endpoint);

    while (cluster != nullptr) {

        MatterCluster* matter_cluster = new MatterCluster(this, cluster);

        _clustersMap[matter_cluster->getClusterId()] = matter_cluster;

        ESP_LOGD(TAG, "Added cluster with ID %u to endpoint %d", 
                                    matter_cluster->getClusterId(), getEndpointId());
        
        // Move to the next cluster structure
        cluster = esp_matter::cluster::get_next(cluster);
    }
}

uint32_t MatterEndpoint::getClusterCount() const
{
    if (!_endpoint) {
        return 0;
    }

    uint16_t count = 0;
    esp_matter::cluster_t *cluster = esp_matter::cluster::get_first(_endpoint);

    while (cluster != NULL) {
        count++;
        // Fetch the next cluster using the current one as a reference
        cluster = esp_matter::cluster::get_next(cluster);
    }

    return count;

}

MatterCluster* MatterEndpoint::getCluster(uint32_t clusterId)
{
    MatterCluster* cluster = _clustersMap[clusterId];
    if (cluster  == nullptr) {
        ESP_LOGW(TAG, "getCluster - No cluster with ID %d in endpoint %d, return nullptr", 
                                        clusterId, getEndpointId());
    }

    return cluster;
}
