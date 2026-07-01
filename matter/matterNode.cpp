/*
  matter component for ESP32
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "matterNode.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <nvs_flash.h>

#include "esp_err.h"
#include <esp_log.h>




MatterNode* MatterNode::getInstance()
{
   static MatterNode instance;
   return &instance;
}


MatterNode::MatterNode()
{ 
    ESP_ERROR_CHECK(nvs_flash_init());

    node::config_t node_config;

    // node handle can be used to add/modify other endpoints.
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));
  
}

MatterNode::~MatterNode()
{
    //TODO del all ZbEndPoint objects
}
