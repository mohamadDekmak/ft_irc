#ifndef SERVER_HPP
#define SERVER_HPP

#include "IrcContract.hpp"

// Networking class: handles connections and sending/receiving messages.
// The functions will be implemented later.
class Server : public ITransport
{
public:
    virtual ~Server() {}
    virtual bool queueReply(ClientId id, const std::string& line);
    virtual void requestDisconnect(ClientId id);
};

#endif
