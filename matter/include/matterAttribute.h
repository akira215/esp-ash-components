/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once


#include <esp_matter_endpoint.h>


class MatterCluster;

/// @brief Matter Cluster class
/// store maps of cluster and cluster list as per SDK requirements
class MatterAttribute
{
    MatterCluster*              _cluster = nullptr;
    esp_matter::attribute_t*    _attribute = nullptr;

public:

    /// @brief Constructor create the end point
    MatterAttribute(MatterCluster* cluster, esp_matter::attribute_t* attribute);
    ~MatterAttribute();

    uint32_t getAttributeId() { return esp_matter::attribute::get_id(_attribute); }
    uint16_t getFlags() { return esp_matter::attribute::get_flags(_attribute); }
    
};

