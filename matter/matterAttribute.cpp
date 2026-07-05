/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "matterAttribute.h"
#include <matterCluster.h>
#include <matterNode.h>

static const char *TAG = "MatterAttribute";



MatterAttribute::MatterAttribute(MatterCluster* cluster,esp_matter::attribute_t* attribute) :
                         _cluster(cluster), _attribute(attribute)
{ 

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
