/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "blinkTask.h"
#include "esp_zigbee_core.h"
#include "cppzb_ep.h"


#include <esp_err.h>

#include <vector>

#if !CONFIG_ZB_LED || CONFIG_ZB_LED==-1 
    #warning "No led has been defined, zigbee lib will not use led"
#else
    #define ZB_USE_LED
#endif

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

    #ifdef ZB_USE_LED
    static BlinkTask*           _ledBlinking;
    static GpioOutput           _led;
    #endif
public:
  
    //ZbNode();
    ~ZbNode();

    //Singletons should not be cloneable.
    ZbNode(ZbNode &other) = delete;

    //Singletons should not be assignable.
    void operator=(const ZbNode &) = delete;

    static ZbNode* getInstance();

    /// @brief Handle all the zb event. It is called by esp_zb_app_signal_handler
    void handleBdbEvent(esp_zb_app_signal_t *event);

   
    /// @brief Start Network steering
    /// @param param is not used, it is just to comply with esp_zb_callback_t
    static void joinNetwork(uint8_t param = 0);

    /// @brief will trigger the device to leave the network
    /// all related infos including the nvs will be deleted
    static void leaveNetwork();
    
    void rejoinNetwork();
    
    void joinOrLeaveNetwork();

    void _init();

    static bool isJoined();

    void start();

protected:
    
    static void zbTask(void *pvParameters);

    void handleDeviceReboot(esp_err_t err);
    void handleNetworkSteering(esp_err_t err);
    void handleNetworkJoinAndRejoin();
    void handleLeaveNetwork(esp_err_t err);
    void handleNetworkStatus(esp_err_t err);
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

