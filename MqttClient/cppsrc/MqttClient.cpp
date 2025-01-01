#include "MqttClient.hpp"
#include <vector>

char MqttClient::TAG[] = "MqttClient";

esp_mqtt_client_config_t MqttClient::mqttConfig;
eventHandleCallback* MqttClient::event_callback;

MqttClient::MqttClient(std::string node_name) :
        id{node_name}, connected{false}
{
}

esp_err_t MqttClient::initialize(std::string host_id, 
                    const uint16_t port, 
                    std::string user, 
                    std::string password,
                    uint16_t keepalive)
{
    if(host_id.size() <= 3)
    {
        return ESP_ERR_INVALID_ARG;
    }
    
    mqttConfig.broker.address.hostname = host_id.c_str();
    mqttConfig.broker.address.port = port;
    mqttConfig.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
    mqttConfig.credentials.client_id = id.c_str();
    mqttConfig.session.last_will.topic = id.c_str();
    mqttConfig.session.last_will.msg   = "offline";
    mqttConfig.session.keepalive = keepalive;

    event_callback = mqtt_event_handler_cb;

     if( (!user.empty()) && (!password.empty()) )
    {
        mqttConfig.credentials.username = user.c_str();
        mqttConfig.credentials.authentication.password = password.c_str();
    }

    client = esp_mqtt_client_init(&MqttClient::mqttConfig);
    if(client == NULL)
    {
        return ESP_ERR_INVALID_RESPONSE;
    }
    if(esp_mqtt_client_register_event(client, 
                                      (esp_mqtt_event_id_t) ESP_EVENT_ANY_ID, 
                                      MqttClient::mqtt_event_handler, client) != ESP_OK)
    {
        ESP_LOGE(TAG, "MQTT register event");
        return ESP_ERR_INVALID_STATE;
    }
    if(esp_mqtt_client_start(client) != ESP_OK)
    {
        ESP_LOGE(TAG, "MQTT start client");
        return ESP_FAIL;
    }
    connected = true;
    return ESP_OK;
}

esp_err_t MqttClient::start()
{
    if(client == NULL)
    {
        ESP_LOGE(TAG, "start without init");
        return ESP_FAIL;
    }

    if(connected == true)
    {
        ESP_LOGI(TAG, "Already connected");
        return ESP_ERR_INVALID_STATE;    
    }

    if(esp_mqtt_client_start(client) == ESP_OK)
    {
        connected = true;
        return ESP_OK;
    }
    return ESP_FAIL;
}
esp_err_t MqttClient::stop()
{
    connected = false;
    return esp_mqtt_client_stop(client);
}

esp_err_t MqttClient::connect()
{
    return esp_mqtt_client_reconnect(client);
}

esp_err_t MqttClient::disconnect()
{
    return esp_mqtt_client_disconnect(client);
}

esp_err_t MqttClient::publish(std::string topic, std::string value, uint8_t qos, uint8_t retain_flag)
{
    std::string tpc = id;
    tpc.append("/");
    tpc.append(topic);
    if( (qos > 3) || (retain_flag > 1) )
    {
        return ESP_ERR_INVALID_ARG;
    }
    return (esp_err_t) esp_mqtt_client_publish(client, tpc.c_str(), value.c_str(), value.size(), qos, retain_flag);
}

esp_err_t MqttClient::subscribe(std::string topic, uint8_t qos)
{
    
    if(client == NULL)
    {
        return ESP_FAIL;
    }
    if(!connected)
    {
        connect();
    }
    if(esp_mqtt_client_subscribe(client, topic.c_str(), qos) > 0)
    {
        return ESP_OK;
    } else
    {
        return ESP_FAIL;
    }
    
}


esp_err_t MqttClient::unsubscribe(std::string topic)
{
    if(client == NULL)
    {
        return ESP_FAIL;
    }
    if(!connected)
    {
        connect();
    }
    if(esp_mqtt_client_unsubscribe(client, topic.c_str()) > 0)
    {
        return ESP_OK;
    } else
    {
        return ESP_FAIL;
    }
}

esp_err_t MqttClient::mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch(event->event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            
            ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);
            break;
        default:
            break;
    }

    return ESP_OK;
}

void MqttClient::mqtt_event_handler(void *handler_args, 
                                    esp_event_base_t base, 
                                    int32_t event_id, 
                                    void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, (int) event_id);
    MqttClient::event_callback((esp_mqtt_event_handle_t) event_data);
}
