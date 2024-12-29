#ifndef MQTTTOPICPARSER_HPP
#define MQTTTOPICPARSER_HPP

#include <string>
#include <iostream>
#include <vector>

class MqttTopicParser
{
    public:
    static std::vector<std::string> parse(std::string topic);
    static std::vector<std::string> parse(const char* topic, size_t topic_length);
    static void print(std::vector<std::string> str_vector);
};

#endif // MQTTTOPICPARSER_HPP