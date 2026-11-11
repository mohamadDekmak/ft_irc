#ifndef SERVER_HPP
#define SERVER_HPP

#include "IrcContract.hpp"
#include <string>

class Server : public ITransport
{
private:
    int _port;
    std::string _password;
    int _serverFd;

public:
    Server(int port, const std::string& password);
    virtual ~Server();

    bool createSocket();
    bool bindSocket();
    bool listenSocket();

    virtual bool queueReply(ClientId id, const std::string& line);
    virtual void requestDisconnect(ClientId id);
};

#endif
