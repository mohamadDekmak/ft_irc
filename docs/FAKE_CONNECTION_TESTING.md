# Fake connections for Owner B

This is a standalone test driver. It does **not** open sockets or implement IRC commands.

## Run

```sh
c++ -Wall -Wextra -Werror -std=c++98 -Iinclude tests/fake_connection.cpp -o fake_irc_test
./fake_irc_test
```

Type:

```text
connect 1
connect 2
send 1 PASS secret
send 1 NICK Mohamad
send 2 JOIN #general
disconnect 1
quit
```

Expected: connection callbacks, command callbacks, and a demo reply printed in the terminal. No real authentication or channel behavior yet.

## How Owner B uses this

- `FakeTransport` implements `ITransport`: `queueReply` prints a reply, `requestDisconnect` logs a request.
- `DemoSession` implements `IIrcSession` only as an example.
- Once Owner B implements `CommandHandler` with a transport dependency, replace `DemoSession session(transport)` with `CommandHandler session(transport)`, include its header, and compile with its .cpp dependencies.
- Keep `connect` and `disconnect` in the test driver: they simulate networking calling `onClientConnected` and `onClientDisconnected` exactly once.
- `send` calls `onCommand` with a complete logical IRC line (no CRLF).
- Do not put fake networking into production `Server` or `main.cpp`.

`requestDisconnect` is a request, not an automatic disconnect in this simple demo. The test driver must perform cleanup and notify the session, as the real server will.
