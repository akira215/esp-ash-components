/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  Zigbee End Point Class
*/
#include "zbEndpoint.h"
#include "zbCluster.h"
#include "zbNode.h"

#include <iostream> // TODEL
#include <esp_log.h> // TODEL
#include "esp_err.h"
#include "esp_check.h"

static const char *ZCL_TAG = "ZCL_CPP";



ZbCluster::ZbCluster()
{
 
}

ZbCluster::~ZbCluster()
{
    // Destructor seems useless as the doc says:
    // After successful registration, the SDK will retain a copy of the 
    // whole data model, 
    // the ep_list will be freed.
    //esp_event_handler_unregister();
}

void ZbCluster::_init(uint16_t id, bool isClient){
    _cluster.cluster_id = id;
    _cluster.attr_count = 0;
 
    _cluster.attr_list = _attr_list;
    isClient ? _cluster.role_mask = ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE :
                _cluster.role_mask = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE;
    _cluster.manuf_code = 0; // TODO check
    _cluster.cluster_init = nullptr;//esp_zb_zcl_cluster_init_t 
                                    // cluster init callback
           
}

void ZbCluster::_copyAttributes(const ZbCluster& other)
{
    esp_zb_attribute_list_t* attr_list = other._attr_list->next;

    while(attr_list){
        if (getAttribute(attr_list->attribute.id) == nullptr)
           addAttribute(attr_list->attribute.id, attr_list->attribute.data_p);

        attr_list = attr_list->next;
    }

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


bool ZbCluster::setAttribute(uint16_t attr_id, void* value)
{
    esp_zb_lock_acquire(portMAX_DELAY);

    esp_zb_zcl_status_t res = esp_zb_zcl_set_attribute_val(_endPoint->getId(),
                 getId(), 
                 isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : 
                                ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 
                 attr_id, 
                 value, 
                 false);
    esp_zb_lock_release();

    return res == ESP_ZB_ZCL_STATUS_SUCCESS; 
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
    cmd_req.direction = isClient() ? ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV : 
                                    ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI;

    esp_zb_lock_acquire(portMAX_DELAY); 
    ret = esp_zb_zcl_custom_cluster_cmd_req(&cmd_req);
    esp_zb_lock_release();

    return ret;
}

uint8_t ZbCluster::readAttribute(std::span<uint16_t> attrList, uint8_t dst_endpoint, 
                                    uint16_t short_addr)
{
    esp_zb_zcl_read_attr_cmd_t cmd_req;
    uint8_t ret;

    cmd_req.zcl_basic_cmd.dst_addr_u.addr_short = short_addr;
    cmd_req.zcl_basic_cmd.dst_endpoint = dst_endpoint;
    cmd_req.zcl_basic_cmd.src_endpoint = _endPoint->getId();

    cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    
    cmd_req.clusterID = getId();
    
    cmd_req.attr_number = attrList.size();
    cmd_req.attr_field = attrList.data();
    
    esp_zb_lock_acquire(portMAX_DELAY);                                               
	ret = esp_zb_zcl_read_attr_cmd_req( &cmd_req);
    esp_zb_lock_release();

    return ret;

}

uint8_t ZbCluster::readAttribute(uint16_t attrId, uint8_t dst_endpoint, 
                                    uint16_t short_addr)
{
    uint16_t arr[1]; arr[0] = attrId;
    return readAttribute(std::span(arr));
}

void ZbCluster::attributeWasSet(uint16_t attr_id, void* value)
{ 
    postEvent(ATTR_UPDATED_REMOTELY, attr_id, value);
}

esp_err_t ZbCluster::attributesWereRead(esp_zb_zcl_read_attr_resp_variable_t* attrs)
{
    esp_zb_zcl_attr_t* local_attr = nullptr;
    std::vector<attribute_t> attrToPost;
    while (attrs){

        if(attrs->status == ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            // look up if the attribute exist in this cluster
            local_attr = esp_zb_zcl_get_attribute(_endPoint->getId(), 
                                getId(),
                                isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : 
                                                ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                attrs->attribute.id );
            if(local_attr){
                if(local_attr->type == attrs->attribute.data.type){
                    //everything is fine, setup the value locally
                    esp_zb_zcl_status_t ret = esp_zb_zcl_set_attribute_val(_endPoint->getId(),
                                getId(),
                                isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 
                                local_attr->id, 
                                attrs->attribute.data.value, 
                                false);
                    if(ret != ESP_ZB_ZCL_STATUS_SUCCESS)
                        ESP_LOGW(ZCL_TAG, "Endpoint (%d), Cluster (%d), read attr id (%d) error setting local value (%x)",
                        _endPoint->getId(), getId(), attrs->attribute.id, ret);
                    else 
                        //postEvent(ATTR_UPDATED_AFTER_READ,local_attr->id, local_attr->data_p);
                        attrToPost.push_back({local_attr->id, local_attr->data_p});
                } else { // Read attribute type is not the same as cluster attr type
                    ESP_LOGW(ZCL_TAG, "Endpoint (%d), Cluster (%d), read attr id (%d) is type (%x) as local type is (%x)",
                        _endPoint->getId(), getId(), attrs->attribute.id, attrs->attribute.data.type, local_attr->type );
                }
            } else {  // attr don't exist
                ESP_LOGW(ZCL_TAG, "Endpoint (%d), Cluster (%d), read attr id (%d): attr doesn't locally exist",
                        _endPoint->getId(), getId(), attrs->attribute.id);
            }
        } else {
            ESP_LOGW(ZCL_TAG, "Endpoint (%d), Cluster (%d), read attr id (%d): error status(%d)",
                        _endPoint->getId(), getId(), attrs->attribute.id, attrs->status);
        }
        attrs = attrs->next;
    } // while(attrs)

    postEvent(ATTR_UPDATED_AFTER_READ, attrToPost);

    return ESP_OK;
}

void ZbCluster::setReporting(uint16_t attr_id)
{
//uint8_t esp_zb_zcl_config_report_cmd_req(esp_zb_zcl_config_report_cmd_t *cmd_req)
//esp_zb_zcl_reporting_info_t *esp_zb_zcl_find_reporting_info(esp_zb_zcl_attr_location_info_t attr_info);
    esp_zb_zcl_config_report_cmd_t report;

    report.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
    report.clusterID = getId();
    


    uint8_t esp_zb_zcl_config_report_cmd_req(esp_zb_zcl_config_report_cmd_t *cmd_req);


    
    // Config the reporting info
    /*
    esp_zb_zcl_reporting_info_t reporting_info;
    reporting_info.direction = ESP_ZB_ZCL_REPORT_DIRECTION_SEND;
    reporting_info.ep = _endPoint->getId();
    reporting_info.cluster_id = getId();
    reporting_info.cluster_role = isClient() ?  ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE :
                                                ESP_ZB_ZCL_CLUSTER_SERVER_ROLE;
    reporting_info.attr_id = attr_id;
    
    reporting_info.u.send_info.min_interval = 1;
    reporting_info.u.send_info.max_interval = 670;
    reporting_info.u.send_info.def_min_interval = 1;
    reporting_info.u.send_info.def_max_interval = 960;
    reporting_info.u.send_info.delta.u16 = 100;
    
    reporting_info.dst.profile_id = ESP_ZB_AF_HA_PROFILE_ID;
    reporting_info.manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC;

    esp_zb_zcl_update_reporting_info(&reporting_info);   
*/
    //esp_zb_zcl_config_report_cmd_req();
}

/// Events ////////////////////////////////////////////////////////////////////////////
void ZbCluster::postEvent(clusterEvent_t event, std::vector<attribute_t> attrs)
{
    // Call all the registered callback Id
    for (auto & cb : _clusterEventHandlers) {
        //cb(event, attrId, value);
        ZbNode::_eventLoop->enqueue(std::bind(std::ref(cb), event, std::move(attrs)));
    }
}


void ZbCluster::postEvent(clusterEvent_t event, uint16_t attrId, void* value)
{
    std::vector<attribute_t> oneElement = {{attrId,value}};
    //std::vector<int> v = {1, 2, 3, 4};
    postEvent(event,std::move(oneElement));
}
