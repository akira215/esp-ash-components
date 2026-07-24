/*
  matter component for ESP32
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include <cstdint>
#include <esp_matter.h>
#include <esp_err.h>

#include "esp_matter_attribute_utils.h"

#include "matterValue.h"

class MatterEndpoint;


#include <unordered_map>
#include <vector>


#include "eventLoop.h"


#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "esp_openthread_types.h"
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                           \
    {                                                                                   \
        .radio_mode = RADIO_MODE_NATIVE,                                                \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                            \
    {                                                                                   \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,                              \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                            \
    {                                                                                   \
        .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10, \
    }
#endif



/*
#ifdef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME
    #undef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME
#endif
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME "Akira Corp"

#ifdef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME
    #undef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME
#endif
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME "Akira Device"

#ifdef CHIP_DEVICE_CONFIG_DEFAULT_NODE_LABEL
    #undef CHIP_DEVICE_CONFIG_DEFAULT_NODE_LABEL
#endif
#define CHIP_DEVICE_CONFIG_DEFAULT_NODE_LABEL "Akira Node"
*/


// Singleton class to manage matter node
class MatterNode
{
public:
    using attributeEvent_t = esp_matter::attribute::callback_type_t;
    using identifyEvent_t = esp_matter::identification::callback_type_t;
    
    /// @brief attribute update callback type
    using attrUpdateCallback_t = std::function<void(attributeEvent_t,  // PRE or POST_UPDATE
                                    MatterValue*,                     // value
                                    void*)>;                        // priv_data
    
    using identifyCallback_t = std::function<void(identifyEvent_t,  // START, STOP, EFFECT,
                                    uint8_t,                    // effectId
                                    uint8_t,                     // effectVariant
                                    void*)>;                    // priv_data

private:
    /// @brief Constructor is protected (singleton) 
    MatterNode();

    // Type aliases to make the code highly readable
    using ClusterMap_t    = std::unordered_map<uint32_t, std::vector<attrUpdateCallback_t>>;   // AttributID -> 
    using EndpointMap_t   = std::unordered_map<uint32_t, ClusterMap_t>;     // ClusterID -> 
    using NodeMap_t       = std::unordered_map<uint16_t, EndpointMap_t>;    // EndpointID Base Node Database

    static NodeMap_t   _handlersMap;  // This maps contains all the registered handlers for all attributes
    static EventLoop*  _eventLoop;

    static std::unordered_map<uint16_t,identifyCallback_t>    _identifyMap;

    esp_matter::node_t*                             _node = nullptr;
    std::unordered_map<uint16_t,MatterEndpoint*>    _endpointsMap;

    // A generic helper tag to pass the type context
    template <typename T> struct type_holder {};

    // Universal ADL bridge to create the endpoint based on config type
    template <typename ConfigType>
    esp_matter::endpoint_t* call_createEndpoint( ConfigType *config, uint8_t flags, void *priv_data, type_holder<ConfigType>) {
        // This allows ADL to automatically resolve down to the matching child namespace!
        using namespace esp_matter::endpoint;
        // Unqualified call allows the compiler to find the 'create' that matches your ConfigType
        return create(getEspNode(), config, flags, priv_data);
    }

public:  
    ~MatterNode();

    //Singletons should not be cloneable.
    MatterNode(MatterNode &other) = delete;

    //Singletons should not be assignable.
    void operator=(const MatterNode &) = delete;

    /// @brief Instanciante the obj or return the point to the unique obj
    static MatterNode* getInstance();

    // TODEL
    esp_matter::node_t* getEspNode() { return _node; };

    // Create an Endpoint type should be for example extended_color_light::config_t
    //template <typename T>
    //MatterEndpoint* createEndpoint(T& config);
        
    void factoryReset();
    
    void start();

    MatterEndpoint* getEndpoint(uint16_t endpointId);

    template <typename ConfigType>
    MatterEndpoint* createEndpoint(ConfigType* config,
                                    uint8_t flags = esp_matter::ENDPOINT_FLAG_NONE, 
                                    void *priv_data = nullptr);

    

    /// @brief register attribute update handler for this attribute.
    /// Update handler shall be type attrUpdateCallback_t : 
    /// void(esp_matter::attribute::callback_type_t, esp_matter_attr_val_t*,void*) 
    /// @param func pointer to the method ex: &Main::clusterHandler
    /// @param instance instance of the object for this handler (ex: this)
    template<typename C, typename... Args>
    void registerAttrUpdateHandler(void (C::* func)(Args...), 
                                    C* instance, 
                                    uint16_t endpointId,
                                    uint32_t clusterId,
                                    uint32_t attrId) {
        // A lambda captures the function pointer and instance, 
        // and forwards any number of incoming arguments using a parameter pack.
            _handlersMap[endpointId][clusterId][attrId].push_back([instance, func](Args&&... args) {
            (instance->*func)(std::forward<Args>(args)...);
        });
    }

    /// @brief register identify .
    /// Update handler shall be type identifyCallback_t : 
    /// void(identifyEvent_t,uint8_t, uint8_t, void*)
    /// @param func pointer to the method ex: &Main::clusterHandler
    /// @param instance instance of the object for this handler (ex: this)
    template<typename C, typename... Args>
    void registerIdentifyHandler(void (C::* func)(Args...), 
                                    C* instance, 
                                    uint16_t endpointId) {
        // A lambda captures the function pointer and instance, 
        // and forwards any number of incoming arguments using a parameter pack.
            _identifyMap[endpointId] = ([instance, func](Args&&... args) {
            (instance->*func)(std::forward<Args>(args)...);
        });
    }

 
private:
    // This callback is called for every attribute update. The callback implementation shall
    // handle the desired attributes and return an appropriate error code. If the attribute
    // is not of your interest, please do not return an error code and strictly return ESP_OK.
    // priv_data is set in the esp_matter::node::config_t if required 
    static esp_err_t attribute_update_cb(esp_matter::attribute::callback_type_t type, uint16_t endpointId, 
                                            uint32_t clusterId, uint32_t attributeId, 
                                            esp_matter_attr_val_t *val, void *priv_data);
    
    // This callback is invoked when clients interact with the Identify Cluster.
    // In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
    static esp_err_t identification_cb(esp_matter::identification::callback_type_t type, uint16_t endpointId, 
                                    uint8_t effectId, uint8_t effectVariant, void *priv_data);

    // Callback for Matter stack events. App can register callbacks
    static void matter_event_cb(const chip::DeviceLayer::ChipDeviceEvent *event, intptr_t arg);

};

// Include the implementation at the very bottom
#include "matterNode_impl.tpp" 