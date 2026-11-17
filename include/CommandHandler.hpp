#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "IrcContract.hpp"
#include <map>
#include <string>
#include <vector>
#include <iostream>

struct IrcClientState
{
    bool passwordAccepted;
    std::string nickname;
    std::string username;
    std::string realname;
    bool registered;

    IrcClientState()
        : passwordAccepted(false), registered(false)
    {}
};

struct ParsedCommand
{
    std::string command;
    std::vector<std::string> params;
};

class CommandHandler : public IIrcSession
{
private:
    ITransport& _transport;
    std::string _password;
    std::map<ClientId, IrcClientState> _clients;
    ParsedCommand ParseCommand(const std::string& line) const;
    void handlePass(ClientId id, const ParsedCommand& cmd);
    void handleNick(ClientId id, const ParsedCommand& cmd);
    bool isValidNickname(const std::string& nick) const;
    bool sameNickname(const std::string& a, const std::string& b) const;
    void handleUser(ClientId id, const ParsedCommand& cmd);
    void tryRegister(ClientId id);

public:
    explicit CommandHandler(ITransport& transport, const std::string& password);
    virtual ~CommandHandler() {}
    virtual void onClientConnected(ClientId id);
    virtual void onCommand(ClientId id, const std::string& line);
    virtual void onClientDisconnected(ClientId id);
};

#endif
