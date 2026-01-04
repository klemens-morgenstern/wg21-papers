**Document number:** PXXXXR0
**Date:** 2026-01-04
**Reply-to:** Klemens Morgenstern \<klemens.morgenstern@gmx.net\>
**Audience:** SG1, LEWG

---

# Stoppable Awaitables : Zero-Overhead Cancellation Propagation

## Abstract

This document proposes a minimal protocol for zero-overhead cancellation propagation in C++ coroutines. By introducing the `stoppable_awaitable` concept, awaitables can opt into cancellation propagation without requiring the full sender/receiver protocol. The protocol is library-implementable today; standardization would bless the concept and enable ecosystem-wide adoption.

---

## 1. The Cancellation Problem

Consider a common scenario: a user initiates a network request, then decides to cancel it.

```cpp
task<Data> fetch_data() {
    auto connection = co_await connect();    // How to cancel?
    auto response = co_await send_request(); // How to cancel?
    co_return parse(response);
}
```

When a stop is requested (e.g., user clicks "Cancel"), how do we propagate that cancellation through the entire coroutine chain? Each awaited operation needs to know about the cancellation request to react appropriately—but the standard `await_suspend` has no mechanism to pass this information.

The challenge deepens when coroutines call other coroutines:

```cpp
task<Result> complex_operation() {
    auto data = co_await fetch_data();      // How does fetch_data() know to cancel?
    auto processed = co_await process(data); // How does process() know?
    co_return transform(processed);
}
```

Every nested coroutine is an opportunity to lose the cancellation signal.

---

## 2. Today's Solutions, and Costs

Several approaches exist for cancellation propagation:

### 2.1 Manual Token Threading

```cpp
task<Data> fetch_data(std::stop_token st) {
    auto connection = co_await connect(st);
    if (st.stop_requested()) co_return {};
    auto response = co_await send_request(st);
    co_return parse(response);
}
```

**Problem:** Error-prone. Every function signature must include the token. Easy to forget, easy to lose.

### 2.2 Sender Composition

```cpp
auto sender = fetch() | then([](auto data) { return process(data); });
auto result = sync_wait(with_stop_token(sender, stop_source.get_token()));
```

**Problem:** Only works with P2300 senders, excluding the ecosystem of awaitables (Asio, custom libraries). Requires adopting the full sender/receiver protocol.

### 2.3 Thread-Local or Global State

```cpp
thread_local std::stop_token current_token;

task<Data> fetch_data() {
    if (current_token.stop_requested()) co_return {};
    auto data = co_await connect();
    co_return data;
}
```

**Problem:** Thread-local state doesn't compose across execution contexts. Race conditions when coroutines migrate between threads.

### 2.4 Why This Matters Now

P3552 proposes coroutine tasks with scheduler affinity. The related affine awaitable protocol proposal addresses scheduler affinity, but cancellation is an orthogonal concern that equally benefits from zero-overhead propagation:

**Ecosystem compatibility.** Boost.Asio, libcoro, folly::coro, and countless custom awaitables can adopt cancellation propagation incrementally without converting to the sender/receiver protocol.

**Composability with affinity.** The stoppable protocol composes naturally with the affine awaitable protocol—a task can propagate both a dispatcher and a stop token through the same `await_transform` mechanism.

**No-regret design.** Stoppable awaitables integrate seamlessly with P2300—senders flow through the same `await_transform` and receive optimal handling. The protocol works regardless of sender adoption.

---

## 3. The Solution: One Parameter

The fix is simple: tell the awaitable about cancellation.

```cpp
// Standard: awaitable is blind to cancellation
void await_suspend(std::coroutine_handle<> h);

// Extended: awaitable knows the stop token
template<std::stoppable_token StopToken>
void await_suspend(std::coroutine_handle<> h, StopToken st);
```

One additional parameter enables cancellation propagation.

The awaitable receives the stop token and uses it directly:

```cpp
template<std::stoppable_token StopToken>
void await_suspend(std::coroutine_handle<> h, StopToken st) {
    // Register callback to cancel operation when stop is requested
    stop_callback_ = std::stop_callback(st, [this] { cancel_operation(); });
    start_async_operation(h);
}
```

No manual threading. No thread-local state. The awaitable handles cancellation internally.

### 3.1 Propagation

However, `await_suspend(h, st)` only tells the awaitable about cancellation—it does not propagate the token through nested coroutine chains. Consider:

```cpp
task<int> outer() {
    auto result = co_await inner();  // inner() is also a task
    co_return result;
}

task<int> inner() {
    auto data = co_await fetch();  // fetch() doesn't know about cancellation
    co_return data;
}
```

If `inner()` is a coroutine task that doesn't implement the propagation rules, the stop token is lost when `inner()` awaits other operations. 
Our proposal includes guidance for propagating stop tokens that does not require any additional standard library support beyond `std::stoppable_token`. 
It is a set of rules that awaitable authors can follow to propagate cancellation at zero cost.

---

## 4. The Stoppable Awaitable Protocol

The protocol outlined in this section gives authors tools to solve the lost cancellation problem through stop token propagation.

- **As an Awaitable**: You implement the protocol to receive the caller's stop token (detect their cancellation requests).

- **As a Task Type**: You implement the promise logic to propagate the stop token (pass it to nested awaits).

This protocol is fully implementable as a library today; standardization would bless the concept and reduce boilerplate.

### 4.1 Stoppable Awaitable Concept

An awaitable is *stoppable* if it accepts a stop token in `await_suspend`:

```cpp
template<typename A, typename S = std::stop_token, typename P = void>
concept stoppable_awaitable =
    std::stoppable_token<S> &&
    requires(A a, std::coroutine_handle<P> h, S st) {
        a.await_suspend(h, st);
    };
```

*Remarks:* This concept detects awaitables that participate in the stoppable protocol by providing the extended `await_suspend(h, st)` overload. The stop token is passed by value (it is lightweight and copyable). The awaitable may use the token to detect cancellation requests and react appropriately.

*Lifetime:* The stop token is received by value. The awaitable may retain it for the duration of the asynchronous operation to check for cancellation or register callbacks. The caller owns the stop source; the token is a non-owning view into the stop state.

### 4.2 Awaitable Requirements (callee role)

An awaitable **participates in the stoppable protocol** if it provides an overload:

```cpp
template<std::stoppable_token StopToken = std::never_stop_token>
std::coroutine_handle<> await_suspend(std::coroutine_handle<> h, StopToken st = {}) {
    // Use st to detect/react to cancellation
    if (st.stop_requested())  // Cancel immediately
        return h;
        
    // Register callback for async cancellation
    stop_callback_ = std::stop_callback(st, [this] { cancel_operation(); });
    start_async_operation(h);
}
```

Semantics:
- The awaitable should use the stop token `st` to detect cancellation requests.
- When `st.stop_requested()` returns `true`, the awaitable should cancel its operation and resume the caller promptly.
- The awaitable may register a `std::stop_callback` to react to cancellation asynchronously.

### 4.3 Task/Promise Requirements (caller role)

A task type (its promise) **propagates stop tokens** if it:

1) **Stores the caller's stop token.**
   - Provides a way to set/hold a stop token instance for the lifetime of the promise.

2) **Forwards the stop token on every await.**
   - In `await_transform`, when awaiting `A`, pass the stored stop token to the awaited object via a stoppable awaiter:
     ```cpp
     // stop_token_: whatever you store for the caller's stop token
     return stoppable_awaiter{std::forward<Awaitable>(a), stop_token_};
     ```
   - The callee receives it as `StopToken` in `await_suspend(h, st)`.
   - If `A` is not stoppable, the stop token is not forwarded to that awaitable (it cannot react to cancellation), but propagation continues to subsequent awaits.

3) **Provides both await paths for its own awaitability.**
   - `await_suspend(caller)` (legacy, no stop token available).
   - `await_suspend(caller, stop_token)` (stoppable, stop token available) that stores stop token + continuation before resuming the task's coroutine.

   In the stoppable path, `await_suspend(caller, st)` typically:
   - Stores the continuation handle (caller) and the stop token `st` in the promise, then
   - Returns the task's coroutine handle to start execution, enabling later forwarding (via `await_transform`) and cancellation checking throughout the coroutine.

4) **Checks for stop requests at suspension points.**
   - Before starting expensive operations, check `stop_token_.stop_requested()`.
   - In `await_ready`, optionally return `true` immediately if stop was requested (short-circuit with cancellation error).
   - In `await_resume`, optionally return an error if cancellation occurred.

### 4.4 Propagation Rule

- The stop token is set once at the top-level task (e.g., `run_with_token(st, task)` or from a `std::stop_source`).
- Each `co_await` forwards the same stop token to the awaited object, or specifies another token.
- Any awaited object that implements `await_suspend(h, st)` can use that token to detect and react to cancellation, preserving stoppability through arbitrary nesting.
- If an awaited object is non-stoppable, it simply cannot be cancelled mid-operation, but the stop token continues to propagate to subsequent awaits in the chain.

---

## 5. Combining with Affinity Protocol

The stoppable protocol and the affine awaitable protocol (PXXXX) are **independent and composable**. They do not require a combined signature; instead, each protocol layer wraps the previous through recursive `await_transform` application:

```cpp
// Each await_transform layer wraps the result of the previous
// Order: inner awaitable -> affine_awaiter -> stoppable_awaiter (or vice versa)

// In a task supporting both protocols:
template<std::affine_awaitable Awaitable>
auto await_transform(Awaitable&& a)
{
   affine_awaiter aw{std::forward<Awaitable>(a), &dispatcher_};
   // forward to the next await_transform layer if available
   if constexpr (requires {this->await_transform(aw);})
      return await_transform(std::move(aw));
   else
      return aw;
}

template<std::stoppable_awaitable Awaitable>
auto await_transform(Awaitable&& a)
{
   stoppable_awaiter aw{std::forward<Awaitable>(a), stop_token_};

   if constexpr (requires {this->await_transform(aw);})
      return await_transform(std::move(aw));
   else
      return aw;
}
```

Each awaiter only needs to implement its own protocol. The `stoppable_awaiter` forwards to the inner awaitable's `await_suspend`, which may itself forward the dispatcher. This layered approach means:

- Awaitables implement one or both protocols independently
- Task types compose protocol layers through nested awaiters
- The order of layers is implementation-defined (affine-then-stoppable or stoppable-then-affine)
- Each layer passes its parameter via the appropriate `await_suspend` overload
- When no stop token is set, `std::never_stop_token` provides zero-cost passthrough

---

## 6. Non-normative: Implementation Examples

The protocol defined in section 4 can be implemented entirely as a library. This section describes helper types and patterns that implement the protocol, but **these are not part of this proposal**. They are provided for illustration.

### 6.1 stoppable_awaiter

Used in `await_transform` to wrap stoppable awaitables and inject the stop token:

```cpp
template<typename Awaitable, typename StopToken>
struct stoppable_awaiter {
    Awaitable awaitable_;
    StopToken stop_token_;

    bool await_ready() {
        if (stop_token_.stop_requested()) return true; // Short-circuit
        return awaitable_.await_ready();
    }

    template<typename Promise>
    auto await_suspend(std::coroutine_handle<Promise> h) {
        if constexpr (stoppable_awaitable<Awaitable, StopToken>) {
            return awaitable_.await_suspend(h, stop_token_);
        } else {
            return awaitable_.await_suspend(h);
        }
    }

    auto await_resume() {
        if (stop_token_.stop_requested()) {
            throw std::system_error(std::make_error_code(std::errc::operation_canceled));
        }
        return awaitable_.await_resume();
    }
};
```

### 6.2 stoppable_promise

CRTP mixin for promise types, providing stop token storage and propagation:

```cpp
template<typename Derived>
struct stoppable_promise {
    std::stop_token stop_token_;

    void set_stop_token(std::stop_token st) { stop_token_ = st; }
    std::stop_token get_stop_token() const { return stop_token_; }

    template<typename Awaitable>
    auto await_transform(Awaitable&& a)
    {
       stoppable_awaiter aw{std::forward<Awaitable>(a), stop_token_};
       auto* d = static_cast<Derived*>(this);

       if constexpr (requires {d->await_transform(aw);})
          return d->await_transform(std::move(aw));
       else
          return aw;
    }
};
```

### 6.3 Example: Cancellable I/O Operation

```cpp
struct async_read_awaitable {
    socket& sock_;
    buffer& buf_;
    std::optional<std::stop_callback<std::function<void()>>> callback_;

    bool await_ready() { return false; }

    // Legacy path
    void await_suspend(std::coroutine_handle<> h) {
        sock_.async_read(buf_, [h] { h.resume(); });
    }

    // Stoppable path
    template<std::stoppable_token StopToken>
    void await_suspend(std::coroutine_handle<> h, StopToken st) {
        if (st.stop_requested()) {
            h.resume();
            return;
        }
        callback_.emplace(st, [this] { sock_.cancel(); });
        sock_.async_read(buf_, [h] { h.resume(); });
    }

    std::size_t await_resume() { return sock_.bytes_transferred(); }
};
```

---

## 7. Benefits

**Zero-Overhead Cancellation**

The stoppable protocol adds minimal overhead—a stop token is typically just a pointer. Non-stoppable code paths remain unchanged. When combined with the affine protocol, both concerns are handled through the same mechanism.

**Ecosystem Compatibility**

Existing awaitable libraries can add stoppable support incrementally:

1. Add `await_suspend(h, st)` overload
2. Use the stop token to detect/react to cancellation
3. Existing code continues to work unchanged

**Composable with Senders**

Stop tokens are already central to P2300's design. Stoppable awaitables use the same `std::stoppable_token` concept, ensuring seamless interoperability.

**Layered Design**

The protocols compose through recursive `await_transform` wrapping:
- Base: Standard `await_suspend(h)` (no affinity, no cancellation)
- Affine layer: `affine_awaiter` wraps awaitable, calls `await_suspend(h, d)`
- Stoppable layer: `stoppable_awaiter` wraps awaitable, calls `await_suspend(h, st)`
- Combined: Nested awaiters (e.g., `stoppable_awaiter{affine_awaiter{a, d}, st}`)

Each layer is independent—awaitables implement whichever protocols they support, and task types compose layers as needed. When no stop token is needed, `std::never_stop_token` provides zero-cost passthrough.

### 7.1 Implementation Patterns (Non-Normative)

#### 7.1.1 Non-Breaking Overload Addition

Adding `await_suspend(h, st)` alongside existing `await_suspend(h)` is completely non-breaking:

| Context | Awaitable Type | Overload Selected |
|---------|---------------|-------------------|
| Legacy awaitable (only `await_suspend(h)`) | Non-stoppable | `await_suspend(h)` (legacy path) |
| Stoppable awaitable (both overloads) | Stoppable | `await_suspend(h, st)` (stoppable path) |
| Legacy code awaiting stoppable awaitable | Stoppable | `await_suspend(h)` (backward compatible) |
| Stoppable-aware code awaiting legacy awaitable | Non-stoppable | `await_suspend(h)` (fallback to legacy) |

#### 7.1.2 Graceful Degradation

Unlike affinity (where losing the dispatcher means resuming on the wrong thread), losing the stop token means an operation cannot be cancelled—it simply runs to completion. This graceful degradation makes the protocol safe to adopt incrementally.

#### 7.1.3 Error Handling Strategies

The protocol does not prescribe error handling. Implementations may choose:

- **Exception:** `throw std::system_error(std::make_error_code(std::errc::operation_canceled))`
- **Expected:** `return std::unexpected(std::errc::operation_canceled)`
- **Optional:** `return std::nullopt`
- **Custom:** Application-specific error types

#### 7.1.4 Call-Site Stop Token Override

The stop token can be modified at the call-site where `co_await` is used. This allows individual operations to use a different stop token than the one propagated by the task:

```cpp
task<Data> fetch_data() {
    // Use the task's propagated stop token (default)
    auto a = co_await operation_a();

    // Override with a different stop token for this specific operation
    std::stop_source local_source;
    auto b = co_await with_stop_token(operation_b(), local_source.get_token());

    // Use std::never_stop_token to make an operation non-cancellable
    auto c = co_await with_stop_token(operation_c(), std::never_stop_token{});

    co_return combine(a, b, c);
}
```

This enables fine-grained control over cancellation semantics without affecting the overall task's stop token propagation.

---

## 8. Proposed Wording

*Relative to N4981.*

### 8.1 Header `<coroutine>` synopsis

Add to the `<coroutine>` header synopsis:

```cpp
namespace std {
  // [coroutine.stoppable.concept], stoppable awaitable concept
  template<class A, class S, class P = void>
    concept stoppable_awaitable = see-below;
}
```

### 8.2 Stoppable awaitable concept [coroutine.stoppable.concept]

```cpp
template<class A, class S, class P = void>
concept stoppable_awaitable =
  stoppable_token<S> &&
  requires(A a, coroutine_handle<P> h, S st) {
    a.await_suspend(h, st);
  };
```

*Remarks:* This concept detects awaitables that participate in the stoppable awaitable protocol (section 4) by providing the extended `await_suspend(h, st)` overload. The stop token is passed by value, and the awaitable may use it to detect cancellation requests and react appropriately (e.g., by cancelling the underlying operation and resuming the caller promptly).

---

## 9. Summary

This proposal standardizes a minimal protocol for zero-overhead cancellation propagation in C++ coroutines:

| Component | Purpose |
|-----------|---------|
| `stoppable_awaitable<A,S,P>` | Concept detecting extended `await_suspend(h, st)` protocol |

**The protocol (section 4):** Awaitables that accept a stop token in `await_suspend(h, st)` can detect and react to cancellation requests. Task types propagate the stop token through `co_await` chains, ensuring every nested operation can respond to cancellation.

**The vision:** Every coroutine can be cancelled. No manual threading, no lost signals, no rewrites.

---

## Acknowledgements

Thanks to Vinnie Falco for the affine awaitable protocol which this proposal parallels. Thanks to Lewis Baker for insights on coroutine cancellation patterns.

---

## References

### WG21 Papers

- **[P2300R10]** Michał Dominiak, Georgy Evtushenko, Lewis Baker, Lucian Radu Teodorescu, Lee Howes, Kirk Shoop, Michael Garland, Eric Niebler, Bryce Adelstein Lelbach. *`std::execution`*. [https://wg21.link/P2300R10](https://wg21.link/P2300R10)

- **[P3552R3]** Dietmar Kühl, Maikel Nadolski. *Add a Coroutine Task Type*. [https://wg21.link/P3552](https://wg21.link/P3552)

- **[PXXXX]** Vinnie Falco. *Affine Awaitables: Zero-Overhead Scheduler Affinity*. (Companion paper)

- **[N4981]** Thomas Köppe. *Working Draft, Standard for Programming Language C++*. [https://wg21.link/N4981](https://wg21.link/N4981)

### Existing Awaitable Ecosystem

- **Boost.Asio** — Cross-platform C++ library for network and low-level I/O programming, with coroutine support and cancellation slots.
  [https://www.boost.org/doc/libs/release/doc/html/boost_asio.html](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)

- **cppcoro** — Lewis Baker's coroutine library providing cancellable task types.
  [https://github.com/lewissbaker/cppcoro](https://github.com/lewissbaker/cppcoro)

- **folly::coro** — Meta's coroutine library with cancellation token support.
  [https://github.com/facebook/folly/tree/main/folly/coro](https://github.com/facebook/folly/tree/main/folly/coro)

### Background Reading

- Lewis Baker. *C++ Coroutines: Understanding the promise type*.
  [https://lewissbaker.github.io/2018/09/05/understanding-the-promise-type](https://lewissbaker.github.io/2018/09/05/understanding-the-promise-type)

---

## Revision History

### R0 (2026-01-04)

Initial revision. This paper proposes a stoppable awaitable protocol parallel to the affine awaitable protocol:

- Introduces the `stoppable_awaitable` concept for zero-overhead cancellation propagation
- Defines the protocol for stop token propagation through coroutine chains
- Shows how to combine with the affine awaitable protocol for both affinity and cancellation


