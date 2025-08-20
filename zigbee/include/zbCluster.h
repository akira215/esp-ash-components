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
#include <string>

#include <cstring>
#include <cstdlib>

//#include "esp_event.h"
#include "esp_zigbee_core.h"

#include <functional>
#include <utility>
#include <iostream> // TODEL
class ZbEndPoint;

static const char *TAG_CLUSTER = "ZbCluster";

//TODO unregister handler
//TODO implement custom event loop for Zigbee

class ZbCluster
{
public:
    typedef enum {
        ATTR_UPDATED_AFTER_READ    = 0x00,
        ATTR_UPDATED_REMOTELY,
        ATTR_REPORTED
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

    // Struct of attr [bytes]
    // Name         :  size | offset
    // -----------------------------
    // Id           :   2   |   0
    // type         :   1   |   2
    // Access       :   1   |   3
    // ManufCode    :   2   |   4
    // Data P       :   4   |   6  ==> 10 = sizeof(esp_zb_attribute_list_t)
    // -----------------------------
    // cluster_id   :   2   |   10
    // struct*      :   4   |   12
    // -----------------------------
    // attr size    :   4   |   16 (shall be = 12 as data is 12 byte)
    // data @       :   12  |   20 ==> total 36 bytes
    // -----------------------------
    // Lost ???     :   4   |   32 
    // End          :   0   |   36
/* 
    /// @brief struct to load custom attributes.
    /// attr_size shall always be equal to 12 bytes
    typedef struct zbAttribute_s {
        esp_zb_attribute_list_t   esp_attr_list;     // ESP attribute struct 16 bytes
        uint32_t                attr_size;   // Padding 4 bytes  
        char                value[12];      // Attribute value 16 bytes                     
    } ESP_ZB_PACKED_STRUCT
    zbAttribute_t;

    /// @brief Build a new attribute with value max 12 bytes
    /// @return 
    zbAttribute_t* newZbAttribute (void* value){
        zbAttribute_t* attr = (zbAttribute_t*)malloc(sizeof(zbAttribute_t));
        attr->attr_size = 12;
        // Stupidly copy 12 bytes from source, without checking the type,
        // as memory has been allocated
        std::memcpy(&attr->value, value, 12);
        // setup the pointer to the data as offset is fixed
        attr->esp_attr_list.attribute.data_p = &attr->value;

        // Could be changed afterward
        attr->esp_attr_list.next = 0;

        return attr;
    } 
*/

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

    /// @brief add a custom attribute to any cluster.
    /// Value will be copied, no need to alloc memory
    virtual void addCustomAttribute(uint16_t attr_id, void* value, 
                    uint8_t attr_type, uint8_t attr_access);
    
    /// @brief To be implemented using esp_zb_cluster_list_add_xxxxx_cluster
    virtual void addToList(esp_zb_cluster_list_t* cluster_list) = 0;


    /// @brief get a pointer tp attribute struct
    /// @param id of the attribute
    /// @return point to the esp_zb_zcl_attr_t, nullptr if attr does not exist
    esp_zb_zcl_attr_t* getAttribute(uint16_t attr_id) const;

    /// @brief get the cluster ID
    /// @return the Cluster ID
    uint16_t getId() const;

    /// @brief get the Endpoint ID
    /// Cluster shall be attached prior to call this method
    /// @return the Endpoint ID
    uint8_t getEndpointId() const;

    
    bool isClient() const;
    bool isServer() const;

    void setEndPoint(ZbEndPoint* parent);

    esp_zb_zcl_cluster_t* getClusterStruct();

    void attributeWasSet(uint16_t attr_id, void* value);
    bool setAttribute(uint16_t attr_id, void* value);

    // cmd that initiated the event from external device
    void defaultCommandTriggered(uint8_t cmd);

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


    /// @brief Set reporting of an attribute
    /// @param attrId the Id of the attribute to be read
    /// @param reportable_change value for wich report will be trigger after at least min_interval
    /// @param min_interval min interval between 2 reports whatever value change occured
    /// @param max_interval max interval for the next report whatever value change occured
    void setReporting(uint16_t attr_id, void* reportable_change, uint16_t min_interval = 60, 
                        uint16_t max_interval = 3600);


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
