/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  Zigbee End Point Class
*/

#include "cppzb_cluster.h"

#include <iostream> // TODEL

ZbCluster::ZbCluster(uint16_t id, bool isClient)
{
    cluster.cluster_id = id;
    cluster.attr_count = 0;
    isClient ? cluster.role_mask = ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE :
                cluster.role_mask = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE;
    cluster.cluster_init = nullptr;     //esp_zb_zcl_cluster_init_t  cluster init callback function 

/*
    esp_zb_basic_cluster_cfg_s basic_cfg;
    basic_cfg.zcl_version  =    ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE;
    basic_cfg.power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_DEFAULT_VALUE;                                                                       
    _basic_cluster = esp_zb_basic_cluster_create(&basic_cfg);
*/
    //manuf = "5AKIRA";
    std::cout<< "ZbCluster constructor"<< std::endl;
}

ZbCluster::~ZbCluster()
{
    
}

void ZbCluster::addAttribute(uint16_t attr_id,
                        esp_zb_zcl_attr_type_t type, 
                        esp_zb_zcl_attr_access_t access,
                        std::any value)
{
    // Store the attribute value
    _attribute_datas.push_back(value);

    esp_zb_attribute_list_t newAttribute;

    newAttribute.attribute.id = attr_id;
    newAttribute.attribute.type = type;
    newAttribute.attribute.access = access; 
    newAttribute.attribute.manuf_code = 0;
    newAttribute.attribute.data_p = (&_attribute_datas.back());

    newAttribute.cluster_id = cluster.cluster_id;

    //Store the Attribute structure
    _vecAttrList.push_back(newAttribute);

    // Update the next pointer of the previous
    if(_vecAttrList.size()>1)
        _vecAttrList.rbegin()[1].next = &_vecAttrList.back();
    else // if this is the first, set the pointer of the cluster attr list
        cluster.attr_list = &_vecAttrList.front();

    // Incfrement Cluster attr count
    cluster.attr_count += 1;

}

void ZbCluster::getAttribute(uint16_t attr_id)
{

    
    std::cout<< "attribute" << std::endl;

}