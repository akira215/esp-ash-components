/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "esp_matter_data_model.h"
#include <cstdint>
#include <esp_matter_endpoint.h>
#include <esp_log.h>

#include <app/clusters/identify-server/identify-server.h> // For Identify struct

#include "matterCluster.h"
#include "matterUtils.h"

#include <unordered_map>



#define ENDPOINT_CONFIG(DeviceType) ::esp_matter::endpoint::DeviceType::config_t



// From esp_matter_data_model.cpp
#define MATTER_MAX_SEMANTIC_TAG_COUNT 3

class MatterNode;


/// @brief Matter Endpoint class
/// store maps of cluster and cluster list as per SDK requirements
class MatterEndpoint
{
    esp_matter::endpoint_t* _endpoint = nullptr;
    MatterNode*             _node     = nullptr;

    std::unordered_map<uint32_t,MatterCluster*> _clustersMap;
/*
    // From esp_matter_data_model.cpp
    struct deviceType_t {
        uint8_t version;
        uint32_t id;
    };

    struct endpoint_t {
        uint16_t endpoint_id;
        bool enabled;
        uint8_t device_type_count;
        deviceType_t device_types[CONFIG_ESP_MATTER_MAX_DEVICE_TYPE_COUNT];
        uint16_t flags;
        uint16_t parent_endpoint_id;
        void *priv_data;
        Identify *identify;
        chip::app::DataModel::EndpointCompositionPattern composition_pattern;
        uint8_t semantic_tag_count;
        chip::app::DataModel::Provider::SemanticTag semantic_tags[MATTER_MAX_SEMANTIC_TAG_COUNT];
        void* cluster_list;
        struct _endpoint *next;
    };
*/
    // A generic helper tag to pass the type context
    template <typename T> struct type_holder {};

    // Universal ADL bridge to create the endpoint based on config type
    template <typename ConfigType>
    auto call_create(esp_matter::node_t *node, ConfigType *config, uint8_t flags, void *priv_data, type_holder<ConfigType>) {
        // This allows ADL to automatically resolve down to the matching child namespace!
        using namespace esp_matter::endpoint;
        // Unqualified call allows the compiler to find the 'create' that matches your ConfigType
        return create(node, config, flags, priv_data);
    }

    // Run when endpoint is created to populate the cluster map for this endpoint
    void populate_cluster_map();

public:
  
    /// @brief Constructor create the end point
    MatterEndpoint(MatterNode* node);
    ~MatterEndpoint();

    template <typename ConfigType>
    void create_endpoint(esp_matter::node_t *node,       
                        ConfigType *config, 
                        uint8_t flags = esp_matter::ENDPOINT_FLAG_NONE, 
                        void *priv_data = nullptr) 
    {
        _endpoint  = call_create(node, config, flags, priv_data, type_holder<ConfigType>{});
        ABORT_NODE_ON_FAILURE(_endpoint != nullptr, ESP_LOGE("MatterEndpoint", "Failed to create extended endpoint"));
        
        // Populate the cluster map for this endpoint
        populate_cluster_map();
    }

    esp_matter::endpoint_t* getEspEndpoint() { return _endpoint; }
    uint16_t getEndpointId() { 
        if(_endpoint)
            return esp_matter::endpoint::get_id(_endpoint); 
        return (uint16_t)(-1);
    } 

    // If flags = esp_matter::CLUSTER_FLAG_NONE, the first cluster is returned
    MatterCluster* getCluster(uint32_t clusterId);

    /// @brief register identify .
    /// Update handler shall be type identifyCallback_t : 
    /// void(identifyEvent_t,uint8_t, uint8_t, void*)
    /// @param func pointer to the method ex: &Main::clusterHandler
    /// @param instance instance of the object for this handler (ex: this)
    template<typename C, typename... Args>
    void registerIdentifyHandler(void (C::* func)(Args...), 
                                    C* instance );


};

// Include the implementation at the very bottom
#include "matterEndpoint_impl.tpp" 



