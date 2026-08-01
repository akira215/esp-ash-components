/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "matterEndpoint.h"
#include "matterUtils.h"

template <typename ConfigType>
MatterEndpoint* MatterNode::createEndpoint(ConfigType* config,
                                uint8_t flags, 
                                void *priv_data) 
{
    esp_matter::endpoint_t* endpoint  = call_createEndpoint(config, flags, priv_data, type_holder<ConfigType>{});

    ABORT_NODE_ON_FAILURE(endpoint != nullptr, 
      ESP_LOGE("MatterEndpoint", "Failed to create extended endpoint"));
        
    MatterEndpoint* mEndpoint = new MatterEndpoint(this, endpoint);
    //endpoint->create_endpoint(_node, &config);
    _endpointsMap[mEndpoint->getEndpointId()] = mEndpoint;
    return mEndpoint;
}

