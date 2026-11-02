# ft_irc

A C++98 IRC server project for 42. This repository currently contains **only a starter scaffold**, not a working IRC server.

## Goal
Build a single non-blocking TCP server that accepts multiple IRC clients, handles registration and messaging, and manages channels and operator commands.

## Build
```sh
make
./ircserv 6667 mypassword
```

**Note:** the executable is currently a placeholder. It does not open a socket or accept connections yet.

## Components
- `Server`: TCP listening socket, poll loop, connection lifecycle, I/O.
- `Client`: per-connection state, registration, input/output buffers.
- `CommandHandler`: parse/dispatch IRC commands and produce responses.
- `Channel`: channel membership, topic, operators, modes.

## Suggested division
- Networking owner: Server, client connection lifecycle, poll, buffers.
- IRC logic owner: command parser, registration, messaging, channels, operator commands.
- Shared: agree on Client/CommandHandler interfaces first, then integration, tests, review.

## Workflow
Use feature branches and pull requests; coordinate changes to shared headers.

[Visual roadmap](https://www.tldraw.com/f/EJjRvs5CFtkGYacR4CiiO) | [GitHub issues](https://github.com/mohamadDekmak/ft_irc/issues)

## Contract-first parallel development

The agreed C++98 boundary lives in [include/IrcContract.hpp](include/IrcContract.hpp): `ITransport` is implemented by Networking, and `IIrcSession` is implemented by IRC Logic. Shared messages are **logical IRC lines without CRLF**; the transport appends CRLF once and queues non-blocking writes.

Read the owner guides before starting work:

- [Owner A — Networking responsibilities, tasks and mock handler](docs/OWNER_A_NETWORKING.md)
- [Owner B — IRC Logic responsibilities, tasks and mock transport](docs/OWNER_B_IRC_LOGIC.md)

The contract headers define interfaces only; implementations and integration tests are still TODO. Coordinate signature changes between both owners. [Contracts visual board](https://www.tldraw.com/f/o9MEJy0d4XEmQQHFCcceU).
