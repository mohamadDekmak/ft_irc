#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "IrcContract.hpp"

// IRC Logic class: understands commands and manages users and channels.
// The functions will be implemented later.
class CommandHandler : public IIrcSession
{
public:
    virtual ~CommandHandler() {}
    virtual void onClientConnected(ClientId id);
    virtual void onCommand(ClientId id, const std::string& line);
    virtual void onClientDisconnected(ClientId id);
};

#endif
