/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "matterCluster.h"
#include <matterAttribute.h>
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


MatterAttribute* MatterCluster::getAttribute(uint32_t attributeId)
{
    if (_attributesMap.contains(attributeId)) {
        return _attributesMap[attributeId];
    }
    ESP_LOGW(TAG, "getAttribute - No attribute with ID %d to cluster %d, return nullptr", attributeId, getClusterId());
    return nullptr;
}

void MatterCluster::addFeature()
{
    /*
    esp_matter::cluster::color_control::feature::hue_saturation::add(color_control_cluster);
    
    if (esp_matter::cluster::add_feature(color_control_cluster, hs_feature_flags) == ESP_OK) {
        ESP_LOGI(TAG, "Successfully added Hue/Saturation feature to Extended Color Light!");
    } else {
        ESP_LOGE(TAG, "Failed to add Hue/Saturation feature.");
    }

    if (_attributesMap.contains(attributeId)) {
        return _attributesMap[attributeId];
    }
    ESP_LOGW(TAG, "getAttribute - No attribute with ID %d to cluster %d, return nullptr", attributeId, getClusterId());
    return nullptr;
    */
}

MatterAttribute* MatterCluster::addAttribute(uint32_t attributeId, uint8_t flags, MatterValue value)
 {
    if (_attributesMap.contains(attributeId)) {
        ESP_LOGI(TAG,"addAttribute id %d in cluster %d already exists", attributeId, getClusterId());
        return _attributesMap[attributeId];
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