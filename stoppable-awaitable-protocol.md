# Stoppable Awaitable Protocol (Specification)

Minimal, library-agnostic requirements for propagating stop tokens through coroutine `co_await` chains.

## 1. Stop Token

- A stop token is any type `S` that satisfies `std::stoppable_token<S>`.
  - `s.stop_requested()` returns `true` if a stop has been requested.
  - `s.stop_possible()` returns `true` if a stop may be requested in the future.
  - The token must be copyable and thread-safe for concurrent access.
- **Lifetime:** In `await_suspend`, the stop token is received by value (it is lightweight and copyable). The awaitable may retain it for the duration of the asynchronous operation to check for cancellation. The caller owns the stop source; the token is a non-owning view into the stop state.

## 2. Awaitable Requirements (callee role)

An awaitable **participates in the stoppable protocol** if it provides an overload:

```cpp
template<std::stoppable_token StopToken>
auto await_suspend(std::coroutine_handle<> h, StopToken st);
```

Semantics:
- The awaitable should use the stop token `st` to detect cancellation requests.
- When `st.stop_requested()` returns `true`, the awaitable should cancel its operation and resume the caller promptly.
- The awaitable may register a stop callback via `std::stop_callback` to react to cancellation asynchronously.
- The awaitable resumes the caller directly (or via dispatcher if combined with the affinity protocol).

## 3. Task/Promise Requirements (caller role)

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
   - `await_suspend(caller, stop_token)` (stoppable, stop token available) that stores the stop token + continuation before resuming the task's coroutine.

   In the stoppable path, `await_suspend(caller, st)` typically:
   - Stores the continuation handle (caller) and the stop token `st` in the promise, then
   - Returns the task's coroutine handle to start execution, enabling later forwarding (via `await_transform`) and cancellation checking throughout the coroutine.

4) **Checks for stop requests at suspension points.**
   - Before starting expensive operations, check `stop_token_.stop_requested()`.
   - In `await_ready`, optionally return `true` immediately if stop was requested (short-circuit with cancellation error).
   - In `await_resume`, optionally throw or return an error if cancellation occurred.

## 4. Propagation Rule

- The stop token is set once at the top-level task (e.g., `run_with_token(st, task)` or from a `std::stop_source`).
- Each `co_await` forwards the same stop token to the awaited object.
- Any awaited object that implements `await_suspend(h, st)` can use that token to detect and react to cancellation, preserving stoppability through arbitrary nesting.
- If an awaited object is non-stoppable, it simply cannot be cancelled mid-operation, but the stop token continues to propagate to subsequent awaits in the chain.

## 5. Combining with Affinity Protocol

The stoppable protocol can be combined with the affinity protocol by accepting both parameters,
provided the affinity protocol provides a dispatcher concept.

```cpp
template<std::dispatcher Dispatcher, std::stoppable_token StopToken = >
auto await_suspend(std::coroutine_handle<> h, Dispatcher const& d, StopToken st);
```

The combined approach enables both scheduler affinity and cancellation propagation through the same coroutine chain.

## 6. Notes

- The helper types (e.g., `stoppable_promise`, `stoppable_task`, `stoppable_awaiter`) would be convenience implementations of this protocol. They are not the protocol itself.
- Stop token propagation is zero-cost when not used (empty token or `std::never_stop_token`).
- For operations that cannot be cancelled mid-flight (e.g., synchronous work), the stop token check at the next suspension point provides cooperative cancellation.
- Libraries may choose to throw `std::system_error` or return an error of `std::errc::operation_canceled` when cancellation is detected; the protocol does not prescribe the error handling mechanism.
