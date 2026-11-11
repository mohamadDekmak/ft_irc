#ifndef IRCCONTRACT_HPP
#define IRCCONTRACT_HPP

#include <string>

// A unique number for each connected client. It is not a socket number or nickname.
typedef unsigned long ClientId;

// Networking receives and sends messages. IRC Logic understands the commands.
// Messages passed between them do not include the final "\r\n".

class ITransport
{
public:
    virtual ~ITransport() {}

    // IRC Logic asks Networking to send a message to this client.
    // Networking adds "\r\n" and sends it when the socket is ready.
    // Returns false if the client is unavailable or the message cannot be queued.
    virtual bool queueReply(ClientId id, const std::string& line) = 0;

    // IRC Logic asks Networking to disconnect this client.
    // Networking closes the connection and reports the disconnect once.
    virtual void requestDisconnect(ClientId id) = 0;
};

class IIrcSession
{
public:
    virtual ~IIrcSession() {}

    // Networking tells IRC Logic that a new client connected.
    virtual void onClientConnected(ClientId id) = 0;

    // Networking gives IRC Logic one complete command, without "\r\n".
    virtual void onCommand(ClientId id, const std::string& line) = 0;

    // Networking tells IRC Logic that a client disconnected.
    // IRC Logic removes the client's nickname and channel memberships.
    virtual void onClientDisconnected(ClientId id) = 0;
};

#endif
