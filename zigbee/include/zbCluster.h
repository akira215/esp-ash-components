/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include <vector>
#include <list>
#include <span>
#include <memory>
#include <cstring>
#include <string>
//#include "esp_event.h"
#include "esp_zigbee_core.h"

#include <functional>
#include <utility>
#include <iostream> // TODEL
class ZbEndPoint;


//TODO unregister handler
//TODO implement custom event loop for Zigbee

class ZbCluster
{
public:
    typedef enum {
        ATTR_UPDATED_AFTER_READ    = 0x00,
        ATTR_UPDATED_REMOTELY
    } clusterEvent_t;

    typedef struct {
        uint16_t attrId;
        void* value;
    }attribute_t;

private:

    esp_zb_zcl_cluster_t _cluster;
    ZbEndPoint* _endPoint = nullptr;

    /// @brief cluster callback type
    //typedef std::function<void(clusterEvent_t, uint16_t, void*)> clusterCallback_t;
    typedef std::function<void(clusterEvent_t, std::vector<attribute_t>)> clusterCallback_t;
    std::vector<clusterCallback_t> _clusterEventHandlers;   

protected:
    esp_zb_attribute_list_t* _attr_list;

    /// @brief struct of attribute for returning type and access function
    struct attrType_t {
        uint8_t type;
        uint8_t access;
    };

protected:
    void _init(uint16_t id, bool isClient);

    /// @brief Copy attributes from an existing ZbCluster
    /// Attributes that already exists are not overided
    /// @param other the source cluster
    void _copyAttributes(const ZbCluster& other);
    
    
    void postEvent(clusterEvent_t event, std::vector<attribute_t> attrs);
    void postEvent(clusterEvent_t event, uint16_t attrId, void* value);

public:
    /// @brief Constructor
    ZbCluster();
    ~ZbCluster();
    
    /// @brief To be implemented using esp_zb_xxxxx_cluster_add_attr
    virtual void addAttribute(uint16_t attr_id, void* value) = 0;
    
    /// @brief To be implemented using esp_zb_cluster_list_add_xxxxx_cluster
    virtual void addToList(esp_zb_cluster_list_t* cluster_list) = 0;


    /// @brief get a pointer tp attribute struct
    /// @param id of the attribute
    /// @return point to the esp_zb_zcl_attr_t, nullptr if attr does not exist
    esp_zb_zcl_attr_t* getAttribute(uint16_t attr_id) const;

    uint16_t getId() const;
    bool isClient() const;
    bool isServer() const;

    void setEndPoint(ZbEndPoint* parent);

    esp_zb_zcl_cluster_t* getClusterStruct();

    void attributeWasSet(uint16_t attr_id, void* value);
    bool setAttribute(uint16_t attr_id, void* value);

    uint8_t sendCommand(uint16_t cmd); // TODO add data

    /// @brief Send a read attr command on the network
    /// @param attrLsit list of attr Id to be r²ead
    /// @param dst_endpoint endpoint on the remote device
    /// @param short_addr address of the remote device
    /// @return transaction number
    uint8_t readAttribute(std::span<uint16_t> attrList, uint8_t dst_endpoint = 1, 
                                    uint16_t short_addr = 0x0000);

    /// @brief Send a read attr command on the network
    /// @param attrId the Id of the attribute to be read
    /// @param dst_endpoint endpoint on the remote device
    /// @param short_addr address of the remote device
    /// @return transaction number
    uint8_t readAttribute(uint16_t attrId , uint8_t dst_endpoint = 1, 
                        uint16_t short_addr = 0x0000); // TODO add data
    
    /// @brief callback from ZbNode that will update attr locally
    /// @param readAttrs the list of received attr that were asked to be read
    /// @return 
    esp_err_t attributesWereRead(esp_zb_zcl_read_attr_resp_variable_t* readAttrs);



    void setReporting(uint16_t attr_id);


    /// @brief register event handler for this cluster.
    /// Event handler shall be type clusterCallback_t : 
    /// void(eventType, uint16_t attrId, void* value) 
    /// @param func pointer to the method ex: &Main::clusterHandler
    /// @param instance instance of the object for this handler (ex: this)
    ///template<typename C, typename... Args>
    ///void registerEventHandler(void (C::* func)(Args...), C* instance) {
    ///    _clusterEventHandlers.push_back(std::bind(func,std::ref(*instance),
    ///       std::placeholders::_1,
    ///        std::placeholders::_2,
    ///        std::placeholders::_3));
    ///};

    template<typename C, typename... Args>
    void registerEventHandler(void (C::* func)(Args...), C* instance) {
        _clusterEventHandlers.push_back(std::bind(func,std::ref(*instance),
                std::placeholders::_1,
                std::placeholders::_2));
    };

    //void registerEventHandler(clusterCallback_t handler);

private:
   
};
