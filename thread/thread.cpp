/*
  thread
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "thread.h"

#include <string.h>
#include "nvs_flash.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_openthread_lock.h"
#include "esp_openthread_netif_glue.h"

#include "openthread/dataset.h"
#include "openthread/logging.h"
#include "esp_vfs_eventfd.h"



static const char *TAG = "Thread";


//static handler
void Thread::thread_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    
    
    // Safety check (redundant if registered cleanly, but recommended)
    if (event_base != OPENTHREAD_EVENT) {
        return;
    }
    Thread* instance = static_cast<Thread*>(arg);
    
    switch ((esp_openthread_event_t)event_id) {
        case OPENTHREAD_EVENT_START:
            ESP_LOGI(TAG, "OpenThread stack started");
            break;
            
        case OPENTHREAD_EVENT_ATTACHED:
            ESP_LOGI(TAG, "OpenThread successfully attached to network");
            break;
            
        case OPENTHREAD_EVENT_DETACHED:
            ESP_LOGW(TAG, "OpenThread detached from network");
            break;
            
        case OPENTHREAD_EVENT_IF_UP:
            ESP_LOGI(TAG, "OpenThread NetIF is UP");
            break;

        case OPENTHREAD_EVENT_GOT_IP6:
            ESP_LOGI(TAG, "OpenThread interface obtained an IPv6 address");
            break;
        
        case OPENTHREAD_EVENT_ROLE_CHANGED: {
            otDeviceRole role = *(otDeviceRole*)event_data;
        
            switch (role) {
                case OT_DEVICE_ROLE_DISABLED:
                    ESP_LOGI(TAG, "Thread network disabled.");
                    break;
                case OT_DEVICE_ROLE_DETACHED:
                    ESP_LOGI(TAG, "Searching / Connecting to Thread network...");
                    break;
                case OT_DEVICE_ROLE_CHILD:
                    ESP_LOGI(TAG, "Thread connected role: Child");
                    break;
                case OT_DEVICE_ROLE_ROUTER:
                    ESP_LOGI(TAG, "Thread connected role: Router");
                    break;
                case OT_DEVICE_ROLE_LEADER:
                    ESP_LOGI(TAG, "Thread connected role: Leader");
                    break;
            }}
            break;

        default:
            ESP_LOGW(TAG, "Unhandled OpenThread event ID: %ld", event_id);
            break;
    }

    instance->callEventHandlers((esp_openthread_event_t)event_id);
    
}

//Static
void Thread::thread_loop_task(void *aContext)
{
    ESP_LOGI(TAG, "Launching OpenThread Task");
    
    // Blocking function
    esp_openthread_launch_mainloop();
    
    // In case function end, just for safety
    vTaskDelete(NULL);
}


// Static Utility function to convert a hex string to raw bytes
size_t Thread::hex_string_to_bytes(const char *hex, uint8_t *bytes, size_t max_len) {
    size_t len = strlen(hex);
    if (len % 2 != 0 || len / 2 > max_len) return 0;
    for (size_t i = 0; i < len; i += 2) {
        unsigned int val;
        sscanf(&hex[i], "%2x", &val);
        bytes[i / 2] = (uint8_t)val;
    }
    return len / 2;
}

Thread::Thread()
{
     // Initializing the ESP32 system and OpenThread platform
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

 
    // Registering the OpenThread event handler
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        OPENTHREAD_EVENT,               // Base de l'événement
        ESP_EVENT_ANY_ID,               //OPENTHREAD_EVENT_ROLE_CHANGED,  // ID de l'événement précis à écouter
        &Thread::thread_event_handler,  // Notre fonction de rappel
        this, 
        NULL
    ));
    
}

void Thread::init()
{    
   
    
    // register the eventfd VFS 
    const esp_vfs_eventfd_config_t eventfd_config =
                                ESP_VFS_EVENTD_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config)); 

    // COnfiguration and initialization of the OpenThread stack
    esp_openthread_platform_config_t config = {};
    config.radio_config.radio_mode = RADIO_MODE_NATIVE; // Utilise la radio 802.15.4 interne
    config.host_config.host_connection_mode = HOST_CONNECTION_MODE_NONE;
    config.port_config.storage_partition_name = "nvs";
    config.port_config.task_queue_size = 10;
    config.port_config.netif_queue_size = 10;
    ESP_ERROR_CHECK(esp_openthread_init(&config));

    // Linking openthread with the ESP-NETIF layer to manage IPv6 addresses and routing
    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_OPENTHREAD();
    esp_netif_t *openthread_netif = esp_netif_new(&netif_cfg);
    esp_netif_attach(openthread_netif, esp_openthread_netif_glue_init(&config));

    // TVL application and protocol start (Thread Safe)
    if (esp_openthread_lock_acquire(portMAX_DELAY)) {
        _instance = esp_openthread_get_instance();

        // TVL Hexadécimal in byte table
        uint8_t tlv_bytes[OT_OPERATIONAL_DATASET_MAX_LENGTH];
        size_t tlv_len = hex_string_to_bytes(CONFIG_TARGET_THREAD_TLV, tlv_bytes, sizeof(tlv_bytes));

        if (tlv_len > 0) {
            otOperationalDatasetTlvs dataset_tlvs;
            memcpy(dataset_tlvs.mTlvs, tlv_bytes, tlv_len);
            dataset_tlvs.mLength = (uint8_t)tlv_len;

            // TLV injection directly in the active stack
            if (otDatasetSetActiveTlvs(_instance, &dataset_tlvs) == OT_ERROR_NONE) {
                // Activation of the Thread stack and IPV6 interface, start searching network
                otIp6SetEnabled(_instance, true);
                otThreadSetEnabled(_instance, true);
            }
        }
        otLoggingSetLevel(OT_LOG_LEVEL_WARN);

        esp_openthread_lock_release();
    }


    // Clear NVS storage to ensure a fresh start for the OpenThread stack
    //otInstanceFactoryReset(esp_openthread_get_instance());

    // Launching thread task in a task as it is Blocking, check stack size and priority
    xTaskCreate(Thread::thread_loop_task, "OpenThread Loop", 5120, NULL, 5, NULL);
}
