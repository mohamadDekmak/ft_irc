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

## How to start today without Owner A

Create a **FakeTransport** in `tests/` implementing `ITransport`, capturing replies into memory:

```cpp
#include "IrcContract.hpp"
#include <map>
#include <string>
#include <vector>

class FakeTransport : public ITransport {
public:
    std::map<ClientId, std::vector<std::string> > replies;
    virtual bool queueReply(ClientId id, const std::string& line) {
        replies[id].push_back(line);
        return true;
    }
    virtual void requestDisconnect(ClientId id) { (void)id; }
};
```

Construct the real `CommandHandler` with this fake transport (constructor dependency injection to be agreed jointly). Then test by calling the interface directly:

```cpp
handler.onClientConnected(1);
handler.onClientConnected(2);
handler.onCommand(1, "PASS secret123");
handler.onCommand(1, "NICK ali");
handler.onCommand(1, "USER ali 0 * :Ali");
handler.onCommand(2, "PASS secret123");
handler.onCommand(2, "NICK sara");
handler.onCommand(2, "USER sara 0 * :Sara");
handler.onCommand(1, "PRIVMSG sara :hello");
```

Assert the fake transport collected a correctly formatted PRIVMSG for ClientId 2. Next test JOIN, channel PRIVMSG, operator permissions, and error responses. **No TCP socket or running Server required.** FakeTransport is only for tests; the real Server later implements the same interface.

## Integration handoff to Owner A

Show that `onCommand` handles complete IRC lines, `queueReply` contains correct IRC wire content **without** CRLF, and `onClientDisconnected` cleans session/channel state. Then replace FakeTransport with Server; no command code should change.

## Joint decisions / do not silently change

- Where IRC session state lives: prefer an IRC-owned session map keyed by ClientId, with Client transport state owned by Networking.
- How CommandHandler receives `ITransport&` and server password.
- Output failure policy, disconnect timing, line size and buffer limits.
- Shared `Client.hpp` ownership; any contract change needs both owners' approval.
- Real IRC client expectations, registration welcome replies, PING/PONG and QUIT handling.

## Ready for review when

The fake-transport tests cover PASS/NICK/USER, direct PRIVMSG, JOIN/PART, channel broadcast, KICK/INVITE/TOPIC/MODE, malformed commands, disconnect cleanup and permission errors. The handler never performs socket I/O and can run against the real Server without rewriting logic.
