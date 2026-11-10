/*
  mqtt
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/
#include "mqtt_client.h"
#include <string>
#include <functional>

#pragma once


class MqttClient
{
public:
    /// @brief attribute update callback type
    using dataCallback_t = std::function<void(int,          // msg_id
                                            std::string,    // topic
                                            std::string)>;  // data
    
    using connectionCallback_t = std::function<void(bool,   // connected
                                                int)>;      // session_presentdata or event error_handle->error_type

private:

    // @brief topic struct to keep track of the subscribed topics
    typedef struct topic_node {
        std::string topic_name;
        int last_msg_id = 0;         
        bool is_subscribed = false;   
        int qos = 0; // Quality of Service level for the topic
    } topic_node_t;

    bool _isConnected = false;
    esp_mqtt_client_handle_t _client = nullptr;
    std::vector<dataCallback_t>         _dataHandlers;
    std::vector<connectionCallback_t>   _connectionHandlers;
    std::vector<topic_node_t>           _topics;  

public:
    MqttClient();
    void start();
    void stop();
    void sendData(const std::string& topic, const std::string& data, 
                                        int qos = 0, int retain = 0);
    esp_mqtt_client_handle_t getClient() { return _client; }

    bool isConnected() { return _isConnected; }
    void setConnected(bool connected = true) { _isConnected = connected; }

    void subscribeTopic(std::string topic, int qos = 0);
    void UnsubscribeTopic(std::string topic);
    bool topicExists(std::string topic); 
    bool topicSubscribed(std::string topic); 

    void resubsribeAllTopics();
    void removeTopic(std::string topic);
    void markTopicSubscribed(std::string topic, int msgId, bool subscribed = true);
    void markAllTopicSubscribed(bool subscribed);

    
    /// @brief register attribute update handler for this attribute.
    /// Update handler shall be type eventCallback_t : 
    /// void(int msg_id, std::string topic, std::string data)
    /// @param func pointer to the method ex: &Main::eventHandler
    /// @param instance instance of the object for this handler (ex: this)
    template<typename C, typename... Args>
    void registerDataHandler(void (C::* func)(Args...), C* instance){
        _dataHandlers.push_back([instance, func](Args&&... args) {
            (instance->*func)(std::forward<Args>(args)...);
        });
    }
    
    /// @brief register connection handler for this attribute.
    /// shalle be type connectionCallback_t :
    /// void(bool connected, int session_present)
    template<typename C, typename... Args>
    void registerConnectionHandler(void (C::* func)(Args...), C* instance){
        _connectionHandlers.push_back([instance, func](Args&&... args) {
            (instance->*func)(std::forward<Args>(args)...);
        });
    }

    
    

private:
    static void mqtt_event_handler(void *handler_args, esp_event_base_t base, 
                                        int32_t event_id, void *event_data);
    


}; // MqttClient Class
