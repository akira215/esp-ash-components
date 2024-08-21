/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once


#include "esp_zigbee_core.h"
#include "cppzb_ep.h"
#include "persistedValue.h"
#include <esp_err.h>

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
    std::vector<ZbEndPoint*>    _vecEndPoint;
    esp_zb_ep_list_t*           _ep_list;

public:

        
    //ZbNode();
    ~ZbNode();
    static ZbNode* getInstance();

    void handleBdbEvent(esp_zb_app_signal_t *event);

    void joinNetwork();
    void rejoinNetwork();
    void leaveNetwork();
    void joinOrLeaveNetwork();

    void _init();

    bool isJoined();

    void start();

protected:
    void handleDeviceReboot(esp_err_t err);
    void handleNetworkJoinAndRejoin();
    void handleLeaveNetwork();
    void handleRejoinFailure();
    /*
    void handlePollResponse(ZPS_tsAfPollConfEvent* pEvent);
    void handleZdoBindUnbindEvent(ZPS_tsAfZdoBindEvent * pEvent, bool bind);
    void handleZdoDataIndication(ZPS_tsAfEvent * pEvent);
    void handleZdoEvents(ZPS_tsAfEvent* psStackEvent);
    void handleZclEvents(ZPS_tsAfEvent* psStackEvent);
    void handleAfEvent(BDB_tsZpsAfEvent *psZpsAfEvent);
*/

private:
    /// @brief Constructor is private (singleton)
    ZbNode();
    //void _init();



};

