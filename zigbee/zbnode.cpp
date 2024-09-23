/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "zbNode.h"

#include <stdio.h>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>
#include "esp_check.h"
#include "esp_err.h"
#include <string.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//#include "ha/esp_zigbee_ha_standard.h"

#include "esp_log.h"


#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE in idf.py menuconfig to compile sensor (End Device) source code.
#endif

// Static init
TaskHandle_t ZbNode::_zbTask = NULL;
esp_zb_ep_list_t* ZbNode::_ep_list = nullptr;
std::map<uint8_t,ZbEndPoint*> ZbNode::_endPointMap = {};

#ifdef ZB_USE_LED
BlinkTask* ZbNode::_ledBlinking = nullptr;
GpioOutput ZbNode::_led { (gpio_num_t)CONFIG_ZB_LED }; //TODO led pin number in config file
#endif


static const char *ZB_TAG = "ZB_CPP";


/// @brief this Callback shall be implemented in esp_zb stack
/// it just call the ZbNode method
/// @param event 
/// @return 
extern "C" void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    ZbNode::getInstance()->handleBdbEvent(static_cast<esp_zb_app_signal_type_t>(*signal_struct->p_app_signal),
                            signal_struct->esp_err_status,
                            static_cast<uint32_t*>(esp_zb_app_signal_get_params(signal_struct->p_app_signal)));
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, 
                    const void *message)
{
    return ZbNode::getInstance()->handleZbActions(callback_id, message);               
}

ZbNode* ZbNode::getInstance()
{
   static ZbNode instance;
   return &instance;
}

ZbNode::ZbNode()
{ 
    esp_zb_platform_config_t config;
    config.radio_config.radio_mode = ZB_RADIO_MODE_NATIVE;
    config.host_config.host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE;
    
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    // Initialize Network 
    esp_zb_cfg_t zb_nwk_cfg;
    zb_nwk_cfg.esp_zb_role = ZB_DEVICE_TYPE;
    zb_nwk_cfg.install_code_policy = INSTALLCODE_POLICY_ENABLE;
    zb_nwk_cfg.nwk_cfg.zczr_cfg.max_children = ED_MAX_CHILDREN;
    zb_nwk_cfg.nwk_cfg.zed_cfg.ed_timeout = ED_AGING_TIMEOUT;
    zb_nwk_cfg.nwk_cfg.zed_cfg.keep_alive = ED_KEEP_ALIVE;

    esp_zb_init(&zb_nwk_cfg);

    _ep_list = esp_zb_ep_list_create();

}

ZbNode::~ZbNode()
{
    //TODO del all ZbEndPoint objects
}

void ZbNode::ledFlash(uint64_t speed)
{
    #ifdef ZB_USE_LED
    if(speed == 0) 
    {
        if(_ledBlinking){
        delete _ledBlinking;
        _ledBlinking = nullptr;
        }
        _led.off();
    } else if(speed == -1) {
        if(_ledBlinking){
        delete _ledBlinking;
        _ledBlinking = nullptr;
        }
        _led.on();
    } else {
        if(!_ledBlinking)
            _ledBlinking = new BlinkTask(_led, speed); // very short flash
        else
            _ledBlinking->setBlinkPeriod(speed);
    }
    #endif

}

bool ZbNode::isJoined()
{
    return esp_zb_bdb_dev_joined();
}

//void ZbNode::handleBdbEvent(esp_zb_app_signal_t *event)
void ZbNode::handleBdbEvent(esp_zb_app_signal_type_t signal_type,
                        esp_err_t status,
                        uint32_t *p_data)
{
    switch(signal_type)
    {
        case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
            ESP_LOGD(ZB_TAG, "Initialize Zigbee stack");
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
            break;
        case ESP_ZB_ZDO_SIGNAL_LEAVE:
            handleLeaveNetwork(status);
            break;
        case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
        case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
            handleDeviceReboot(status);
            break;
        case ESP_ZB_BDB_SIGNAL_STEERING:
            handleNetworkSteering(status);
            break;
        case ESP_ZB_ZDO_SIGNAL_PRODUCTION_CONFIG_READY:
            // esp_zb_set_node_descriptor_manufacturer_code(uint16_t manufacturer_code);
            ESP_LOGD(ZB_TAG, "Config Ready, status: %s ",esp_err_to_name(status));
            break;
        case ESP_ZB_SE_SIGNAL_REJOIN:
            ESP_LOGD(ZB_TAG, "Device Rejoin: %s ",esp_err_to_name(status));
            break;
        case ESP_ZB_NLME_STATUS_INDICATION:
            handleNetworkStatus(status, static_cast<void*>(p_data));
            break;
        case ESP_ZB_BDB_SIGNAL_FINDING_AND_BINDING_TARGET_FINISHED:
            ESP_LOGI(ZB_TAG, "*************Binding Target ****************");
            break;
        case ESP_ZB_BDB_SIGNAL_FINDING_AND_BINDING_INITIATOR_FINISHED:
            ESP_LOGI(ZB_TAG, "*************Binding Initiator ****************");
            break;
        default:
            ESP_LOGW(ZB_TAG, "ZDO signal: %s (0x%x), status: %s", 
                    esp_zb_zdo_signal_to_string(signal_type), signal_type,
                    esp_err_to_name(status));
            break;
    }
}

void ZbNode::handleDeviceReboot(esp_err_t err)
{
    if (err == ESP_OK) {
        //ESP_LOGI(TAG, "Deferred driver initialization %s", deferred_driver_init() ? "failed" : "successful");
        ESP_LOGD(ZB_TAG, "Device started up in %s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "non");
        if (esp_zb_bdb_is_factory_new()) {
            joinNetwork();
        } else {
            ESP_LOGD(ZB_TAG, "Device rebooted successfully, Short Address: (0x%04hx)",
                    esp_zb_get_short_address());
            ledFlash(0);
            //bindAttribute(10); //////////TODEL////////////////////////////////////////////////
        }
    } else {
        // commissioning failed 
        ESP_LOGW(ZB_TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err));
        ESP_LOGI(ZB_TAG, "Retrying in 1 sec...");
        esp_zb_scheduler_alarm((esp_zb_callback_t)joinNetwork, 0, 1000);
    }
}

void ZbNode::handleNetworkStatus(esp_err_t err, void* data)
{
    
    if(isJoined()) 
    {
       ledFlash(0);
    } else {
        ledFlash(50); //very short flash
    }

    struct nlme_status_indication {
				uint8_t status;
				uint16_t network_addr;
				uint8_t unknown_command_id;
			} __attribute__((packed)) *params = static_cast<struct nlme_status_indication*>(data);
    ESP_LOGW(ZB_TAG, "NLME status indication: %02x addr(0x%04x) Cmd(%02x)",
				params->status, params->network_addr, params->unknown_command_id);
}

void ZbNode::handleNetworkSteering(esp_err_t err)
{

    if (err == ESP_OK) {

        esp_zb_ieee_addr_t extended_pan_id;
        esp_zb_get_extended_pan_id(extended_pan_id);
        ESP_LOGD(ZB_TAG, "Joined network successfully  \
                    (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, \
                    PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                    extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                    extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                    esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
        

        ledFlash(0);
    } else {
        uint32_t delay_ms = 500;
        ESP_LOGW(ZB_TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err));
        ESP_LOGI(ZB_TAG, "Retrying in : %ld ms", delay_ms);
        esp_zb_scheduler_alarm((esp_zb_callback_t)joinNetwork, 0, delay_ms);
    }
    
}

void ZbNode::handleLeaveNetwork(esp_err_t err)
{
    #ifdef ZB_USE_LED
    ledFlash(-1);
    #endif

    ESP_LOGW(ZB_TAG, "Device left the network (status: %s)", esp_err_to_name(err));
}


esp_err_t ZbNode::joinNetwork(uint8_t param)
{
    if(isJoined()){
        ESP_LOGI(ZB_TAG, "Device already joined, should leave before trying to join");
        return ESP_ERR_NOT_ALLOWED;
    }
    
    ESP_LOGD(ZB_TAG, "Start network steering");

    ledFlash(FAST_BLINK);

    return esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);

}

void ZbNode::leaveNetwork()
{
    if(!isJoined()){
        ESP_LOGI(ZB_TAG, "Device is not joined, useless leave request");
        return;
    }
    
    ESP_LOGD(ZB_TAG, "Leaving the network");

    ledFlash(50);
    
    esp_zb_zdo_mgmt_leave_req_param_t param{};

    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_get_long_address(param.device_address);
    param.dst_nwk_addr = 0xffff;
    param.rejoin = 0;

    esp_zb_zdo_device_leave_req(&param, nullptr, nullptr);
    esp_zb_lock_release();
    //esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);

}

void ZbNode::start()
{
    //Register the device 
    esp_zb_device_register(_ep_list);
    esp_zb_core_action_handler_register(zb_action_handler);
    //xTaskCreate(zbTask, "Zigbee_Device", 4096, NULL, 5, NULL);
    xTaskCreate(zbTask, "Zigbee_Device", CONFIG_ZB_STACK_DEPTH, NULL, 5, &_zbTask);
}

void ZbNode::zbTask(void *pvParameters)
{

    esp_zb_set_primary_network_channel_set(ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK); //TODO evaluate this macro
    //esp_zb_set_secondary_network_channel_set(ESP_ZB_SECONDARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));

    esp_zb_stack_main_loop();
}


/*---------------------------------------------------------------------------------------------*/

void ZbNode::addEndPoint(ZbEndPoint& ep)
{   
    uint8_t EpId = ep.getId();
    ESP_LOGD(ZB_TAG,"Adding endpoint id %d", EpId);
    _endPointMap[EpId] = &ep;

    esp_zb_ep_list_add_ep(_ep_list, 
                        _endPointMap[EpId]->getClusterList(), 
                        _endPointMap[EpId]->getConfig());
}

ZbEndPoint* ZbNode::getEndPoint(uint8_t endp_id)
{
    auto it = _endPointMap.find(endp_id);
    if (it == _endPointMap.end()){
         ESP_LOGW(ZB_TAG, "getEndPoint - No endpoint %d found", endp_id);
         return nullptr;
    }
    return it->second;
}

/*---------------------------------------------------------------------------------------------*/
//// TODO include in class !
static esp_err_t zb_attribute_reporting_handler(const esp_zb_zcl_report_attr_message_t *message)
{
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, ZB_TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, ZB_TAG, "Received message: error status(%d)",
                        message->status);
    ESP_LOGI(ZB_TAG, "Received report from address(0x%x) src endpoint(%d) to dst endpoint(%d) cluster(0x%x)", message->src_address.u.short_addr,
             message->src_endpoint, message->dst_endpoint, message->cluster);
    ESP_LOGI(ZB_TAG, "Received report information: attribute(0x%x), type(0x%x), value(%d)\n", message->attribute.id, message->attribute.data.type,
             message->attribute.data.value ? *(uint8_t *)message->attribute.data.value : 0);
    return ESP_OK;
}


esp_err_t ZbNode::handlingCmdDefaultResp(const esp_zb_zcl_cmd_default_resp_message_t *msg)
{
    ESP_RETURN_ON_FALSE(msg, ESP_FAIL, ZB_TAG, "Empty message");
    ESP_RETURN_ON_FALSE(msg->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, 
                        ZB_TAG, "Default response received message: error status(%d)",
                        msg->info.status);
    ESP_LOGD(ZB_TAG, "Default response status(%d) from src endpoint(%d) cluster(0x%x) cmd was(%d)", 
                        msg->status_code,
                        msg->info.src_endpoint, 
                        msg->info.cluster, 
                        msg->resp_to_cmd);
    return ESP_OK;   
}

esp_err_t ZbNode::handlingCmdSetAttribute(const esp_zb_zcl_set_attr_value_message_t *msg)
{
    
    ESP_RETURN_ON_FALSE(msg, ESP_FAIL, ZB_TAG, "Set Attr - Empty message");
    ESP_RETURN_ON_FALSE(msg->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, 
                        ZB_TAG, "Set Attribute received message: error status(%d)",
                        msg->info.status);
    ESP_LOGD(ZB_TAG, "Received set attr message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", 
            msg->info.dst_endpoint, msg->info.cluster,
            msg->attribute.id, msg->attribute.data.size);

    auto it = _endPointMap.find(msg->info.dst_endpoint);
    if (it == _endPointMap.end()){
         ESP_LOGW(ZB_TAG, "Set Attr - No endpoint %d found", msg->info.dst_endpoint);
         return ESP_ERR_NOT_FOUND;
    }

    ZbCluster* cluster = it->second->getCluster(msg->info.cluster, false);
    if (!cluster){
        ESP_LOGW(ZB_TAG, "Set Attr - No cluster %d found for endpoint %d", 
                        msg->info.cluster, msg->info.dst_endpoint);
        return ESP_ERR_NOT_FOUND;
    }

    bool res = cluster->attributeWasSet(msg->attribute.id, msg->attribute.data.value);
    if(!res){
        ESP_LOGW(ZB_TAG, "Set Attr - No callback for endpoint(%d), cluster(0x%x), attribute(0x%x)",
                msg->info.dst_endpoint, msg->info.cluster,
                msg->attribute.id);
        return ESP_ERR_NOT_FOUND;
    }

    return ESP_OK;   
}


esp_err_t ZbNode::handleZbActions(esp_zb_core_action_callback_id_t callback_id, 
                                        const void *message)
{
    esp_err_t ret = ESP_OK;
    switch (callback_id) {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
        ret = handlingCmdSetAttribute((esp_zb_zcl_set_attr_value_message_t*)message);
        break;
    case ESP_ZB_CORE_REPORT_ATTR_CB_ID:
        ret = zb_attribute_reporting_handler((esp_zb_zcl_report_attr_message_t *)message);
        break;
    
    case ESP_ZB_CORE_CMD_DEFAULT_RESP_CB_ID:
        ret = handlingCmdDefaultResp((esp_zb_zcl_cmd_default_resp_message_t *)message);
        break;
    /*
    case ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID:
        ret = zb_read_attr_resp_handler((esp_zb_zcl_cmd_read_attr_resp_message_t *)message);
        break;
    case ESP_ZB_CORE_CMD_REPORT_CONFIG_RESP_CB_ID:
        ret = zb_configure_report_resp_handler((esp_zb_zcl_cmd_config_report_resp_message_t *)message);
        break;
        */
    default:
        ESP_LOGW(ZB_TAG, "Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////
esp_err_t ZbNode::sendCommand(uint8_t endp, uint16_t cluster_id, bool isClient, uint16_t cmd)
{
    auto it = _endPointMap.find(endp);
    if (it == _endPointMap.end()){
         ESP_LOGW(ZB_TAG, "Send command - No endpoint %d found", endp);
         return ESP_ERR_NOT_FOUND;
    }

    ZbCluster* cluster = it->second->getCluster(cluster_id, isClient);
    if (!cluster){
        ESP_LOGW(ZB_TAG, "Send command - No cluster %d found for endpoint %d", 
                        endp, cluster_id);
        return ESP_ERR_NOT_FOUND;
    }

    cluster->sendCommand(cmd);

    return ESP_OK;   
}

esp_err_t ZbNode::setAttribute(uint8_t endp, uint16_t cluster_id, bool isClient, uint16_t attrId, void* value)
{
    auto it = _endPointMap.find(endp);
    if (it == _endPointMap.end()){
         ESP_LOGW(ZB_TAG, "Set Attr - No endpoint %d found", endp);
         return ESP_ERR_NOT_FOUND;
    }

    ZbCluster* cluster = it->second->getCluster(cluster_id, isClient);
    if (!cluster){
        ESP_LOGW(ZB_TAG, "Set Attr - No cluster %d found for endpoint %d", 
                        endp, cluster_id);
        return ESP_ERR_NOT_FOUND;
    }

    /////cluster->set;

    return ESP_OK;   
}


////////////////////////////////////////////////////////////////////////////////////////////////
