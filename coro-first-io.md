| Document | D0000 |
|----------|-------|
| Date:       | 2025-12-31
| Reply-to:   | Vinnie Falco \<vinnie.falco@gmail.com\>
| Audience:   | SG1, LEWG

---

# Coroutine-First I/O: A Type-Erased Affine Framework

## Abstract

This paper asks: *if we design an asynchronous framework from the ground up with networking as the primary use case, what does the best possible API look like?*

We present a coroutine-first I/O framework that achieves executor flexibility without sacrificing encapsulation. The *affine awaitable protocol* propagates execution context through coroutine chains without embedding executor types in public interfaces. Platform I/O types remain hidden in translation units, while composed algorithms expose only `task` return types.

A central design choice: we consciously trade one pointer indirection per I/O operation for complete type hiding. This tradeoff—approximately 1-2 nanoseconds of overhead—eliminates the template complexity, compile time bloat, and ABI instability that have long plagued C++ async libraries. In contrast, `std::execution`'s `connect(sender, receiver)` returns a fully-typed `operation_state`, forcing implementation types through every API boundary.

We compare our networking-first design against `std::execution` (P2300) and observe significant divergence. Analysis of P2300 and its evolution (P3826) reveals a framework driven by GPU and parallel workloads—P3826's focus is overwhelmingly GPU/parallel with no networking discussion. Core networking concerns—strand serialization, I/O completion contexts, platform integration—remain unaddressed. The query-based context model that P3826 attempts to fix is unnecessary when context propagates forward through coroutine machinery.

Our framework demonstrates what emerges when networking requirements drive the design rather than being adapted to a GPU-focused abstraction.

---

## 1. The Problem: Callback Hell Meets Template Hell

Traditional asynchronous I/O frameworks face an uncomfortable choice. The callback model—templated on executor and completion handler—achieves zero-overhead abstraction but leaks implementation details into every public interface:

```cpp
template<class Executor, class Handler>
void async_read(Executor ex, Handler&& handler);
```

Every composed operation must propagate these template parameters, creating a cascade of instantiations. The executor type, the handler type, and often an allocator type all become part of the function signature. Users cannot hold heterogeneous operations in containers. Libraries cannot define stable ABIs. The "zero-cost abstraction" exacts its price in compile time, code size, and API complexity.

Type erasure offers an escape, but the traditional approach—`std::function` for handlers, `any_executor` for executors—introduces heap allocations at every composition boundary. Our benchmarks show this penalty compounds: a four-level operation using fully type-erased callbacks allocates 601 times per invocation and runs 5.6× slower than native templates.

We sought a third path: *what would an asynchronous I/O framework look like if we designed it from first principles for networking?* Not adapting an existing abstraction, not generalizing from parallel algorithms, but starting with sockets, buffers, strands, and completion ports as the primary concerns.

The answer diverges significantly from `std::execution`. Where templates propagate through every interface, we use structural type erasure. Where algorithm customization requires domain transforms, we need none—networking has one implementation per platform; TCP servers don't run on GPUs. Section 2.5 explores these differences in detail.

---

## 2. Motivation: Why Not the Networking TS?

The Networking TS (and its progenitor Boost.Asio) is the de facto standard for asynchronous I/O in C++. It is mature, well-tested, and supports coroutines through completion tokens. Why build something new?

### 2.1 The Template Tax

The Networking TS design philosophy—zero-overhead abstraction through templates—incurs costs that compound in large codebases:

**Every async operation signature includes executor and handler types:**
```cpp
template<class Executor, class ReadHandler>
void async_read(basic_socket<Protocol, Executor>& s,
                MutableBufferSequence const& buffers,
                ReadHandler&& handler);
```

**Composed operations propagate these types:**
```cpp
template<class Executor, class Handler>
void async_http_request(basic_socket<tcp, Executor>& sock,
                        http::request const& req,
                        Handler&& handler);
```

This creates:
- **N×M template instantiations** for N operations × M executor/handler combinations
- **Header-only dependencies** that must be recompiled on change
- **Binary size growth** that can reach megabytes in complex applications
- **Compile times measured in minutes** for moderate codebases

### 2.2 The Encapsulation Problem

The Networking TS templates expose implementation details through public interfaces:

```cpp
// User sees:
class http_client
{
public:
    template<class Executor, class Handler>
    void async_get(Executor ex, std::string url, Handler&& h);
};
```

The user is forced to know:
- What executor types are valid
- What handler signatures are expected
- That the implementation uses sockets at all

Platform types (`OVERLAPPED`, `io_uring_sqe`) are hidden, but the *structure* of the async machinery leaks through every API boundary.

**Our approach:**
```cpp
class http_client
{
public:
    task async_get(std::string url);  // That's it.
};
```

The implementation—sockets, buffers, executors—lives in the translation unit. The interface is stable. The ABI is stable. Compilation is fast.

### 2.3 Coroutine-Compatible vs Coroutine-First

The Networking TS added coroutine support through completion tokens like `use_awaitable`:

```cpp
co_await async_read(socket, buffer, use_awaitable);
```

This adapts callback-based operations for coroutines. It works, but:
- **Double indirection**: Callback machinery wraps coroutine machinery
- **Executor handling is manual**: `co_spawn` and `bind_executor` required
- **Error handling diverges**: Exceptions vs `error_code` vs `expected`
- **Mental model mismatch**: Writing coroutines that think in callbacks

Our design is **coroutine-first**: the suspension/resumption model is the foundation, not an adapter. Executor propagation is automatic. Type erasure is structural. The callback path (`dispatch().resume()`) is the compatibility layer, not the other way around.

### 2.4 When to Use the Networking TS Instead

The Networking TS remains the right choice when:
- You need callback-based APIs for C compatibility
- Template instantiation cost is acceptable
- You're already invested in the Asio ecosystem
- Maximum performance with zero abstraction is required
- Standardization timeline matters for your project

Our framework is better suited when:
- Coroutines are the primary programming model
- Public APIs must hide implementation details
- Compile time and binary size matter
- ABI stability is required across library boundaries
- Clean, simple interfaces are prioritized

### 2.5 Why Not std::execution?

The C++26 `std::execution` framework (P2300) provides sender/receiver abstractions for asynchronous programming. We analyzed both the base proposal and its evolution to understand how well it addresses networking.

#### 2.5.1 P2300: Networking Mentioned, Not Addressed

P2300 references networking and GPU/parallel topics in roughly equal measure. Superficially balanced. However, the *nature* of these references differs:

| Aspect | GPU/Parallel | Networking |
|--------|--------------|------------|
| Concrete primitives | `bulk` algorithm, parallel scheduler | None — deferred to P2762 |
| Platform integration | CUDA mentioned by name | No IOCP, epoll, or io_uring |
| Serialization | N/A | **Zero strand mentions** |
| Status | Normative specification | Illustrative examples only |

P2300 explicitly defers networking:

> "Dietmar Kuehl has proposed networking APIs that use the sender/receiver abstraction (see P2762)." — P2300, Section 1.4.1.3

The framework provides composition primitives but **no I/O operations, no buffer management, and no strand serialization**. Networking exists as examples, not as design drivers.

**What P2762 actually proposes:**

P2762, the proposal P2300 defers to, explicitly chooses the backward query model as "most useful":

> "Injecting the used scheduler from the point where the asynchronous work is actually used."

Yet the paper immediately acknowledges the late-binding problem:

> "When the scheduler is injected through the receiver the operation and used scheduler are brought together rather late, i.e., when `connect(sender, receiver)`ing and when the work graph is already built."

P2762 also raises per-operation cancellation overhead as a concern—the same issue our socket-level approach solves—but offers no solution:

> "Repeatedly doing that operation while processing data on a socket may be a performance concern... This area still needs some experimentation and, I think, design."

P2762 leaves core networking features unspecified:

| Feature | P2762 Status |
|---------|--------------|
| Scheduler interface | "It isn't yet clear how such an interface would actually look" |
| Strand serialization | Not mentioned |
| Buffer pools | "There is currently no design" |

This confirms our thesis: networking is being adapted to `std::execution` rather than driving its design. The proposal acknowledges problems with the query model, identifies performance concerns we solve, yet leaves core networking abstractions unaddressed.

#### 2.5.2 P3826: GPU-First Evolution

P3826R2, "Fix or Remove Sender Algorithm Customization," reveals where `std::execution`'s complexity originates. The paper identifies a fundamental flaw:

> "Many senders do not know where they will complete until they know where they will be started."

The proposed fix adds `get_completion_domain<Tag>` queries, `indeterminate_domain<>` types, and double-transform dispatching. This machinery exists to select GPU vs CPU algorithm implementations.

**P3826 contains over 100 references to GPU, parallel execution, and hardware accelerators. It contains zero references to networking, sockets, or I/O patterns.**

| Concern | GPU/Parallel (P3826's Focus) | Networking (Unaddressed) |
|---------|------------------------------|--------------------------|
| Algorithm dispatch | Critical — GPU vs CPU kernel | Irrelevant — one implementation per platform |
| Completion location | May differ from start | Same context or OS completion port |
| Domain customization | Required for acceleration | Unused overhead |

Networking requires different things entirely:
- **Strand serialization**: Ordering guarantees, not algorithm selection
- **I/O completion contexts**: IOCP, epoll, io_uring integration
- **Executor propagation**: Ensuring handlers run on the correct thread
- **Buffer lifetime management**: Zero-copy patterns, scatter/gather

None appear in P3826's analysis.

#### 2.5.3 The Query Model Problem

Both P2300 and P3826 share a design choice: **backward queries** for execution context.

```cpp
// std::execution pattern: query sender/receiver for properties
auto sched = get_scheduler(get_env(rcvr));
auto domain = get_domain(get_env(sndr));

// P3826 addition: tell sender where it starts
auto domain = get_completion_domain<set_value_t>(get_env(sndr), get_env(rcvr));
```

This requires senders to be self-describing—problematic when context is only known at the call site. P3826 attempts to fix this by passing hints, but the fix adds complexity rather than questioning the query model itself.

#### 2.5.4 Forward Propagation Alternative

Our coroutine model propagates context forward:

```cpp
// Coroutine-first: context flows from caller to callee
async_run(my_executor, my_task());  // Executor injected at root
// await_transform propagates executor_ref to all children — no queries needed
```

The executor is always known because it flows with control flow, not against it. No domain queries. No completion scheduler inference. No transform dispatching.

#### 2.5.5 Observations

Comparing our networking-first design with `std::execution` reveals significant divergence:

1. **What we need, they don't have**: Strand serialization, platform I/O integration, buffer management
2. **What they have, we don't need**: Domain-based algorithm dispatch, completion domain queries, sender transforms
3. **Context flow**: They query backward; we inject forward

The `std::execution` framework may eventually support networking, but its evolution is driven by GPU and parallel workloads. The complexity P3826 adds—to fix problems networking doesn't have—suggests that networking was not a primary design consideration.

A framework designed from networking requirements produces a simpler, more direct solution.

#### 2.5.6 The std::execution Tax

If networking is required to integrate with `std::execution`, I/O libraries must pay a complexity tax:

- **Query protocol compliance**: Implement `get_env`, `get_domain`, `get_completion_scheduler`—even if only to return defaults
- **Concept satisfaction**: Meet sender/receiver requirements designed for GPU algorithm dispatch
- **Transform machinery**: Domain transforms execute even when they select the only available implementation
- **API surface expansion**: Expose attributes and queries irrelevant to I/O operations
- **Type leakage**: `connect(sender, receiver)` returns a fully-typed `operation_state`, forcing implementation details through every API boundary

The type leakage deserves emphasis. In the sender/receiver model:

```cpp
// std::execution pattern
execution::sender auto snd = socket.async_read(buf);
execution::receiver auto rcv = /* ... */;
auto state = execution::connect(snd, rcv);  // Type: connect_result_t<Sender, Receiver>
```

The `connect_result_t` type encodes the full operation state. Algorithms that compose senders must propagate these types:

```cpp
// From P2300: operation state types leak into composed operations
template<class S, class R>
struct _retry_op {
    using _child_op_t = stdexec::connect_result_t<S&, _retry_receiver<S, R>>;
    optional<_child_op_t> o_;  // Nested operation state, fully typed
};
```

This means operation state types propagate through every algorithm boundary, senders cannot hide their implementation behind stable interfaces, and changing an I/O implementation changes the sender's type—breaking ABI.

This tax exists regardless of whether networking benefits from it. A socket that returns `default_domain` still participates in the dispatch protocol. The P3826 machinery runs, finds no customization, and falls through to the default—overhead for nothing.

**The question is not whether P2300/P3826 break networking code.** They don't—defaults work. **The question is whether networking should pay for abstractions it doesn't use.** Our analysis suggests the cost is not justified when a simpler, networking-native design achieves the same goals.

---

## 3. The Executor Model

C++20 coroutines provide type erasure *by construction*—but not through the handle type. `std::coroutine_handle<void>` and `std::coroutine_handle<promise_type>` are both just pointers with identical overhead. The erasure that matters is *structural*:

1. **The frame is opaque**: Callers see only a handle, not the promise's layout
2. **The return type is uniform**: All coroutines returning `task` have the same type, regardless of body
3. **Suspension points are hidden**: The caller doesn't know where the coroutine may suspend

This structural erasure is often lamented as overhead, but we recognized it as opportunity: *the allocation we cannot avoid can pay for the type erasure we need*.

A coroutine's promise can store execution context *by reference*, receiving it from the caller at suspension time rather than at construction time. This deferred binding enables a uniform `task` type that works with any executor, without encoding the executor type in its signature.

We define an executor as any type satisfying the `is_executor` concept:

```cpp
template<class T>
concept is_executor = requires(T const& t, coro h, work* w) {
    { t.dispatch(h) } -> std::convertible_to<coro>;  // Returns handle for symmetric transfer
    t.post(w);                                        // Queue work for later execution
    { t == t } -> std::convertible_to<bool>;
};
```

The distinction between `dispatch` and `post`:
- **dispatch**: Immediately invoke the coroutine. Used when resuming on the correct executor.
- **post**: Enqueue work for the event loop. Used for I/O completion and cross-executor transitions.

Executors must be equality-comparable to enable optimizations when source and target executors are identical.

### 3.1 Type-Erased Executor Reference

To store executors without encoding their type, we introduce `executor_ref`—a non-owning, type-erased reference:

```cpp
struct executor_ref
{
    struct ops
    {
        coro (*dispatch_coro)(void const*, coro) = nullptr;
        void (*post_work)(void const*, work*) = nullptr;
        bool (*equals)(void const*, void const*) = nullptr;
    };

    template<class Executor>
    static constexpr ops ops_for{ /* lambdas */ };

    ops const* ops_ = nullptr;
    void const* ex_ = nullptr;
};
```

The `ops` table has static storage duration, instantiated once per executor type. The executor itself is stored by pointer, assuming the referent outlives the reference. This is safe because executors are typically owned by I/O contexts with longer lifetimes than any operation.

Equality comparison short-circuits on the `ops_` pointer—if two references have different operation tables, they wrap different types and cannot be equal:

```cpp
bool operator==(executor_ref const& other) const noexcept
{
    return ops_ == other.ops_ && ops_->equals(ex_, other.ex_);
}
```

### 3.2 Executor Composition and Placement

A subtle but important property: executor types are fully preserved at call sites even though they're type-erased internally. This enables zero-overhead composition at the API boundary while maintaining uniform internal representation.

**Composable executor wrappers work naturally:**

```cpp
template<class Ex>
struct strand {
    Ex inner_;
    
    coro dispatch(coro h) const {
        // Serialization logic, then delegate to inner
        return inner_.dispatch(h);
    }
    // ...
};

// Full type information at the call site
strand<pool_executor> s{pool.get_executor()};
async_run(s, my_task());  // strand<pool_executor> passed by value
```

When `async_run` stores the executor in the root task's frame, it preserves the complete type. The `strand` wrapper's serialization logic remains inlinable. Only when the executor propagates to child tasks—as `executor_ref`—does type erasure occur. The erasure boundary is pushed to where it matters least: internal propagation rather than the hot dispatch path.

**Why executor customization matters—even single-threaded:**

Executors encode *policy*, not just parallelism. A single-threaded `io_context` still benefits from executor customization:

- **Serialization**: `strand` ensures operations on one connection don't interleave, even when callbacks fire in arbitrary order
- **Deferred execution**: `post` vs `dispatch` controls stack depth and allows pending work to run before continuing
- **Instrumentation**: Wrapper executors can measure latency, count operations, or propagate tracing context
- **Prioritization**: Critical control messages can preempt bulk data transfers
- **Testing**: Manual executors enable deterministic unit tests without real async timing

The `is_executor` concept and `executor_ref` type erasure support all these patterns without baking policy into I/O object types.

**Why the executor belongs at the call site, not on the I/O object:**

- **The caller knows the concurrency requirements.** A multithreaded `io_context` may need strand-wrapped executors; a single-threaded context may not. The socket cannot know this—it only knows how to perform I/O.

- **I/O objects become context-agnostic.** A socket works identically whether wrapped in a strand, run on a thread pool, or executed on a single-threaded context. The same socket type serves all deployment scenarios.

- **Composition happens at the edge.** Users compose executors (`strand<pool_executor>`, `priority_executor<strand<...>>`) at the point of use. The I/O object doesn't need N template parameters for N possible wrapper combinations.

**The coroutine advantage:**

Traditional callback-based designs often embed the executor in the I/O object's type, creating signatures like `basic_socket<Protocol, Executor>`. This leaks concurrency policy into type identity—two sockets with different executors have different types, complicating containers and APIs.

Coroutines invert this relationship. The executor enters at `async_run` or `run_on`, propagates invisibly through `executor_ref`, and reaches I/O operations via the affine awaitable protocol's `await_transform`. The I/O object participates in executor selection without encoding it in its type:

```cpp
// Traditional: executor embedded in socket type
basic_socket<tcp, strand<pool_executor>> sock;

// Coroutine model: executor supplied at launch
socket sock;  // No executor type parameter
async_run(strand{pool.get_executor()}, use_socket(sock));
```

The socket receives its executor indirectly—through the coroutine machinery—yet still benefits from the caller's choice of strand wrapping, thread pool, or any other executor composition.

---

## 4. Platform I/O: Hiding the Machinery

A central goal is encapsulation: platform-specific types (`OVERLAPPED`, `io_uring_sqe`, file descriptors) should not appear in public headers. We achieve this through *preallocated, type-erased operation state*.

### 4.1 The Socket Abstraction

```cpp
struct socket
{
    socket() : op_(new state) {}

    async_awaitable async_io();

private:
    struct state : work
    {
        coro h_;
        executor_ref ex_;
        // OVERLAPPED, HANDLE, etc. — hidden here
        
        void operator()() override
        {
            ex_.dispatch(h_)();  // Resume the returned handle
        }
    };

    std::unique_ptr<state> op_;
};
```

The `state` structure:
1. Inherits from `work`, enabling intrusive queuing
2. Stores the coroutine handle and executor reference needed for completion
3. Contains platform-specific members (OVERLAPPED, handles) invisible to callers
4. Is allocated *once* at socket construction, not per-operation

### 4.2 Intrusive Work Queue

Submitted work uses an intrusive singly-linked list:

```cpp
struct work
{
    virtual ~work() = default;
    virtual void operator()() = 0;
    work* next = nullptr;
};
```

This design eliminates container allocations—each work item carries its own link. The `queue` class manages head and tail pointers with O(1) push and pop. Combined with the preallocated socket state, I/O operations require *zero allocations* in steady state.

### 4.3 The Encapsulation Tradeoff

We pay a cost for translation unit hiding: one level of indirection through the preallocated `state` pointer. This is a **conscious design choice** that addresses the most common complaints from C++ users about template-heavy async libraries:

- **"Asio compile times are killing us"** — Template instantiation is expensive
- **"I can't read these error messages"** — Deep template nesting obscures intent
- **"Changing the socket breaks everything"** — ABI instability forces full rebuilds
- **"Why is my binary 50MB?"** — N×M instantiations bloat executables

**The alternative: frame-embedded operation state**

If I/O types were exposed in headers, operation state could live directly in the coroutine frame:

```cpp
// Hypothetical: types exposed, state in frame
template<class Socket>
task<size_t> async_read(Socket& s, buffer buf) {
    typename Socket::read_op op{s, buf};  // State in coroutine frame
    co_await op;
    co_return op.bytes_transferred();
}
```

This eliminates the indirection—the operation state is allocated as part of the coroutine frame, not separately. When the compiler knows the concrete types, it can potentially inline everything. This is the path `std::execution` takes with `connect_result_t` (see §2.5.6).

**Why we chose encapsulation:**

| Concern | Frame Embedding | Our Approach |
|---------|-----------------|--------------|
| Platform types in headers | `OVERLAPPED`, `io_uring_sqe` visible | Hidden in `.cpp` |
| ABI stability | Breaks on implementation change | Stable across versions |
| Compile time | Full template instantiation | Minimal header parsing |
| Binary size | N×M instantiations | One per operation |
| Refactoring cost | Recompile all users | Recompile one TU |

The cost is **one pointer dereference per I/O operation**—typically 1-2 nanoseconds. Section 9 shows this is negligible for I/O-bound workloads.

---

## 5. The Affine Awaitable Protocol

The core innovation is how execution context flows through coroutine chains. We extend the standard awaitable protocol with an *affine* overload of `await_suspend` that returns a coroutine handle for symmetric transfer:

```cpp
template<class Executor>
std::coroutine_handle<> await_suspend(coro h, Executor const& ex) const;
```

This two-argument form receives both the suspended coroutine handle and the caller's executor, and returns the next coroutine to resume. The caller's promise uses `await_transform` to inject the executor:

```cpp
template<class Awaitable>
auto await_transform(Awaitable&& a)
{
    struct transform_awaiter
    {
        std::decay_t<Awaitable> a_;
        promise_type* p_;
        
        auto await_suspend(std::coroutine_handle<Promise> h)
        {
            return a_.await_suspend(h, p_->ex_);  // Inject executor, return handle
        }
    };
    return transform_awaiter{std::forward<Awaitable>(a), this};
}
```

This mechanism achieves implicit executor propagation: child coroutines inherit their parent's executor without explicit parameter passing.

### 5.1 Symmetric Transfer

A deliberate design choice: `await_suspend` returns `std::coroutine_handle<>` rather than `void`. When `await_suspend` returns a handle, the runtime resumes that coroutine *without growing the stack*—effectively a tail call. This prevents stack overflow in deep coroutine chains.

Without symmetric transfer:
```
A finishes → dispatch(B) → B finishes → dispatch(C) → ...
                ↓                          ↓
           stack grows              stack grows more
```

With symmetric transfer:
```
A finishes → return B → runtime resumes B → return C → runtime resumes C
             (constant stack depth)
```

The executor's `dispatch` method also returns a handle:

```cpp
coro dispatch(coro h) const
{
    return h;  // Return handle for symmetric transfer
}
```

If the executor must post rather than dispatch (cross-thread), it returns `std::noop_coroutine()`.

### 5.2 Sender/Receiver Compatibility

The design is compatible with P3552 and `std::execution`. The `dispatch()` method returns a `std::coroutine_handle<>` that can be used in two ways:

- **Symmetric transfer** (coroutine context): Return the handle from `await_suspend`
- **Explicit resume** (sender context): Call `handle.resume()` directly

```cpp
// In a sender's completion:
ex.dispatch(continuation).resume();
```

If `dispatch()` posts work rather than dispatching inline, it returns `std::noop_coroutine()`. Calling `.resume()` on this is defined as a no-op, making the API safe in all contexts:

```cpp
coro dispatch(coro h) const
{
    return h;  // Return for symmetric transfer OR explicit resume
}

// Coroutine caller:
return ex.dispatch(h);        // Symmetric transfer

// Sender/callback caller:
ex.dispatch(h).resume();      // Explicit resume, noop if posted
```

This means one executor interface serves both coroutines and senders with no conditional code paths.

### 5.3 The Task Type

The `task` type represents a lazy, composable coroutine:

```cpp
struct task
{
    struct promise_type
    {
        executor_ref ex_;           // Where this task runs
        executor_ref caller_ex_;    // Where to resume caller
        coro continuation_;          // Caller's coroutine handle
        // ...
    };

    std::coroutine_handle<promise_type> h_;
    bool has_own_ex_ = false;
};
```

When awaited, a task:
1. Stores the caller's continuation and executor
2. Either inherits the caller's executor (default) or uses its own (if `set_executor` was called)
3. Returns the child's handle for symmetric transfer (or `noop_coroutine()` if posted)

At completion (`final_suspend`), the task returns the continuation for symmetric transfer:

```cpp
std::coroutine_handle<> await_suspend(coro h) const noexcept
{
    std::coroutine_handle<> next = std::noop_coroutine();
    if(p_->continuation_)
        next = p_->caller_ex_.dispatch(p_->continuation_);
    h.destroy();
    return next;
}
```

---

## 6. Executor Injection: Launch and Switch

Coroutines need executors at two points: at the **root** (where does the whole operation run?) and **mid-chain** (how do I switch to a different executor for one sub-operation?). This section covers both primitives.

### 6.1 Mid-Chain Switching: `run_on`

Any coroutine can switch executors mid-operation using `run_on`:

```cpp
template<class Executor>
task run_on(Executor const& ex, task t)
{
    t.set_executor(ex);
    return t;
}

// Usage:
co_await run_on(other_executor, some_operation());
```

When awaited, a task with its own executor posts a starter work item to that executor rather than resuming inline. Upon completion, it dispatches back to the *caller's* executor—not its own—ensuring seamless return to the original context.

This separation of `ex_` (where I run) and `caller_ex_` (where my caller resumes) enables crossing executor boundaries without explicit continuation management.

**Practical use cases for mid-chain switching:**

```cpp
// Offload CPU-intensive work to a thread pool
co_await run_on(cpu_pool, compute_heavy_task());

// Serialize access to shared state
co_await run_on(strand, protected_state_update());

// Instrument a specific subtree with tracing
co_await run_on(traced_executor, operation_to_measure());

// Prioritize urgent work
co_await run_on(high_priority_executor, send_control_message());
```

These patterns work identically whether the underlying context is single-threaded or multi-threaded—the executor abstraction is about policy, not just parallelism.

### 6.2 Root Ownership: `async_run`

Top-level coroutines present a lifetime challenge: the executor must outlive all operations, but the coroutine owns only references. We solve this with a wrapper coroutine that *owns* the executor:

```cpp
template<class Executor>
struct root_task
{
    struct starter : work { /* embedded in promise */ };
    
    struct promise_type
    {
        Executor ex_;   // Owned by value
        starter start_; // Embedded—no allocation needed
        // ...
    };
    std::coroutine_handle<promise_type> h_;
};

template<class Executor>
void async_run(Executor ex, task t)
{
    auto root = wrapper<Executor>(std::move(t));
    root.h_.promise().ex_ = std::move(ex);
    
    // Post embedded starter—avoids allocation since root_task is long-lived
    root.h_.promise().start_.h_ = root.h_;
    root.h_.promise().ex_.post(&root.h_.promise().start_);
    root.release();
}
```

The `root_task` frame lives on the heap and contains the executor by value. All child tasks receive `executor_ref` pointing into this frame. The frame self-destructs at `final_suspend`, after all children have completed.

This design imposes overhead only at the root—intermediate tasks pay nothing for executor storage.

---

## 7. Frame Allocator Customization

The default coroutine allocation strategy—one heap allocation per frame—is suboptimal for repeated operations. We introduce a *frame allocator protocol* that allows I/O objects to provide custom allocation strategies.

### 7.1 The Frame Allocator Concepts

```cpp
template<class A>
concept frame_allocator = requires(A& a, void* p, std::size_t n) {
    { a.allocate(n) } -> std::convertible_to<void*>;
    { a.deallocate(p, n) } -> std::same_as<void>;
};

template<class T>
concept has_frame_allocator = requires(T& t) {
    { t.get_frame_allocator() } -> frame_allocator;
};
```

The `has_frame_allocator` concept allows I/O objects to opt-in explicitly by providing a `get_frame_allocator()` member function.

### 7.2 Why I/O Objects?

The frame allocator lives on I/O objects rather than the executor or root task—a Goldilocks choice that balances accessibility, lifetime, and efficiency:

- **The executor is type-erased.** Our design propagates `executor_ref` through coroutine chains. Type erasure means the executor cannot carry concrete allocator state without either templating on allocator type (defeating type erasure) or adding another indirection layer (adding overhead).

- **The root task is too distant.** While the root task owns the executor by value, intermediate coroutines only see `executor_ref`. Tunneling allocator access through every `co_await` would add indirection costs, and the allocation pattern wouldn't match the I/O pattern.

- **I/O objects have the right lifetime.** A socket outlives its operations. Frames for `socket.async_read()` are allocated when the call begins and freed when it completes—while the socket remains alive. The allocator naturally follows the object initiating the operation.

- **Locality of allocation sizes.** Operations on the same I/O object tend to produce similar frame sizes. A socket's read operations all generate roughly equal frames. Per-object pools achieve better cache locality and recycling efficiency than global pools shared by unrelated operations.

- **C++20 machinery supports it naturally.** The `promise_type::operator new` overloads receive coroutine parameters directly. Detecting `has_frame_allocator` on the first or second parameter is zero-overhead—no runtime dispatch, no extra storage. The I/O object is already there as the natural first argument.

- **A global fallback handles everything else.** Coroutines without I/O object parameters—lambdas, wrappers, pure computation—fall back to a global frame pool. Universal recycling without universal plumbing.

### 7.3 Task Integration

The `task::promise_type` overloads `operator new` to detect frame allocator providers:

```cpp
struct promise_type
{
    // First parameter has get_frame_allocator()
    template<has_frame_allocator First, class... Rest>
    static void* operator new(std::size_t size, First& first, Rest&...);

    // Second parameter (for member functions where this comes first)
    template<class First, has_frame_allocator Second, class... Rest>
    static void* operator new(std::size_t size, First&, Second& second, Rest&...)
        requires (!has_frame_allocator<First>);

    // Default: no frame allocator
    static void* operator new(std::size_t size);

    static void operator delete(void* ptr, std::size_t size);
};
```

When a coroutine's first or second parameter satisfies `has_frame_allocator`, the frame is allocated from that object's allocator. Otherwise, the global heap is used.

### 7.4 Allocation Tagging

To enable unified deallocation, we prepend a header to each frame:

```cpp
struct alloc_header
{
    void (*dealloc)(void* ctx, void* ptr, std::size_t size);
    void* ctx;
};
```

The header stores a deallocation function pointer and context. When `operator delete` is called, it reads the header to determine whether to use the custom allocator or the global heap.

### 7.5 Thread-Local Frame Pool

I/O objects implement `get_frame_allocator()` returning a pool with thread-local caching:

```cpp
class frame_pool
{
    void* allocate(std::size_t n)
    {
        // 1. Try thread-local free list
        // 2. Try global pool (mutex-protected)
        // 3. Fall back to heap
    }

    void deallocate(void* p, std::size_t)
    {
        // Return to thread-local free list
    }
};
```

After the first iteration, frames are recycled without syscalls. The global pool handles cross-thread returns safely.

---

## 8. Allocation Analysis

With recycling enabled for both models, we achieve zero steady-state allocations:

| Operation | Callback (recycling) | Coroutine (pooled) |
|-----------|---------------------|-------------------|
| async_io (1 level) | 0 | 0 |
| async_read_some (2 levels) | 0 | 0 |
| async_read (3 levels) | 0 | 0 |
| async_request (100 iterations) | 0 | 0 |

### 8.1 Recycling Matters for Both Models

A naive implementation of either model performs poorly. Without recycling:
- **Callbacks**: Each I/O operation allocates and deallocates operation state
- **Coroutines**: Each coroutine frame is heap-allocated and freed

The key optimization for *both* models is **thread-local recycling**: caching recently freed memory for immediate reuse by the next operation.

### 8.2 Callback Recycling

For callbacks, we implement a single-block thread-local cache:

```cpp
struct op_cache
{
    static void* allocate(std::size_t n)
    {
        auto& c = get();
        if(c.ptr_ && c.size_ >= n)
        {
            void* p = c.ptr_;
            c.ptr_ = nullptr;
            return p;
        }
        return ::operator new(n);
    }

    static void deallocate(void* p, std::size_t n)
    {
        auto& c = get();
        if(!c.ptr_ || n >= c.size_)
        {
            ::operator delete(c.ptr_);
            c.ptr_ = p;
            c.size_ = n;
        }
        else
            ::operator delete(p);
    }
};
```

The pattern is: **delete before dispatch**. When an I/O operation completes, it deallocates its state *before* invoking the completion handler. If that handler immediately starts another operation, the allocation finds the just-freed memory in the cache.

### 8.3 Coroutine Frame Pooling

For coroutines, we use a global frame pool that all coroutines share, regardless of whether they have explicit frame allocator parameters:

```cpp
// Default operator new uses global pool
static void* operator new(std::size_t size)
{
    static frame_pool alloc(frame_pool::make_global());
    // ... allocate from pool
}
```

This ensures that *all* coroutines—including lambdas, wrappers, and tasks without I/O object parameters—benefit from frame recycling. The pool uses thread-local caching with a global overflow pool for cross-thread scenarios.

### 8.4 Amortized Cost

Both models achieve **zero steady-state allocations** after warmup. The first iteration populates the caches; all subsequent operations recycle memory without syscalls.

---

## 9. Performance Comparison

### 9.1 Clang with Frame Elision

Benchmarks compiled with Clang 20.1, `-O3`, Windows x64, with `[[clang::coro_await_elidable]]`:

| Operation | Callback | Coroutine | Ratio |
|-----------|----------|-----------|-------|
| async_io | 3 ns | 21 ns | 7.0× |
| async_read_some | 4 ns | 25 ns | 6.3× |
| async_read (10×) | 44 ns | 99 ns | 2.3× |
| async_request (100×) | 498 ns | 750 ns | **1.5×** |

### 9.2 MSVC Comparison

The same benchmarks compiled with MSVC 19.x, RelWithDebInfo, Windows x64:

| Operation | Callback | Coroutine | Ratio |
|-----------|----------|-----------|-------|
| async_io | 5 ns | 58 ns | 11.6× |
| async_read_some | 5 ns | 69 ns | 13.8× |
| async_read (10×) | 60 ns | 226 ns | 3.8× |
| async_request (100×) | 673 ns | 1749 ns | 2.6× |

MSVC's coroutine implementation is approximately 2× slower than Clang's for this workload. The difference stems from:
- Less aggressive inlining of coroutine machinery
- No support for `[[clang::coro_await_elidable]]`
- Different code generation for symmetric transfer

### 9.3 Analysis

**Overhead Ratio Improves with Depth**: The key observation is that the callback/coroutine ratio *improves* as operation complexity increases:

| Depth | Clang Ratio | MSVC Ratio |
|-------|-------------|------------|
| 1 operation | 7.0× | 11.6× |
| 10 operations | 2.3× | 3.8× |
| 100 operations | **1.5×** | 2.6× |

This is because coroutine overhead is *fixed per suspension*, while the useful work (queue operations) scales with depth. For `async_request`:
- Clang: 750 ns / 100 ops = 7.5 ns per I/O
- MSVC: 1749 ns / 100 ops = 17.5 ns per I/O
- Callback: ~5 ns per I/O

### 9.4 Real-World Context

For I/O-bound workloads:
- Network RTT: 100,000+ ns
- Disk access: 10,000+ ns  
- Coroutine overhead: 7-18 ns per suspension (compiler-dependent)

Even with MSVC, the coroutine overhead is **0.02%** of a typical network operation. The 1.5-2.6× overhead for `async_request` is the cost of compositional async/await syntax versus hand-optimized state machines—a reasonable trade-off for most applications.

---

## 10. The Unavoidable Cost: `resume()` Opacity

Coroutine performance is inherently limited by the opacity of `std::coroutine_handle<>::resume()`. The compiler cannot inline across resume boundaries because:

1. The coroutine may be suspended at any of multiple `co_await` points
2. The frame address is only known at runtime
3. Resume effectively performs an indirect jump through the frame's resumption pointer

Note: This overhead is unrelated to handle typing. Whether you hold `coroutine_handle<void>` or `coroutine_handle<promise_type>`, the resume operation is identical—an indirect jump through the frame's resumption pointer. The opacity is intrinsic to the coroutine model, not an artifact of type erasure.

This prevents optimizations that callbacks enable: register allocation across async boundaries, constant propagation through handlers, and dead code elimination of unused paths.

### 10.1 HALO and Coroutine Elision

HALO (Heap Allocation eLision Optimization) can inline coroutine frames when the compiler can prove:
1. The coroutine is immediately awaited
2. The frame doesn't escape the caller's lifetime
3. The coroutine's lifetime is bounded by the caller

Standard HALO is limited because our design—where tasks store handles and pass through executors—causes the frame to "escape" into the task object.

However, **Clang provides `[[clang::coro_await_elidable]]`**, an attribute that enables more aggressive frame elision:

```cpp
#ifdef __clang__
#define CORO_AWAIT_ELIDABLE [[clang::coro_await_elidable]]
#else
#define CORO_AWAIT_ELIDABLE
#endif

struct CORO_AWAIT_ELIDABLE task { /* ... */ };
```

With this attribute, Clang can elide nested coroutine frames into the parent's frame when directly awaited. Our benchmarks show a **26% performance improvement** with this attribute enabled:

| Metric | Without Attribute | With Attribute |
|--------|-------------------|----------------|
| async_request | 1001 ns | 750 ns |
| Overhead ratio | 2.0× | 1.5× |

This optimization is Clang-specific. MSVC does not currently support coroutine await elision, contributing to its 2× slower coroutine performance.

### 10.2 Compiler Differences

| Feature | Clang 20.x | MSVC 19.x |
|---------|-----------|-----------|
| `[[coro_await_elidable]]` | ✓ | ✗ |
| Frame elision (HALO) | Aggressive | Conservative |
| Symmetric transfer | Optimized | Less optimized |
| Overall coroutine speed | Faster | ~2× slower |

For performance-critical coroutine code, Clang currently provides superior optimization. MSVC's coroutine implementation continues to improve, but production code should account for this difference.

**A note on template coroutines:**

During development, we encountered a case where MSVC could not compile templated coroutines that use symmetric transfer:

```cpp
// Works on MSVC
inline task async_op(socket& s) { co_await s.async_io(); }

// Sometimes fails on MSVC with error C4737
template<class Stream>
task async_op(Stream& s) { co_await s.async_io(); }
```

The compiler emits error C4737 ("Unable to perform required tail call. Performance may be degraded") and treats it as a hard error, even though the code is valid C++20 and compiles successfully on Clang. The error originates in the code generator rather than the front-end, so `#pragma warning(disable: 4737)` has no effect.

This appears to be a [known issue from 2021](https://developercommunity.visualstudio.com/t/Coroutine-compilation-resulting-in-erro/1510427) that sometimes affects template instantiations involving symmetric transfer. Our non-template coroutines work correctly on both compilers, but users attempting to write generic coroutine adapters or stream wrappers should be aware of this limitation.

This raises a broader concern: C++20 coroutines are now five years post-standardization. If a major compiler still sometimes fails to compile valid coroutine code, it suggests that implementations may struggle to keep pace with language complexity. Features that outpace compiler support create fragmentation—code that works on one toolchain fails on another, forcing developers to choose between expressiveness and portability.

### 10.3 Implemented Mitigations

1. **Frame pooling** (Section 7): Custom `operator new/delete` with thread-local caching eliminates allocation overhead after warmup
2. **`[[clang::coro_await_elidable]]`**: Enables frame elision for nested coroutines on Clang
3. **Symmetric transfer** (Section 5.1): Returning handles from `await_suspend` prevents stack growth
4. **Preallocated I/O state** (Section 4.1): Socket operation state is allocated once, not per-operation
5. **Global frame pool fallback**: Coroutines without explicit frame allocator parameters still benefit from pooling

---

## 11. Design Trade-offs

| Aspect | Callback Model | Coroutine Model |
|--------|---------------|-----------------|
| API Complexity | High (templates everywhere) | Low (uniform `task` type) |
| Compile Time | Poor (deep template instantiation) | Good (type erasure at boundaries) |
| Runtime (shallow, Clang) | Excellent (~3-4 ns) | Moderate (~21-25 ns) |
| Runtime (deep, Clang) | ~500 ns for 100 ops | ~750 ns for 100 ops (1.5×) |
| Runtime (deep, MSVC) | ~670 ns for 100 ops | ~1750 ns for 100 ops (2.6×) |
| Allocations (optimized) | 0 (recycling allocator) | 0 (frame pooling) |
| Handle overhead | N/A | Zero (`handle<void>` = `handle<T>`) |
| Sender compatibility | N/A | Native (`dispatch().resume()` pattern) |
| Code Readability | Callback chains / state machines | Linear `co_await` sequences |
| Debugging | Stack traces fragmented | Stack traces fragmented |
| Encapsulation | Poor (leaky templates) | Excellent (hidden state) |
| Customization | Explicit allocator parameters | Trait-based detection |
| Compiler optimization | Uniform | Clang >> MSVC |
| I/O state location | Per-operation or preallocated | Preallocated (one indirection) |
| ABI stability | Types leak through templates | Stable (platform types hidden) |

---

## 12. Conclusion

We have demonstrated a coroutine-first asynchronous I/O framework that achieves:

1. **Clean public interfaces**: No templates, no allocators, just `task`
2. **Hidden platform types**: I/O state lives in translation units
3. **Flexible executors**: Any type satisfying `is_executor` works; composition happens at call sites
4. **Zero steady-state allocations**: Frame pooling and recycling eliminate allocation overhead
5. **Conscious encapsulation tradeoff**: One pointer indirection buys ABI stability, fast compilation, and readable interfaces

The affine awaitable protocol—injecting execution context through `await_transform`—provides the mechanism. Coroutines are not free, but for I/O-bound workloads where operations take microseconds, the 1.5-2.6× overhead vanishes into the noise.

The comparison with `std::execution` is instructive. That framework uses backward queries to discover execution context (`get_domain`, `get_completion_scheduler`) and requires elaborate machinery (P3826) to fix the resulting problems—problems networking doesn't have. Our design sidesteps these issues: context flows forward, I/O objects need no executor template parameters, and algorithm customization is unnecessary because networking has one implementation per platform.

This divergence suggests that **networking deserves first-class design consideration**, not adaptation to frameworks optimized for GPU workloads. The future of asynchronous C++ need not be a single universal abstraction—it may be purpose-built frameworks that excel at their primary use cases while remaining interoperable at the boundaries.

---

## References

1. [N4775](https://wg21.link/n4775) — C++ Extensions for Coroutines (Gor Nishanov)
2. [P0443R14](https://wg21.link/p0443r14) — A Unified Executors Proposal for C++ (Jared Hoberock, Michael Garland, Chris Kohlhoff, et al.)
3. [P2300R10](https://wg21.link/p2300) — std::execution (Michał Dominiak, Georgy Evtushenko, Lewis Baker, Lucian Radu Teodorescu, Lee Howes, Kirk Shoop, Eric Niebler)
4. [P2762R2](https://wg21.link/p2762) — Sender/Receiver Interface for Networking (Dietmar Kühl)
5. [P3552R3](https://wg21.link/p3552) — Add a Coroutine Task Type (Dietmar Kühl, Maikel Nadolski)
6. [P3826R2](https://wg21.link/p3826) — Fix or Remove Sender Algorithm Customization (Lewis Baker, Eric Niebler)
7. [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html) — Asynchronous I/O library (Chris Kohlhoff)
8. [Asymmetric Transfer](https://lewissbaker.github.io/) — Blog series on C++ coroutines (Lewis Baker)

---

## Acknowledgements

This paper builds on the foundational work of many contributors to C++ asynchronous programming:

**Dietmar Kühl** for P2762 and P3552, which explore sender/receiver networking and coroutine task types respectively. His clear articulation of design tradeoffs in P2762—including the late-binding problem and cancellation overhead concerns—helped crystallize our understanding of where the sender model introduces friction for networking.

**Lewis Baker** for his pioneering work on C++ coroutines, the Asymmetric Transfer blog series, and his contributions to P2300 and P3826. His explanations of symmetric transfer and coroutine optimization techniques directly informed our design.

**Eric Niebler** and the P2300 authors for developing the sender/receiver abstraction. While we argue that networking benefits from a different approach, the rigor of their work provided a clear baseline for comparison.

**Chris Kohlhoff** for Boost.Asio, which has served the C++ community for nearly two decades and established many of the patterns we build upon—and some we consciously depart from.

The analysis in this paper is not a critique of these authors' contributions, but rather an exploration of whether networking's specific requirements are best served by adapting to general-purpose abstractions or by purpose-built designs. We hope this work contributes constructively to that ongoing discussion.
