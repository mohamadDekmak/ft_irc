#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "IrcContract.hpp"

// Owner B implements IRC behavior here. Networking is accessed via ITransport.
class CommandHandler : public IIrcSession
{
private:
    ITransport& _transport;

public:
    explicit CommandHandler(ITransport& transport);
    virtual ~CommandHandler() {}
    virtual void onClientConnected(ClientId id);
    virtual void onCommand(ClientId id, const std::string& line);
    virtual void onClientDisconnected(ClientId id);
};

#endif
