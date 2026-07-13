/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once


#include <esp_matter_endpoint.h>

#include <unordered_map>
#include "matterValue.h"
#include "esp_matter_feature.h"


// trick to force wrapper to select correct create function 
// as user_label::config_t is an alias of  commom::config_t
namespace esp_matter {
namespace cluster {
namespace user_label {

struct adl_config_t : common::config_t {
    using common::config_t::config_t;
};

} // namespace user_label
} // namespace cluster
} // namespace esp_matter



#define CLUSTER_ID(ClusterName) ::chip::app::Clusters::ClusterName::Id

// cluster and feature are in the file esp_matter_feature.h
// WARNING clusterName is not the same as the cluster name for endpoint
#define FEATURE_CONFIG(clusterName, featureName) \
    ::esp_matter::cluster::clusterName::feature::featureName::config_t

#define chip_cluster ::chip::app::Clusters

class MatterEndpoint;
class MatterAttribute;


/// @brief Matter Cluster class
/// store maps of cluster and cluster list as per SDK requirements
class MatterCluster
{
    MatterEndpoint*                     _endpoint = nullptr;
   
    std::unordered_map<uint32_t,MatterAttribute*> _attributesMap;
/*
    // from esp_matter_data_model.cpp
    struct command_t {
        uint32_t command_id;
        uint16_t flags;
        esp_matter::command::callback_t callback;
        esp_matter::command::callback_t user_callback;
        struct _command *next;
    };

    struct event_t {
        uint32_t event_id;
        struct _event *next;
    };

    struct cluster_t {
        uint32_t cluster_id;
        uint16_t endpoint_id;
        uint8_t flags;
        const esp_matter::cluster::function_generic_t *functions;
        esp_matter::cluster::plugin_server_init_callback_t plugin_server_init_callback;
        esp_matter::cluster::delegate_init_callback_t delegate_init_callback;
        void *delegate_pointer;
        esp_matter::cluster::add_bounds_callback_t add_bounds_callback;
        esp_matter::cluster::initialization_callback_t init_callback;
        esp_matter::cluster::shutdown_callback_t shutdown_callback;
        chip::DataVersion data_version;
        void *attribute_list; // If attribute is managed internally, the actual pointer type is
                              //  _internal_attribute_t. When operating attribute_list, do check the flags first! 
        command_t *command_list;
        event_t *event_list;
        struct _cluster *next;
    };
    */

     // A generic helper tag to pass the type context
    template <typename T> struct type_holder {};

    // Universal ADL bridge to add feature based on config type
    template <typename ConfigType>
    auto call_add(ConfigType *config, type_holder<ConfigType>) {
        // This allows ADL to automatically resolve down to the matching child namespace!
        using namespace esp_matter::cluster;
        // Unqualified call allows the compiler to find the 'add' that matches your ConfigType
        return add(_cluster, config);
    }

    esp_matter::cluster_t*              _cluster = nullptr;

    void populateAttributes();

public:

    /// @brief Constructor create the cluster
    MatterCluster(MatterEndpoint* enpoint, esp_matter::cluster_t* cluster);
    ~MatterCluster();

    /// @brief access esp_matter pointer directly
    operator esp_matter::cluster_t* () const {
        return _cluster;
    }

    MatterEndpoint* getEndpoint() {
        return _endpoint;
    }

    uint32_t getClusterId() { 
        if (_cluster)
            return esp_matter::cluster::get_id(_cluster); 
        return (uint32_t)(-1);
    }

    uint8_t  getFlags()     { 
        if (_cluster)
            return esp_matter::cluster::get_flags(_cluster);
        return (uint8_t)(-1);
    }

    bool     isClient()     { return (getFlags()  & esp_matter::CLUSTER_FLAG_CLIENT) != 0; }
    bool     isServer()     { return (getFlags()  & esp_matter::CLUSTER_FLAG_SERVER) != 0; }

    // Flags is 0x01 CLUSTER_FLAG_SERVER - 
    // 0x02 CLUSTER_FLAG_CLIENT - 
    // 0x04 CLUSTER_FLAG_SERVER_CLIENT - 
    // 0x08 CLUSTER_FLAG_GEN_CLUSTER - 
    // 0x10 CLUSTER_FLAG_SHUTDOWN_FUNCTION
    // 
    MatterAttribute* getAttribute(uint32_t attributeId);

    template <typename ConfigType>
    void addFeature(ConfigType *config) 
    {
        esp_err_t err  = call_add( config, type_holder<ConfigType>{});
        if(err != ESP_OK)
            ESP_LOGE("MatterCluster", "Failed to add feature");
        
        // Populate the attribute map 
        populateAttributes(); 
    }
    
    MatterAttribute* addAttribute(uint32_t attributeId, 
                            uint8_t flags = esp_matter::ATTRIBUTE_FLAG_NONE, 
                            MatterValue value = MatterValue());
    
};

