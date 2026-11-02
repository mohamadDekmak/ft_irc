#ifndef SERVER_HPP
#define SERVER_HPP

#include "IrcContract.hpp"

// Owner A: TCP sockets, poll, per-client buffers and lifecycle.
// Interface declarations only; networking implementation comes in C1-C5.
class Server : public ITransport
{
public:
    virtual ~Server() {}
    virtual bool queueReply(ClientId id, const std::string& line);
    virtual void requestDisconnect(ClientId id);
};

#endif
