# MQTT Client
A basix MQTT client for esp idf v5.2

## Usage
Create an instance of class MqttClient using the constructor `MqttClient mqttClient(mqtt_id)`, 
where `mqtt_id` is the id of the client as `const char*`.
Then initialize the instance using the initialize method passing the broker URI as const char,
the port as uint16_t, the user name as std::string and the password as std::string.
```
 mqttClient.initialize(mqtt_uri, mqtt_port,
                          mqtt_user,
                          mqtt_pw);
```

Start the client using the start method.
```
 mqttClient.start();
```

To publish a message, use the publish method passing the topic as std::string,
the payload as std::string as well as the qualix of serive (uint8_t = 0, 1 or 2) and
the retain flag (uint8_t where 0=do not retain, 1=retain).
```
mqttClient.publish(mqtt_topic, mqtt_msg,
                          mqtt_qos,
                          mqtt_retain);
```

To stop the client use disconnect method and stop method:
```
mqttClient.disconnect(); 
mqttClient.stop();
```
