/*
  matter component for ESP32
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "matterNode.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <nvs_flash.h>

#include <stdio.h>
#include "esp_err.h"
#include <esp_log.h>




MatterNode* MatterNode::getInstance()
{
   static MatterNode instance;
   return &instance;
}


MatterNode::MatterNode()
{ 
    //ESP_ERROR_CHECK(nvs_flash_init());
  
}

MatterNode::~MatterNode()
{
    //TODO del all ZbEndPoint objects
}
