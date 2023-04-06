#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>


const std::string SERVER_IP         = "127.5.24.1";

class TcpServer
{
private:
    int new_socket{0};
    int server_fd{0};
public:
    TcpServer();
    int start();
    void stop();
    std::string receive();
    int transmit(const std::string &t_message);
    ~TcpServer();
};

#endif // TCP_SERVER_HPP