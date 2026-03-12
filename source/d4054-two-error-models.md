---
title: "Two Error Models"
document: D4054R0
date: 2026-03-12
reply-to:
  - "Vinnie Falco <vinnie.falco@gmail.com>"
audience: LEWG
---

## Abstract

Operations partition naturally into two classes based on their postcondition structure. Infrastructure operations have binary outcomes: the postcondition was satisfied or it was not. Compound-result operations produce a status classification and associated data, always paired. The partition is not specific to asynchronous programming - `std::from_chars` and POSIX `read()` exhibit the same split. The partition becomes a concrete problem when the sender three-channel model forces a routing decision that synchronous return values do not face. This paper identifies the partition and examines the three-channel model against it. This paper is informational and proposes no action.

---

## Revision History

### R0: March 2026 (post-Croydon mailing)

- Initial version.

---

## 1. Disclosure

The author developed [P4007R0](https://wg21.link/p4007r0)<sup>[6]</sup> ("Senders and Coroutines") and publishes coroutine-native I/O libraries. The analysis below holds regardless of whether any alternative design exists. Every claim is sourced to published committee papers, OS specifications, or cited implementations.

---

## 2. The Partition

Operations fall into two classes based on their postcondition structure. The partition is not about asynchrony. It is about what the operation promises to return.

### 2.1 Infrastructure Operations

Launch, schedule, dispatch, allocate, enqueue, open. The operation either executes or it does not. An error means the postcondition was violated.

| Operation          | Success                | Failure                  |
| ------------------ | ---------------------- | ------------------------ |
| `malloc`           | Block returned         | Allocation failed        |
| `fopen`            | File handle returned   | Open failed              |
| `pthread_create`   | Thread running         | Creation failed          |
| GPU kernel launch  | Kernel running         | Launch failed            |
| Thread pool submit | Task queued            | Pool exhausted           |
| Timer arm          | Timer armed            | Resource limit           |
| Mutex acquire      | Lock held              | Deadlock / timeout       |

Every row is binary. Success is one thing. Failure is another. There is no middle ground, no partial outcome, no compound result. The operation either satisfied its postcondition or it did not. This is true whether the operation is synchronous (`malloc`, `fopen`) or asynchronous (GPU launch, timer arm).

### 2.2 Compound-Result Operations

Read, write, connect, accept, parse, convert. The operation completes and returns a classification of what happened, paired with associated data.

| Operation        | Result                        |
| ---------------- | ----------------------------- |
| `read`           | `(status, bytes_transferred)` |
| `write`          | `(status, bytes_written)`     |
| `from_chars`     | `(ptr, errc)`                 |
| `strtol`         | `(value, endptr, errno)`      |
| `accept`         | `(status, peer_socket)`       |

`std::from_chars` is synchronous. `read` may be synchronous or asynchronous. Both return compound results where the status and the associated data are inseparable. The partition follows from postcondition structure, not from how the operation is dispatched.

The status code is not evidence of a postcondition violation. It is the postcondition. `read(fd, buf, n)` promises to report what happened on the descriptor. `from_chars(first, last, value)` promises to report how far it parsed and whether parsing succeeded. Every status code fulfills that promise:

| error_code    | bytes | What happened  | Postcondition violated? |
| ------------- | ----- | -------------- | ----------------------- |
| success       | N     | Full read      | No                      |
| success       | M < N | Partial read   | No                      |
| `EOF`         | 0     | Peer closed    | No                      |
| `ECONNRESET`  | 0     | Peer reset     | No                      |
| `EWOULDBLOCK` | 0     | Try again      | No                      |
| `EBADF`       | 0     | Invalid fd     | **Yes**                 |

Only `EBADF`-class errors - invalid file descriptor, bad address, not a socket - are postcondition violations. Every other outcome is the operation correctly reporting the state of the world.

### 2.3 Compound Results in Practice

The compound-result pattern is not a design preference. It is how systems report outcomes when the result carries more than a boolean:

- **io_uring.** Each completion queue entry (CQE) carries `res` (byte count or negative errno) and `flags`. One structure. One dequeue. The kernel does not route success and failure to different queues.

- **IOCP.** `GetQueuedCompletionStatus` returns a `BOOL`, an `lpNumberOfBytesTransferred`, and an `lpOverlapped`. The error and the byte count arrive together in one call.

- **POSIX.** `read()` returns `ssize_t`. On success, the byte count. On failure, `-1` with `errno` set. Both values are available at the same call site.

- **C++ Standard Library.** `std::from_chars` returns `from_chars_result{ptr, ec}`. The pointer and the error code arrive together in one struct. The caller inspects both at the same call site.

Three OS families, the C++ standard library, the same shape: status and data arrive as a pair because they are a single result.

### 2.4 Where the Partition Becomes a Problem

For synchronous code, compound results are straightforward. The function returns a struct or a tuple. The caller inspects both fields at the same call site. No routing decision is required.

C++ has had two channels since exceptions were introduced: `return` and `throw`. They are mutually exclusive execution paths. When a function throws, the return value is destroyed. For compound-result operations, this is the same structural problem: a `write` that throws on `ECONNRESET` destroys the 500-byte transfer count the application needed.

The industry solved this decades ago: use the value channel whenever the result is not 100% failure. `std::from_chars` returns `{ptr, errc}` instead of throwing on parse failure. POSIX `read()` returns a byte count instead of raising a signal on EOF. Asio returns `(error_code, size_t)` as a pair. The error channel - `throw` - is reserved for genuine postcondition violations: `EBADF`, bad address, not a socket. Everything else goes through the value channel, keeping the pair intact. The reason is not performance. It is information preservation.

The sender three-channel model reintroduced the problem the industry had already learned to avoid - and extended it from two channels to three. `set_value`, `set_error`, and `set_stopped` are mutually exclusive execution paths. A compound result that was a single return value must now be split across channels. The remaining sections examine what happens when the three-channel model meets each side of the partition.

---

## 3. The Infrastructure Error Model

The sender three-channel model assigns semantic meaning to each channel:

- `set_value(args...)` - the postcondition was satisfied; here are the results
- `set_error(err)` - the postcondition was violated; here is why
- `set_stopped()` - the operation was cancelled before attempting the postcondition

For infrastructure operations, this mapping is unambiguous:

```cpp
// Timer sender: binary outcome
auto timer_sender = schedule_at(scheduler, deadline);
// Success: set_value()       - timer expired
// Failure: set_error(ec)     - resource exhaustion
// Cancel:  set_stopped()     - cancelled before expiry
```

```cpp
// Thread pool submit: binary outcome
auto submit_sender = on(pool.get_scheduler(), work);
// Success: set_value(result) - work executed
// Failure: set_error(ec)     - pool rejected
// Cancel:  set_stopped()     - cancelled before dispatch
```

Channel assignment is deterministic. No information is lost. No adaptation is required. `then` sees the result. `upon_error` sees the error. `upon_stopped` sees the cancellation. The channels compose with generic algorithms: `retry` retries on `set_error`, `when_all` cancels siblings on failure, `upon_error` routes errors to handlers.

The three-channel model is correct for this class. `std::execution` serves it well.

---

## 4. The Compound-Result Error Model

For compound-result operations, the postcondition is: "return the outcome classification and associated data." This postcondition is always satisfied. The operation always reports what happened.

The status code is not a failure signal. It is a vocabulary:

- `ECONNRESET` - the peer reset the connection
- `EPIPE` - the write end is closed
- `ETIMEDOUT` - the peer did not respond within the timeout
- `EWOULDBLOCK` - nothing available now; try again
- `EOF` - the peer closed the connection gracefully

Each requires different application handling. None indicates that the operation failed to operate. `ECONNRESET` means "fatal, abort the transaction" in an HTTP request handler, but "done, expected closure" in a long-polling connection that the server intentionally drops. The same error code, classified differently depending on the protocol state the application holds.

Partial results are normal. A `write` that sends 500 of 1,000 bytes before `ECONNRESET` produces `(ECONNRESET, 500)`. Both values are needed. The 500 tells the application how much of the message was acknowledged. The status tells it why the rest was not.

Chris Kohlhoff identified this in [P2430R0](https://wg21.link/p2430r0)<sup>[2]</sup> ("Partial success scenarios with P2300," 2021):

> "Due to the limitations of the `set_error` channel (which has a single 'error' argument) and `set_done` channel (which takes no arguments), partial results must be communicated down the `set_value` channel."

POSIX has modeled I/O this way since 1988<sup>[9]</sup>. Asio has modeled it this way since 2003. Every language runtime that provides async I/O - Go, Rust, Python, C#, Java - delivers compound results. This is not a novel observation. It is forty years of convergent practice.

---

## 5. The Channel Split

When a sender-based I/O operation completes, it must choose a channel. The byte count and the status code - which the OS delivered as a pair - are now split:

```cpp
async_read(socket, buffer)
    | then([](size_t n) {
          // byte count visible
          // error_code invisible
      })
    | upon_error([](std::error_code ec) {
          // error_code visible
          // byte count invisible
      });
```

`then` and `upon_error` are mutually exclusive execution paths. The programmer cannot inspect both values at the same call site. The I/O result was a pair. The model split it.

To recover the pair, the programmer must defeat the channels:

```cpp
async_read(socket, buffer)
    | into_expected
    | then([](std::expected<size_t, std::error_code> result) {
          // both visible again
      });
```

This works. But the programmer paid the cost of the channel model - the split, the adaptor, the wrapping - to undo the channel model. The channels are not helping. They are an obstacle being worked around. `let_error` is the same pattern: catch the error, re-emit through `set_value`. Channel laundering.

Consider a partial write. The application asks to write 1,000 bytes. The kernel accepts 500 before the connection dies. `set_error(ECONNRESET)` discards the 500. The application needs that number to know what was acknowledged by the peer - whether to retry, what offset to resume from, whether the transaction can be salvaged. The channel model destroyed it.

### 5.1 Known Mappings

Two known attempts to solve the channel assignment problem for I/O illustrate the structural difficulty. Neither involves coroutines. Both operate at the receiver level: which of `set_value`, `set_error`, `set_stopped` does the I/O sender call when the kernel completion arrives?

**The default mapping.** [beman.net](https://github.com/bemanproject/net)<sup>[7]</sup>, the reference sender-based networking implementation:

```cpp
if (!ec)
    set_value(std::move(rcvr), n);
else
    set_error(std::move(rcvr), ec);
```

All non-zero error codes go to `set_error`. The byte count is discarded unconditionally. Total information loss on any error.

**The refined mapping.** Peter Dimov proposed a convention in [P4007R0](https://wg21.link/p4007r0)<sup>[6]</sup> (Section 3.6) that preserves partial results by discriminating the channel based on the byte count and error code category:

| Completion                     | Channel             |
| ------------------------------ | ------------------- |
| `(eof, n)` for any n          | `set_value(eof, n)` |
| `(canceled, n)` for any n     | `set_stopped()`     |
| `(ec, 0)` where ec is not eof | `set_error(ec)`     |
| `(ec, n)` where n > 0         | `set_value(ec, n)`  |

This is the only known mapping that preserves partial results on the error path. Dimov characterized it as "ad hoc."<sup>[6]</sup> The mapping requires semantic knowledge of specific error conditions: EOF is distinguished from cancellation, which is distinguished from all other errors. It is domain-specific I/O logic embedded in the channel routing.

Even this mapping has costs. Zero-byte errors lose data - they go to `set_error` with no byte count. The classification is context-free: the I/O layer decides which channel before the application can inspect the result. `ECONNRESET` is classified irrevocably at the I/O layer, before the protocol handler - the only code that knows whether `ECONNRESET` is fatal or expected - has a chance to see it.

Both mappings demonstrate the same structural problem: any function from `(error_code, size_t)` to `{set_value, set_error, set_stopped}` must either lose data, embed domain-specific classification at the wrong layer, or bypass the channels entirely. The problem is not the mapping. The problem is that the mapping is required at all.

---

## 6. Three Defenses, Three Concessions

Three positions are available to defend the three-channel model for I/O. Each concedes the thesis.

### 6.1 Accept the Information Loss

Argue that the byte count does not matter when an error occurs. `set_error(ECONNRESET)` is sufficient; the partial transfer count is unneeded.

This contradicts POSIX semantics, where `write()` returns the number of bytes written even when a subsequent call would fail. It contradicts Asio's `(error_code, size_t)` convention, which has delivered both values for twenty years. It contradicts network programming practice across every language and framework. Partial writes are normal. The byte count is how the application knows what was acknowledged.

### 6.2 Route Everything Through `set_value`

Call `set_value(error_code, bytes)` for all outcomes. The pair stays intact. No information loss.

But `set_error` and `set_stopped` now serve no purpose for I/O senders. Generic algorithms that dispatch on channels - `retry`, `when_all`, `upon_error` - cannot distinguish I/O success from I/O failure because both arrive through `set_value`. The three-channel model reduces to one channel with a structured result. The channels are not wrong. They are irrelevant.

### 6.3 Classify Errors Into Channels

Route "routine" errors like `ECONNRESET` through `set_value` and "exceptional" errors like `ENOMEM` through `set_error`. This is the refined mapping from Section 5.1 taken further.

This requires a taxonomy of error codes that does not exist in POSIX, Windows, or any OS API. It redefines `set_value` from "the operation succeeded" to "here is a result that might contain an error." And the classification must happen at the I/O layer, before the application - the only code with enough context to classify correctly - has seen the result. `ECONNRESET` is fatal in one protocol and expected in another. No context-free classification can get this right.

### 6.4 The Fork

Each position concedes the thesis:

- **(a)** loses I/O data that applications need
- **(b)** makes the channels irrelevant for I/O
- **(c)** redefines what the channels mean and classifies at the wrong layer

There is no fourth position. The three-channel model either loses data, becomes unused, or changes its semantics when applied to I/O.

---

## 7. Domain Boundary

The finding is not that the three-channel model is wrong. It is that the model has a domain. The partition exists independent of asynchrony. The channel problem exists because the sender model forces a routing decision.

| Domain           | Sync/Async | Error model      | Three-channel model fits? |
| ---------------- | ---------- | ---------------- | ------------------------- |
| `malloc`         | Sync       | Infrastructure   | Yes                       |
| `fopen`          | Sync       | Infrastructure   | Yes                       |
| GPU dispatch     | Async      | Infrastructure   | Yes                       |
| Thread pool      | Async      | Infrastructure   | Yes                       |
| Timer            | Async      | Infrastructure   | Yes                       |
| `from_chars`     | Sync       | Compound-result  | No                        |
| `strtol`         | Sync       | Compound-result  | No                        |
| Read / Write     | Either     | Compound-result  | No                        |
| Connect / Accept | Either     | Compound-result  | No                        |
| DNS resolve      | Either     | Compound-result  | No                        |

The reference implementation of `std::execution`<sup>[8]</sup> targets compile-time work graphs and GPU dispatch. Those are infrastructure operations with binary outcomes. The model is correct for its design domain. The compound-result operations - whether synchronous or asynchronous - are outside that domain.

---

## 8. Conclusion

Two error models exist. They are not a matter of taste. They follow from the postcondition structure of the operations. The partition is not about asynchrony.

Infrastructure operations have binary postconditions: satisfied or violated. `malloc` returns a pointer or null. A GPU kernel launches or it does not. The three-channel model maps these cleanly.

Compound-result operations produce a status classification paired with associated data. `from_chars` returns `{ptr, errc}`. `read` returns `(status, bytes_transferred)`. The status code is the postcondition, not evidence of its violation. The three-channel model cannot represent these results without losing data, bypassing the channels, or redefining what the channels mean.

Synchronous code handles compound results naturally - the function returns a struct and the caller inspects both fields. The partition becomes a problem when the sender three-channel model forces a routing decision that splits the pair before the application sees it.

The partition is the same one Chris Kohlhoff identified in 2021<sup>[2]</sup>. It is the same one every OS kernel enforces by delivering status and byte count as a single completion. It is the reason `std::from_chars` returns a struct with two fields. It is the reason Asio delivers `(error_code, size_t)` as a pair. It is not new. It is forty years old.

---

## References

1. [P2300R10](https://wg21.link/p2300r10) - "std::execution" (Micha&lstrok; Dominiak et al., 2024). https://wg21.link/p2300r10

2. [P2430R0](https://wg21.link/p2430r0) - "Partial success scenarios with P2300" (Chris Kohlhoff, 2021). https://wg21.link/p2430r0

3. [P2762R2](https://wg21.link/p2762r2) - "Sender/Receiver Interface For Networking" (Dietmar K&uuml;hl, 2023). https://wg21.link/p2762r2

4. [P3570R2](https://wg21.link/p3570r2) - "Optional variants in sender/receiver" (Fabio Fracassi, 2025). https://wg21.link/p3570r2

5. [P3552R3](https://wg21.link/p3552r3) - "Add a Coroutine Task Type" (Dietmar K&uuml;hl, Maikel Nadolski, 2025). https://wg21.link/p3552r3

6. [P4007R0](https://wg21.link/p4007r0) - "Senders and Coroutines" (Vinnie Falco, Mungo Gill, 2026). https://wg21.link/p4007r0

7. [bemanproject/net](https://github.com/bemanproject/net) - Sender/receiver networking library. https://github.com/bemanproject/net

8. [stdexec](https://github.com/NVIDIA/stdexec) - Reference implementation of std::execution. https://github.com/NVIDIA/stdexec

9. IEEE Std 1003.1 - POSIX `read()` / `write()` specification. https://pubs.opengroup.org/onlinepubs/9799919799/
