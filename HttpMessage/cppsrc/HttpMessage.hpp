#ifndef HTTP_MESSAGE_HPP
#define HTTP_MESSAGE_HPP

#include <map>
#include <string>

class HttpMessage
{
private:
    int port{0};
    std::string host;
    std::string m_message;
    std::map<std::string, std::string> http_parameter;
    
public:
    HttpMessage(std::string t_message);
    ~HttpMessage();
    int parse();
    std::map<std::string, std::string> getParameter() const;
};

#endif // HTTP_MESSAGE_HPP