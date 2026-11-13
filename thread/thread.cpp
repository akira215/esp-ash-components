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

#include "esp_ota_ops.h"
#include "esp_system.h"
#include "lwip/sockets.h"


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
            {
                ESP_LOGI(TAG, "OpenThread interface obtained an IPv6 address");
                const otNetifAddress *unicastAddrs = otIp6GetUnicastAddresses(instance->getOtInstance());
                for (const otNetifAddress *addr = unicastAddrs; addr; addr = addr->mNext) {
                    char addr_str[OT_IP6_ADDRESS_STRING_SIZE];
                    otIp6AddressToString(&addr->mAddress, addr_str, sizeof(addr_str));
                    // Skip link-local (fe80::) and mesh-local RLOC (0000:00ff:fe00) addresses
                    if (!addr->mRloc && addr->mAddress.mFields.m8[0] != 0xfe) {
                        ESP_LOGI("THREAD_IP", "Routable IPv6 Address: %s", addr_str);
                    }
                }
                instance->startOtaTask();
            }
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

// Static task that listen to CONFIG_OTA_PORT on an IPv6 UDP port, 
// receives raw firmware chunks, and flashes them using the ESP-IDF OTA API. 
// Send an "END" packet from your host script to finalize the update and reboot
void Thread::thread_ota_task(void *aContext)
{
    ESP_LOGI(TAG, "Launching OpenThread OTA listener");
    
   int sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Failed to create IPv6 socket");
        vTaskDelete(NULL);
    }

    struct sockaddr_in6 server_addr = {};
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(CONFIG_OTA_PORT);
    server_addr.sin6_addr = in6addr_any;


    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Socket bind failed");
        close(sock);
        vTaskDelete(NULL);
    }

    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == nullptr) {
        ESP_LOGE(TAG, "Not able to get the next partition, OTA task will end, OTA impossible");
        close(sock);
        vTaskDelete(NULL);
    }

    esp_ota_handle_t ota_handle = 0;
    bool ota_started = false;

    uint8_t rx_buffer[CONFIG_OTA_BUFFER_SIZE];
    struct sockaddr_in6 source_addr;
    socklen_t socklen = sizeof(source_addr);

    ESP_LOGI(TAG, "Listening for OTA chunks on UDP port %d...", CONFIG_OTA_PORT);

    uint32_t expected_seq = 0;

    while (1) {
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer), 0,
                        (struct sockaddr *)&source_addr, &socklen);
        if (len < 0) {
            ESP_LOGE(TAG, "recvfrom failed");
            break;
        }

        // End of transmission
        if (len == 3 && memcmp(rx_buffer, "END", 3) == 0) {
            if (ota_started) {
                if (esp_ota_end(ota_handle) == ESP_OK &&
                    esp_ota_set_boot_partition(update_partition) == ESP_OK) {
                    sendto(sock, "OK", 2, 0, (struct sockaddr *)&source_addr, socklen);
                    ESP_LOGI(TAG, "OTA complete! Rebooting...");
                    vTaskDelay(pdMS_TO_TICKS(500));
                    close(sock);
                    esp_restart();
                } else {
                    ESP_LOGE(TAG, "OTA validation or boot partition switch failed");
                }
            }
            break;
        }

        // Each data packets should contain [4 Bytes SEQ] + [data]
        if (len <= 4) continue;

        uint32_t seq;
        memcpy(&seq, rx_buffer, 4);
        seq = ntohl(seq);


        // If packet already received (re sending), send ACK without writing again
        if (seq < expected_seq) {
            uint32_t ack = htonl(seq);
            sendto(sock, &ack, 4, 0, (struct sockaddr *)&source_addr, socklen);
            continue;
        }

        // Ignore if futur packet arrive too soon
        if (seq > expected_seq) {
            ESP_LOGW(TAG, "Paquet hors séquence : reçu %lu, attendu %lu", seq, expected_seq);
            continue;
        }

        if (!ota_started) {
            if (esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle) != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_begin failed");
                break;
            }
            ota_started = true;
        }

        // Only write data (ignore 4 bytes of SEQ)
        if (esp_ota_write(ota_handle, rx_buffer + 4, len - 4) != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_write failed");
            esp_ota_abort(ota_handle);
            break;
        }

        // send ACK for this SEQ
        uint32_t ack = htonl(seq);
        sendto(sock, &ack, 4, 0, (struct sockaddr *)&source_addr, socklen);
        expected_seq++;
    } // while(1)

    close(sock);
    vTaskDelete(NULL);
}


// Static task to check if OTA was ok,
// It waits up to 60 seconds for the OpenThread node to attach to the mesh as a Child, Router, or Leader. 
// If it fails to attach in time, it triggers a rollback.
void Thread::ota_rollback_verify_task(void *pvParameters)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;

    // Check if the running partition is awaiting verification
    if (esp_ota_get_state_partition(running, &ota_state) != ESP_OK ||
        ota_state != ESP_OTA_IMG_PENDING_VERIFY) {
        ESP_LOGI(TAG, "Firmware already verified or running from factory partition.");
        vTaskDelete(NULL);
    }

    ESP_LOGW(TAG, "New firmware pending verification. Waiting for Thread network attachment...");

    bool attached = false;

    for (int elapsed = 0; elapsed < CONFIG_THREAD_ATTACH_TIMEOUT_SEC; elapsed++) {
        // Acquire OpenThread stack lock before calling ot* APIs
        if (esp_openthread_lock_acquire(pdMS_TO_TICKS(1000))) {
            otInstance *instance = esp_openthread_get_instance();
            otDeviceRole role = otThreadGetDeviceRole(instance);
            esp_openthread_lock_release();

            if (role == OT_DEVICE_ROLE_CHILD ||
                role == OT_DEVICE_ROLE_ROUTER ||
                role == OT_DEVICE_ROLE_LEADER) {
                attached = true;
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (attached) {
        ESP_LOGI(TAG, "Attached to Thread network! Marking firmware as valid.");
        esp_ota_mark_app_valid_cancel_rollback();
    } else {
        ESP_LOGE(TAG, "Failed to attach within %d seconds! Rolling back to previous firmware...",
                 CONFIG_THREAD_ATTACH_TIMEOUT_SEC);
        esp_ota_mark_app_invalid_rollback_and_reboot();
    }

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


void Thread::startOtaTask()
{
    if ((!_otaTask)&&(CONFIG_OTA_ENABLE))
    {
        xTaskCreate(Thread::thread_ota_task, "thread_ota_task", 4096, NULL, 5, &_otaTask);
        xTaskCreate(Thread::ota_rollback_verify_task, "ota_verify", 3072, NULL, 5, NULL);
    }
}