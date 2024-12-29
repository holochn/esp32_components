idf_component_register(
    SRCS "cppsrc/MqttClient.cpp" "cppsrc/MqttTopicParser.cpp" 
    INCLUDE_DIRS "cppsrc"
    REQUIRES mqtt
)