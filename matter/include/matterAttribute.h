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


#define ATTRIBUTE_ID(ClusterName, AttributeName) ::chip::app::Clusters::ClusterName::Attributes::AttributeName::Id

class MatterCluster;

/// @brief Matter Cluster class
/// store maps of cluster and cluster list as per SDK requirements
class MatterAttribute
{
    MatterCluster*              _cluster = nullptr;
    esp_matter::attribute_t*    _attribute = nullptr;

    
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

    uint32_t getAttributeId() { return esp_matter::attribute::get_id(_attribute); }
    uint16_t getFlags() { return esp_matter::attribute::get_flags(_attribute); }

    void setDeferredPersistence(bool enable = true);

    esp_matter::attribute_t* getEspAttribute() { return _attribute; }
    
    /// @brief register attribute update handler for this attribute.
    /// Update handler shall be type clusterCallback_t : 
    /// void(eventType, uint16_t attrId, void* value) 
    /// @param func pointer to the method ex: &Main::clusterHandler
    /// @param instance instance of the object for this handler (ex: this)
    ///template<typename C, typename... Args>
    ///void registerEventHandler(void (C::* func)(Args...), C* instance) {
    ///    _clusterEventHandlers.push_back(std::bind(func,std::ref(*instance),
    ///       std::placeholders::_1,
    ///        std::placeholders::_2,
    ///        std::placeholders::_3));
    ///};
    /*
    template<typename C, typename... Args>
    void registerAttrUpdateHandler(void (C::* func)(Args...), C* instance) {
        // A lambda captures the function pointer and instance, 
        // and forwards any number of incoming arguments using a parameter pack.
        _attrUpdateHandlers.push_back([instance, func](Args&&... args) {
            (instance->*func)(std::forward<Args>(args)...);
        });
    }
        */

};

