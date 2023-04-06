#include "MqttTopicParser.hpp"

std::vector<std::string> MqttTopicParser::parse(std::string topic)
{
    std::vector<std::string> result;
    int pos;

    do
    {
        pos = topic.find("/");
        result.push_back(topic.substr(0, pos));
        topic = topic.substr(pos+1);
    } while ( pos != std::string::npos );

    return result;
}

std::vector<std::string> MqttTopicParser::parse(const char* topic, size_t topic_length)
{
    std::vector<std::string> result;
    std::string tpc;
    int pos;

    tpc.append(topic, topic_length);
    do
    {
        pos = tpc.find("/");
        result.push_back(tpc.substr(0, pos));
        tpc = tpc.substr(pos+1);
    } while ( pos != std::string::npos );

    return result;
}

void MqttTopicParser::print(std::vector<std::string> str_vector)
{
    for(auto item : str_vector)
    {
        std::cout << item << '\n';
    }
}