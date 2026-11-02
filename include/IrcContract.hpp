#ifndef IRCCONTRACT_HPP
#define IRCCONTRACT_HPP

#include <string>

// Stable connection identity, allocated by Networking. Never reuse during a run.
// It is not a socket fd or a nickname.
typedef unsigned long ClientId;

// A logical IRC line is a command without its trailing CRLF.
// A wire IRC line is a logical line followed by exactly "\r\n".
// Networking owns CRLF framing, buffering, socket reads and writes.
// IRC Logic owns parsing, registration, nicknames and channel state.

class ITransport
{
public:
    virtual ~ITransport() {}

    // Queue ONE logical IRC line for an active client.
    // Implementation appends exactly one CRLF and flushes only on writable poll.
    // Returns false when the client is unknown/disconnected or cannot be queued.
    // Never calls blocking send() here.
    virtual bool queueReply(ClientId id, const std::string& line) = 0;

    // Request a connection close. Networking performs actual fd cleanup and
    // invokes onClientDisconnected exactly once; no direct close() in IRC Logic.
    virtual void requestDisconnect(ClientId id) = 0;
};

class IIrcSession
{
public:
    virtual ~IIrcSession() {}

    // Called once after an accepted socket has an assigned ClientId.
    virtual void onClientConnected(ClientId id) = 0;

    // Called once per complete CRLF-terminated command, without CRLF.
    // Empty/malformed lines are handled safely by IRC Logic.
    virtual void onCommand(ClientId id, const std::string& line) = 0;

    // Called exactly once on connection teardown, after onClientConnected.
    // Must remove nick and channel memberships. Do not send to this id.
    virtual void onClientDisconnected(ClientId id) = 0;
};

#endif
