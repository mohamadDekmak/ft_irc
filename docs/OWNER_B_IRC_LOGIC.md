# Owner B — IRC Logic, Registration, Messaging and Channels

## Goal and ownership

Implement IRC protocol behavior **without waiting for sockets/poll**. You own command parsing/dispatch, registration, nickname uniqueness, user lookup, private and channel messaging, channels, operator commands, numeric replies and protocol state. You do **not** call `recv()`, `send()`, `poll()`, or `close()`.

**Issues:** [R1 #7–R5 #11](https://github.com/mohamadDekmak/ft_irc/issues/7), [P1 #24](https://github.com/mohamadDekmak/ft_irc/issues/24), [M1 #12–M5 #16](https://github.com/mohamadDekmak/ft_irc/issues/12), [CH1 #17–CH7 #23](https://github.com/mohamadDekmak/ft_irc/issues/17). Shared QA: [QA1 #25](https://github.com/mohamadDekmak/ft_irc/issues/25), [QA2 #26](https://github.com/mohamadDekmak/ft_irc/issues/26).

## Frozen boundary: what you must deliver

See `include/IrcContract.hpp`. Implement `IIrcSession` in `CommandHandler`:

```cpp
void onClientConnected(ClientId id);
void onCommand(ClientId id, const std::string& logicalLine);
void onClientDisconnected(ClientId id);
```

- `ClientId` is a stable identity assigned by Networking, **not** a socket fd or nickname. Nickname changes must not break references.
- `onClientConnected`: initialize IRC session/registration state for this id.
- `onCommand`: receive **one complete command line without CRLF**. Parse safely; enforce registration/permissions; update IRC state; send zero or more responses via `ITransport::queueReply(id, logicalLine)`.
- `onClientDisconnected`: remove nickname mapping, invitations as appropriate, channel memberships/operator state; never reply to the closed id.
- Outgoing `queueReply` takes **one logical IRC line without CRLF**; Networking appends `\r\n` and sends asynchronously. Check the boolean return and avoid unsafe assumptions about disconnected recipients.
- Request closure through `ITransport::requestDisconnect(id)`, never direct `close()`.
- All callbacks are synchronous on the single event-loop thread; do not hold references into state that may be erased during disconnect.

**Owned files (suggested):** `src/CommandHandler.cpp`, `src/Channel.cpp`, `include/Channel.hpp`, IRC session state and logic tests. Coordinate changes to `include/Client.hpp` and shared headers.

## Task sequence and definition of done

1. **R1 + P1:** Decide IRC session fields (PASS accepted, nickname, username, registered, etc.), nickname lookup, and centralized parser/dispatcher. Parse IRC command and trailing parameter (`:` consumes the rest of the line). Ensure malformed input cannot crash.
2. **R2–R4:** PASS password check; NICK uniqueness/validity; USER registration; registration completes only when prerequisites hold. Return appropriate IRC replies/errors.
3. **M1–M2:** Parse PRIVMSG targets and trailing text, deliver private messages to nicknames, correct sender prefix and errors.
4. **CH1–CH2:** Channel class, membership/operator state, JOIN and PART, JOIN broadcasts, and access rules.
5. **M3–M5:** Channel PRIVMSG fan-out to all members except sender, permission/errors, integration tests.
6. **CH3–CH6:** KICK, INVITE, TOPIC, MODE flags i/t/k/o/l; correct operator permissions, key and limit checks.
7. **R5 + CH7:** Full lifecycle, registration errors, nickname changes, disconnect cleanup, and end-to-end tests.
8. **QA1 + QA2:** Validate with a real IRC client, C++98 compile, malformed input, protocol interoperability and subject compliance. Coordinate PING/PONG, QUIT and any client-required compatibility behavior; do not assume a real client only sends mandatory commands.

## Test your real logic without Owner A

The repository already contains a fake networking driver at `tests/fake_connection.cpp`. It calls the **real** `CommandHandler`, not a demo session. You do not need to write your own fake connection.

Run from the repository root:

```sh
c++ -Wall -Wextra -Werror -std=c++98 -Iinclude tests/fake_connection.cpp src/CommandHandler.cpp -o fake_irc_test
./fake_irc_test
```

If your logic adds other source files (for example `src/Channel.cpp`), add them to the compile command.

Try these inputs:

```text
connect 1
connect 2
send 1 PASS secret123
send 1 NICK ali
send 1 USER ali 0 * :Ali
send 2 PASS secret123
send 2 NICK sara
send 2 USER sara 0 * :Sara
send 1 PRIVMSG sara :hello
disconnect 1
quit
```

The driver maps `connect` to `onClientConnected`, `send` to `onCommand`, and `disconnect` to `onClientDisconnected`. The fake `queueReply` prints `[reply to ID] ...`. A `requestDisconnect` made during a command is handled by the driver after the callback, with one disconnect notification.

**Important:** `CommandHandler.cpp` currently has TODO callbacks and does not implement PASS/NICK/USER/JOIN or other IRC behavior. Initially you will see connection/command logs but no real replies. Implement the behavior in `CommandHandler`; the same test driver then exercises your new code without changes. The constructor already takes `ITransport&`, stored as `_transport`. Use `_transport.queueReply(id, logicalLine)` and `_transport.requestDisconnect(id)`.

This test is for IRC logic, not TCP, poll, framing or partial writes. `send` supplies one complete logical line without CRLF. Keep fake code out of the production server. The test driver does not automatically enforce IRC correctness; add assertions for replies and state as logic grows.

## Integration handoff to Owner A

Show that `onCommand` handles complete IRC lines, `queueReply` contains correct IRC wire content **without** CRLF, and `onClientDisconnected` cleans session/channel state. Then replace FakeTransport with Server; no command code should change.

## Joint decisions / do not silently change

- Where IRC session state lives: prefer an IRC-owned session map keyed by ClientId, with Client transport state owned by Networking.
- `CommandHandler` receives `ITransport&` through its constructor; agree how to pass and validate the server password before implementing PASS.
- Output failure policy, disconnect timing, line size and buffer limits.
- Shared `Client.hpp` ownership; any contract change needs both owners' approval.
- Real IRC client expectations, registration welcome replies, PING/PONG and QUIT handling.

## Ready for review when

The fake-transport tests cover PASS/NICK/USER, direct PRIVMSG, JOIN/PART, channel broadcast, KICK/INVITE/TOPIC/MODE, malformed commands, disconnect cleanup and permission errors. The handler never performs socket I/O and can run against the real Server without rewriting logic.
