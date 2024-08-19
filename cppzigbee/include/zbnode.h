/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zboss_api.h"
#include "cppzb_ep.h"
#include <esp_err.h>

#include "esp_zigbee_core.h"

#include <vector>

#define INSTALLCODE_POLICY_ENABLE       false   /* enable the install code policy for security */
#define ED_MAX_CHILDREN 10
#define ED_AGING_TIMEOUT                ESP_ZB_ED_AGING_TIMEOUT_64MIN
#define ED_KEEP_ALIVE                   3000    /* 3000 millisecond */

#ifdef CONFIG_ZB_ZED // menuconfig Zigbee device type
    #define ZB_DEVICE_TYPE  ESP_ZB_DEVICE_TYPE_ED
#elif //CONFIG_ZB_ZCZR=y or CONFIG_ZB_ZGPD
    #define ZB_DEVICE_TYPE = ESP_ZB_DEVICE_TYPE_ROUTER
#endif

#define ESP_ZB_DEFAULT_RADIO_CONFIG()                           \
    {                                                           \
        .radio_mode = ZB_RADIO_MODE_NATIVE,                     \
    }

#define ESP_ZB_DEFAULT_HOST_CONFIG()                            \
    {                                                           \
        .host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE,   \
    }

class ZbNode
{
    typedef enum
    {
        NOT_JOINED,
        JOINING,
        JOINED

    } JoinStateEnum;

    public:

        
        //ZbNode();
        ~ZbNode();

        static ZbNode* getInstance();

        void joinNetwork();
        void rejoinNetwork();
        void leaveNetwork();
        void joinOrLeaveNetwork();

        void _init();

        
        void start();
    private:
        /// @brief Constructor
        ZbNode();
        //void _init();

        

    private:
        JoinStateEnum           _connectionState; // TODO Shall pe in NVS
        std::vector<ZbEndPoint*>    _vecEndPoint;
        esp_zb_ep_list_t*           _ep_list;

};

