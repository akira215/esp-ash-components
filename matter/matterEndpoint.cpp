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

/*
template <typename T>
void MatterEndpoint::setConfig(T& config, uint8_t flags) 
{
    // Le compilateur cherche 'process' dans le namespace de T
    decltype(auto) ep = create(_node->getNode(), config, flags); 
    if (ep != nullptr) {
        ESP_LOGE(TAG, "Failed to create endpoint");
    }
}
*/