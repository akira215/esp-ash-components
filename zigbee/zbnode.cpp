/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "zbnode.h"

#include <stdio.h>

#include <esp_log.h>
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

#ifdef ZB_USE_LED
BlinkTask* ZbNode::_ledBlinking = nullptr;
GpioOutput ZbNode::_led { (gpio_num_t)CONFIG_ZB_LED }; //TODO led pin number in config file
#endif


static const char *ZB_TAG = "ESP_ZB_CPP";

/// @brief this Callback shall be implemented in esp_zb stack
/// it just call the ZbNode method
/// @param event 
/// @return 
void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    ZbNode::getInstance()->handleBdbEvent(signal_struct);
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

    _init();  // TODO move in constructor if require

}

ZbNode::~ZbNode()
{
   

}

void ZbNode::_init()
{
   
   
    // This is in xTaskCreate
    /* Initialize Zigbee stack */
    esp_zb_cfg_t zb_nwk_cfg = {
        ZB_DEVICE_TYPE, //  .esp_zb_role 
        INSTALLCODE_POLICY_ENABLE, // .install_code_policy 
    };
    zb_nwk_cfg.nwk_cfg.zczr_cfg.max_children = ED_MAX_CHILDREN;

    zb_nwk_cfg.nwk_cfg.zed_cfg = {
      ED_AGING_TIMEOUT, // ed_timeout
      ED_KEEP_ALIVE// keep_alive
    };

    esp_zb_init(&zb_nwk_cfg);

    // create the end point list
    //_ep_list = esp_zb_ep_list_create();
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
            ESP_LOGI(ZB_TAG, "Initialize Zigbee stack");
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
        //case ESP_ZB_NWK_LEAVE_TYPE_RESET:
        //    handleLeaveNetwork(err_status);
        //    break;
        case ESP_ZB_ZDO_SIGNAL_PRODUCTION_CONFIG_READY:
            // esp_zb_set_node_descriptor_manufacturer_code(uint16_t manufacturer_code);
            ESP_LOGI(ZB_TAG, "Config Ready, status: %s ",esp_err_to_name(err_status));
            break;
        case ESP_ZB_SE_SIGNAL_REJOIN:
            ESP_LOGI(ZB_TAG, "Device Rejoin: %s ",esp_err_to_name(err_status));
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
        ESP_LOGI(ZB_TAG, "Device started up in %s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "non");
        if (esp_zb_bdb_is_factory_new()) {
            joinNetwork();
        } else {
            ESP_LOGI(ZB_TAG, "Device rebooted successfully");
        }
    } else {
        // commissioning failed 
        ESP_LOGW(ZB_TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err));
    }
}

void ZbNode::handleNetworkStatus(esp_err_t err)
{
    #ifdef ZB_USE_LED
    if(!_ledBlinking)
        _ledBlinking = new BlinkTask(_led, 50); // very short flash
    else
        _ledBlinking->setBlinkPeriod(50);
    #endif
    ESP_LOGI(ZB_TAG, "Network Layer Management, status: %s", 
                    esp_err_to_name(err));
}

void ZbNode::handleNetworkSteering(esp_err_t err)
{

    if (err == ESP_OK) {

        esp_zb_ieee_addr_t extended_pan_id;
        esp_zb_get_extended_pan_id(extended_pan_id);
        ESP_LOGI(ZB_TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                    extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                    extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                    esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
        
        #ifdef ZB_USE_LED
        if(_ledBlinking){
            delete _ledBlinking;
            _ledBlinking = nullptr;
        }
        _led.off();
        #endif

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
    if(_ledBlinking){
        delete _ledBlinking;
        _ledBlinking = nullptr;
    }
    _led.on();
    #endif

    ESP_LOGW(ZB_TAG, "Device left the network (status: %s)", esp_err_to_name(err));
}


void ZbNode::joinNetwork(uint8_t param)
{
    if(isJoined()){
        ESP_LOGI(ZB_TAG, "Device already joined, should leave before trying to join");
        return;
    }
    
    ESP_LOGI(ZB_TAG, "Start network steering");

    #ifdef ZB_USE_LED
    if(!_ledBlinking)
        _ledBlinking = new BlinkTask(_led, FAST_BLINK); 
    else
         _ledBlinking->setBlinkPeriod(FAST_BLINK);
    #endif

    esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);

}

void ZbNode::leaveNetwork()
{
    if(!isJoined()){
        ESP_LOGI(ZB_TAG, "Device is not joined, useless leave request");
        return;
    }
    
    ESP_LOGI(ZB_TAG, "Leaving the network");

    #ifdef ZB_USE_LED
    if(!_ledBlinking)
        _ledBlinking = new BlinkTask(_led, 50); // very short flash
    else
        _ledBlinking->setBlinkPeriod(50);
    #endif
    
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
    //xTaskCreate(zbTask, "Zigbee_Device", 4096, NULL, 5, NULL);
    xTaskCreate(zbTask, "Zigbee_Device", 4096, NULL, 5, &_zbTask);
}

void ZbNode::zbTask(void *pvParameters)
{
    // Loop through end point to register
    /*
    for(ZbEndPoint* ep: _vecEndPoint)
        ESP_ERROR_CHECK(esp_zb_ep_list_add_ep(_ep_list, 
                                    ep->getClusterList(), 
                                    ep->getConfig()));
     
    //Register the device 
    esp_zb_device_register(_ep_list);
    */

    // Config the reporting info  
    /*
    esp_zb_zcl_reporting_info_t reporting_info = {
        .direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV,
        .ep = HA_ESP_SENSOR_ENDPOINT,
        .cluster_id = ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
        .cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        .dst.profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .u.send_info.min_interval = 1,
        .u.send_info.max_interval = 0,
        .u.send_info.def_min_interval = 1,
        .u.send_info.def_max_interval = 0,
        .u.send_info.delta.u16 = 100,
        .attr_id = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
        .manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC,
    };

    esp_zb_zcl_update_reporting_info(&reporting_info);
*/
    esp_zb_set_primary_network_channel_set(ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK); //TODO evaluate this macro
    ESP_ERROR_CHECK(esp_zb_start(false));

    esp_zb_main_loop_iteration();
}