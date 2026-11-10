/*
  mqtt
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>
#include <esp_err.h>
#include "mqttClient.h"

#include <algorithm>


static const char *TAG = "MqttClient";


//static handler
void MqttClient::mqtt_event_handler(void *handler_args, esp_event_base_t base, 
                                                int32_t event_id, void *event_data)
{
    
    MqttClient* instance = static_cast<MqttClient*>(handler_args);

    ESP_LOGV(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
    //esp_mqtt_client_handle_t client = event->client;
    //int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");
        instance->setConnected();

        // if session is not recognized, subscribe to all topics again
        if (!event->session_present) {
            instance->resubsribeAllTopics();
        } else {
            // broker saved the session (Clean Session = false), 
            // subscriptions are still active!
            instance->markAllTopicSubscribed(true);
        }

        // Call the registred handlers
        for (const auto& handler : instance->_connectionHandlers) {
            handler(true, event->session_present);
        }

        break;
        
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "MQTT_EVENT_DISCONNECTED");
        instance->setConnected(false);

        // If non persistent session, reset all subscription
        instance->markAllTopicSubscribed(false);

        // Call the registred handlers
        for (const auto& handler : instance->_connectionHandlers) {
            handler(false, static_cast<int>(event->error_handle->error_type));
        }

        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGD(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d, return code=0x%02x ", 
                                                event->msg_id, (uint8_t)*event->data);
        instance->markTopicSubscribed(std::string(event->topic, event->topic_len), event->msg_id, true); // Mark the topic as subscribed
        break;
        
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGD(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        instance->removeTopic(std::string(event->topic, event->topic_len)); // Remove the topic from the list
        break;
        
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        // TODO implement control that data has been sent
        break;
        
    case MQTT_EVENT_DATA:
        ESP_LOGD(TAG, "MQTT_EVENT_DATA");
       
        // Call registered data handlers
        for (const auto& handler : instance->_dataHandlers) {
            handler(event->msg_id, std::string(event->topic, event->topic_len), 
                                    std::string(event->data, event->data_len));
        }
        break;
        
    case MQTT_EVENT_ERROR:
        ESP_LOGW(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGW(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGW(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGW(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGW(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
        
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    } // switch

    
}


MqttClient::MqttClient()
{
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri = CONFIG_MQTT_BROKER_URL;
    mqtt_cfg.broker.address.port = CONFIG_MQTT_BROKER_PORT;

    mqtt_cfg.credentials.username = CONFIG_MQTT_USERNAME;
    mqtt_cfg.credentials.authentication.password = CONFIG_MQTT_PASSWORD;



    _client = esp_mqtt_client_init(&mqtt_cfg);

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(_client, MQTT_EVENT_ANY, MqttClient::mqtt_event_handler, this);
    
}

void MqttClient::start()
{
    esp_mqtt_client_start(_client);
}

void MqttClient::stop()
{
    esp_mqtt_client_stop(_client);
}

void MqttClient::subscribeTopic(std::string topic, int qos) 
{
    
    if (topicSubscribed(topic)) {
        ESP_LOGW(TAG, "Topic %s already subscribed. Skipping subscription.", topic.c_str());
        return;
    }
    
    // Send request to broker and get msg_id
    int msg_id = esp_mqtt_client_subscribe(_client, topic.c_str(), qos);
   
    // create a new topic entry
    if(!topicExists(topic)) 
        _topics.emplace_back(topic_node_t{topic, msg_id, false, qos});
    else {
        // Update existing topic node
        for (auto& item : _topics) {
            if (item.topic_name == topic) {
                item.last_msg_id = msg_id;
                item.is_subscribed = false; // Reset subscription status until confirmed
                break;
            }
        }
    }

    if (msg_id < 0) {
        ESP_LOGE(TAG, "Failed to send subscribe for %s", topic);
    } else {
        ESP_LOGD(TAG, "sent subscribe successful topic=%s, msg_id=%d", topic.c_str(), msg_id);
    }

}

void MqttClient::UnsubscribeTopic(std::string topic){
    
    // Send request to broker and get msg_id
    int msg_id = esp_mqtt_client_unsubscribe(_client, topic.c_str());
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Failed to send unsubscribe for %s", topic);
        return;
    }
}


bool MqttClient::topicExists(std::string topic) {
    
    for (const auto& item : _topics) {
        if (item.topic_name == topic)
            return true; // Found the topic      
    }
    return false; // Topic not found in the list
}

bool MqttClient::topicSubscribed(std::string topic) {
    
    for (const auto& item : _topics) {
        if (item.topic_name == topic) {
            return item.is_subscribed; // Found the topic, return its subscription status
        }
    }
    return false; // Topic not found in the list
}

void MqttClient::resubsribeAllTopics() {
    for (auto& item : _topics) {
        item.last_msg_id = esp_mqtt_client_subscribe(_client, item.topic_name.c_str(), item.qos);
        item.is_subscribed = false; // Reset subscription status until confirmed
    }
}

void MqttClient::removeTopic(std::string topic) {

    auto it = std::remove_if(_topics.begin(), _topics.end(), [&](const topic_node_t& item) {
        return item.topic_name == topic;
    });

    if (it != _topics.end()) {
        _topics.erase(it, _topics.end());
        ESP_LOGD(TAG, "Removed topic %s from the list.", topic.c_str());
    } else {
        ESP_LOGW(TAG, "Topic %s not found in the list. Cannot remove.", topic.c_str());
    }
}

void MqttClient::markTopicSubscribed(std::string topic, int msgId, bool subscribed) {
    for (auto& item : _topics) {
        if (item.topic_name == topic) {
            item.last_msg_id = msgId;
            item.is_subscribed = subscribed;
            break;
        }
    }
}

void MqttClient::markAllTopicSubscribed(bool subscribed) {
    for (auto& item : _topics) {
        item.last_msg_id = 0;
        item.is_subscribed = subscribed;
     }
}



void MqttClient::sendData(const std::string& topic, const std::string& data, int qos, int retain)
{
    if(!_isConnected)
        return;

    int msg_id =esp_mqtt_client_publish(_client, topic.c_str(), data.c_str(), 0, qos, retain);

	if (msg_id == 0) 
        ESP_LOGD(TAG, "Sent Data: %s", data.c_str());
	else 
        ESP_LOGD(TAG, "Error msg_id:%d while sending data", msg_id);
}