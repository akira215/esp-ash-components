/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "esp_matter_data_model.h"
#include <esp_matter_endpoint.h>


class MatterNode;


/// @brief Matter Endpoint class
/// store maps of cluster and cluster list as per SDK requirements
class MatterEndpoint
{
    esp_matter::endpoint_t* _endpoint = nullptr;
    MatterNode* _node = nullptr;
        
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
    }

    esp_matter::endpoint_t* getEndpoint() { return _endpoint; }


};

