/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  Zigbee End Point Class
*/

#include "zbCluster.h"

#include <iostream> // TODEL

ZbCluster::ZbCluster(uint16_t id, bool isClient)
{
    esp_zb_attribute_list_t attr_0; //attr 0 is null
    attr_0.cluster_id = 0;
    attr_0.next = 0;
    attr_0.attribute.id = 0;
    attr_0.attribute.type = 0;
    attr_0.attribute.access = 0;
    attr_0.attribute.manuf_code = 0;
    attr_0.attribute.data_p = 0;

    _attrList.push_back(attr_0);

    _cluster.cluster_id = id;
    _cluster.attr_count = 0;
    _cluster.attr_list = &_attrList.back();
    isClient ? _cluster.role_mask = ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE :
                _cluster.role_mask = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE;
    _cluster.manuf_code = 0; // TODO check
    _cluster.cluster_init = nullptr;//esp_zb_zcl_cluster_init_t cluster init callback
    
    std::cout << "cluster attr list addr "<< _cluster.attr_list << std::endl;
    std::cout << "vector addr            "<< &_attrList.back() << std::endl;
    std::cout << "vector elmnt id  "<< _attrList.back().attribute.id << std::endl;
    std::cout << "id 0 "<< _cluster.attr_list->attribute.id << std::endl;
    std::cout << "next 0 "<< _cluster.attr_list->next << std::endl;


    // Add global mandatory attribute ClusterRevision 0xfffd
    addAttribute(0xfffd, ESP_ZB_ZCL_ATTR_TYPE_U16, 
                    ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, (uint16_t)(4));
    
    
    std::cout << std::endl << "After addAttribute" << std::endl;
    std::cout << "cluster attr list addr  "<< _cluster.attr_list << std::endl;
    std::cout << "vector addr             "<< &_attrList.back() << std::endl;
    std::cout << "vector first elmt  "<< &_attrList.front() << std::endl;
    std::cout << "id 0 "<< _cluster.attr_list->attribute.id << std::endl;
    std::cout << "next 0 "<< _cluster.attr_list->next << std::endl;
    
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
    _attrDatas.push_back(value);

    esp_zb_attribute_list_t newAttribute;

    newAttribute.cluster_id = _cluster.cluster_id;
    newAttribute.next = 0;
    newAttribute.attribute.id = attr_id;
    newAttribute.attribute.type = type;
    newAttribute.attribute.access = access; 
    newAttribute.attribute.manuf_code = 0xffff; // TODO check
    newAttribute.attribute.data_p = (&_attrDatas.back());

    // Update next pointer
    esp_zb_attribute_list_t** n = &_attrList.back().next;
    _attrList.push_back(newAttribute);
    (*n) = &_attrList.back();


    // TODO check what is cluster attr_count;
    // Increment Cluster attr count
    // _cluster.attr_count += 1;

}

esp_zb_zcl_cluster_t* ZbCluster::getClusterStruct()
{
    std::cout << std::endl << "Before returning " << std::endl;
    std::cout << "cluster attr list addr  "<< _cluster.attr_list << std::endl;
    std::cout << "next first              "<< _cluster.attr_list->next << std::endl;
    std::cout << "id 0 "<< _cluster.attr_list->attribute.id << std::endl;
    std::cout << "id 1  "<< _cluster.attr_list->next->attribute.id  << std::endl;
    
    return &_cluster;
}

void ZbCluster::getAttribute(uint16_t attr_id)
{

    
    std::cout<< "attribute" << std::endl;

}