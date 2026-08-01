/*
  matter component for ESP32
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "matterNode.h"
#include "matterEndpoint.h"

#include "matterUtils.h"


#include <nvs_flash.h>
#include <esp_log.h>

#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>



#include <string>

static const char *TAG = "MatterNode";

// Init static members:
MatterNode::NodeMap_t                       MatterNode::_handlersMap = {};
MatterMap<MatterNode::identifyCallback_t>   MatterNode::_identifyMap = {};
MatterMap<MatterEndpoint*>                  MatterNode::_endpointsMap = {};
EventLoop* MatterNode::_eventLoop = new EventLoop("MatterEventLoop");

// Static 
esp_err_t MatterNode::identification_cb(esp_matter::identification::callback_type_t type, uint16_t endpointId, uint8_t effectId,
                                       uint8_t effectVariant, void *priv_data)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "Identification callback endpoint %d: type: %u, effect: %u, variant: %u", 
                                                            endpointId, type, effectId, effectVariant);

    const identifyCallback_t* cb = _identifyMap.get(static_cast<uint32_t>(endpointId));

    if(cb) {
        _eventLoop->enqueue(std::bind(std::ref(*cb), type, effectId, effectVariant, std::move(priv_data)));
        ESP_LOGD(TAG, "Identification Endpoint %d - type: %u, effect: %u, variant: %u", endpointId, type, effectId, effectVariant);
    }

    return err;
}

// Static
esp_err_t MatterNode::attribute_update_cb(esp_matter::attribute::callback_type_t type, 
                                            uint16_t endpointId, uint32_t clusterId,
                                            uint32_t attributeId, esp_matter_attr_val_t *val, void *priv_data)
{
    esp_err_t err = ESP_OK;

    const EndpointMap_t* endpointMap = _handlersMap.get(static_cast<uint32_t>(endpointId));
    if(endpointMap) {
        const ClusterMap_t* clusterMap = endpointMap->get(clusterId);
        if(clusterMap){
            auto attrVector = clusterMap ->get(attributeId);
            for (auto & cb : (*attrVector) ) {
                _eventLoop->enqueue(std::bind(std::ref(cb), type, std::move(static_cast<MatterValue*>(val)), std::move(priv_data)));
                ESP_LOGD(TAG, "Cluster %d - attribute %d - type %u : event posted,",  clusterId, attributeId, (*val).type);
            }
        }
    }

    return err;
}

// Static
void MatterNode::matter_event_cb(const chip::DeviceLayer::ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        ESP_LOGI(TAG, "Fabric removed successfully");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            chip::CommissioningWindowManager  &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(CONFIG_K_TIMEOUT);
            if (!commissionMgr.IsCommissioningWindowOpen()) {
                /* After removing last fabric, this example does not remove the Wi-Fi credentials
                 * and still has IP connectivity so, only advertising on DNS-SD.
                 */
                CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                                            chip::CommissioningWindowAdvertisement::kDnssdOnly);
                if (err != CHIP_NO_ERROR) {
                    ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                }
            }
        }
        break;
    }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        ESP_LOGI(TAG, "Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        ESP_LOGI(TAG, "Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        ESP_LOGI(TAG, "Fabric is committed");
        break;

    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
        ESP_LOGI(TAG, "BLE deinitialized and memory reclaimed");
        break;

    default:
        break;
    }
}



MatterNode* MatterNode::getInstance()
{
   static MatterNode instance;
   return &instance;
}


MatterNode::MatterNode()
{ 
    ESP_ERROR_CHECK(nvs_flash_init());

    esp_matter::node::config_t node_config;

    // TODO not working
    std::string data = "Akira Node";
    std::strcpy(node_config.root_node.basic_information.node_label , data.c_str());
    ESP_LOGE(TAG, "Name of the device root node: %s", node_config.root_node.basic_information.node_label);

    // node handle can be used to add/modify other endpoints.
    _node = esp_matter::node::create(&node_config, attribute_update_cb, identification_cb);
    
    
    ABORT_NODE_ON_FAILURE(_node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));

    // Initialize the OTA Requestor agent
#if CONFIG_ENABLE_OTA_REQUESTOR
    esp_err_t err =esp_matter_ota_requestor_init();
    if (err != ESP_OK)
        ESP_LOGE(TAG, "Error calling esp_matter_ota_requestor_init: %d", err);
#endif
  
}

MatterNode::~MatterNode()
{
    _endpointsMap.clear();
}

void MatterNode::factoryReset()
{
    ESP_LOGI(TAG, "Performing Node factory reset .....");
    esp_matter::factory_reset();
}

void MatterNode::start()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    /* Set OpenThread platform config */
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
#endif

    // Matter start 
    esp_err_t err = esp_matter::start(matter_event_cb);
    ABORT_NODE_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));


#if CONFIG_ENABLE_ENCRYPTED_OTA
    err = esp_matter_ota_requestor_encrypted_init(s_decryption_key, s_decryption_key_len);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to initialized the encrypted OTA, err: %d", err));
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
#if CONFIG_OPENTHREAD_CLI
    esp_matter::console::otcli_register_commands();
#endif
    esp_matter::console::init();
#endif
}

MatterEndpoint* MatterNode::getEndpoint(uint16_t endpointId)
{
    // return nullptr if it doesn't exist
    return _endpointsMap[static_cast<uint32_t>(endpointId)];
}




