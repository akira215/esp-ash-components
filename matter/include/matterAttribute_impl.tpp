/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "matterCluster.h"
#include "matterEndpoint.h"
#include "matterNode.h"

template<typename C, typename... Args>
void MatterAttribute::registerAttrUpdateHandler(void (C::* func)(Args...), C* instance) {
        uint32_t attributeId = getAttributeId();
       
    if(attributeId == (uint32_t)(-1)){
        ESP_LOGE("MatterAttribute", "Unable to register attribute, unknown attribute" );
        return;
    }
    if(!_cluster){
        ESP_LOGE("MatterAttribute", "Unable to register attribute %d, cluster not registered", attributeId );
        return;
    }
    uint32_t clusterId = _cluster->getClusterId();
    if(clusterId == (uint32_t)(-1)){
        ESP_LOGE("MatterAttribute", "Unable to register attribute %d, unknown cluster", attributeId );
        return;
    }
    MatterEndpoint* endpoint = _cluster->getEndpoint();
     if(!endpoint){
        ESP_LOGE("MatterAttribute", "Unable to register attribute %d - cluster %d, endpoint not registered", attributeId, clusterId );
        return;
    }
    uint16_t endpointId = endpoint->getEndpointId();
    if(endpointId  == (uint16_t)(-1)){
        ESP_LOGE("MatterAttribute", "Unable to register attribute %d - cluster %d, unknown endpoint", attributeId, clusterId );
        return;
    }

    MatterNode::getInstance()->registerAttrUpdateHandler(func, instance, endpointId, clusterId, attributeId);
}
        