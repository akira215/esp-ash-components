/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "matterCluster.h"
#include "esp_matter_data_model.h"
#include <matterEndpoint.h>

static const char *TAG = "MatterCluster";

MatterCluster::MatterCluster(MatterEndpoint* enpoint, esp_matter::cluster_t* cluster) :
                            _endpoint(enpoint), _cluster(cluster)
{
    esp_matter::attribute_t *attribute = esp_matter::attribute::get_first(cluster);
    while (attribute) {

      MatterAttribute* matter_attribute = new MatterAttribute(this, attribute);

      _attributesMap[esp_matter::attribute::get_id(attribute)] = matter_attribute;
      ESP_LOGD(TAG, "Added attribute with ID %d to cluster %d", matter_attribute->getAttributeId(), getClusterId());
        
        
      attribute = esp_matter::attribute::get_next(attribute);
    }

}

MatterCluster::~MatterCluster()
{
    // TODO remove attribute and endpoints
}
