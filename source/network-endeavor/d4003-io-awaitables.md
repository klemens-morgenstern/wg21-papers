---
title: "Ask: IoAwaitable for Coroutine-Native Byte-Oriented I/O"
document: P4003R1
date: 2026-03-31
reply-to:
  - "Vinnie Falco <vinnie.falco@gmail.com>"
  - "Steve Gerbino <steve@gerbino.co>"
  - "Mungo Gill <mungo.gill@me.com>"
audience: LEWG
---

## Abstract

This paper asks the committee to advance the _IoAwaitable_ protocol as the standard vocabulary for coroutine-native byte-oriented I/O.

We start from the motivating use case, inspired by the Networking TS:

```
auto [ec, n] = co_await socket.read_some( buf );
```

Then, we add only enough infrastructure to make this work and not one thing more.

A companion paper, [P4172R0](https://wg21.link/p4172r0)<sup>[1]</sup>, provides the design rationale, evidence framework, preemptive objections, and analysis of alternative approaches.

Everything in this paper comes from a complete implementation of I/O and networking on three platforms: [Capy](https://github.com/cppalliance/capy)<sup>[2]</sup> (protocol) and [Corosio](https://github.com/cppalliance/corosio)<sup>[3]</sup>. A self-contained demonstration is available on [Compiler Explorer](https://godbolt.org/z/Wzrb7McrT)<sup>[22]</sup>.

---

## Revision History

### R1: March 2026 (post-Croydon mailing)

* Example wording removed.
* Paper now asks for floor time.
* Design choices, rationale, post-adoption retrospectives moved to companion paper.
* Companion links and reference [1] point to [P4172R0](https://wg21.link/p4172r0); introduction states narrow-waist motivation; disclosure superscript aligned on the second P4100 link.

### R0: March 2026 (pre-Croydon mailing)

* Initial version.

---

## 1. Disclosure

The author provides information and serves at the pleasure of the committee.

This paper is part of the [Network Endeavor](https://wg21.link/p4100r0)<sup>[32]</sup> ([P4100R0](https://wg21.link/p4100r0)<sup>[32]</sup>), a project to bring coroutine-native byte-oriented I/O to C++.

The author developed and maintains [Capy](https://github.com/cppalliance/capy)<sup>[2]</sup> and [Corosio](https://github.com/cppalliance/corosio)<sup>[3]</sup> and believes coroutine-native byte-oriented I/O is the correct foundation for networking in C++.

Coroutine-native byte-oriented I/O and `std::execution` address different domains and should coexist in the C++ standard.

---

## 2. Introduction

This paper is the first normative increment of the [Network Endeavor](https://wg21.link/p4100r0)<sup>[32]</sup>: a **narrow waist** of concepts, environment type, and launch functions so coroutine-native byte I/O libraries share the same suspension and allocation boundary. It is not a full networking stack. Higher layers still pick buffer and reactor strategies; without this waist, every library keeps an incompatible task and environment model.

This paper complements [P2300R10](https://wg21.link/p2300r10)<sup>[4]</sup> in the standard. Byte-oriented I/O is sequential by nature; GPU dispatch and parallel algorithms are DAG-shaped. _IoAwaitable_ and `std::execution` address different domains and should coexist. See [P4172R0](https://wg21.link/p4172r0)<sup>[1]</sup> for coexistence sketches, objections, and evidence.

### 2.1 What WG21 Says

> "...'why doesn't C++ have networking support?'"<sup>[5]</sup> - Stroustrup (2019, WG21 reflector, quoted with permission)

> "...it's time to provide networking support..."<sup>[6]</sup> - Voutilainen (2017)

> "networking is a priority of the Direction Group...Users are clamouring for its addition."<sup>[7]</sup> - Kohlhoff (2019)

### 2.2 What SG14 Says

> "SG14 advise that Networking (SG4) should not be built on top of P2300. The allocation patterns required by P2300 are incompatible with low-latency networking requirements."<sup>[8]</sup> - Michael Wong, SG14 (2026)

> "Standardize 'Direct Style' I/O - Prioritize P4003 (or similar Direct Style concepts) as the C++29 Networking model. It offers the performance of Asio/Beast with the ergonomics of Coroutines, maintaining the 'Zero-Overhead' principle."<sup>[8]</sup> - Michael Wong, SG14 (2026)

Disclosure: The author is the creator of [Boost.Beast](https://github.com/boostorg/beast)<sup>[9]</sup>.

### 2.3 What The Experts Say

> The Coroutines TS provided a wonderful way to write asynchronous code as if you were writing synchronous code.<sup>[10]</sup> - Lewis Baker (2020)

> One criticism of senders/receivers is that they can be challenging to work with and difficult to fully understand.<sup>[11]</sup> - Lucian Radu Teodorescu (P2300 co-author)

> Coroutines provide a much friendlier approach to concurrency.<sup>[11]</sup> - Lucian Radu Teodorescu (2024)

### 2.4 What The Users Say

> A derivatives exchange is porting from Asio callbacks to coroutine-native I/O. Early results: it works. - Mungo Gill ([P4125R1](https://wg21.link/p4125r1)<sup>[27]</sup>, 2026)

> I think coroutines are a godsend in writing expressive code.<sup>[12]</sup> - Jeremy Ong (2021)

> We end up writing long chains of callbacks (continuations) and doing a lot of manual buffer management. Asio with C++20 coroutines has dramatically improved this situation.<sup>[13]</sup> - xc-jp (2022)

> "Write asynchronous code ... with the readability of synchronous code."<sup>[14]</sup> - James Pascoe (ACCU 2022)

### 2.5 What The Author Says

```
auto [ec, n] = co_await socket.read_some( buf );
```

---

## 3. What Coroutines Need for I/O

What follows is the minimum.

### 3.1 How to Suspend

The following statement suspends the coroutine:

```cpp
auto [ec, n] = co_await socket.read_some( buf );
```

| Element  | Type                          | Description                        |
|----------|-------------------------------|------------------------------------|
| `ec`     | `std::error_code`             | Result of the operation            |
| `n`      | `std::size_t`                 | Number of bytes transferred        |
| `socket` | I/O object                    | Bound to an execution context      |
| `buf`    | Bidirectional range of buffers| Where bytes are read into          |

When this line executes:

1. The coroutine suspends at `co_await`. Control passes to `read_some`, which holds the coroutine handle.
2. `read_some` submits the operation to the OS (epoll, IOCP, io_uring, kqueue).
3. The OS completes the read. Now the reactor holds a coroutine handle and needs to wake the coroutine:

```cpp
std::coroutine_handle<> h = /* ...the suspended coroutine... */;

h.resume();  // but WHERE? on which thread? under whose control?
```

This is the question that drives the entire protocol. The reactor cannot just call `h.resume()` - that resumes on the reactor's thread, possibly while holding a lock, possibly re-entering code that is not re-entrant. Something must decide where and how the coroutine wakes up.

For this line to work, the awaitable behind `socket.read_some(buf)` needs three things at the moment of suspension: who resumes me (executor), should I stop (stop token), and where do child frames come from (frame allocator).

### 3.2 How to Resume

The reactor has a coroutine handle. It cannot just call `resume()`. Something decides where and how the coroutine wakes up. That something is the executor:

```cpp
executor ex = /* ...the coroutine's executor... */;

ex.post( h );
```

The executor queues the coroutine for resumption under the application's control. That is its entire job. The full _Executor_ concept is presented in Section 4.

### 3.3 How to Stop

The stop mechanism should be invisible until you need it. Most coroutines never touch it. But when you need it, it should be obvious:

```cpp
auto token = co_await get_stop_token;

if (token.stop_requested())
    co_return;
```

One line to get the token. One check. Built on `std::stop_token`.

### 3.4 How to Launch

A regular function cannot `co_await` (see [P4035R0](https://wg21.link/p4035r0)<sup>[15]</sup> for a discussion of coroutine escape hatches). To start a coroutine chain, you call a launch function:

```cpp
run_async( ex )( my_coroutine() );
```

The executor is required. The stop token and frame allocator are optional. This is where the three concerns from 3.1-3.3 come together - the launch function binds them to the coroutine chain.

### 3.5 How to Allocate

Every coroutine has a frame, and every frame must be allocated. This is why coroutines look slow. Despite the cost, the right frame allocator makes coroutines performant. Thus, the frame allocator must be a first-class citizen.

Frame allocators come in two versions:

1. A classic typed allocator (e.g. `std::allocator`, a custom pool allocator) - the user's own type
2. A `std::pmr::memory_resource*` - type-erased, for when the concrete type does not matter

| Platform    | Frame Allocator  | Time (ms) | Speedup |
|-------------|------------------|----------:|--------:|
| MSVC        | Recycling        |   1265.2  |   3.10x |
| MSVC        | mimalloc         |   1622.2  |   2.42x |
| MSVC        | `std::allocator` |   3926.9  |       - |
| Apple clang | Recycling        |   2297.08 |   1.55x |
| Apple clang | `std::allocator` |   3565.49 |       - |

The protocol must:

- Provide a reasonable, customizable default
- Propagate the frame allocator to every coroutine frame in the chain automatically
- Keep function signatures clean, unless the programmer needs otherwise
- Allow a coroutine to `co_await` a new chain with a different frame allocator

---

## 4. The I/O Awaitable Protocol

What follows is the minimum as well.

### 4.1 `io_env`

The `io_env` struct contains three members that a coroutine needs for I/O: the executor, the stop token, and the frame allocator:

```cpp
struct io_env
{
    executor_ref executor;
    std::stop_token stop_token;
    std::pmr::memory_resource* frame_allocator = nullptr;
};
```

### 4.2 _IoAwaitable_

Implementations and library authors provide types satisfying _IoAwaitable_:

```cpp
template< typename A >
concept IoAwaitable =
    requires(
        A a, std::coroutine_handle<> h, io_env const* env )
    {
        a.await_suspend( h, env );
    };
```

The two-argument `await_suspend` is the mechanism. The caller's `await_transform` injects the environment as a pointer parameter - no templates, no type leakage. A `task` needs only one template parameter. The environment is passed as a pointer because the launch function owns the `io_env` and every coroutine in the chain borrows it - pointer semantics make the ownership model explicit.

The two-argument signature is also a compile-time boundary check. A non-compliant awaitable fails to compile. A compliant awaitable in a non-compliant coroutine fails to compile. Both sides of every suspension point are statically verified. In a world with multiple coexisting async models, a coroutine that accidentally `co_await`s across model boundaries should fail at compile time, not silently misbehave at runtime. See [P4172R0](https://wg21.link/p4172r0)<sup>[1]</sup> for the alternative design discussion and detailed analysis.

### 4.3 _Executor_

```cpp
struct continuation
{
    std::coroutine_handle<> h;
    continuation* next = nullptr;
};

template<class E>
concept Executor =
    std::is_nothrow_copy_constructible_v<E> &&
    std::is_nothrow_move_constructible_v<E> &&
    requires( E& e, E const& ce, E const& ce2,
              continuation& c ) {
        { ce == ce2 } noexcept -> std::convertible_to<bool>;
        { ce.context() } noexcept;
        requires std::is_lvalue_reference_v<
            decltype(ce.context())> &&
            std::derived_from<
                std::remove_reference_t<
                    decltype(ce.context())>,
                execution_context>;
        { ce.on_work_started() } noexcept;
        { ce.on_work_finished() } noexcept;
        { ce.dispatch( c ) }
            -> std::same_as< std::coroutine_handle<> >;
        { ce.post(c) };
    };

class executor_ref
{
    void const* ex_ = nullptr;
    detail::executor_vtable const* vt_ = nullptr;

public:
    template<Executor E>
    executor_ref(E const& e) noexcept
        : ex_(&e), vt_(&detail::vtable_for<E>) {}

    std::coroutine_handle<> dispatch(continuation& c) const;
    void post(continuation& c) const;
    execution_context& context() const noexcept;
};
```

The `continuation` struct pairs a `coroutine_handle<>` with an intrusive `next` pointer, allowing executors to queue continuations without allocating a separate node - eliminating the last steady-state allocation in the hot path. `dispatch` returns a `coroutine_handle<>` for symmetric transfer: if the caller is already in the executor's context, it returns `c.h` directly for zero-overhead resumption. Otherwise it queues and returns `noop_coroutine()`. `post` always defers. The `executor_ref` type-erases any _Executor_ as two pointers - one indirection (~1-2 nanoseconds<sup>[16]</sup>) is negligible for I/O operations at 10,000+ nanoseconds. See [P4172R0](https://wg21.link/p4172r0)<sup>[1]</sup> for detailed semantics.

### 4.4 `execution_context`

```cpp
class execution_context
{
public:
    class service
    {
    public:
        virtual ~service() = default;
    protected:
        service() = default;
        virtual void shutdown() = 0;
    };

    execution_context( execution_context const& ) = delete;
    execution_context& operator=(
        execution_context const& ) = delete;
    ~execution_context();
    execution_context();

    template<class T> T& use_service();
    template<class T, class... Args>
        T& make_service( Args&&... args );

    std::pmr::memory_resource*
        get_frame_allocator() const noexcept;
    void set_frame_allocator(
        std::pmr::memory_resource* mr ) noexcept;

protected:
    void shutdown() noexcept;
    void destroy() noexcept;
};

template<class X>
concept ExecutionContext =
    std::derived_from<X, execution_context> &&
    requires(X& x) {
        typename X::executor_type;
        requires Executor<typename X::executor_type>;
        { x.get_executor() } noexcept
            -> std::same_as<typename X::executor_type>;
    };
```

An executor's `context()` returns the `execution_context` - the base class for anything that runs work. The platform reactor lives here. Services provide singletons with ordered shutdown. I/O objects hold a reference to their execution context, not to an executor. This design borrows from [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)<sup>[17]</sup>.

The execution context holds the default frame allocator. The user can optionally override it, and every coroutine chain launched through that context uses it. This is how the "reasonable, customizable default" from Section 3.5 works in practice.

### 4.5 Frame Allocator Delivery

There are exactly two ways to deliver the allocator to `operator new`:

1. **The parameter list.** This is `allocator_arg_t` - it always works and is always available as a fallback, but it should not be the only option.
2. **State.** The allocator is stored somewhere the `operator new` can find it.

The protocol specifies accessor functions but leaves the storage mechanism to the implementer:

```cpp
std::pmr::memory_resource*
get_current_frame_allocator() noexcept;

void
set_current_frame_allocator(
    std::pmr::memory_resource* mr) noexcept;
```

See [P4172R0](https://wg21.link/p4172r0)<sup>[1]</sup> for the timing constraint analysis, execution window, `safe_resume` protocol, and responses to common concerns.

### 4.6 `IoRunnable` and Launch Functions

Within a coroutine chain, _IoAwaitable_ alone is sufficient - `co_await` handles lifetime, result extraction, and exception propagation natively. But launch functions like `run_async` cannot `co_await`. They need access to the promise to manage lifetime and extract results. _IoRunnable_ adds this interface:

```cpp
template<typename T>
concept IoRunnable =
    IoAwaitable<T> &&
    requires { typename T::promise_type; } &&
    requires( T& t, T const& ct,
              typename T::promise_type const& cp,
              typename T::promise_type& p )
    {
        { ct.handle() } noexcept
            -> std::same_as<
                std::coroutine_handle<
                    typename T::promise_type> >;
        { cp.exception() } noexcept
            -> std::same_as< std::exception_ptr >;
        { t.release() } noexcept;
        { p.set_continuation(
            std::coroutine_handle<>{} ) } noexcept;
        { p.set_environment(
            static_cast<io_env const*>(
                nullptr) ) } noexcept;
    } &&
    ( std::is_void_v<
        decltype(
            std::declval<T&>().await_resume()) > ||
      requires( typename T::promise_type& p ) {
          p.result();
      });
```

Launch functions use two-phase invocation to ensure the frame allocator is set before the child coroutine's frame is allocated:

```cpp
// Launch from non-coroutine code
run_async( ex )( my_task() );
run_async( ex, stop_token, alloc )( my_task() );

// Switch executor within a coroutine
co_await run( worker_ex )( compute() );
```

See [P4172R0](https://wg21.link/p4172r0)<sup>[1]</sup> for detailed examples and implementation guidance.

---

## 5. _IoAwaitable_ Is Structured Concurrency

- `co_await` enforces a lexical boundary. The child completes before the parent continues.
- RAII works inside coroutines. Deterministic destruction is guaranteed.
- Cancellation propagates forward. Destruction propagates backward. Both are automatic.
- For byte I/O, the language provides what a library would reimplement.

- `when_all` and `when_any` in [Capy](https://github.com/cppalliance/capy)<sup>[2]</sup> (`include/boost/capy/when_all.hpp`, `when_any.hpp`)

```cpp
auto [ec, counts] = co_await when_all(std::move(reads));

auto result = co_await when_any(std::move(reads));
```

1. The awaitable owns the suspended coroutine handle.
2. The awaitable submits the operation and transfers ownership to the executor.
3. The executor resumes the coroutine, returning ownership to the coroutine body.

There is always an owner: the coroutine body, the awaitable, or the executor.

**IoAwaitable is structured concurrency.**

### 5.1 Structurable Building Blocks Are More Fundamental

When handlers are provided, the launch is structured - both outcomes are
explicitly routed by the caller:

```cpp
run_async( ex,
    [](int result)         { /* use result */   },
    [](std::exception_ptr) { /* handle error */ }
)( compute() );
```

Without handlers, the result is discarded and exceptions rethrow on the
executor thread. This unstructured path is intentional - [P4035R0](https://wg21.link/p4035r0)<sup>[15]</sup>
explains why launch functions cannot `co_await` and therefore need an escape
hatch. The protocol provides structure when the caller uses it; it does not
forbid escape when the caller needs it.

The handlers are the primitive. Higher-level structured concurrency abstractions
are built from them. A `counting_scope` - tracking N concurrent tasks,
joining when all complete, propagating exceptions - is a handler policy:

```cpp
class counting_scope {
    /* atomic state machine: idle -> waiting -> done */

public:
    template<Executor Ex, IoRunnable Task>
    void spawn(Ex ex, Task t) {
        run_async(ex,
            [this](auto&&...) noexcept          { on_done(); },
            [this](std::exception_ptr e) noexcept { on_except(e); }
        )(std::move(t));
    }

    [[nodiscard]] /* awaitable */ join() noexcept;

private:
    void on_done() noexcept;                     // decrements count, resumes waiter on zero
    void on_except(std::exception_ptr) noexcept; // stores first ep, then on_done()
};
```

A full implementation is available in [Capy](https://github.com/cppalliance/capy)<sup>[2]</sup>
(`include/boost/capy/ex/when_all.hpp`).

The protocol provides the building block. `counting_scope` is one policy built
on it. The argument that senders provide structured concurrency and coroutines
do not has it backwards: the _IoAwaitable_ protocol is the layer from which
structured concurrency constructs are assembled.

### 5.2 The Unsafe Interface Is Still Needed

`main()` cannot `co_await`. Every async program has exactly one place where
synchronous code must launch the first coroutine. [P4035R0](https://wg21.link/p4035r0)<sup>[15]</sup>
explains why this escape hatch must exist:

```cpp
int main() {
    io_context ctx;
    run_async( ctx.get_executor() )( server_main() );
    ctx.run();
}
```

### 5.3 `run` Is Always Structured Concurrency

Within a coroutine chain, `run` switches executor, stop token, or allocator
for a subtask. It is always `co_await`ed - the lexical boundary is enforced
by the language:

```cpp
// Switch execution context
co_await run( worker_ex )( compute() );

// Override cancellation
co_await run( source.get_token() )( sensitive_op() );

// Switch allocator for this subtask
co_await run( pool )( alloc_heavy_op() );

// All three
co_await run( worker_ex, source.get_token(), pool )( compute() );
```

`run` cannot be detached. The parent suspends at `co_await` and resumes only
when the child completes. There is no unstructured path through `run`.

---

## 6. Why Not `exec::as_awaitable`?

`std::execution` provides `exec::as_awaitable`, which wraps a sender as an awaitable.
These are the costs coroutines pay based on what I/O returns.

| Property                                       | I/O returns awaitable | I/O returns sender             |
|------------------------------------------------|-----------------------|--------------------------------|
| Frame allocations                              | 1                     | 1                              |
| Per-operation allocation (type-erased stream)  | 0                     | 1                              |
| `await_ready` can return `true`                | Yes                   | No - structurally impossible   |
| Synchronous-complete overhead                  | 0                     | `connect`/`start` overhead     |

Type-erasing either argument to `connect(sndr, rcvr)` requires a heap allocation on every operation.

Routing coroutine byte-oriented I/O through `std::execution` structurally disadvantages coroutines under P2300R10's `connect`/`start` architecture.

The sender composition algebra also does not apply to compound I/O results - such as `[ec, n]` - without data loss or shared state; the sender three-channel model is in tension with `error_code` as a value-channel result.<sup>[30,31]</sup>

See [P4172R0](https://wg21.link/p4172r0)<sup>[1]</sup> for detailed analysis.

[P3482R1](https://wg21.link/p3482r1)<sup>[23]</sup>, a TAPS-based networking proposal, defines
`async_receive()` and `async_send()` as sender-returning operations. Every row in the
table above applies. Capy and Corosio ship on Linux, Windows, and macOS today. The
authors welcome a direct comparison with any equivalent implementation.

---

## 7. Suggested Straw Polls

[P0592R0](https://wg21.link/p0592r0) (2017)<sup>[6]</sup> listed networking as a C++20 priority. [P0592R2](https://wg21.link/p0592r2)<sup>[18]</sup> elevated it to must-work-on-first for C++23. [P0592R5](https://wg21.link/p0592r5)<sup>[19]</sup> dropped it from C++26. Three standard cycles, the same priority, no result.<sup>[28]</sup> This paper provides the exploration that three standard cycles have been waiting for.

SG4 polled at Kona (November 2023) on [P2762R2](https://wg21.link/p2762r2) "Sender/Receiver Interface For Networking"<sup>[20]</sup>:

> *"Networking should support only a sender/receiver model for asynchronous operations; the Networking TS's executor model should be removed"*
>
> | SF | F | N | A | SA |
> |----|---|---|---|----|
> |  5 | 5 | 1 | 0 |  1 |
>
> Consensus.

Source: SG4 minutes, Kona 2023 (November 8), on P2762R2<sup>[21]</sup>.

[P2762R2](https://wg21.link/p2762r2)<sup>[20]</sup> presents two options: the sender/receiver model and the Networking TS executor model. Coroutine-native byte-oriented I/O is neither, and was not among the alternatives considered at the time of that poll. Three history papers in this series document why: the analyses that shaped those decisions examined executor models under a framing that does not apply to coroutine executors, and the coroutine executor concept did not exist when those decisions were made.<sup>[24,25,26]</sup> The committee's prior LEWG consensus (October 2021) that sender/receiver covers networking also predates the coroutine-native model; the published evidence behind that claim is surveyed in [P4097R0](https://wg21.link/p4097r0).<sup>[29]</sup>

**Poll 1.** A coroutine-native byte-oriented I/O model is a distinct approach from both the Networking TS executor model and the sender/receiver model.

**Poll 2.** New research into coroutine-native byte-oriented I/O, not available at the time of the Kona poll, warrants consideration.

**Poll 3.** Frame allocator propagation for coroutine byte-oriented I/O should not require `allocator_arg_t` in every coroutine signature.

**Poll 4.** A standard vocabulary for coroutine byte-oriented I/O should support separate compilation with a single template parameter on the task type.

**Poll 5.** LEWG should advance the _IoAwaitable_ protocol as the standard vocabulary for coroutine-native byte-oriented I/O.

---

## 8. Conclusion

[Capy](https://github.com/cppalliance/capy)<sup>[2]</sup> and [Corosio](https://github.com/cppalliance/corosio)<sup>[3]</sup> ship _IoAwaitable_ on Linux, Windows, and macOS. `std::execution` serves DAG-shaped work; byte I/O coroutines are inherently sequential. The two belong in the standard together. The time is now.

---

## Acknowledgements

**Gor Nishanov** - The C++20 coroutines language feature is the foundation every word
of this paper rests on. Without the coroutines TS, the IoAwaitable protocol has nothing
to build on.

**Christopher Kohlhoff** - The `execution_context`, executor, and service model in
Section 4 derives directly from Boost.Asio. The design choices in this paper are
possible because Kohlhoff built them first and proved them in production.

**Eric Niebler, Bryce Adelstein Lelbach, Micha&lstrok; Dominiak, Lewis Baker, Lee Howes,
Kirk Shoop, Jeff Garland, Georgy Evtushenko, and Lucian Radu Teodorescu** - The authors
of P2300R10. The structured concurrency guarantees, completion channel model, and
scheduler design in `std::execution` defined the async vocabulary this paper must
complement and interoperate with. The bridge papers in the Network Endeavor exist
because P2300 is in the standard.

**Ville Voutilainen** - The P0592 series documented networking as a committee priority
across three standard cycles and established the public record that Section 7 draws on.

**Dietmar K&uuml;hl** - P2762R2 defined the precise scope of the Kona 2023 SG4
ballot, making it possible to demonstrate that coroutine-native byte-oriented I/O was
not among the alternatives considered.

**Michael Wong and SG14** - P4029R0 provided the formal constituency position that
networking should not be built on P2300, cited in Section 2.2.

**Lucian Radu Teodorescu** - Published a detailed public analysis of sender/receiver
complexity that clarified the ergonomic tradeoffs for the wider community, cited in
Section 2.3.

**Lewis Baker** - The symmetric transfer technique, documented in the cited blog post,
is the mechanism that makes the zero-overhead coroutine resumption path in Section 4.2
possible.

---

## References

1. [P4172R0](https://wg21.link/p4172r0) - "Design: IoAwaitable for Coroutine-Native Byte-Oriented I/O" (Vinnie Falco, Steve Gerbino, Mungo Gill, 2026). https://wg21.link/p4172r0 Note: `wg21.link` and open-std.org URLs for this paper are not live until the draft is published to the mailing; use the posted revision when available.
2. [Capy](https://github.com/cppalliance/capy/tree/bf080326659b2a9cc954763da702d15c32eb7085) - _IoAwaitable_ protocol implementation (Vinnie Falco, Steve Gerbino). https://github.com/cppalliance/capy/tree/bf080326659b2a9cc954763da702d15c32eb7085
3. [Corosio](https://github.com/cppalliance/corosio) - Coroutine-native I/O library (Vinnie Falco, Steve Gerbino). https://github.com/cppalliance/corosio
4. [P2300R10](https://wg21.link/p2300r10) - "`std::execution`" (Dominiak, Baker, Evtushenko, Teodorescu, Howes, Shoop, Garland, Niebler, Lelbach, 2024). https://wg21.link/p2300r10
5. Bjarne Stroustrup, WG21 reflector mailing list (2019). Quoted with permission. Not independently verifiable online.
6. [P0592R0](https://wg21.link/p0592r0) - "To boldly suggest an overall plan for C++20" (Ville Voutilainen, 2017). https://wg21.link/p0592r0
7. [P1446R0](https://wg21.link/p1446r0) - "Reconsider the Networking TS for inclusion in C++20" (Christopher Kohlhoff, 2019). https://wg21.link/p1446r0
8. [P4029R0](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4029r0.pdf) - "The SG14 Priority List for C++29/32" (Michael Wong, SG14, 2026). https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4029r0.pdf
9. [Boost.Beast](https://github.com/boostorg/beast) - HTTP and WebSocket library (Vinnie Falco). https://github.com/boostorg/beast
10. [Understanding Symmetric Transfer](https://lewissbaker.github.io/2020/05/11/understanding_symmetric_transfer) - (Lewis Baker, 2020). https://lewissbaker.github.io/2020/05/11/understanding_symmetric_transfer
11. [Senders/receivers in C++](https://lucteo.ro/2024/08/12/senders-receivers-in-cxx/) - (Lucian Radu Teodorescu, 2024). https://lucteo.ro/2024/08/12/senders-receivers-in-cxx/
12. [C++20 Coroutines: sketching a minimal async framework](https://www.jeremyong.com/cpp/2021/01/04/cpp20-coroutines-a-minimal-async-framework/) - (Jeremy Ong, 2021). https://www.jeremyong.com/cpp/2021/01/04/cpp20-coroutines-a-minimal-async-framework/
13. [Composing C++20 Asio coroutines](https://xc-jp.github.io/blog-posts/2022/03/03/Asio-Coroutines.html) - TCP stream I/O with Boost.Asio (xc-jp, 2022). https://xc-jp.github.io/blog-posts/2022/03/03/Asio-Coroutines.html
14. [How to Use C++20 Coroutines for Networking](https://jamespascoe.github.io/accu2022/) - ACCU 2022 (James Pascoe, 2022). https://jamespascoe.github.io/accu2022/
15. [P4035R0](https://wg21.link/p4035r0) - "Support: The Need for Escape Hatches" (Vinnie Falco, 2026). https://wg21.link/p4035r0
16. [Optimizing Away C++ Virtual Functions May Be Pointless](https://www.youtube.com/watch?v=i5MAXAxp_Tw) - CppCon 2023 (Shachar Shemesh). https://www.youtube.com/watch?v=i5MAXAxp_Tw
17. [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html) - Asynchronous I/O library (Chris Kohlhoff). https://www.boost.org/doc/libs/release/doc/html/boost_asio.html
18. [P0592R2](https://wg21.link/p0592r2) - "To boldly suggest an overall plan for C++23" (Ville Voutilainen, 2019). https://wg21.link/p0592r2
19. [P0592R5](https://wg21.link/p0592r5) - "To boldly suggest an overall plan for C++26" (Ville Voutilainen, 2022). https://wg21.link/p0592r5
20. [P2762R2](https://wg21.link/p2762r2) - "Sender/Receiver Interface For Networking" (Dietmar K&uuml;hl, 2023). https://wg21.link/p2762r2
21. [SG4 minutes, Kona 2023](https://wiki.isocpp.org/2023-11_Kona:SG4) - SG4, November 8 2023, on P2762R2. https://wiki.isocpp.org/2023-11_Kona:SG4
22. [Compiler Explorer](https://godbolt.org/z/Wzrb7McrT) - Self-contained _IoAwaitable_ demonstration. https://godbolt.org/z/Wzrb7McrT
23. [P3482R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3482r1.html) - "Design for C++ networking based on IETF TAPS" (Thomas Rodgers, Dietmar K&uuml;hl, 2024). https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3482r1.html
24. [P4094R0](https://wg21.link/p4094r0) - "History: The Unification of Executors and P0443" (Vinnie Falco, 2026). https://wg21.link/p4094r0
25. [P4095R0](https://wg21.link/p4095r0) - "History: The Basis Operation and P1525" (Vinnie Falco, 2026). https://wg21.link/p4095r0
26. [P4096R0](https://wg21.link/p4096r0) - "History: Coroutine Executors and P2464R0" (Vinnie Falco, 2026). https://wg21.link/p4096r0
27. [P4125R1](https://wg21.link/p4125r1) - "Report: Coroutine-Native I/O at a Derivatives Exchange" (Mungo Gill, 2026). https://wg21.link/p4125r1
28. [P4099R0](https://wg21.link/p4099r0) - "History: The Twenty-One Year Networking Arc" (Vinnie Falco, 2026). https://wg21.link/p4099r0
29. [P4097R0](https://wg21.link/p4097r0) - "History: The Networking Claim and P2453R0" (Vinnie Falco, 2026). https://wg21.link/p4097r0
30. [P4090R0](https://wg21.link/p4090r0) - "Info: Sender I/O: A Constructed Comparison" (Vinnie Falco, Steve Gerbino, 2026). https://wg21.link/p4090r0
31. [P4091R0](https://wg21.link/p4091r0) - "Info: Error Models of Regular C++ and the Sender Sub-Language" (Vinnie Falco, 2026). https://wg21.link/p4091r0
32. [P4100R0](https://wg21.link/p4100r0) - "The Network Endeavor: Coroutine-Native I/O for C++29" (Vinnie Falco et al., 2026). https://wg21.link/p4100r0
