/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "esp_matter_data_model.h"
//#include "esp_matter_cluster.h"
#include <cstdint>
#include <esp_matter_endpoint.h>
#include <esp_log.h>

#include <app/clusters/identify-server/identify-server.h> // For Identify struct

#include "matterCluster.h"
#include "matterMap.h"

class MatterNode;

#define ENDPOINT_CONFIG(DeviceType) ::esp_matter::endpoint::DeviceType::config_t



// From esp_matter_data_model.cpp
#define MATTER_MAX_SEMANTIC_TAG_COUNT 3

class MatterNode;


/// @brief Matter Endpoint class
/// store maps of cluster and cluster list as per SDK requirements
class MatterEndpoint
{
    MatterNode*             _node     = nullptr;
    esp_matter::endpoint_t* _endpoint = nullptr;
    
    MatterMap<MatterCluster*> _clustersMap;
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

    // Universal ADL bridge to create the cluster based on config type
    template <typename ConfigType>
    esp_matter::cluster_t* call_createCluster(ConfigType *config, uint8_t flags, type_holder<ConfigType>) {
        // This allows ADL to automatically resolve down to the matching child namespace!
        using namespace esp_matter::cluster;
        // Unqualified call allows the compiler to find the 'create' that matches your ConfigType

        return create( _endpoint, config, flags);
    }


    // Run when endpoint is created to populate the cluster map for this endpoint
    void populateCluster();

public:
  
    /// @brief Constructor create the end point
    MatterEndpoint(MatterNode* node, esp_matter::endpoint_t* endpoint);
    ~MatterEndpoint();

    esp_matter::endpoint_t* getEspEndpoint() { return _endpoint; }

    uint16_t getEndpointId() { 
        if(_endpoint)
            return esp_matter::endpoint::get_id(_endpoint); 
        return (uint16_t)(-1);
    } 

    template <typename ConfigType>
    MatterCluster* createCluster( ConfigType *config, 
                        uint8_t flags = esp_matter::ENDPOINT_FLAG_NONE ) 
    {
        esp_matter::cluster_t *cluster = call_createCluster(config, flags, type_holder<ConfigType>{});
        if (cluster == nullptr){
            ESP_LOGE("MatterEndpoint", "Failed to create cluster");
            return nullptr;
        }
        // Populate the cluster map for this endpoint

        MatterCluster* mCluster = new MatterCluster(this, cluster);
        if (mCluster == nullptr)
            ESP_LOGE("MatterEndpoint", "Failed to create matter cluster");

        if (_clustersMap.contains(mCluster->getClusterId()))
            ESP_LOGE("MatterEndpoint", "Cluster with Id %d already exists in the endpoint %d, it will be erased",
                                        mCluster->getClusterId(),
                                        getEndpointId());

        _clustersMap[mCluster->getClusterId()] = mCluster;

        return mCluster;
    }


    // If flags = esp_matter::CLUSTER_FLAG_NONE, the first cluster is returned
    MatterCluster* getCluster(uint32_t clusterId);

    uint32_t getClusterCount() const;

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



