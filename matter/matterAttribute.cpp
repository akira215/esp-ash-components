/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/


#include "matterAttribute.h"
#include <matterCluster.h>
#include <matterEndpoint.h>
#include <matterNode.h>

static const char *TAG = "MatterAttribute";



MatterAttribute::MatterAttribute(MatterCluster* cluster,esp_matter::attribute_t* attribute) :
                         _cluster(cluster), _attribute(attribute)
{ 
    _attributeId = getAttributeId();
       
    if(_attributeId == (uint32_t)(-1)){
        ESP_LOGE(TAG, "Unable to register attribute, unknown attribute" );
        return;
    }
    if(!_cluster){
        ESP_LOGE(TAG, "Unable to register attribute %d, cluster not registered", _attributeId );
        return;
    }
    _clusterId = _cluster->getClusterId();
    if(_clusterId == (uint32_t)(-1)){
        ESP_LOGE(TAG, "Unable to register attribute %d, unknown cluster", _attributeId );
        return;
    }
    MatterEndpoint* endpoint = _cluster->getEndpoint();
     if(!endpoint){
        ESP_LOGE(TAG, "Unable to register attribute %d - cluster %d, endpoint not registered", _attributeId, _clusterId );
        return;
    }
    _endpointId = endpoint->getEndpointId();
    if(_endpointId  == (uint16_t)(-1)){
        ESP_LOGE(TAG, "Unable to register attribute %d - cluster %d, unknown endpoint", _attributeId, _clusterId );
        return;
    }
}

MatterAttribute::~MatterAttribute()
{
    // TODO remove attribute and endpoints
}

void MatterAttribute::setDeferredPersistence(bool enable)
{
    if(enable)
        esp_matter::attribute::set_deferred_persistence(_attribute);
    else
    {
        if(!_attribute)
            ESP_LOGE(TAG, "Attribute cannot be NULL resetting Deferred Persistence");
        else
        {
            // only removing the flag, TODO check if callback shall be removed
            _attr_t* attrStruct = reinterpret_cast<_attr_t*>(_attribute);
            attrStruct->flags &= ~esp_matter::ATTRIBUTE_FLAG_DEFERRED;
        }
      
    }
}

MatterValue MatterAttribute::getValue()
{
    MatterValue raw_val(false);
    if(!_attribute)
    {
        ESP_LOGE(TAG, "Invalid attribute accessing it by getValue");
        return raw_val;
    }
    esp_matter::attribute::get_val(_attribute, &raw_val);

    return raw_val;
}
