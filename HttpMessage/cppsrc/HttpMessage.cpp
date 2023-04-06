#include "HttpMessage.hpp"
#include <string.h>

HttpMessage::HttpMessage(std::string t_message)
{
    m_message = std::move(t_message);
}

HttpMessage::~HttpMessage()
{
}

int HttpMessage::parse()
{
    std::string param;
    std::string value;
    bool params = false;
    int i=6;
    if( m_message.substr(0,3).compare("GET") == 0 ) {
        if(m_message[5] != 32) // blank space
        {
            // requesting a page
            i=5;
            std::string page;
            while( (m_message[i] != 32) && (m_message[i] != 63 ) ) // blank or ?
            {
                ++i;
            }
            page = m_message.substr(5,i-5);

            if( page.compare("favicon.ico") == 0 )
            {
                return 200;
            }

            if(page.compare("index.html") == 0)
            {
                std::string param;
                std::string value;
                params = true;
            }
        }
    }

    if( m_message.substr(0,4).compare("POST") == 0 ) {
        if(m_message[6] != 32) // blank space
        {
            // requesting a page
            i=6;
            std::string page;
            while( (m_message[i] != 32) && (m_message[i] != 63 ) ) // blank or ?
            {
                ++i;
            }
            page = m_message.substr(6,i-6);

            if( page.compare("favicon.ico") == 0 )
            {
                return 0;
            }

            if(page.compare("index.html") == 0)
            {
                params = true;
                ++i;
            }
        }
    }

    if( m_message.substr(i,4).compare("HTTP") == 0 ) {
        i+=5;
    } else {
        return 400; // bad request
    }
    if( m_message.substr(i,3).compare("1.1") != 0 ) {
        return 505; // HTTP Version Not Supported
    }
    i+=5;
    if( m_message.substr(i,5).compare("Host:") != 0 ) {
        perror("Host");
        return 418; // I'm a teapot
    }
    i+=6;
    
    while(m_message.at(i) != ':')
    {
        host.push_back(m_message.at(i));
        ++i;
    }
    ++i;
    
    std::string str_port;
    while(m_message.at(i) != '\n')
    {
        str_port.push_back(m_message.at(i));
        ++i;
    }
    port = stoi(str_port);

    if(params)
    {
        i = m_message.find("\r\n\r\n");
        i+=4;

        while(i < m_message.length())
        {
            if(m_message.at(i) == '{')
            {
                ++i;
            }
            int j = i;
            while(m_message.at(i) != '=')
            {
                ++i;
            }
            param = m_message.substr(j, i-j);
            printf("param: %s\n", param.c_str());
            ++i; // skip "="
            j=i;
            while( true )
            {
                if( (i >= m_message.length()) || (m_message.at(i) == '&'))
                {
                    break;
                }
                ++i;
            }
            
            if( i > m_message.length())
            {
                value = m_message.substr(j, m_message.length()-j); 
                break;   
            } else {
                value = m_message.substr(j, i-j);
                ++i;
            }
            printf("value: %s\n", value.c_str());
        }
    }
    
    return 200;
}

std::map<std::string, std::string> HttpMessage::getParameter() const
{
    return http_parameter;
}
