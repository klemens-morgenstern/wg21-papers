---
title: "What C++20 Coroutines Already Buy The Standard"
document: P4088R0
date: 2026-03-29
reply-to:
  - "Vinnie Falco <vinnie.falco@gmail.com>"
  - "C++ Alliance Proposal Team"
audience: LEWG
---

## Abstract

C++ already got an asynchronous model: regular C++20 coroutines.

The programmer writes `for`, `if`, structured bindings, and `co_await` - the idiom they already know, made asynchronous by three keywords. Underneath, five language mechanisms combine to produce type-erased streams, separate compilation, and ABI stability - properties the committee could not ship in twenty-one years of networking attempts. This paper traces the causal chain from mechanism to library and states the price.

---

## Revision History

### R0: March 2026 (post-Croydon mailing)

- Initial version.

---

## 1. Disclosure

The author provides information and serves at the pleasure of the chair.

This paper is part of the Network Endeavor ([P4100R0](https://wg21.link/p4100r0)<sup>[18]</sup>), a thirteen-paper project to bring networking to C++29 using a coroutine-native approach. The author developed and maintains [Corosio](https://github.com/cppalliance/corosio)<sup>[6]</sup> and [Capy](https://github.com/cppalliance/capy)<sup>[7]</sup> and believes coroutine-native I/O is the correct foundation for networking in C++.

The author regards `std::execution` as an important contribution to C++ and supports its standardization for the domains it serves well - GPU dispatch, heterogeneous execution, and compile-time work-graph composition among them. The author's position is narrower: that networking and stream I/O are better served by a coroutine-native facility that can coexist with senders and interoperate where the domains meet.

The author asks for nothing.

---

## 2. Two Models Already Ship

### 2.1 Three Keywords

Coroutine I/O is not a new programming model. It is `for`, `if`, `while`, `break`, `return`, structured bindings - the language the programmer already writes. Three keywords are new: `co_await`, `co_return`, `co_yield`.

The coroutine "tutorial" is: write regular code, put `co_await` before async operations.

### 2.2 Thirty Algorithms

The sender model replaces these constructs with library equivalents: `let_value` for local variables, `then` for function calls, `upon_error` for `catch`, `when_all` for sequential statements, `repeat_effect_until` for `for`. [P4014R1](https://wg21.link/p4014r1)<sup>[21]</sup> "The Sender Sub-Language" is a progressive tutorial of all thirty sender algorithms in C++26. The `retry` algorithm requires approximately 120 lines of template machinery as a sender; the coroutine equivalent is seven lines. The four-layer composition example in [P4014R1](https://wg21.link/p4014r1)<sup>[21]</sup> Section 12 collapses from interleaved sender pipelines into a single coroutine.

| Model      | New vocabulary                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| ---------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Coroutines | `co_await`, `co_return`, `co_yield`                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Senders    | `just`, `just_error`, `just_stopped`, `sync_wait`, `then`, `upon_error`, `upon_stopped`, `let_value`, `let_error`, `let_stopped`, `schedule`, `starts_on`, `continues_on`, `on`, `affine_on`, `schedule_from`, `read_env`, `write_env`, `unstoppable`, `when_all`, `when_all_with_variant`, `split`, `into_variant`, `stopped_as_optional`, `stopped_as_error`, `bulk`, `bulk_chunked`, `bulk_unchunked`, `associate`, `spawn_future` |

### 2.3 The Standard Ships Both

[P3552R3](https://wg21.link/p3552r3)<sup>[8]</sup> "Add a Coroutine Task Type" Section 9.4.1 [task.overview] defines the result:

> "The `task` class template represents a sender that can be used as the return type of coroutines."

One type. Two models. `std::execution::task` is a coroutine that is also a sender.

On September 28, 2020, LEWG polled: "We must have a single async model for the C++ Standard Library." No consensus ([P2453R0](https://wg21.link/p2453r0)<sup>[9]</sup> "2021 October Library Evolution Poll Outcomes", Poll 1 comment). On September 28, 2021, the Executors telecon polled again ([P2453R0](https://wg21.link/p2453r0)<sup>[9]</sup>):

> "We believe we need one grand unified model for asynchronous execution in the C++ Standard Library, that covers structured concurrency, event based programming, active patterns, etc."
>
> SF:4 / WF:9 / N:5 / WA:5 / SA:1 - No consensus (leaning in favor).

The "one model" premise was polled twice. It achieved consensus neither time. `task` ships two models.

### 2.4 The Chronology

C++20 was ratified in 2020 with coroutines as a language feature. Every major compiler implements them. Production codebases have used them for six years. `std::execution` was adopted into the working draft at St. Louis in June 2024<sup>[19]</sup> and ships in C++26. Networking is not in the C++ standard. Twenty-one years from [N1925](https://wg21.link/n1925)<sup>[15]</sup>.

### 2.5 The Incumbent

Coroutines are the language's computation model for async. `std::execution` is the library's. Both are in the standard. Both must succeed. The question is what each one is for.

If the concern is ecosystem fragmentation from two async models, the fragmentation was introduced when `std::execution` was adopted alongside coroutines. Coroutine-native I/O does not introduce a second model. It completes the first. C++20 gave the committee `co_await`, `coroutine_handle<>`, and `promise_type`. It did not give the committee standard I/O operations that use them.

The price of two models is paid. The question is whether to get value from both. This paper documents what happens when you build I/O on the model the language already provides.

---

## 3. What Senders Buy

The sender/receiver model is an achievement.

Eric Niebler described the philosophical foundation in 2020<sup>[10]</sup>:

> "Structured concurrency brings the Modern C++ style to our async programs by making async lifetimes correspond to ordinary C++ lexical scopes, eliminating the need for reference counting to manage object lifetime."

Child operations complete before their parents. Lexical scopes govern async lifetimes. The same discipline that makes synchronous C++ safe - RAII, deterministic destruction, nested scopes - extends to asynchronous code. This is the sender model's deepest conviction.

The practical motivation is equally clear. Niebler wrote in 2024<sup>[11]</sup>:

> "Every library that exposes asynchrony uses a slightly different callback API. If you want to chain two async operations from two different libraries, you're going to need to write a bunch of glue code to map this async abstraction to that async abstraction. It's the Tower of Babel problem."

One standard async abstraction solves the interoperability problem. Senders provide the common vocabulary.

Senders and coroutines are not either/or. Niebler framed this directly<sup>[11]</sup>:

> "Returning a sender is a great choice: your users can await the sender in a coroutine if they like, or they can avoid the coroutine frame allocation and use the sender with a generic algorithm like `then()` or `when_all()`. The lack of allocations makes senders an especially good choice for embedded developers."

The sender model provides zero-allocation pipelines through a specific design choice: `connect(sender, receiver)` produces an operation state that aggregates all data before `start()` is called. Niebler described the consequence<sup>[11]</sup>:

> "We can launch lots of async work with complex dependencies with only a single dynamic allocation or, in some cases, no allocations at all."

Separating construction from launch lets the pipeline aggregate all state before any work starts. The optimizer sees the full pipeline. This is possible because the operation state is parameterized on the receiver type - the compiler knows the complete type at every stage.

Senders also provide completion signatures as type-level contracts. The sender declares how it can complete. A type mismatch between pipeline stages is a compile error. The three-channel model - `set_value`, `set_error`, `set_stopped` - routes results by channel, and generic algorithms like `retry`, `when_all`, and `upon_error` dispatch on the channel without knowing the concrete sender type.

These are deployed at scale. [P2470R0](https://wg21.link/p2470r0)<sup>[12]</sup> "Slides for presentation of P2300R2" documented the deployments: Facebook ("monthly users number in the billions"), NVIDIA ("fully invested in P2300... we plan to ship in production"), and Bloomberg (experimentation). GPU dispatch, infrastructure, HPC - the domains where compile-time work graphs, zero-allocation pipelines, and heterogeneous composition deliver their full value.

---

## 4. One Frame

Coroutines are not free. Three costs are irreducible.

**Frame allocation.** When a function becomes a coroutine, the compiler moves everything that would normally live on the stack - every local variable, every function parameter, the suspension point that records where execution left off, and the awaitable machinery that manages resumption - into a heap-allocated block called the coroutine frame. Every coroutine that suspends must allocate this frame through `operator new`. The frame size is determined by the compiler. The caller cannot `sizeof` it, cannot stack-allocate it, cannot embed it in a struct. HALO ([P0981R0](https://wg21.link/p0981r0)<sup>[13]</sup> "Halo: coroutine Heap Allocation eLision Optimization") can elide the allocation when the compiler proves the frame's lifetime is bounded by the caller's scope, but no compiler guarantees HALO. The recycling allocator ([recycling_memory_resource](https://github.com/cppalliance/capy/blob/p4088r0/include/boost/capy/ex/recycling_memory_resource.hpp)<sup>[7]</sup>) amortizes the cost to a thread-local pool lookup - nanoseconds instead of microseconds - but the allocation still happens. Senders do not pay this cost. Sender operation states can be stack-allocated or embedded in the parent's operation state.

**Opaque resume.** The compiler cannot see through `std::coroutine_handle<>::resume()`. Every suspension point is an optimization barrier. The optimizer cannot inline across it. In tight inner loops this is measurable. This is the fundamental cost of type erasure through the handle. Senders do not pay this cost. Sender operation states are fully visible to the optimizer within a pipeline.

**Reference lifetime hazard.** Coroutine parameters are copied into the frame at the call site, but references are copied as references, not as values. A `const std::string&` parameter stores the reference in the frame. If the caller's string goes out of scope before the first suspension point, the reference dangles. Lambda captures by reference have the same hazard. This is a correctness cost, not a performance cost. Google built `Co<T>` as immovable and prvalue-only specifically to prevent it ([P3801R0](https://wg21.link/p3801r0)<sup>[14]</sup> "Concerns about the design of `std::execution::task`"). Senders do not share this hazard in the same way - the operation state owns copies of everything passed through `connect`.

At the baseline, the price does not make coroutines slower. A [benchmark](https://github.com/sgerbino/capy/tree/pr/beman-bench/bench/beman)<sup>[20]</sup> of 100,000,000 `read_some` calls on concrete streams measures both models at ~30-31 ns/op with zero allocations. The frame allocation is a cost that buys something. It is not a cost that slows you down.

You pay the price once. A coroutine that does fifty reads pays one frame allocation and fifty zero-cost resumptions. The frame that you cannot avoid is the same frame that holds the operation state, the local variables, and the result. The type erasure that blocks inlining is the same type erasure that gives you `any_stream`, `task<T>` with one parameter, and non-template operation states.

The frame you cannot avoid is the frame that pays for everything in Section 7.

---

## 5. Twenty Years, Nine Lines

The committee has been trying to standardize networking since [N1925](https://wg21.link/n1925)<sup>[15]</sup> (2005). The contract that every attempt has been built on comes from Asio.

### 5.1 Asio's `AsyncReadStream`

The [Boost.Asio documentation](https://www.boost.org/doc/libs/1_87_0/doc/html/boost_asio/reference/AsyncReadStream.html)<sup>[16]</sup> defines `AsyncReadStream` as a named requirement with two operations:

| operation                  | type                                       | semantics                                            |
| -------------------------- | ------------------------------------------ | ---------------------------------------------------- |
| `a.get_executor()`         | satisfying Executor requirements           | Returns the associated I/O executor                  |
| `a.async_read_some(mb, t)` | determined by async operation requirements | completion signature `void(error_code ec, size_t n)` |

Two operations. Twenty years. The contract has not changed. The Networking TS formalized it. The committee could not ship it. But the contract survived because it is correct.

### 5.2 The C++20 Concept

[ReadStream](https://github.com/cppalliance/capy/blob/p4088r0/include/boost/capy/concept/read_stream.hpp)<sup>[7]</sup> formalizes the same contract as a C++20 concept:

```cpp
template<typename T>
concept ReadStream =
    requires(T& stream,
             mutable_buffer_archetype buffers)
    {
        { stream.read_some(buffers) }
            -> IoAwaitable;
        requires awaitable_decomposes_to<
            decltype(stream.read_some(buffers)),
            std::error_code, std::size_t>;
    };
```

`read_some` takes a buffer. The result satisfies `IoAwaitable`. The result decomposes to `(error_code, size_t)` via structured bindings.

Nine lines. Twenty years of contract, nine lines of concept.

The semantic requirements match Asio's: if `buffer_size(buffers) > 0` and `!ec`, then `n >= 1 && n <= buffer_size(buffers)` - at least one byte was read. If `ec`, then `n` is the number of bytes read before the I/O condition arose. I/O conditions are reported via the `error_code` component. Library failures (such as allocation failure) are reported via exceptions. The caller must ensure the buffer memory remains valid until the `co_await` expression returns.

### 5.3 What Changed

Two things vanished.

The completion token disappeared. In Asio, `async_read_some(mb, t)` takes a completion token `t` that determines the async model - callback, future, coroutine, or use_awaitable. In the coroutine model, the coroutine *is* the completion mechanism. There is no token. There is no choice. The coroutine suspends and resumes. The simplification is the trade-off.

`get_executor()` disappeared entirely. I/O objects do not carry executors. The caller provides the executor through `io_env` at `await_suspend` time. This resolves a long-standing Asio confusion where the I/O object has an executor and the completion token has a different executor, and the user must understand which one governs resumption. In the coroutine model there is one executor, it comes from the caller, and the I/O object never sees it.

What stayed: `read_some` takes a buffer, returns `(error_code, size_t)`. The named requirement became a concept. The language caught up to the contract.

---

## 6. Why Did It Take Twenty Years?

Networking stalled because networking is not the hard problem. Asynchrony is the hard problem, and asynchrony has three domains: bulk-parallel execution, heterogeneous work-graph composition, and serial stream I/O. C++ already standardized the first two. The parallel algorithms in C++17 serve the bulk-parallel domain through execution policies. `std::execution` in C++26 serves the work-graph and heterogeneous domain through sender/receiver composition. The serial I/O domain - networking, files, pipes, TLS - has no standard facility. [P0443](https://wg21.link/p0443r14)<sup>[23]</sup> spent fourteen revisions from 2016 to 2020 trying to unify all three domains into a single executor model, reconciling a work-executor suited to bulk dispatch with a continuation-executor suited to I/O chaining (the retrospective series [P4094R0](https://isocpp.org/files/papers/P4094R0.pdf)<sup>[24]</sup> through [P4099R0](https://isocpp.org/files/papers/P4099R0.pdf)<sup>[25]</sup> documents the full history). The unification imperative - the unexamined assumption that one model must cover all three domains - was the obstacle. The answer was three models, each serving its domain.

The domains that attracted institutional investment shipped. Bulk-parallel execution had compiler vendors. Heterogeneous dispatch had GPU manufacturers. Networking had one independent author without institutional backing. This is how volunteer-based standardization works when some volunteers are funded and some are not. It is a structural observation, not a defect.

C++20 coroutines did not exist when the networking debate was at its peak. The Networking TS was designed for a language without `co_await`. By the time compilers shipped coroutines and production codebases validated them, the committee had committed to `std::execution` as the universal async model. The window where coroutine-native I/O was both technically possible and the parallel domain was already resolved did not open until 2025. The coroutine-native approach could not have been proposed earlier because its prerequisites had not converged.

Asio supports every async model - callbacks, futures, coroutines, and completion tokens. That universality served users but created a standardization obstacle: the I/O operations must be templates, the operation states must be parameterized on the completion handler type, and the stream model cannot be type-erased without per-operation allocation. The coroutine-native approach succeeds by committing to one completion mechanism and giving up the rest. Nobody could demonstrate what that commitment produces until someone made it. Section 7 is that demonstration.

The committee has heard networking proposals before. It would be reasonable to ask why this attempt should succeed where others have not. The answer is structural. `std::execution` shipping in C++26 resolves the parallel and heterogeneous domain, which means the serial I/O domain can now be addressed without reopening the parallel question. The Network Endeavor ([P4100R0](https://wg21.link/p4100r0)<sup>[18]</sup>) is backed by a non-profit with engineering resources - not a solo effort by an independent author. Two libraries ship today ([Capy](https://github.com/cppalliance/capy)<sup>[7]</sup> and [Corosio](https://github.com/cppalliance/corosio)<sup>[6]</sup>) with a Boost review scheduled. The team has implementation experience across the stream model - the author built Boost.Beast on Asio's architecture. Thirteen papers document the technical case, the retrospective record, and the bridge to `std::execution`. If coroutine I/O ships, C++ arrives at three async models - each serving its domain. P0443 tried to unify three into one. The answer, it turns out, was three.

---

## 7. What The Frame Buys

Asio made a different set of trade-offs. It supports callbacks, futures, coroutines, and completion tokens. That flexibility serves every async model a user might want. But the flexibility has a cost: the I/O operations must be templates, the operation states must be parameterized on the completion handler type, and the stream model cannot be type-erased without per-operation allocation. These are not defects. They are consequences of supporting every completion model simultaneously.

When you commit to coroutines as the only completion mechanism, you give up that flexibility. You cannot use callbacks. You cannot use futures. You cannot use completion tokens. That is a real cost. But the commitment unlocks optimizations and ergonomics that no other trade-off can deliver. The operation state becomes concrete. The stream becomes type-erasable. The library becomes separately compilable. The API becomes ABI-stable. None of these are possible when the completion mechanism is a template parameter.

The frame allocation is the cost every design avoids. This design accepts it. The frame the compiler already built is a type-erased storage location that the awaitable reuses at zero per-operation cost. A sender-based design can achieve the same downstream properties - type-erased operation states, pre-allocated storage, ABI-stable vtables - but pays a per-operation allocation because `connect(sender, receiver)` must heap-allocate the type-erased operation state each time the concrete type is unknown at compile time. The coroutine frame provides for free what the sender model must purchase per operation. Each consequence in the chain that follows is possible only because the previous one holds. Remove any link and the rest collapse. The chain is not a list of independent benefits. It is a single argument, nine steps long, and it begins with the frame.

### 7.1 The Caller Is Erased

<table>
<tr><th>Awaitable</th><th>Sender</th></tr>
<tr>
<td><pre><code>struct read_awaitable
{
    bool await_ready();
    void await_suspend(
        std::coroutine_handle&lt;&gt; h);
        // caller erased
    io_result&lt;size_t&gt;
        await_resume();
};</code></pre></td>
<td><pre><code>template&lt;class Receiver&gt;
struct read_operation
{
    Receiver rcvr_;
        // caller stamped in
    void start() noexcept;
};</code></pre></td>
</tr>
</table>

Same operation. Left: the caller is erased behind `coroutine_handle<>`. Right: the caller's type is stamped into the operation state as `Receiver`. The `<>` is the fork in the road. Everything in 7.2 through 7.9 follows from this difference.

`std::function` erases a callable. `coroutine_handle<>` erases a resumable. One is a library convention. The other is a language primitive. Both hide the concrete type behind a fixed interface. The difference is that `std::function` must allocate its own storage. `coroutine_handle<>` points to a frame the compiler already built.

When a coroutine `co_await`s an awaitable, the awaitable's `await_suspend` receives `std::coroutine_handle<>`. That handle is type-erased. The awaitable does not know what the caller's return type is, what the caller's promise type is, what local variables the caller holds, or where the caller will resume. The caller's entire identity - its variables, its suspension point, its future - is behind an opaque pointer. The awaitable sees one thing: a handle it can resume or destroy.

Any function that returns an awaitable gets structural type erasure of the caller for free. The I/O operation's state does not depend on who is waiting for it. A `read_op` struct is the same whether the caller is `task<int>`, `task<void>`, or any other coroutine type. The caller is erased. The operation state is concrete.

The cost is real. Because the handle is opaque, the compiler cannot optimize through `resume()`. It cannot inline the caller into the awaitable or the awaitable into the caller. The optimization barrier from Section 4 is the same mechanism that provides the type erasure. You do not get one without the other.

In the sender model, `connect(sender, receiver)` stamps the receiver's type into the operation state. The optimizer sees the full pipeline - it can inline across operation boundaries, eliminate dead code, and propagate constants through the entire chain. That is the strength for GPU pipelines (Section 3). The cost of that visibility is that the I/O operation's state depends on who is waiting for it. The same `async_read` connected to two different receivers produces two different operation state types.

Two models. One trades optimization for erasure. The other trades erasure for optimization. The sender model's causal chain diverges here. The receiver-parameterized operation state gives the optimizer full pipeline visibility - the strength documented in Section 3. The chain that follows in 7.2 through 7.9 requires a concrete operation state. The sender model can achieve this by type-erasing the receiver, but each `connect` call must heap-allocate the operation state because the concrete type is unknown at compile time. The coroutine model achieves it at zero per-operation cost - the frame the compiler already built is the storage the awaitable reuses. One unnecessary allocation per operation is one unnecessary allocation per operation, regardless of what else is in the I/O path.

From here forward, the paper walks the coroutine chain at zero per-operation cost. The sender model can follow the same chain at one allocation per operation. Section 8 documents the price.

### 7.2 The Operation State Is Concrete

Windows (IOCP). [overlapped_op](https://github.com/cppalliance/corosio/blob/p4088r0/include/boost/corosio/native/detail/iocp/win_overlapped_op.hpp)<sup>[6]</sup> and [read_op](https://github.com/cppalliance/corosio/blob/p4088r0/include/boost/corosio/native/detail/iocp/win_socket.hpp)<sup>[6]</sup>:

```cpp
struct overlapped_op : OVERLAPPED
{
    std::coroutine_handle<> h;
    capy::executor_ref      ex;
    std::error_code*        ec_out;
    std::size_t*            bytes_out;
    DWORD                   bytes_transferred;
};

struct read_op : overlapped_op
{
    WSABUF wsabufs[16];
    DWORD  wsabuf_count;
    DWORD  flags;
    win_socket_internal& internal;
};
```

Linux (epoll). [epoll_op.hpp](https://github.com/cppalliance/corosio/blob/p4088r0/include/boost/corosio/native/detail/epoll/epoll_op.hpp)<sup>[6]</sup>:

```cpp
struct epoll_read_op final
    : reactor_read_op<epoll_op> {};

struct epoll_write_op final
    : reactor_write_op<
        epoll_op, epoll_write_policy> {};
```

Not templates. Not parameterized on the caller. Known at library-build time. The same `read_op` serves every coroutine that reads from the socket, regardless of the caller's type.

### 7.3 The Operation State Lives in the Socket

[win_socket_internal](https://github.com/cppalliance/corosio/blob/p4088r0/include/boost/corosio/native/detail/iocp/win_socket.hpp)<sup>[6]</sup>:

```cpp
class win_socket_internal
{
    connect_op conn_;
    read_op    rd_;
    write_op   wr_;
    SOCKET     socket_ = INVALID_SOCKET;
    int        family_  = AF_UNSPEC;
};
```

Three operation states. Members of the socket. Pre-allocated when the socket is created. Not allocated per-operation. 10,000 sockets means 10,000 `read_op` instances of one known type, allocated once, reused for every read.

### 7.4 Zero Per-Operation Allocation

[any_read_stream](https://github.com/cppalliance/capy/blob/p4088r0/include/boost/capy/io/any_read_stream.hpp)<sup>[7]</sup> type-erases any `ReadStream`. The erasure is on the awaitable, not the stream. The [vtable](https://github.com/cppalliance/capy/blob/p4088r0/include/boost/capy/io/any_read_stream.hpp)<sup>[7]</sup> dispatches `await_ready`, `await_suspend`, `await_resume` through function pointers:

```cpp
struct vtable
{
    void (*construct_awaitable)(
        void*, void*,
        std::span<mutable_buffer const>);
    bool (*await_ready)(void*);
    std::coroutine_handle<> (*await_suspend)(
        void*, std::coroutine_handle<>,
        io_env const*);
    io_result<std::size_t> (*await_resume)(void*);
    void (*destroy_awaitable)(void*) noexcept;
    std::size_t awaitable_size;
    std::size_t awaitable_align;
    void (*destroy)(void*) noexcept;
};
```

The awaitable storage is pre-allocated at construction time and reused for every `read_some` call. `read_some` constructs the real awaitable into cached storage, dispatches through the vtable, and destroys it on resume. One allocation at construction. Zero per-operation.

The [benchmark](https://github.com/sgerbino/capy/tree/pr/beman-bench/bench/beman)<sup>[20]</sup> measures the cost. 100,000,000 `read_some` calls on a single thread, no-op stream, five runs per configuration:

| Stream type  | capy IoAwaitable    |       | P2300 sender        |       |
| ------------ | ------------------: | ----: | ------------------: | ----: |
|              | ns/op               | al/op | ns/op               | al/op |
| Native       | 31.4                | 0     | 30.0                | 0     |
| Abstract     | 32.1                | 0     | 53.5                | 1     |
| Type-erased  | 36.4                | 0     | 53.4                | 1     |

At the native level, both models are equivalent. Under type erasure, awaitables add +5 ns and zero allocations. Senders add +23 ns and one allocation per operation. The cost is structural: `await_suspend` takes a type-erased `coroutine_handle<>`, so the awaitable's size is known at construction time. `connect(receiver)` produces an operation state whose type depends on both the sender and the receiver. When either side is type-erased, the operation state must be heap-allocated per operation.

### 7.5 Compile Once

Because `any_read_stream` has a fixed layout, a function accepting `any_read_stream&` goes in a `.cpp` file:

```cpp
// dump.hpp
#include <boost/capy/task.hpp>
#include <boost/capy/io/any_read_stream.hpp>

capy::task<> dump(capy::any_read_stream& in);
```

```cpp
// dump.cpp
#include "dump.hpp"
#include <boost/capy/buffers.hpp>
#include <iostream>

capy::task<> dump(capy::any_read_stream& in)
{
    char buf[1024];
    for(;;)
    {
        auto [ec, n] = co_await in.read_some(
            capy::mutable_buffer(buf, sizeof buf));
        if(ec)
            break;
        std::cout.write(buf, n);
    }
}
```

The header includes only [Capy](https://github.com/cppalliance/capy)<sup>[7]</sup>. No platform headers. No [Corosio](https://github.com/cppalliance/corosio)<sup>[6]</sup>. No sockets. The `.cpp` compiles once. Consumers include the header and link. The stream behind `any_read_stream` could be a TCP socket, a TLS session, a file, or a test mock. Nothing recompiles.

This is also the [Capy](https://github.com/cppalliance/capy)<sup>[7]</sup>/[Corosio](https://github.com/cppalliance/corosio)<sup>[6]</sup> split point. Capy delivers the abstract layer: `task<T>`, `any_read_stream`, `any_write_stream`, `any_stream`, buffer concepts, stream concepts, `when_all`, `when_any`, the frame allocator. Pure C++20. No platform dependency. Corosio delivers the platform layer: `tcp_socket`, `tls_stream`, timers, DNS, signals. Capy delivers value on its own - sans-I/O protocols, buffered streams, test mocks, all compile against Capy without Corosio. The architecture that made separate compilation possible is the same architecture that made the library split possible. One frame. Two libraries. Zero recompilation.

### 7.6 Both Modes, One Abstraction

`co_await` checks `await_ready()` first. If the awaitable returns `true`, no suspension happens - the coroutine continues synchronously. A memory buffer, a test mock, a zlib decompressor, a base64 decoder - they all satisfy `ReadStream` by returning immediately-ready awaitables. The same `dump` function from Section 7.5 works whether the stream suspends for kernel I/O or returns instantly from a buffer. The algorithm does not know the difference.

A pipeline of `tcp_socket` -> `tls_stream` -> `decompression_stream` -> HTTP parser works regardless of which layers suspend. The synchronous layers pay zero suspension cost. No `is_async` flag. No separate sync API. One abstraction.

### 7.7 Forty Years of ABI

The vtable layout of `any_read_stream` does not change. Libraries compiled today work with new transports tomorrow. A `tls_stream` implementation compiled against OpenSSL 3.0 satisfies `ReadStream`. A future implementation compiled against a post-quantum TLS library will satisfy the same concept, plug into the same `any_read_stream`, and work with every library that was compiled against the old transport. No recompilation. No relinking. The forty-year-old contract - `read_some` takes a buffer, returns `(error_code, size_t)` - is the ABI.

ABI commitments are permanent. `std::regex` is slow because its ABI cannot change. The C++11 `std::string` ABI break was traumatic. The question is whether this vtable will need to change.

The contract behind it:

```
read_some(buffer) -> (error_code, size_t)
```

This is the POSIX `read()` contract with a C++ interface. Five change vectors exist. None applies.

- **New buffer types.** The `ReadStream` concept is already generic over buffer type. New buffer representations satisfy the existing concept. The vtable is unaffected.
- **New error conditions.** `std::error_code` is extensible via error categories. QUIC errors, post-quantum TLS errors, io_uring-specific errors - each registers a new category. The return type does not change.
- **New return values.** A read operation takes a buffer and reports how many bytes were transferred and whether an error occurred. No I/O model in any language returns anything else.
- **Cancellation.** Stop tokens propagate through `io_env`, not through the return type. Cancellation does not touch the contract.
- **New I/O patterns.** `read_some` is the primitive. Scatter/gather, vectored I/O, and read-until are composed from `read_some` by algorithms. The primitive does not change because the compositions do.

The contract has survived POSIX, BSD sockets, IOCP, Asio, the Networking TS, io_uring, and every transport from TCP to QUIC.

### 7.8 The User Chooses

The user chooses the trade-off. [io_stream](https://github.com/cppalliance/corosio/blob/p4088r0/include/boost/corosio/io/io_stream.hpp)<sup>[6]</sup>, [tcp_socket](https://github.com/cppalliance/corosio/blob/p4088r0/include/boost/corosio/tcp_socket.hpp)<sup>[6]</sup>, [native_tcp_socket](https://github.com/cppalliance/corosio/blob/p4088r0/include/boost/corosio/native/native_tcp_socket.hpp)<sup>[6]</sup>:

```
io_stream                        // abstract (Layer 3)
    |
tcp_socket                       // concrete (Layer 2)
    |
native_tcp_socket<Backend>       // native   (Layer 1)
```

**Abstract** (`io_stream`): virtual dispatch, ABI-stable, separately compiled. The compilation firewall. Business logic accepts `any_stream&` and never sees a platform header. Maximum compilation speed. Maximum insulation. The cost is virtual dispatch per I/O operation - nanoseconds against a microsecond syscall.

**Concrete** (`tcp_socket`): protocol-specific API - bind, connect, shutdown - still virtual dispatch, still separately compiled. Application code lives here. The full socket API without platform headers in the caller's translation unit.

**Native** (`native_tcp_socket<Backend>`): templated on the platform backend. Member function shadowing eliminates the vtable. Full inlining. Zero overhead. The cost is that the platform backend header is included and the code is in a header. Hot paths and benchmarks live here.

The user does not choose once for the whole application. Different layers coexist. A library accepts `any_stream&` (abstract). An application creates `tcp_socket` (concrete). A benchmark uses `native_tcp_socket<epoll>` (native). All three interoperate through the inheritance chain. The user gets the best of everything - compilation firewall where they want insulation, zero overhead where they want performance.

### 7.9 The Frame Subsidizes Everything

The coroutine frame paid for in Section 4 holds the local variables, the suspension point, and the result. `any_read_stream` works without per-operation allocation because the caller's frame already exists. The frame allocation you cannot avoid subsidizes the type erasure you want. This is the payoff for the price.

Per-operation allocations by execution model and stream type:

| Stream type  | `capy::task` | `beman::task` | sender pipeline |
| ------------ | -----------: | ------------: | --------------: |
| Native       |            0 |             0 |               0 |
| Abstract     |            0 |             1 |               1 |
| Type-erased  |            0 |             1 |               1 |

The frame you already paid for is the reason the other two rows are zero.

Additional properties that ride on the same frame:

- **Compile-time domain gate.** The two-argument `await_suspend(coroutine_handle<>, io_env const*)` is a deliberate trade-off. The alternative was a single-argument `await_suspend` that extracts the environment from the promise, costing one fewer parameter. The two-argument form was chosen because it buys compile-time enforcement: any awaitable that does not accept `io_env const*` is a type error inside an I/O task. Foreign awaitables that do not speak the I/O protocol are rejected by the compiler, not by a runtime mismatch. The pointer is the cost. The domain gate is the benefit. [IoAwaitable](https://github.com/cppalliance/capy/blob/p4088r0/include/boost/capy/concept/io_awaitable.hpp)<sup>[7]</sup>.

- **Compound result preservation.** `auto [ec, n] = co_await sock.read_some(buf)`. Both values visible. No channel split. No data loss. The three-channel model routes results by channel. Compound results must choose a channel, losing data on the error path ([P4090R0](https://isocpp.org/files/papers/P4090R0.pdf)<sup>[2]</sup>, [P4091R0](https://isocpp.org/files/papers/P4091R0.pdf)<sup>[3]</sup>).

- **Symmetric transfer.** `await_suspend` returns `coroutine_handle<>`. O(1) stack depth regardless of chain length.

- **One-parameter `task<T>`.** [task.hpp](https://github.com/cppalliance/capy/blob/p4088r0/include/boost/capy/task.hpp)<sup>[7]</sup>: `template<typename T = void> struct task`. One parameter. No Environment. The promise carries the environment.

- **Structured concurrency.** [when_all](https://github.com/cppalliance/capy/blob/p4088r0/include/boost/capy/when_all.hpp)<sup>[7]</sup> and [when_any](https://github.com/cppalliance/capy/blob/p4088r0/include/boost/capy/when_any.hpp)<sup>[7]</sup>. Both return `task<>`. Stop tokens propagate through `io_env`. Children complete before the parent resumes.

---

## 8. One Allocation Per Operation

Senders can achieve type-erased I/O. They can get ABI stability. The benchmark in Section 7.4 documents the cost: one allocation per operation, +23 ns. One allocation per read. Completely reasonable.

A custom allocator can pool the operation states. The pool must know each operation state's size at construction time - the size depends on both the sender and the receiver, so the pool is parameterized on the pipeline shape. The pool must be threaded through the API - the I/O object, the connect call, or the execution context must carry it. The pool must be managed per-connection - 10,000 connections means 10,000 pools, each sized for the operation states that connection's pipeline produces.

The allocation purchased something. The receiver-parameterized operation state gives the optimizer full pipeline visibility - the strength documented in Section 3. That visibility is what makes senders the right choice for GPU dispatch, HPC, and compile-time work graphs. The allocation is the price of stamping the receiver into the operation state when the receiver is type-erased.

The coroutine model provides zero-allocation type erasure as a language consequence. The frame the compiler already built is the storage the awaitable already uses. No pool. No size calculation. No API threading. No per-connection management. The language feature provides what the workaround stack reconstructs.

---

## 9. Anticipated Objections

**Q: Why not type-erase senders with `any_sender`?**

A: `any_sender` type-erases the sender, not the receiver. `connect(any_sender, receiver)` still stamps the receiver type into the operation state. The operation state remains a template. `any_sender` erases the sender's identity from the caller. The coroutine model erases the caller's identity from the I/O operation. Section 7.1 documents the distinction. Section 8 documents the cost.

**Q: Does `std::execution::task` not already bridge both models?**

A: It does. Section 2.3 documents this. `task` is a coroutine that is also a sender. The question is not whether two models coexist - they already do. The question is whether the coroutine side of that coexistence carries I/O facilities that exploit the properties `coroutine_handle<>` provides: concrete operation states, type-erased streams, separate compilation, ABI stability. `task` bridges the models. It does not provide I/O.

**Q: Two async models are harder to teach.**

A: Coroutines are not a model to teach. They are the code the programmer already writes - `for`, `if`, structured bindings - with three keywords added. The sender model requires thirty algorithms, each replacing a language construct the programmer already knows. [P4014R1](https://wg21.link/p4014r1)<sup>[21]</sup> is a progressive tutorial of all thirty. The `retry` algorithm: approximately 120 lines as a sender, seven as a coroutine. The four-layer composition in [P4014R1](https://wg21.link/p4014r1)<sup>[21]</sup> Section 12 collapses from interleaved sender pipelines into a single coroutine. The teachability cost was paid when `std::execution` was adopted alongside coroutines in C++26. Coroutine-native I/O does not add a model. It adds I/O operations to the idiom the programmer already uses.

**Q: Coroutines were not designed for I/O.**

A: Correct. The five properties were designed for generality - async patterns, lazy evaluation, generators. Section 7 documents what they produce when applied to I/O. The substrate is emergent.

**Q: The fragmentation argument is revisionist history.**

A: The chronology is a fact. C++20 ratified 2020. `std::execution` adopted June 2024<sup>[19]</sup>. The paper documents which model shipped first.

**Q: `std::execution` is a library, not a second computation model.**

A: `std::execution::task` ([P3552R3](https://wg21.link/p3552r3)<sup>[8]</sup>) is a coroutine type. The library requires language coroutine support to function. Whether the second model is a "language model" or a "library model" does not change the chronology or the benchmark data.

**Q: This picks the coroutine model as the winner for I/O.**

A: The `IoAwaitable` concept ([P4003R1](https://wg21.link/p4003r1)<sup>[22]</sup> "The Narrowest Execution Model for Networking") requires one function: `await_suspend(coroutine_handle<>, io_env const*)`. The protocol is two concepts, one type-erased executor, and a frame allocator cache. The specification fits in six pages. It says nothing about sockets, streams, TCP, TLS, or HTTP. Remove `await_ready` and you lose the synchronous fast path. Remove `io_env` and you lose the executor and stop token. Nothing is removable. This is the narrowest enabler. What gets built on top is the committee's decision.

**Q: This is too much scope for C++29.**

A: [P4003R1](https://wg21.link/p4003r1)<sup>[22]</sup> is two concepts, one executor, and a frame allocator cache. Six pages. The Network Endeavor ([P4100R0](https://wg21.link/p4100r0)<sup>[18]</sup>) is modular - each paper stands independently. If one paper ships, that is progress. If four ship, that is a foundation. The team's target is pencils down by December 2028: reviewed, stable proposals with implementation experience.

---

## 10. The Design Fork

Six objections to the coroutine-native model are well-known. We concede every one.

1. Every coroutine that suspends pays a heap allocation. Senders do not.
2. `coroutine_handle<>::resume()` is an optimization barrier. Senders give the optimizer full pipeline visibility.
3. References captured in the coroutine frame can dangle. Sender operation states own copies.
4. Coroutines do not provide compile-time work graphs, static completion signature checking, or heterogeneous child composition.
5. Coroutines do not provide the composition algebra - `retry`, `when_all` sibling cancellation, `upon_error`.
6. Two async models in the standard library are harder to teach and maintain than one.

These are real costs. The table below documents what each model provides in return. Every row is a downstream consequence of one design choice:

```cpp
// Awaitable                        // Sender
void await_suspend(                 template<class Receiver>
    coroutine_handle<> h);          struct read_operation {
    // caller erased                    Receiver rcvr_;
                                        // caller stamped in
                                    };
```

`coroutine_handle<>` erases the caller. `connect(sender, receiver)` stamps the caller into the operation state.

| Property                                        | Senders | Coroutines | Need |
| ----------------------------------------------- | ------- | ---------- | ---- |
| **Optimization**                                |         |            |      |
| Full pipeline visibility to optimizer           | Yes     |            |      |
| Zero-allocation composition (no frame)          | Yes     |            |      |
| Compile-time work graphs                        | Yes     |            |      |
| Static completion signature checking            | Yes     |            |      |
| Heterogeneous child composition (GPU+net+timer) | Yes     |            |      |
| **Type erasure and compilation**                |         |            |      |
| Structural type erasure of the caller           |         | Yes        |      |
| Operation state is not a template               |         | Yes        |      |
| Operation state can live in the I/O object      |         | Yes        |      |
| Stream can be type-erased (zero per-op alloc)   |         | Yes        |      |
| Type-erased I/O overhead: +5 ns, 0 alloc        |         | Yes        |      |
| Type-erased I/O overhead: +23 ns, 1 alloc       | Yes     |            |      |
| Bridge crossing: ~10-14 ns, 0 alloc             | Yes     | Yes        |      |
| I/O library compiles once (separate compilation)|         | Yes        |      |
| ABI stability across transport changes          |         | Yes        |      |
| **Error handling**                              |         |            |      |
| Non-exception error channel                     | Yes     | Yes        |      |
| Compound I/O result preservation (ec + n)       |         | Yes        |      |
| Composition algebra for infrastructure errors   | Yes     |            |      |
| Composition algebra for compound I/O results    |         |            |      |
| **Structured concurrency**                      |         |            |      |
| when_all with child lifetime guarantee          | Yes     | Yes        |      |
| when_any / race with cancellation propagation   | Yes     | Yes        |      |
| Async scope (spawn + join)                      | Yes     |            |      |
| Stop token propagation                          | Yes     | Yes        |      |
| **Ecosystem**                                   |         |            |      |
| I/O learning surface: 3 keywords                |         | Yes        |      |
| I/O learning surface: 30 algorithms             | Yes     |            |      |
| Single-parameter task type (no fragmentation)   |         | Yes        |      |
| Cross-library interop without bridges           |         | Yes        |      |
| Production deployment at scale                  | Yes     | Yes        |      |
| **Costs**                                       |         |            |      |
| Heap allocation per coroutine frame             |         | Yes        |      |
| Opaque resume (optimization barrier)            |         | Yes        |      |
| Reference lifetime hazard in coroutine frames   |         | Yes        |      |
| Complexity at the I/O call site                 | Yes     |            |      |

The two property sets are mutually exclusive consequences of the design fork. The first five rows follow from stamping the receiver into the operation state. The next six rows follow from erasing the caller behind `coroutine_handle<>`. Neither model can acquire the other's properties without surrendering its own. The Need column is left for the reader.

---

## 11. Conclusion

The committee designed five language mechanisms for generality. None was designed for I/O. Combined, they produce a substrate that resolves the problems that have kept networking out of the C++ standard since 2005: template explosion, compile-time cost, allocation control, and ABI instability. The causal chain runs from one frame allocation through concrete operation states, type-erased streams, separate compilation, and ABI stability to a three-layer architecture where the user chooses the trade-off.

C++20 shipped the language. The programmer already knows how to use it. This paper documents the library it produces.

---

## Acknowledgments

The author thanks Chris Kohlhoff for Asio's stream model, buffer sequences, and executor architecture - twenty years of production deployment is the foundation this work builds on; Eric Niebler, Kirk Shoop, Lewis Baker, and their collaborators for `std::execution`; Gor Nishanov for the coroutine model's explicit support for task type diversity; Dietmar K&uuml;hl for `beman::execution` and [P3552R3](https://wg21.link/p3552r3); Ian Petersen for identifying an asymmetry in an earlier draft and for confirming the equivalence between sender and coroutine dispatch; Ville Voutilainen for broadening the compound-result problem beyond I/O and for [P2464R0](https://wg21.link/p2464r0); Jens Maurer for framing the design spectrum; Herb Sutter for identifying the need for tutorials and documentation; Jonathan M&uuml;ller for confirming the symmetric transfer gap in [P3801R0](https://wg21.link/p3801r0); Peter Dimov for the refined channel mapping; Klemens Morgenstern for Boost.Cobalt and the cross-library bridges; Steve Gerbino for co-developing the constructed comparison, bridge implementations, and Corosio; and Mungo Gill, Mohammad Nejati, and Michael Vandeberg for feedback.

---

## References

1. [P4089R0](https://isocpp.org/files/papers/P4089R0.pdf) - "On the Diversity of Coroutine Task Types" (Vinnie Falco, 2026). https://isocpp.org/files/papers/P4089R0.pdf

2. [P4090R0](https://isocpp.org/files/papers/P4090R0.pdf) - "Sender I/O: A Constructed Comparison" (Vinnie Falco, Steve Gerbino, 2026). https://isocpp.org/files/papers/P4090R0.pdf

3. [P4091R0](https://isocpp.org/files/papers/P4091R0.pdf) - "Two Error Models" (Vinnie Falco, 2026). https://isocpp.org/files/papers/P4091R0.pdf

4. [P4092R0](https://isocpp.org/files/papers/P4092R0.pdf) - "Consuming Senders from Coroutine-Native Code" (Vinnie Falco, Steve Gerbino, 2026). https://isocpp.org/files/papers/P4092R0.pdf

5. [P4093R0](https://isocpp.org/files/papers/P4093R0.pdf) - "Producing Senders from Coroutine-Native Code" (Vinnie Falco, Steve Gerbino, 2026). https://isocpp.org/files/papers/P4093R0.pdf

6. [cppalliance/corosio](https://github.com/cppalliance/corosio) - Coroutine-native networking library. https://github.com/cppalliance/corosio

7. [cppalliance/capy](https://github.com/cppalliance/capy) - Coroutine I/O primitives library. https://github.com/cppalliance/capy

8. [P3552R3](https://wg21.link/p3552r3) - "Add a Coroutine Task Type" (Dietmar K&uuml;hl, Maikel Nadolski, 2025). https://wg21.link/p3552r3

9. [P2453R0](https://wg21.link/p2453r0) - "2021 October Library Evolution Poll Outcomes" (Bryce Adelstein Lelbach, Fabio Fracassi, Ben Craig, 2022). https://wg21.link/p2453r0

10. Eric Niebler, "Structured Concurrency," 2020. https://ericniebler.com/2020/11/08/structured-concurrency/

11. Eric Niebler, "What are Senders Good For, Anyway?" 2024. https://ericniebler.com/2024/02/04/what-are-senders-good-for-anyway/

12. [P2470R0](https://wg21.link/p2470r0) - "Slides for presentation of P2300R2" (Eric Niebler, 2021). https://wg21.link/p2470r0

13. [P0981R0](https://wg21.link/p0981r0) - "Halo: coroutine Heap Allocation eLision Optimization" (Gor Nishanov, 2018). https://wg21.link/p0981r0

14. [P3801R0](https://wg21.link/p3801r0) - "Concerns about the design of `std::execution::task`" (Jonathan M&uuml;ller, 2025). https://wg21.link/p3801r0

15. [N1925](https://wg21.link/n1925) - "A Proposal to Add Networking Utilities to the C++ Standard Library" (Chris Kohlhoff, 2005). https://wg21.link/n1925

16. Boost.Asio AsyncReadStream requirements. https://www.boost.org/doc/libs/1_87_0/doc/html/boost_asio/reference/AsyncReadStream.html

17. [P2464R0](https://wg21.link/p2464r0) - "Ruminations on networking and executors" (Ville Voutilainen, 2021). https://wg21.link/p2464r0

18. [P4100R0](https://wg21.link/p4100r0) - "The Network Endeavor: Coroutine-Native I/O for C++29" (Vinnie Falco, Steve Gerbino, Michael Vandeberg, Mungo Gill, Mohammad Nejati, 2026). https://wg21.link/p4100r0

19. Herb Sutter, "Trip report: Summer ISO C++ standards meeting (St Louis, MO, USA)," 2024. https://herbsutter.com/2024/07/02/trip-report-summer-iso-c-standards-meeting-st-louis-mo-usa/

20. I/O Read Stream Benchmark. https://github.com/sgerbino/capy/tree/pr/beman-bench/bench/beman

21. [P4014R1](https://wg21.link/p4014r1) - "The Sender Sub-Language" (Vinnie Falco, Mungo Gill, 2026). https://wg21.link/p4014r1

22. [P4003R1](https://wg21.link/p4003r1) - "The Narrowest Execution Model for Networking" (Vinnie Falco, Steve Gerbino, Mungo Gill, 2026). https://wg21.link/p4003r1

23. [P0443R14](https://wg21.link/p0443r14) - "A Unified Executors Proposal for C++" (Jared Hoberock, Michael Garland, Chris Kohlhoff, Chris Mysen, Carter Edwards, Gordon Brown, Michael Wong, 2020). https://wg21.link/p0443r14

24. [P4094R0](https://isocpp.org/files/papers/P4094R0.pdf) - "Retrospective: The Unification of Executors and P0443" (Vinnie Falco, 2026). https://isocpp.org/files/papers/P4094R0.pdf

25. [P4099R0](https://isocpp.org/files/papers/P4099R0.pdf) - "Twenty-One Years: The Arc of Networking in C++" (Vinnie Falco, 2026). https://isocpp.org/files/papers/P4099R0.pdf
