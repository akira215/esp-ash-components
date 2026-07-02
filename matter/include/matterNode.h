/*
  matter component for ESP32
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include <esp_matter.h>
#include <esp_err.h>

#include "esp_matter_attribute_utils.h"
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h" // for task handle

#include "matterEndpoint.h"
#include <vector>


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
    esp_matter::node_t* _node = nullptr;
    std::vector<MatterEndpoint*> _endpoints;

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

    // Create an Endpoint
    template <typename T>
    MatterEndpoint* createEndpoint(T& config){
        MatterEndpoint* endpoint = new MatterEndpoint(this);

        endpoint->create_endpoint(_node, &config);
        _endpoints.push_back(endpoint);

        return endpoint;
    }
        

    void start();


private:
    /// @brief Constructor is private (singleton)
    MatterNode();
 
    // This callback is called for every attribute update. The callback implementation shall
    // handle the desired attributes and return an appropriate error code. If the attribute
    // is not of your interest, please do not return an error code and strictly return ESP_OK.
    // priv_data is set in the esp_matter::node::config_t if required 
    static esp_err_t attribute_update_cb(esp_matter::attribute::callback_type_t type, uint16_t endpoint_id, 
                                            uint32_t cluster_id, uint32_t attribute_id, 
                                            esp_matter_attr_val_t *val, void *priv_data);
    
    // This callback is invoked when clients interact with the Identify Cluster.
    // In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
    static esp_err_t identification_cb(esp_matter::identification::callback_type_t type, uint16_t endpoint_id, 
                                    uint8_t effect_id, uint8_t effect_variant, void *priv_data);

    // Callback for Matter stack events. App can register callbacks
    static void matter_event_cb(const chip::DeviceLayer::ChipDeviceEvent *event, intptr_t arg);

};