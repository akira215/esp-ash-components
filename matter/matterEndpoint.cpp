/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "matterEndpoint.h"
#include <matterNode.h>

static const char *TAG = "MatterEndpoint";

MatterEndpoint::MatterEndpoint(MatterNode* node) : _node(node)
{

}

MatterEndpoint::~MatterEndpoint()
{
    // TODO remove attribute and endpoints
}

void MatterEndpoint::populate_cluster_map()
{
    esp_matter::cluster_t* cluster = esp_matter::cluster::get_first(_endpoint);

    while (cluster != nullptr) {

        MatterCluster* matter_cluster = new MatterCluster(this, cluster);

        _vClusters.push_back(matter_cluster);
        ESP_LOGD(TAG, "Added cluster with ID %u to endpoint %d", matter_cluster->getClusterId(), getEndpointId());
        
        // Move to the next cluster structure
        cluster = esp_matter::cluster::get_next(cluster);
    }
}

