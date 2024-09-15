/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  Zigbee End Point Class
*/
#include "zbEndpoint.h"
#include "zbCluster.h"

#include <iostream> // TODEL



ZbCluster::ZbCluster()
{
    
}

ZbCluster::~ZbCluster()
{
    // Destructor seems useless as the doc says:
    // After successful registration, the SDK will retain a copy of the whole data model, 
    // the ep_list will be freed.
    
}

void ZbCluster::_init(uint16_t id, bool isClient){
    _cluster.cluster_id = id;
    _cluster.attr_count = 0;
    //_cluster.attr_list = &_attrList.back();
    _cluster.attr_list = _attr_list;
    isClient ? _cluster.role_mask = ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE :
                _cluster.role_mask = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE;
    _cluster.manuf_code = 0; // TODO check
    _cluster.cluster_init = nullptr;//esp_zb_zcl_cluster_init_t cluster init callback
    
}

void ZbCluster::_copyAttributes(const ZbCluster& other)
{
    esp_zb_attribute_list_t* attr_list = other._attr_list->next;

    while(attr_list){
        if (getAttribute(attr_list->attribute.id) == nullptr)
           addAttribute(attr_list->attribute.id, attr_list->attribute.data_p);

        attr_list = attr_list->next;
    }
    _callback = other._callback;

}

esp_zb_zcl_cluster_t* ZbCluster::getClusterStruct()
{
    return &_cluster;
}


esp_zb_zcl_attr_t* ZbCluster::getAttribute(uint16_t attr_id) const
{
    esp_zb_attribute_list_t* attr_list = _attr_list->next;

    while(attr_list->attribute.id != attr_id){
        attr_list = attr_list->next;

        if(!attr_list)
            return nullptr;
    }

    return &(attr_list->attribute);
}

uint16_t ZbCluster::getId() const
{
    return _cluster.cluster_id;  
}

bool ZbCluster::isClient() const
{
    if(_cluster.role_mask == ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE)
        return true;
    
    return false;
}

bool ZbCluster::isServer() const
{
    if(_cluster.role_mask == ESP_ZB_ZCL_CLUSTER_SERVER_ROLE)
        return true;
    
    return false;
}

void ZbCluster::setCallback(clusterCb callback)
{
    _callback = callback;
}

bool ZbCluster::setAttribute(uint16_t attr_id, void* value)
{
    if(_callback){
        _callback(attr_id, value);
        return true;
    }
    return false;
}

void ZbCluster::setEndPoint(ZbEndPoint* parent)
{
    _endPoint = parent;
}

uint8_t ZbCluster::sendCommand(uint16_t cmd)
{
    if(!_endPoint)
        return 0;

    esp_zb_zcl_custom_cluster_cmd_req_t cmd_req;
    uint8_t ret;

    cmd_req.zcl_basic_cmd.src_endpoint = _endPoint->getId();
    cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
    cmd_req.profile_id = ESP_ZB_AF_HA_PROFILE_ID;
    cmd_req.cluster_id = getId();
    cmd_req.custom_cmd_id = cmd;
    cmd_req.direction = isClient() ? ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV : ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;

    esp_zb_lock_acquire(portMAX_DELAY); 
    ret = esp_zb_zcl_custom_cluster_cmd_req(&cmd_req);
    esp_zb_lock_release();

    return ret;
}