#include "IrcContract.hpp"
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>

// Fake networking for Owner B. No sockets or poll().
class FakeTransport : public ITransport
{
private:
    std::set<ClientId> _connected;

public:
    void connect(ClientId id) { _connected.insert(id); }
    void disconnect(ClientId id) { _connected.erase(id); }
    bool isConnected(ClientId id) const
    {
        return _connected.find(id) != _connected.end();
    }

    bool queueReply(ClientId id, const std::string& line)
    {
        if (!isConnected(id))
            return false;
        std::cout << "[reply to " << id << "] " << line << std::endl;
        return true;
    }

    void requestDisconnect(ClientId id)
    {
        std::cout << "[disconnect requested for " << id << "]" << std::endl;
        // Actual cleanup is done by the test driver, like real Networking.
    }
};

// Demo only. Replace with Owner B's CommandHandler when it is implemented.
class DemoSession : public IIrcSession
{
private:
    ITransport& _transport;

public:
    DemoSession(ITransport& transport) : _transport(transport) {}

    void onClientConnected(ClientId id)
    {
        std::cout << "[connected] " << id << std::endl;
    }

    void onCommand(ClientId id, const std::string& line)
    {
        std::cout << "[command from " << id << "] " << line << std::endl;
        _transport.queueReply(id, "NOTICE * :Demo only - implement IRC logic here");
    }

    void onClientDisconnected(ClientId id)
    {
        std::cout << "[disconnected] " << id << std::endl;
    }
};

int main()
{
    FakeTransport transport;
    DemoSession session(transport);
    std::set<ClientId> active;
    std::string input;

    std::cout << "Fake IRC connections (no real TCP)." << std::endl;
    std::cout << "Commands: connect <id>, send <id> <IRC line>, disconnect <id>, quit" << std::endl;

    while (std::getline(std::cin, input))
    {
        std::istringstream stream(input);
        std::string action;
        ClientId id;
        stream >> action;
        if (action == "quit")
            break;
        if (!(stream >> id) || id == 0)
        {
            std::cout << "Usage: connect <id> | send <id> <line> | disconnect <id> | quit" << std::endl;
            continue;
        }
        if (action == "connect")
        {
            if (!active.insert(id).second)
            {
                std::cout << "Client already connected" << std::endl;
                continue;
            }
            transport.connect(id);
            session.onClientConnected(id);
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
        }
        else if (action == "send")
        {
            if (active.find(id) == active.end())
            {
                std::cout << "Connect the client first" << std::endl;
                continue;
            }
            std::string line;
            std::getline(stream, line);
            if (!line.empty() && line[0] == ' ')
                line.erase(0, 1);
            session.onCommand(id, line);
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
