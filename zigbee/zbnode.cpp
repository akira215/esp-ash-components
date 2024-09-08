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
//std::list<ZbCluster>  ZbNode::_clusterList = {};

#ifdef ZB_USE_LED
BlinkTask* ZbNode::_ledBlinking = nullptr;
GpioOutput ZbNode::_led { (gpio_num_t)CONFIG_ZB_LED }; //TODO led pin number in config file
#endif


static const char *ZB_TAG = "ZB_CPP";

/// @brief this Callback shall be implemented in esp_zb stack
/// it just call the ZbNode method
/// @param event 
/// @return 
void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    ZbNode::getInstance()->handleBdbEvent(signal_struct);
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, 
                    const void *message)
{
    return ZbNode::getInstance()->handleZbActions(callback_id, message);               
}

static bool zb_apsde_data_indication_handler(esp_zb_apsde_data_ind_t ind)
{
    if (!ind.status && ind.cluster_id == 0xFFF1 && ind.profile_id == 0x0104 && ind.dst_endpoint == 10) {
        ESP_LOGI(ZB_TAG, "APS data size: %ld", ind.asdu_length);
        return true;
    }
    return false;
}

ZbNode* ZbNode::getInstance()
{
   static ZbNode instance;
   return &instance;
}

ZbNode::ZbNode()
{ 
    esp_zb_platform_config_t config;
    config.radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG();
    config.host_config = ESP_ZB_DEFAULT_HOST_CONFIG();
    
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    _initNetwork();  // TODO move in constructor if require

    _initEndPointList();

}

ZbNode::~ZbNode()
{
   

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

void ZbNode::_initNetwork()
{
    // This is in xTaskCreate
    /* Initialize Zigbee stack */
    esp_zb_cfg_t zb_nwk_cfg;
    zb_nwk_cfg.esp_zb_role = ZB_DEVICE_TYPE;
    zb_nwk_cfg.install_code_policy = INSTALLCODE_POLICY_ENABLE;
    zb_nwk_cfg.nwk_cfg.zczr_cfg.max_children = ED_MAX_CHILDREN;
    zb_nwk_cfg.nwk_cfg.zed_cfg.ed_timeout = ED_AGING_TIMEOUT;
    zb_nwk_cfg.nwk_cfg.zed_cfg.keep_alive = ED_KEEP_ALIVE;

    esp_zb_init(&zb_nwk_cfg);

}

void ZbNode::_initEndPointList()
{
    _ep_list = esp_zb_ep_list_create();
}

bool ZbNode::isJoined()
{
    return esp_zb_bdb_dev_joined();
}

void ZbNode::handleBdbEvent(esp_zb_app_signal_t *event)
{
    uint32_t *p_sg_p     = event->p_app_signal;
    esp_err_t err_status = event->esp_err_status;
    esp_zb_app_signal_type_t sig_type = static_cast<esp_zb_app_signal_type_t>(*p_sg_p);

    switch(sig_type)
    {
        case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
            ESP_LOGD(ZB_TAG, "Initialize Zigbee stack");
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
            break;
        case ESP_ZB_ZDO_SIGNAL_LEAVE:
            handleLeaveNetwork(err_status);
            break;
        case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
        case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
            handleDeviceReboot(err_status);
            break;
        case ESP_ZB_BDB_SIGNAL_STEERING:
            handleNetworkSteering(err_status);
            break;
        case ESP_ZB_ZDO_SIGNAL_PRODUCTION_CONFIG_READY:
            // esp_zb_set_node_descriptor_manufacturer_code(uint16_t manufacturer_code);
            ESP_LOGD(ZB_TAG, "Config Ready, status: %s ",esp_err_to_name(err_status));
            break;
        case ESP_ZB_SE_SIGNAL_REJOIN:
            ESP_LOGD(ZB_TAG, "Device Rejoin: %s ",esp_err_to_name(err_status));
            break;
        case ESP_ZB_NLME_STATUS_INDICATION:
            handleNetworkStatus(err_status);
            break;
        default:
            ESP_LOGW(ZB_TAG, "ZDO signal: %s (0x%x), status: %s", 
                    esp_zb_zdo_signal_to_string(sig_type), sig_type,
                    esp_err_to_name(err_status));
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
            //bindAttribute(10); //////////TODEL////////////////////////////////////////////////
        }
    } else {
        // commissioning failed 
        ESP_LOGW(ZB_TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err));
    }
}

void ZbNode::handleNetworkStatus(esp_err_t err)
{
    
    #ifdef ZB_USE_LED
    if(isJoined()) 
    {
       ledFlash(0);
    } else {
        ledFlash(50); //very short flash
    }
    #endif
    ESP_LOGD(ZB_TAG, "Network Layer Management, status: %s", 
                    esp_err_to_name(err));
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


///////////////////////////////////////////////////////////////////


void ZbNode::print_binding_table_cb(const esp_zb_zdo_binding_table_info_t *record, void *user_ctx) {
	if (!record) {
		ESP_LOGI(ZB_TAG, "Binding table nullptr");
		return;
	}
/*
	ESP_LOGI(ZB_TAG, "%03u/%03u [%02x,%02x]: %s/%u/%04x -> %s/%u",
		record->index, record->total, record->count, record->status,
		zigbee_address_string(record->record.src_address).c_str(),
		record->record.src_endp, record->record.cluster_id,
		(record->record.dst_addr_mode == ESP_ZB_ZDO_BIND_DST_ADDR_MODE_16_BIT_GROUP
			? zigbee_address_string(record->record.dst_address.addr_short).c_str()
			: zigbee_address_string(record->record.dst_address.addr_long).c_str()),
		record->record.dst_endp);
  */  
    ESP_LOGI(ZB_TAG, "%03u/%03u [%02x,%02x]: %s/%u/%04x -> %s/%u",
		record->index, record->total, record->count, record->status,
        ZbDebug::addr2string(record->record->src_address).c_str(),
        record->record->src_endp, record->record->cluster_id,
        
        (record->record->dst_addr_mode == ESP_ZB_ZDO_BIND_DST_ADDR_MODE_16_BIT_GROUP
			? "ZbDebug::addr2string(record->record->dst_address.addr_short).c_str()"
			: ZbDebug::addr2string(record->record->dst_address.addr_long).c_str()),
            
		record->record->dst_endp);

	if (record->index + 1 < record->total) {
		esp_zb_scheduler_alarm_cancel(print_binding_table_next, record->index + 1);
		esp_zb_scheduler_alarm(print_binding_table_next, record->index + 1, 100);
	}
}

void ZbNode::print_binding_table_next(uint8_t index) {
	esp_zb_zdo_mgmt_bind_param_t req{};
	req.start_index = index;
	req.dst_addr = esp_zb_get_short_address();

	esp_zb_zdo_binding_table_req(&req, print_binding_table_cb, nullptr);
}


void ZbNode::print_binding_table() {
	ESP_LOGI(ZB_TAG, "Binding table:");
	print_binding_table_next(0);
}

////////////////////////////////////////////////////////////////////////



void ZbNode::start()
{
    //Register the device 
    esp_zb_device_register(_ep_list);
    esp_zb_core_action_handler_register(zb_action_handler);


    esp_zb_aps_data_indication_handler_register(zb_apsde_data_indication_handler);

    //xTaskCreate(zbTask, "Zigbee_Device", 4096, NULL, 5, NULL);
    xTaskCreate(zbTask, "Zigbee_Device", 8192, NULL, 5, &_zbTask);
}

void ZbNode::zbTask(void *pvParameters)
{
/*
    // Config the reporting info  
    esp_zb_zcl_reporting_info_t reporting_info;
    reporting_info.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV;
    reporting_info.ep = 10;
    reporting_info.cluster_id = ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT;
    reporting_info.cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE;
    reporting_info.dst.profile_id = ESP_ZB_AF_HA_PROFILE_ID;
    reporting_info.u.send_info.min_interval = 1;
    reporting_info.u.send_info.max_interval = 670;
    reporting_info.u.send_info.def_min_interval = 1;
    reporting_info.u.send_info.def_max_interval = 960;
    reporting_info.u.send_info.delta.u16 = 100;
    reporting_info.attr_id = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID;
    reporting_info.manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC;

    esp_zb_zcl_update_reporting_info(&reporting_info);   
*/
    //esp_zb_zcl_config_report_cmd_req();

    esp_zb_set_primary_network_channel_set(ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK); //TODO evaluate this macro
    //esp_zb_set_secondary_network_channel_set(ESP_ZB_SECONDARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));

    //esp_zb_main_loop_iteration();
    print_binding_table();

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

//// TODO include in class !
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
    
    ESP_RETURN_ON_FALSE(msg, ESP_FAIL, ZB_TAG, "Empty message");
    ESP_RETURN_ON_FALSE(msg->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, 
                        ZB_TAG, "Set Attribute received message: error status(%d)",
                        msg->info.status);
    ESP_LOGD(ZB_TAG, "Received set attr message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", 
            msg->info.dst_endpoint, msg->info.cluster,
            msg->attribute.id, msg->attribute.data.size);
    
    bool res = _endPointMap[msg->info.dst_endpoint]->getCluster(msg->info.cluster)->
                        setAttribute(msg->attribute.id, msg->attribute.data.value);
    if(!res){
        ESP_LOGW(ZB_TAG, "No callback for endpoint(%d), cluster(0x%x), attribute(0x%x)",
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
