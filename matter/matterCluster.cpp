/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "matterCluster.h"
#include <cstdint>
#include <matterAttribute.h>
#include "esp_matter_data_model.h"
#include <matterEndpoint.h>

static const char *TAG = "MatterCluster";

MatterCluster::MatterCluster(MatterEndpoint* enpoint, esp_matter::cluster_t* cluster) :
                            _endpoint(enpoint), _cluster(cluster)
{
    populateAttributes();
}

MatterCluster::~MatterCluster()
{
    _attributesMap.clear();
}


void MatterCluster::populateAttributes()
{
    uint32_t size = getAttributeCount();
    if (size > _attributesMap.size())
        _attributesMap.reserve(size);

    esp_matter::attribute_t *attribute = esp_matter::attribute::get_first(_cluster);
    while (attribute) {
        if (!_attributesMap.contains(esp_matter::attribute::get_id(attribute))) {
            MatterAttribute* matter_attribute = new MatterAttribute(this, attribute);
            _attributesMap[esp_matter::attribute::get_id(attribute)] = matter_attribute;
            ESP_LOGD(TAG, "Added attribute with ID %d in cluster %d", matter_attribute->getAttributeId(), getClusterId());
        }
        attribute = esp_matter::attribute::get_next(attribute);
    }
}

 uint32_t MatterCluster::getAttributeCount() const
 {
    if (!_cluster) {
        return 0;
    }

    uint32_t count = 0;
    // Pass NULL initially to get the first attribute of the cluster
    esp_matter::attribute_t *attr = esp_matter::attribute::get_first(_cluster);

    while (attr != NULL) {
        count++;
        // Fetch the next attribute using the current one as a reference
        attr = esp_matter::attribute::get_next(attr);
    }

    return count;
 }


MatterAttribute* MatterCluster::getAttribute(uint32_t attributeId)
{
    MatterAttribute* attr = _attributesMap[attributeId];
    if (attr == nullptr) {
        ESP_LOGW(TAG, "getAttribute - No attribute with ID %d to cluster %d, return nullptr", attributeId, getClusterId());
    }

    return attr;
}


MatterAttribute* MatterCluster::addAttribute(uint32_t attributeId, uint8_t flags, MatterValue value)
 {
    MatterAttribute* attr = _attributesMap.at(attributeId);
    if (attr != nullptr) {
        ESP_LOGI(TAG,"addAttribute id %d in cluster %d already exists", attributeId, getClusterId());
        return attr;
    }

    
    // 4. Create and attach the attribute to the parent cluster
    esp_matter::attribute_t *newAttribute = esp_matter::attribute::create(_cluster, 
                                                                attributeId,
                                                                flags, 
                                                                (esp_matter_attr_val_t )(value),
                                                                value.val.a.max);
    
    if (newAttribute == nullptr) {
        ESP_LOGE(TAG,"addAttribute id %d in cluster %d unable to create attribute return nullptr", attributeId, getClusterId());
        return nullptr;  
    }

    MatterAttribute* matter_attribute = new MatterAttribute(this, newAttribute);

    if (matter_attribute == nullptr) {
        ESP_LOGE(TAG,"addAttribute id %d in cluster %d unable to create MatterAttribute return nullptr", attributeId, getClusterId());
        return nullptr;  
    }

    _attributesMap[attributeId] = matter_attribute;

    return matter_attribute;

 }