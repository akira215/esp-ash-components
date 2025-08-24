/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once
#include "zbDebug.h" // TODEL

#include "blinkTask.h"
#include "esp_zigbee_core.h"
#include "zbEndpoint.h"
#include "zbCluster.h"
#include "eventLoop.h"


#include <esp_err.h>

#include <functional>
#include <map>
#include <vector>
/*
#if !CONFIG_ZB_LED || CONFIG_ZB_LED==-1 
    #warning "No led has been defined, zigbee lib will not use led"
#else
    #define ZB_USE_LED
#endif
*/

static const char *ZNODE_TAG = "ZbNode";

#define INSTALLCODE_POLICY_ENABLE       false   /* enable the install code policy for security */
#define ED_MAX_CHILDREN                 10
#define ED_AGING_TIMEOUT                ESP_ZB_ED_AGING_TIMEOUT_64MIN
#define ED_KEEP_ALIVE                   3000    /* 3000 millisecond */

#ifdef CONFIG_ZB_ZED // menuconfig Zigbee device type
    #define ZB_DEVICE_TYPE  ESP_ZB_DEVICE_TYPE_ED
#elif CONFIG_ZB_ZCZR
    #define ZB_DEVICE_TYPE = ESP_ZB_DEVICE_TYPE_ROUTER
#endif


class ZbNode
{
public:
    typedef enum {
        JOINED    = 0x00,
        JOINING,
        NLME_STATUS,
        JOIN_FAIL,
        LEAVING,
        LEFT,
    } nodeEvent_t;
    static EventLoop*                       _eventLoop;

private:
    /// @brief Joined callback type
    typedef std::function<void(nodeEvent_t)> nodeCallback_t;

    static std::map<uint8_t,ZbEndPoint*>    _endPointMap;
    static esp_zb_ep_list_t*                _ep_list;

    std::vector<nodeCallback_t>             _nodeEventHandlers;  
    static TaskHandle_t                     _zbTask;

    //#ifdef ZB_USE_LED
    //static BlinkTask*                       _ledBlinking;
    //static GpioOutput                       _led;
    //#endif


public:
  
    ~ZbNode();

    //Singletons should not be cloneable.
    ZbNode(ZbNode &other) = delete;

    //Singletons should not be assignable.
    void operator=(const ZbNode &) = delete;

    /// @brief Instanciante the obj or return the point to the unique obj
    static ZbNode* getInstance();

    /// @brief register event handler for the node.
    /// Event handler shall be type nodeCallback_t : 
    /// void(nodeEvent_t) 
    /// @param func pointer to the method ex: &Main::clusterHandler
    /// @param instance instance of the object for this handler (ex: this)
    template<typename C>
    void registerNodeEventHandler(void (C::* func)(nodeEvent_t), C* instance) {
        _nodeEventHandlers.push_back(std::bind(func,std::ref(*instance), 
                                    std::placeholders::_1));
    };


    /// @brief Handle all the zb event. It is called by esp_zb_app_signal_handler
    void handleBdbEvent(esp_zb_app_signal_type_t signal_type,
                        esp_err_t status,
                        uint32_t *p_data);
   
    /// @brief Start Network steering
    /// @param param is not used, it is just to comply with esp_zb_callback_t
    void joinNetwork();

    /// @brief will trigger the device to leave the network
    /// all related infos including the nvs will be deleted
    void leaveNetwork();

    /// @brief Test if the device is currently connected to network
    /// @return true if device is connected to a network
    static bool isJoined();

    /// @brief Start the Zigbee stack
    void start();

    /// @brief get the handle to the zbtask
    /// @return NULL if no task created otherwise the handle
    static TaskHandle_t getZbTask() { return _zbTask; }

    /// @brief add the endpoint to the Node
    /// @param ep endpoint to be added
    void addEndPoint(ZbEndPoint& ep);

    /// @brief retrive a pointer to the endpoint from the endpoint id
    /// @param endp_id id of the endpoint
    /// @return a pointer to the endpoint if exists, otherwise nullptr
    ZbEndPoint* getEndPoint(uint8_t endp_id);

    /// @brief Handle zb actions
    esp_err_t handleZbActions(esp_zb_core_action_callback_id_t callback_id, 
                    const void *message);

    void bindAttribute(uint8_t endpoint);
    
    static void bind_cb(esp_zb_zdp_status_t zdo_status, void *user_ctx);
    
    /// @brief send command. Note that no check is done on the cmd itself
    /// @param endp endpoint that send the command
    /// @param cluster_id id of the cluster
    /// @param isClient if the cluster is Client or Server
    /// @param cmd command to be sent
    /// @return ESP_ERR_NOT_FOUND if endp or cluster doesn't exist otherwise ESP_OK
    esp_err_t sendCommand(uint8_t endp, uint16_t cluster_id, 
                    bool isClient,uint16_t cmd);
    
    /// @brief set attribute. Note that this is not required if device is bound
    /// @param endp endpoint that send the command
    /// @param cluster_id id of the cluster
    /// @param isClient if the cluster is Client or Server
    /// @param cmd command to be sent
    /// @return ESP_ERR_NOT_FOUND if endp or cluster doesn't exist otherwise ESP_OK
    esp_err_t setAttribute(uint8_t endp, uint16_t cluster_id, 
                    bool isClient,uint16_t attrId, void* value);
protected:
    
    static void zbTask(void *pvParameters);

    void handleDeviceReboot(esp_err_t err);
    void handleNetworkSteering(esp_err_t err);
    void handleLeaveNetwork(esp_err_t err);
    void handleNetworkStatus(esp_err_t err, void* data);
    //void handleRejoinFailure();


    // Zb Actions
    esp_err_t handlingCmdDefaultResp(const esp_zb_zcl_cmd_default_resp_message_t *msg);
    esp_err_t handlingCmdSetAttribute(const esp_zb_zcl_set_attr_value_message_t *msg);
    esp_err_t handlingCmdReadAttribute(const esp_zb_zcl_cmd_read_attr_resp_message_t *msg);

    static void print_binding_table_cb(const esp_zb_zdo_binding_table_info_t *record, void *user_ctx);
    static void print_binding_table_next(uint8_t index);
    static void print_binding_table() ;



private:
    /// @brief Constructor is private (singleton)
    ZbNode();

    void postEvent(nodeEvent_t event);

};

