#include "CommandHandler.hpp"
#include <iostream>
#include <set>
#include <sstream>
#include <string>

// Fake networking for Owner B: same ITransport interface as the real server.
class FakeTransport : public ITransport
{
private:
    std::set<ClientId> _connected;
    std::set<ClientId> _pendingDisconnect;

public:
    void connect(ClientId id) { _connected.insert(id); }
    void disconnect(ClientId id)
    {
        _connected.erase(id);
        _pendingDisconnect.erase(id);
    }
    bool queueReply(ClientId id, const std::string& line)
    {
        if (_connected.find(id) == _connected.end())
            return false;
        std::cout << "[reply to " << id << "] " << line << std::endl;
        return true;
    }
    void requestDisconnect(ClientId id)
    {
        if (_connected.find(id) != _connected.end())
            _pendingDisconnect.insert(id);
    }
    bool shouldDisconnect(ClientId id) const
    {
        return _pendingDisconnect.find(id) != _pendingDisconnect.end();
    }
};

int main()
{
    FakeTransport transport;
    CommandHandler session(transport, "secret123");
    std::set<ClientId> active;
    std::string input;

    std::cout << "Fake IRC test (real CommandHandler, no TCP)" << std::endl;
    std::cout << "connect <id> | send <id> <IRC line> | disconnect <id> | quit" << std::endl;

    while (std::getline(std::cin, input))
    {
        std::istringstream stream(input);
        std::string action;
        ClientId id = 0;
        stream >> action;
        if (action == "quit")
            break;
        if (!(stream >> id) || id == 0)
        {
            std::cout << "Invalid input" << std::endl;
            continue;
        }
        if (action == "connect")
        {
            if (!active.insert(id).second)
            {
                std::cout << "Already connected" << std::endl;
                continue;
            }
            transport.connect(id);
            session.onClientConnected(id);
            std::cout << "[connected] " << id << std::endl;
        }
        else if (action == "disconnect")
        {
            if (active.erase(id) == 0)
            {
                std::cout << "Unknown client" << std::endl;
                continue;
            }
            transport.disconnect(id);
            session.onClientDisconnected(id);
            std::cout << "[disconnected] " << id << std::endl;
        }
        else if (action == "send")
        {
            if (active.find(id) == active.end())
            {
                std::cout << "Connect client first" << std::endl;
                continue;
            }
            std::string line;
            std::getline(stream, line);
            if (!line.empty() && line[0] == ' ')
                line.erase(0, 1);
            std::cout << "[command from " << id << "] " << line << std::endl;
            session.onCommand(id, line);
            if (transport.shouldDisconnect(id))
            {
                active.erase(id);
                transport.disconnect(id);
                session.onClientDisconnected(id);
                std::cout << "[disconnected] " << id << std::endl;
            }
        }
        else
            std::cout << "Unknown action" << std::endl;
    }

    for (std::set<ClientId>::const_iterator it = active.begin(); it != active.end(); ++it)
    {
        transport.disconnect(*it);
        session.onClientDisconnected(*it);
    }
    return 0;
}
