/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

//#include <vector>
#include <list>
#include <memory>
#include <cstring>
#include <string>
#include "esp_zigbee_core.h"


#include <iostream> // TODEL

class ZbCluster
{
    esp_zb_zcl_cluster_t _cluster;

protected:
    esp_zb_attribute_list_t* _attr_list;

protected:
    void _init(uint16_t id, bool isClient);

public:
    /// @brief Constructor
    ZbCluster();
    ~ZbCluster();

    virtual void addAttribute(uint16_t attr_id, void* value) = 0;
 
    virtual void addToList(esp_zb_cluster_list_t* cluster_list) = 0;

    esp_zb_zcl_attr_t* getAttribute(uint16_t attr_id);

    uint16_t getId() const;
    bool isClient() const;

    esp_zb_zcl_cluster_t* getClusterStruct();

private:

        
};
