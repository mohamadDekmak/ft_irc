#include "Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _serverFd(-1)
{
}

Server::~Server()
{
    if (_serverFd != -1)
        close(_serverFd);
}

bool Server::createSocket()
{
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd == -1)
    {
        std::cerr << "Error: socket creation failed" << std::endl;
        return false;
    }
    return true;
}

bool Server::bindSocket()
{
    if (_serverFd == -1)
        return false;

    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(_port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(_serverFd, reinterpret_cast<struct sockaddr*>(&address),
             sizeof(address)) == -1)
    {
        std::cerr << "Error: bind failed" << std::endl;
        return false;
    }
    return true;
}

// Implemented when client management is added in C2-C4.
bool Server::queueReply(ClientId id, const std::string& line)
{
    (void)id;
    (void)line;
    return false;
}

void Server::requestDisconnect(ClientId id)
{
    (void)id;
}
