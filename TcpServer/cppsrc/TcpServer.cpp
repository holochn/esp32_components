#include "TcpServer.hpp"
#include <unistd.h>

#define WEBCONF_PORT 1234

TcpServer::TcpServer()
{

}

TcpServer::~TcpServer()
{

}

int TcpServer::start()
{
    int opt        = 1;
    struct sockaddr_in server_addr;
    memset( &server_addr, 0, sizeof(server_addr) );
    int addrlen = sizeof(server_addr);
    

    if( (server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0 )
    {
        perror("server socket failed");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, 
                    SO_REUSEADDR | SO_REUSEPORT, 
                    &opt, 
                    sizeof(opt)) )
    {
        perror("server socket set options");
        return -1;
    }

    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(WEBCONF_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);// inet_addr(SERVER_IP.c_str());

    if( bind(server_fd, 
                (struct sockaddr*) &server_addr, 
                addrlen) < 0 )
    {
        perror("server bind to socket");
        return -1;
    }
    
    if( listen(server_fd, 3) < 0 )
    {
        perror("server listen");
        return -1;
    }

    new_socket = accept(server_fd, 
                            (struct sockaddr*) NULL, 
                            (socklen_t*) addrlen);
    if( new_socket < 0 )
    {
        perror("server accept");
        return -1;
    }    

    return 0;
}

void TcpServer::stop()
{
    close(new_socket);
    close(server_fd);
}

std::string TcpServer::receive()
{
    char inBuffer[1024]{0};
    memset(&inBuffer, 0, sizeof(inBuffer));
    int readByte   = 0;

    readByte = read(new_socket, inBuffer, 1024);
    printf("Received: %s\n", inBuffer);
    return std::string(inBuffer);
}

int TcpServer::transmit(const std::string &t_message)
{
    printf("sending %s\n", t_message.c_str());
    return send(new_socket, t_message.c_str(), t_message.length(), 0);
}
