# Owner A — Networking and Connections

## Goal and ownership

Build the non-blocking TCP transport that accepts multiple clients and passes **complete IRC command lines** to IRC Logic. You own socket setup, `poll()`, connection IDs, per-client input/output buffers, disconnects, and cleanup. You do **not** implement PASS, NICK, USER, PRIVMSG, JOIN, or channel permissions.

**Issues:** [C1 #2](https://github.com/mohamadDekmak/ft_irc/issues/2), [C2 #3](https://github.com/mohamadDekmak/ft_irc/issues/3), [C3 #4](https://github.com/mohamadDekmak/ft_irc/issues/4), [C4 #5](https://github.com/mohamadDekmak/ft_irc/issues/5), [C5 #6](https://github.com/mohamadDekmak/ft_irc/issues/6). Shared integration: [R1 #7](https://github.com/mohamadDekmak/ft_irc/issues/7), [P1 #24](https://github.com/mohamadDekmak/ft_irc/issues/24), [QA1 #25](https://github.com/mohamadDekmak/ft_irc/issues/25), [QA2 #26](https://github.com/mohamadDekmak/ft_irc/issues/26).

## Frozen boundary: what you must deliver

See `include/IrcContract.hpp`. Implement `ITransport` in `Server`:

```cpp
bool queueReply(ClientId id, const std::string& logicalLine);
void requestDisconnect(ClientId id);
```

- `ClientId` is a monotonically assigned connection identity, independent of socket fd and mutable nickname. Do not reuse IDs during one server run.
- `queueReply` receives **one IRC logical line without CRLF**, appends exactly `\r\n`, and enqueues it for a writable `poll()` event. It returns `false` for unknown/closed clients or when queuing is refused; never sends synchronously.
- `requestDisconnect` schedules a safe close, rather than deleting a client while its command is still being processed. On actual teardown, call `onClientDisconnected(id)` exactly once.
- Notify the IRC logic object through `IIrcSession`: `onClientConnected(id)` once on accept; `onCommand(id, line)` once per complete CRLF-terminated command, with CRLF removed; `onClientDisconnected(id)` once on teardown.
- Callbacks and transport run in one event-loop thread. Do not let a callback invalidate the currently iterated poll/client container.
- Treat incomplete reads as bytes to buffer, not commands. Handle several commands in one `recv`, and commands split across several `recv` calls.
- Handle partial writes using a persistent output queue and offset. Enable `POLLOUT` only while queued data remains.
- No `recv`/`send` unless readiness was reported by `poll()`; non-blocking fds only. Handle errors, hangups, and disconnected peers safely.
- Apply IRC protocol limits and bounded buffering; coordinate precise limits and overflow policy with Owner B before implementing them.

**Owned files (suggested):** `src/Server.cpp`, `src/Client.cpp`, transport part of `include/Client.hpp`, networking tests. Shared headers are contract-controlled: do not change signatures alone.

## Task sequence and definition of done

1. **C1:** Parse port/password at startup, create non-blocking listening socket, set appropriate socket options, bind/listen; errors clean up correctly. Do not hardcode localhost if remote clients must connect.
2. **C2:** Accept multiple clients, set each fd non-blocking, allocate a stable ClientId, track active connection and transport buffers, notify connected/disconnected.
3. **C3:** One `poll()` loop manages listening socket plus readable/writable client sockets. Never block the entire server on one client.
4. **C4:** Receive buffer extracts CRLF-delimited lines; output queue handles partial sends and backpressure. Correctly process coalesced/split commands.
5. **C5:** Integration with real `IIrcSession`; test multiple clients, malformed input, disconnect, slow peer, partial reads/writes and no leaks.

## How to start today without Owner B

Create a **FakeIrcSession** in `tests/` implementing `IIrcSession`:

```cpp
class FakeIrcSession : public IIrcSession {
public:
    virtual void onClientConnected(ClientId id) { (void)id; }
    virtual void onCommand(ClientId id, const std::string& line) {
        (void)id;
        // Record line in a vector, or ask transport to queue a test response.
        (void)line;
    }
    virtual void onClientDisconnected(ClientId id) { (void)id; }
};
```

Run `./ircserv 6667 secret123` **after you implement startup**, then connect two `nc -C 127.0.0.1 6667` sessions. Send `NICK ali`, two lines together, and a line in pieces. Assert the fake handler receives exactly the logical lines in order. Use a fake response to test `queueReply` and `POLLOUT`. None of these tests requires real PASS/NICK implementation.

## Integration handoff to Owner B

Show that a complete line reaches `onCommand`, a queued reply reaches the correct client, and closing a socket triggers cleanup exactly once. Then Owner B replaces FakeIrcSession with the real handler. Keep mock code outside the production executable.

## Joint decisions / do not silently change

- `ClientId` lifecycle and who owns the `Client` object.
- Maximum line/input/output queue sizes and overflow/disconnect behavior.
- How the `Server` receives an `IIrcSession` reference (constructor or setter).
- Exact policy for handling `queueReply == false`, graceful `QUIT` and disconnect timing.
- Reference IRC client and test matrix.

## Ready for review when

`make` passes with `-Wall -Wextra -Werror -std=c++98`; multiple clients work; all I/O is non-blocking and poll-gated; no leaks or use-after-free; callback ordering is correct; mocks can be swapped for the real IRC handler without changing the contract.
