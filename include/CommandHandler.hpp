#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "IrcContract.hpp"

// Owner B: parser, registration, nicknames, messages, channels and modes.
// Interface declarations only; command implementation comes in P1/R/M/CH.
class CommandHandler : public IIrcSession
{
public:
    virtual ~CommandHandler() {}
    virtual void onClientConnected(ClientId id);
    virtual void onCommand(ClientId id, const std::string& line);
    virtual void onClientDisconnected(ClientId id);
};

#endif
