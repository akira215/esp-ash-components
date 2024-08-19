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



#include <iostream> // todel
#include "esp_log.h"


#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE in idf.py menuconfig to compile sensor (End Device) source code.
#endif

static const char *ZB_TAG = "ESP_ZB_CPP";


/*
void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p     = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = static_cast<esp_zb_app_signal_type_t>(*p_sg_p);
    switch (sig_type) {
        case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
            ESP_LOGI(ZB_TAG, "Initialize Zigbee stack");
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
            break;
        case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
        case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
            if (err_status == ESP_OK) {
                //ESP_LOGI(TAG, "Deferred driver initialization %s", deferred_driver_init() ? "failed" : "successful");
                ESP_LOGI(ZB_TAG, "Device started up in %s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "non");
                if (esp_zb_bdb_is_factory_new()) {
                    ESP_LOGI(ZB_TAG, "Start network steering");
                    esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
                } else {
                    ESP_LOGI(ZB_TAG, "Device rebooted");
                }
            } else {
                // commissioning failed 
                ESP_LOGW(ZB_TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));
            }
            break;
        case ESP_ZB_BDB_SIGNAL_STEERING:
            if (err_status == ESP_OK) {
                esp_zb_ieee_addr_t extended_pan_id;
                esp_zb_get_extended_pan_id(extended_pan_id);
                ESP_LOGI(ZB_TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                        extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                        extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                        esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
            } else {
                ESP_LOGI(ZB_TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
                //esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
            }
            break;
        default:
            ESP_LOGI(ZB_TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type,
                    esp_err_to_name(err_status));
            break;
    }
}
*/
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
    //ESP_ERROR_CHECK(esp_zb_platform_config(&config));
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
    _ep_list = esp_zb_ep_list_create();
}



void ZbNode::joinNetwork()
{
    ESP_LOGI(ZB_TAG, "== Joining the network");

    _connectionState = JOINING;

    // Indicate we are joining
    //LEDTask::getInstance()->triggerSpecialEffect(LED_TASK_NETWORK_CONNECT_EFFECT);

     // Connect to a network status == ZB_TRUE 
    //zb_bool_t status = bdb_start_top_level_commissioning(ZB_BDB_SIGNAL_STEERING);
    //ESP_LOGI(ZB_TAG, "  BDB_eNsStartNwkSteering=%d", status);

}





void ZbNode::start()
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

    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
*/

/*
    ESP_ERROR_CHECK(esp_zb_start(false));

    esp_zb_main_loop_iteration();
*/
}