/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara

  Zigbee Cluster Class
*/
#include "zbEndpoint.h"
#include "zbCluster.h"
#include "zbNode.h"

#include <iostream> // TODEL
#include <esp_log.h> // TODEL
#include "esp_err.h"
#include "esp_check.h"


//static const char *ZCLUSTER_TAG = "ZCL_CPP";



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


void ZbCluster::addCustomAttribute(uint16_t attr_id, void* value, 
                    uint8_t attr_type, uint8_t attr_access)
{
    ESP_ERROR_CHECK(esp_zb_cluster_add_attr(_attr_list, getId(),
                            attr_id, attr_type, attr_access, value));

}

esp_zb_zcl_cluster_t* ZbCluster::getClusterStruct()
{
    return &_cluster;
}


esp_zb_zcl_attr_t* ZbCluster::getAttribute(uint16_t attr_id) const
{
  
    if (getEndpointId()) {
        // If cluster mounted on endpoint, search attribute 
        // using builtin library esp_zb_zcl_get_attribute
        esp_zb_zcl_attr_t* attr = esp_zb_zcl_get_attribute(getEndpointId(),
                        getId(), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, attr_id);
        return attr;
    } else {
        // Need to search attribute even without Endpoint (ZbCluster copy constructor)
        esp_zb_attribute_list_t* attr_list = _attr_list->next;

        if (attr_list == nullptr)
        {
            ESP_LOGW("ZbCluster", "getAttribute error, attribute list is empty,Endpoint %d - Cluster %d - attrId %d",
                    getEndpointId(),getId(), attr_id);
            return nullptr;
        }

        while(attr_list->attribute.id != attr_id){
            attr_list = attr_list->next;

            if(!attr_list){
                ESP_LOGW("ZbCluster", "getAttribute error, attribute not found,Endpoint %d - Cluster %d - attrId %d",
                    getEndpointId(),getId(), attr_id);
                return nullptr;
            }
        }

        return &(attr_list->attribute);
    }

    return nullptr; // Should never reach here
    
}

uint16_t ZbCluster::getId() const
{
    return _cluster.cluster_id;  
}


uint8_t  ZbCluster::getEndpointId() const
{
    if(_endPoint)
        return _endPoint->getId();

    return 0;
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
    ESP_LOGV(ZCLUSTER_TAG, "setAttribute Endpoint (%d), Cluster (%d), attr (%d)",
            getEndpointId(), getId(), attr_id);
    
    esp_zb_lock_acquire(portMAX_DELAY);

    esp_zb_zcl_status_t res = esp_zb_zcl_set_attribute_val(getEndpointId(),
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
    ESP_LOGV(ZCLUSTER_TAG, "setEndPoint Endpoint (%d), Cluster (%d)",
                                            parent, getId());
    _endPoint = parent;
}

/*
void ZbCluster::reportAttribute()
{
    std::cout << "Periodic Task "  << +_updatePeriod << std::endl;
    esp_zb_zcl_report_attr_cmd_t cmd_req;
    cmd_req.zcl_basic_cmd.dst_addr_u.addr_short = 0x0000;
    cmd_req.zcl_basic_cmd.src_endpoint = getEndpointId();
    cmd_req.address_mode = ESP_ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT; //ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT; //ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    cmd_req.clusterID = getId();
    cmd_req.attributeID = ESP_ZB_ZCL_ATTR_METERING_CURRENT_SUMMATION_DELIVERED_ID;
    cmd_req.cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE;                  
    esp_err_t ret;
    esp_zb_lock_acquire(portMAX_DELAY);
    ret = esp_zb_zcl_report_attr_cmd_req(&cmd_req);
    std::cout << "cmd sent "  << +ret << std::endl;
    esp_zb_lock_release();

}
*/

uint8_t ZbCluster::sendCommand(uint16_t cmd)
{
    if(!_endPoint)
        return 0;

    esp_zb_zcl_custom_cluster_cmd_req_t cmd_req;
    uint8_t ret;

    cmd_req.zcl_basic_cmd.src_endpoint = getEndpointId();
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
    cmd_req.zcl_basic_cmd.src_endpoint = getEndpointId();

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
    std::vector<attribute_t> attrToPost;
    attrToPost.push_back({attr_id, value});
    //postEvent(ATTR_UPDATED_REMOTELY, attr_id, value);
    postEvent(ATTR_UPDATED_REMOTELY, attrToPost);
    
}

void ZbCluster::defaultCommandTriggered(uint8_t cmd)
{
    switch (cmd){
        case 0x0a: // Report Attribute command
            postEvent(ATTR_REPORTED, 0, 0);
            break; 
        default:
            ESP_LOGW(ZCLUSTER_TAG, "defaultCommandTriggered with cmd(0x%x) - Add action in ZbCluster - defaultCommandTriggered", cmd);
            break;
    }

}

esp_err_t ZbCluster::attributesWereRead(esp_zb_zcl_read_attr_resp_variable_t* attrs)
{
    esp_zb_zcl_attr_t* local_attr = nullptr;
    std::vector<attribute_t> attrToPost;
    while (attrs){

        if(attrs->status == ESP_ZB_ZCL_STATUS_SUCCESS)
        {
            // look up if the attribute exist in this cluster
            local_attr = esp_zb_zcl_get_attribute(getEndpointId(), 
                                getId(),
                                isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : 
                                                ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                attrs->attribute.id );
            if(local_attr){
                if(local_attr->type == attrs->attribute.data.type){
                    //everything is fine, setup the value locally
                    esp_zb_zcl_status_t ret = esp_zb_zcl_set_attribute_val(getEndpointId(),
                                getId(),
                                isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 
                                local_attr->id, 
                                attrs->attribute.data.value, 
                                false);
                    if(ret != ESP_ZB_ZCL_STATUS_SUCCESS)
                        ESP_LOGW(ZCLUSTER_TAG, "Endpoint (%d), Cluster (%d), read attr id (%d) error setting local value (%x)",
                        getEndpointId(), getId(), attrs->attribute.id, ret);
                    else 
                        //postEvent(ATTR_UPDATED_AFTER_READ,local_attr->id, local_attr->data_p);
                        attrToPost.push_back({local_attr->id, local_attr->data_p});
                } else { // Read attribute type is not the same as cluster attr type
                    ESP_LOGW(ZCLUSTER_TAG, "Endpoint (%d), Cluster (%d), read attr id (%d) is type (%x) as local type is (%x)",
                        getEndpointId(), getId(), attrs->attribute.id, attrs->attribute.data.type, local_attr->type );
                }
            } else {  // attr don't exist
                ESP_LOGW(ZCLUSTER_TAG, "Endpoint (%d), Cluster (%d), read attr id (%d): attr doesn't locally exist",
                        getEndpointId(), getId(), attrs->attribute.id);
            }
        } else {
            ESP_LOGW(ZCLUSTER_TAG, "Endpoint (%d), Cluster (%d), read attr id (%d): error status(%d)",
                        getEndpointId(), getId(), attrs->attribute.id, attrs->status);
        }
        attrs = attrs->next;
    } // while(attrs)

    postEvent(ATTR_UPDATED_AFTER_READ, attrToPost);

    return ESP_OK;
}


// TODO Implement
void ZbCluster::setReporting(uint16_t attr_id, void* reportable_change, uint16_t min_interval, 
                        uint16_t max_interval )
{
    esp_zb_zcl_attr_t* attr = getAttribute(attr_id);
    if (attr == nullptr) {
        ESP_LOGW(ZCLUSTER_TAG, "Unable to setReporting, Endpoint (%d), Cluster (%d),  attr id (%d) doesn't exist",
            getEndpointId(), getId(), attr_id);
        return;
    }
    
    esp_zb_zcl_config_report_cmd_t report_cmd;

    //report_cmd.zcl_basic_cmd.dst_addr_u.addr_short = esp_zb_get_short_address();
    report_cmd.zcl_basic_cmd.dst_addr_u.addr_short = 0x0000; // Coordinator
    report_cmd.zcl_basic_cmd.dst_endpoint = getEndpointId();
    report_cmd.zcl_basic_cmd.src_endpoint = getEndpointId();
    report_cmd.address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
    report_cmd.clusterID = getId();
    
    esp_zb_zcl_config_report_record_t record;
    
    record.direction = ESP_ZB_ZCL_REPORT_DIRECTION_SEND;
    record.attributeID = attr_id;
    record.attrType = attr->type;
    record.min_interval = min_interval;
    record.max_interval = max_interval;
    record.reportable_change = reportable_change;

    report_cmd.record_number = 1;
    report_cmd.record_field = &record;

    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_config_report_cmd_req(&report_cmd);
    esp_zb_lock_release();
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
