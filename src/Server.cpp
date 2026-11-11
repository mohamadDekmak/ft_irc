#include "Server.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

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
