#include "CommandHandler.hpp"

CommandHandler::CommandHandler(ITransport& transport, const std::string& password) : _transport(transport), _password(password){}

void CommandHandler::onClientConnected(ClientId id)
{
    _clients.insert(
        std::make_pair(id, IrcClientState())
    );
    // TODO (Owner B): create IRC registration state for this ClientId.
}

void CommandHandler::onCommand(ClientId id, const std::string& line)
{
    if (_clients.find(id) == _clients.end())
        return ;
    ParsedCommand parsed = ParseCommand(line);
    if (parsed.command.empty())
        return ;
    if (parsed.command == "PASS")
        handlePass(id, parsed);
    else if (parsed.command == "NICK")
        handleNick(id, parsed);
    else if (parsed.command == "USER")
        handleUser(id, parsed);
    else
        std::cout << "Not implemented yet: " << parsed.command << std::endl;
    // TODO (Owner B): parse IRC command and call _transport.queueReply().
}

void CommandHandler::onClientDisconnected(ClientId id)
{
    _clients.erase(id);
    // TODO (Owner B): remove nickname and channel membership state.
}

ParsedCommand CommandHandler::ParseCommand(const std::string& line) const
{
    ParsedCommand result;
    std::size_t i = 0;

    while (i < line.size() && line[i] == ' ')
        i++;
    while (i < line.size() && line[i] != ' ')
    {
        result.command += line[i];
        i++;
    }
    while (i < line.size())
    {
        while (i < line.size() && line[i] == ' ')
            i++;
        if (i >= line.size())
            break;
        if (line[i] == ':')
        {
            result.params.push_back(line.substr(i + 1));
            break;
        }
        std::size_t start = i;
        while (i < line.size() && line[i] != ' ')
            i++;
        result.params.push_back(line.substr(start,i - start));
    }
    return result;
}

void CommandHandler::handlePass(ClientId id, const ParsedCommand& cmd)
{
    std::map<ClientId, IrcClientState>::iterator it = _clients.find(id);
    if (it == _clients.end())
        return ;
    if (it->second.registered)
    {
        _transport.queueReply(id, ":ircser 462 * :You may not register");
        return ;
    }
    if (cmd.params.empty())
    {
        _transport.queueReply(id, ":ircser 461 * PASS :Not enough parameters");
        return ;
    }
    if (cmd.params[0] != _password)
    {
        it->second.passwordAccepted = false;
        _transport.queueReply(id, ":ircser 464 * :Incorrect password");
        return ;
    }
    it->second.passwordAccepted = true;
    tryRegister(id);
}

void CommandHandler::handleNick(ClientId id, const ParsedCommand& cmd)
{
    std::map<ClientId, IrcClientState>::iterator it = _clients.find(id);
    if (it == _clients.end())
        return ;
    if (cmd.params.empty() || cmd.params[0].empty())
    {
        _transport.queueReply(id, ":ircser 431 * :No nicjname given");
        return ;
    }
    std::string newNick = cmd.params[0];
    if (!isValidNickname(newNick))
    {
        _transport.queueReply(id, ":ircser 432 * " + newNick + " :Errorneous nickname");
        return ;
    }
    for (std::map<ClientId, IrcClientState>::const_iterator
        other = _clients.begin();
        other != _clients.end(); ++other)
    {
        if (other->first != id && sameNickname(other->second.nickname, newNick))
        {
             _transport.queueReply(id, ":ircser 433 * " + newNick + " :Nickname is already in use");
             return ;
        }
    }
    std::string oldNick = it->second.nickname;
    if (oldNick == newNick)
        return ;
    it->second.nickname = newNick;
    if (it->second.registered)
    {
        _transport.queueReply(id, ":" + oldNick + " NICK :" + newNick);
    }
    tryRegister(id);
}


bool CommandHandler::isValidNickname(
    const std::string& nick) const
{
    if (nick.empty() || nick.size() > 9)
        return false;

    const std::string firstAllowed =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "[]\\`_^{|}";

    const std::string otherAllowed =
        firstAllowed + "0123456789-";

    if (firstAllowed.find(nick[0]) == std::string::npos)
        return false;

    for (std::size_t i = 1; i < nick.size(); i++)
    {
        if (otherAllowed.find(nick[i]) == std::string::npos)
            return false;
    }

    return true;
}


bool CommandHandler::sameNickname(
    const std::string& a,
    const std::string& b) const
{
    if (a.size() != b.size())
        return false;

    for (std::size_t i = 0; i < a.size(); i++)
    {
        char ca = a[i];
        char cb = b[i];

        if (ca >= 'A' && ca <= 'Z')
            ca = ca - 'A' + 'a';

        if (cb >= 'A' && cb <= 'Z')
            cb = cb - 'A' + 'a';

        if (ca == '[' || ca == '{')
            ca = '{';
        if (cb == '[' || cb == '{')
            cb = '{';

        if (ca == ']' || ca == '}')
            ca = '}';
        if (cb == ']' || cb == '}')
            cb = '}';

        if (ca == '\\' || ca == '|')
            ca = '|';
        if (cb == '\\' || cb == '|')
            cb = '|';

        if (ca == '^' || ca == '~')
            ca = '~';
        if (cb == '^' || cb == '~')
            cb = '~';

        if (ca != cb)
            return false;
    }

    return true;
}


void CommandHandler::tryRegister(ClientId id)
{
    std::map<ClientId, IrcClientState>::iterator it =
        _clients.find(id);

    if (it == _clients.end())
        return;
    IrcClientState& client = it->second;
    if (client.registered)
        return;
    if (!client.passwordAccepted ||
        client.nickname.empty() ||
        client.username.empty())
        return;
    client.registered = true;
    _transport.queueReply(id,
        ":ircserv 001 " + client.nickname +
        " :Welcome to the IRC server");
}


void CommandHandler::handleUser(
    ClientId id,
    const ParsedCommand& cmd)
{
    std::map<ClientId, IrcClientState>::iterator it =
        _clients.find(id);

    if (it == _clients.end())
        return;
    if (it->second.registered || !it->second.username.empty())
    {
        _transport.queueReply(id,
            ":ircserv 462 * :You may not reregister");
        return;
    }
    if (cmd.params.size() < 4 || cmd.params[0].empty())
    {
        _transport.queueReply(id,
            ":ircserv 461 * USER :Not enough parameters");
        return;
    }
    it->second.username = cmd.params[0];
    it->second.realname = cmd.params[3];
    tryRegister(id);
}
