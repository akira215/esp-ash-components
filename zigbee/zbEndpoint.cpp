/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  Zigbee End Point Class
*/

#include "zbEndpoint.h"
#include "zbDebug.h"
#include <iostream>

ZbEndPoint::ZbEndPoint(uint8_t id, uint16_t device_id,
                    uint16_t profile_id, uint32_t device_version)
{
    _cluster_list = esp_zb_zcl_cluster_list_create();

    _endpoint_config.endpoint = id;
    _endpoint_config.app_device_id = device_id;
    _endpoint_config.app_profile_id = profile_id;
    _endpoint_config.app_device_version = device_version;

    esp_zb_basic_cluster_cfg_s basic_cfg;
    basic_cfg.zcl_version  =    ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE;
    basic_cfg.power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_DEFAULT_VALUE;                                                                       
    _basic_cluster = esp_zb_basic_cluster_create(&basic_cfg);

    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(_basic_cluster, 
                            ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, 
                            (void*)MANUFACTURER_NAME));
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(_basic_cluster, 
                            ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, 
                            (void*)MODEL_IDENTIFIER));

    ESP_ERROR_CHECK(esp_zb_cluster_list_add_basic_cluster(_cluster_list, _basic_cluster, 
                            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    
    

    _identify_cfg.identify_time = ESP_ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;    
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_identify_cluster(_cluster_list, 
                esp_zb_identify_cluster_create(&_identify_cfg), 
                ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    ESP_ERROR_CHECK(esp_zb_cluster_list_add_identify_cluster(_cluster_list, 
                esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY), 
                ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE));


    printClusters();

    initZbCluster();
}


ZbEndPoint::~ZbEndPoint()
{
    for(ZbCluster* cluster: _vecCluster)
        delete cluster;
    
    _vecCluster.clear();   
}

void ZbEndPoint::initZbCluster()
{
    ZbCluster basicTest(0,false);
    basicTest.addAttribute(0,ESP_ZB_ZCL_ATTR_TYPE_U8,
                        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,(uint8_t)(0x08));
    basicTest.addAttribute(7,ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM,
                        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,(uint8_t)(0x01));
                        /*
    basicTest.addAttribute(4,ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,
                        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,8);
    basicTest.addAttribute(5,ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,
                        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,8);
                        */
    std::cout << std::endl << std::endl << std::endl; 
    std::cout << " ++++ Zb Implementation Clusters ++++" << std::endl;

    ZbDebug::printCluster(basicTest.getClusterStruct());             

}

ZbEndPoint::ZbEndPoint(const ZbEndPoint& other)
{
    std::cout << "Copy Constructor" <<std::endl; //TODEL
    
    _endpoint_config = other._endpoint_config;

    _cluster_list = other._cluster_list;

}

esp_zb_cluster_list_t* ZbEndPoint::getClusterList()
{
    return _cluster_list;
}

esp_zb_endpoint_config_t ZbEndPoint::getConfig()
{
    return _endpoint_config;
}

ZbCluster* ZbEndPoint::createCluster()
{
    return new ZbCluster(0,true);
}

void ZbEndPoint::addCluster(ZbCluster* cluster)
{
    _vecCluster.push_back(cluster);

}

//////////////////////////////////////////////// DEBUG//////////////////////////////
void ZbEndPoint::printClusters()
{
    std::cout << "----- Clusters DEBUG HELPER----- " << std::endl;
    ZbDebug::printClusterList(_cluster_list);
}
//////////////////////////////////////////////////////////////////////