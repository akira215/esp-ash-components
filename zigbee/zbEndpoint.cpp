/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  Zigbee End Point Class
*/

#include "zbEndpoint.h"




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

}

ZbEndPoint::~ZbEndPoint()
{
    for(ZbCluster* cluster: _vecCluster)
        delete cluster;
    
    _vecCluster.clear();   
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