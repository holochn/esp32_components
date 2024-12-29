#ifndef MQTTCLIENT_HPP
#define MQTTCLIENT_HPP

#include <iostream>
#include "esp_log.h"
#include "esp_types.h"
#include "mqtt_client.h"

typedef esp_err_t eventHandleCallback(esp_mqtt_event_handle_t);

class MqttClient
{
    public:
    MqttClient(std::string node_name);
    esp_err_t initialize(const std::string host_id, 
                    const uint16_t port,
                    const std::string user, 
                    const std::string password,
                    uint16_t keepalive = 60);
    esp_err_t start();
    esp_err_t stop();
    esp_err_t connect();
    esp_err_t disconnect();
    esp_err_t publish(std::string topic, std::string value, uint8_t qos=0, uint8_t retain_flag=0);
    esp_err_t subscribe(std::string topic, uint8_t qos);
    esp_err_t unsubscribe(std::string topic);
    static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
    static void mqtt_event_handler(void *handler_args, 
                                    esp_event_base_t base, 
                                    int32_t event_id, 
                                    void *event_data);
    static esp_err_t handle_received_topic(const char *topic, 
                                    int topic_length);
    static char TAG[];

    static esp_mqtt_client_config_t mqttConfig;
    static eventHandleCallback *event_callback;

    private:
    std::string id;
    std::string host;
    std::string user;
    std::string password;
    uint16_t port;
    uint16_t keepalive;
    esp_mqtt_client_handle_t client;
    bool connected;
};

#endif // MQTTCLIENT_HPP