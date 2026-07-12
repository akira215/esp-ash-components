/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once


#include <esp_matter_endpoint.h>
#include <functional>
#include <vector>
#include <string>

#include "matterValue.h"


#define ATTRIBUTE_ID(ClusterName, AttributeName) ::chip::app::Clusters::ClusterName::Attributes::AttributeName::Id

class MatterCluster;

/// @brief Matter Cluster class
/// store maps of cluster and cluster list as per SDK requirements
class MatterAttribute
{
    MatterCluster*              _cluster = nullptr;
    esp_matter::attribute_t*    _attribute = nullptr;
    uint16_t _endpointId = (uint16_t)(-1);
    uint32_t _clusterId = (uint32_t)(-1);
    uint32_t _attributeId = (uint32_t)(-1);
  
    // This comes from esp_matter_data_model.cpp
    struct _attrBase_t {
        uint16_t flags; // This struct is for attributes managed internally.
        esp_matter_val_type_t attribute_val_type;
        uint32_t attribute_id;
        struct _attribute_base_t *next;
    };

    struct _attr_t : public _attrBase_t {
        esp_matter_val_t attribute_val;
        esp_matter_attr_bounds_t *bounds;
        uint16_t endpoint_id;
        uint32_t cluster_id;
        esp_matter::attribute::callback_t override_callback;
    };

public:

    /// @brief Constructor create the end point
    MatterAttribute(MatterCluster* cluster, esp_matter::attribute_t* attribute);
    ~MatterAttribute();

    uint32_t getAttributeId() { 
        if (_attribute)
            return esp_matter::attribute::get_id(_attribute); 
        return (uint32_t)(-1);
    }

    uint16_t getFlags() { 
        if (_attribute)
            return esp_matter::attribute::get_flags(_attribute); 
        return (uint16_t)(-1);
    }

    MatterCluster* getCluster() {
        return _cluster;
    }


    void setDeferredPersistence(bool enable = true);

    esp_matter::attribute_t* getEspAttribute() { return _attribute; }
    
    /// @brief register attribute update handler for this attribute.
    /// Update handler shall be type attrUpdateCallback_t : 
    /// void(esp_matter::attribute::callback_type_t, esp_matter_attr_val_t*,void*) 
    /// @param func pointer to the method ex: &Main::clusterHandler
    /// @param instance instance of the object for this handler (ex: this)
    template<typename C, typename... Args>
    void registerAttrUpdateHandler(void (C::* func)(Args...), C* instance);


    /// @brief Method to update an attribute whatever the type is 
    template <typename T>
    inline void updateValue(T raw_value) {
        // Automatically deduces T and constructs the perfect struct layout
        MatterValue matter_value;
        matter_value = raw_value;
        //esp_matter_attr_val_t matter_value = make_matter_val(raw_value);
        
        // Pass it straight to the native SDK
        esp_err_t ret;
        ret = esp_matter::attribute::update(_endpointId, _clusterId, _attributeId, &matter_value);
        if(ret != ESP_OK)  
            ESP_LOGE("MatterAttribute","updateValue - esp_matter::attribute::update failed with error code %d", ret); 

    }

    MatterValue getValue();
        

};


// Include the implementation at the very bottom
#include "matterAttribute_impl.tpp" 

