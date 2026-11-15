#include "CommandHandler.hpp"

CommandHandler::CommandHandler(ITransport& transport) : _transport(transport) {}

void CommandHandler::onClientConnected(ClientId id)
{
    (void)id;
    // TODO (Owner B): create IRC registration state for this ClientId.
}

void CommandHandler::onCommand(ClientId id, const std::string& line)
{
    (void)id;
    (void)line;
    // TODO (Owner B): parse IRC command and call _transport.queueReply().
}

void CommandHandler::onClientDisconnected(ClientId id)
{
    (void)id;
    // TODO (Owner B): remove nickname and channel membership state.
}
