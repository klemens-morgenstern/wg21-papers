---
title: "Info: Open Issues in `std::execution::task`"
document: D4007R1
date: 2026-03-12
reply-to:
  - "Vinnie Falco <vinnie.falco@gmail.com>"
  - "Mungo Gill <mungo.gill@me.com>"
audience: LEWG
---

## Abstract

`std::execution::task` ([P3552R3](https://wg21.link/p3552r3)<sup>[1]</sup>) has open issues identified by national ballot comments, LWG issues, and published papers. This paper classifies each issue by whether it can be resolved after C++26 ships or whether shipping forecloses the fix.

---

## Revision History

### R1: March 2026 (prior to Croydon meeting)

* Complete rewrite as an informational classification of open issues.

### R0: February 2026 (pre-Croydon mailing)

* Original analysis of structural gaps. See [P4007R0](https://wg21.link/p4007r0)<sup>[2]</sup>.

---

## 1. Disclosure

The author provides information and serves at the pleasure of the committee.

This paper is part of the Network Endeavor, a project to bring coroutine-native byte-oriented I/O to C++.

The authors developed [P4007R0](https://wg21.link/p4007r0)<sup>[2]</sup> ("Senders and Coroutines"). The classification below holds regardless of whether any alternative design exists.

This paper asks for nothing.

---

## 2. Fixed After Ship

`task`'s `promise_type` is a class template instantiated in user code. Its `operator new`, allocator selection, stop token storage, environment forwarding, and destruction ordering can change between standard revisions without binary incompatibility. These issues are fixable post-ship: [Unusual Allocator Customisation](https://wg21.link/p3796r1)<sup>[3]</sup>, [Flexible Allocator Position](https://wg21.link/p3796r1)<sup>[3]</sup>, [Shadowing The Environment Allocator](https://wg21.link/p3796r1)<sup>[3]</sup>, [Stop Source Always Created](https://wg21.link/p3796r1)<sup>[3]</sup>, [Stop Token Default Constructible](https://wg21.link/p3796r1)<sup>[3]</sup>, [Task Not Actually Lazily Started](https://wg21.link/p3796r1)<sup>[3]</sup>, [Frame Destroyed Too Late](https://wg21.link/p3796r1)<sup>[3]</sup>, [No Default Arguments](https://wg21.link/p3796r1)<sup>[3]</sup>, [`unhandled_stopped` Not `noexcept`](https://wg21.link/p3796r1)<sup>[3]</sup>, [Environment Design Inefficient](https://wg21.link/p3796r1)<sup>[3]</sup>, [Non-Sender Awaitables Unsupported](https://wg21.link/p3796r1)<sup>[3]</sup>, [Future Language Feature Could Avoid `co_yield`](https://wg21.link/p3796r1)<sup>[3]</sup>, [No TLS Capture/Restore Hook](https://wg21.link/p3796r1)<sup>[3]</sup>, [`return_value`/`return_void` Have No Specification](https://wg21.link/p3796r1)<sup>[3]</sup>, [`co_return { args... }` Unsupported](https://wg21.link/p3796r1)<sup>[3]</sup>, [`change_coroutine_scheduler` Requires Assignable Scheduler](https://wg21.link/p3796r1)<sup>[3]</sup>, [Sender-Unaware Coroutines Cannot `co_await` a `task`](https://wg21.link/p3796r1)<sup>[3]</sup>, [Missing Rvalue Qualification](https://wg21.link/p3796r1)<sup>[3]</sup>, [Parameter Lifetime Is Surprising](https://wg21.link/p3801r0)<sup>[4]</sup>, [No Protection Against Dangling References](https://wg21.link/p3801r0)<sup>[4]</sup>, [`co_yield with_error` Is Clunky](https://wg21.link/p3801r0)<sup>[4]</sup>, [`co_await schedule(sch)` Is an Expensive No-Op](https://wg21.link/p3801r0)<sup>[4]</sup>, [Coroutine Cancellation Is Ad-Hoc](https://wg21.link/p3801r0)<sup>[4]</sup>.

`affine_on` semantics, rescheduling behavior, and algorithm customisation are specification-level concerns. Tightening requirements or adding default implementations does not change any published interface. These issues are fixable post-ship: [`affine_on` Default Implementation Lacks Specification](https://wg21.link/p3796r1)<sup>[3]</sup>, [`affine_on` Semantics Not Clear](https://wg21.link/p3796r1)<sup>[3]</sup>, [`affine_on` Shape May Not Be Correct](https://wg21.link/p3796r1)<sup>[3]</sup>, [`affine_on` Shouldn't Forward Stop Requests](https://wg21.link/p3796r1)<sup>[3]</sup>, [`affine_on` Customisation For Other Senders](https://wg21.link/p3796r1)<sup>[3]</sup>, [Starting a `task` Reschedules Unconditionally](https://wg21.link/p3796r1)<sup>[3]</sup>, [Resuming After a `task` Reschedules Unnecessarily](https://wg21.link/p3796r1)<sup>[3]</sup>, [`bulk` vs. `task_scheduler`](https://wg21.link/p3796r1)<sup>[3]</sup>, [No Completion Scheduler](https://wg21.link/p3796r1)<sup>[3]</sup>, [`with_awaitable_senders` Unused](https://wg21.link/p3796r1)<sup>[3]</sup>.

---

## 3. Not Fixable Post-Ship

The issues in this section are items where shipping forecloses the fix.

| Issue                 | References                                                                                                                                                                                                                                                                  | Fixed |
|-----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|:-----:|
| Allocator Timing      | [P3980R0](https://isocpp.org/files/papers/P3980R0.html)<sup>[5]</sup>, [P3796R1](https://wg21.link/p3796r1)<sup>[3]</sup>, [LWG 4356](https://cplusplus.github.io/LWG/issue4356)<sup>[6]</sup>, [US 254-385](https://github.com/cplusplus/nbballot/issues/960)<sup>[7]</sup> | no    |
| Allocator Propagation | [P3980R0](https://isocpp.org/files/papers/P3980R0.html)<sup>[5]</sup>, [P3796R1](https://wg21.link/p3796r1)<sup>[3]</sup>                                                                                                                                                  | no    |
| Error Return          | [P3950R0](https://wg21.link/p3950r0)<sup>[8]</sup>, [P3801R0](https://wg21.link/p3801r0)<sup>[4]</sup>, [P1713R0](https://wg21.link/p1713r0)<sup>[9]</sup>                                                                                                                | no    |
| Symmetric Transfer    | [P2583R3](https://isocpp.org/files/papers/P2583R3.pdf)<sup>[10]</sup>, [US 246-373](https://github.com/cplusplus/nbballot/issues/948)<sup>[11]</sup>, [LWG 4348](https://cplusplus.github.io/LWG/issue4348)<sup>[12]</sup>, [P3801R0](https://wg21.link/p3801r0)<sup>[4]</sup>, [P3796R1](https://wg21.link/p3796r1)<sup>[3]</sup> | no    |

- **Allocator Timing.** `task` does not allow the user to specify an allocator at the launch site for coroutine frame allocation. The coroutine frame is allocated by `operator new` at the call site, before any sender `connect`/`start` machinery runs. The receiver's environment - which carries the allocator - is not yet available at that point. The `allocator_arg` mechanism in [P3552R3](https://wg21.link/p3552r3)<sup>[1]</sup> works around this by requiring the allocator in the coroutine's parameter list, but that locks in a caller-specified approach and forecloses environment-based injection.

- **Allocator Propagation.** `task` does not propagate the frame allocator to child tasks. Each child task allocates its frame independently. Shipping without propagation locks in a design where every launch site must specify its allocator explicitly, foreclosing transparent propagation through the coroutine call tree.

- **Error Return.** `task` requires `co_yield with_error(e)` to propagate an error to the caller. `co_return` cannot carry an error value because `return_value` and `return_void` are mutually exclusive in the current coroutine specification. Shipping this interface locks in the `co_yield` mechanism and forecloses `co_return`-based error propagation, which would require a language change.

- **Symmetric Transfer.** The completion functions (`set_value`, `set_error`, `set_stopped`) and `start()` return `void`, providing no channel to propagate a `coroutine_handle<>`. When a sender completes synchronously, the receiver calls `handle.resume()` on the caller's stack, adding a frame per completion with no upper bound. Shipping this protocol forecloses the `coroutine_handle<>`-returning completion protocol that would enable symmetric transfer.

---

## Acknowledgements

The authors thank Andrzej Krzemie&nacute;ski for feedback that sharpened the scope of this revision. Thanks are also due to Dietmar K&uuml;hl, Michael Hava, Mark Hoemmen, Ian Petersen, and Ville Voutilainen for technical discussion that informed the analysis.

---

## References

Papers, issues, and ballot comments referenced in this document.

### WG21 Papers

1. [P3552R3](https://wg21.link/p3552r3) - "Add a Coroutine Task Type" (Dietmar K&uuml;hl, Maikel Nadolski, 2025). https://wg21.link/p3552r3
2. [P4007R0](https://wg21.link/p4007r0) - "Senders and Coroutines" (Vinnie Falco, Mungo Gill, 2026). https://wg21.link/p4007r0
3. [P3796R1](https://wg21.link/p3796r1) - "Coroutine Task Issues" (Dietmar K&uuml;hl, 2025). https://wg21.link/p3796r1
4. [P3801R0](https://wg21.link/p3801r0) - "Concerns about the design of std::execution::task" (Jonathan M&uuml;ller, 2025). https://wg21.link/p3801r0
5. [P3980R0](https://isocpp.org/files/papers/P3980R0.html) - "Task's Allocator Use" (Dietmar K&uuml;hl, 2026). https://isocpp.org/files/papers/P3980R0.html
6. [LWG 4356](https://cplusplus.github.io/LWG/issue4356) - "connect() should use get_allocator(get_env(rcvr))". https://cplusplus.github.io/LWG/issue4356
7. [US 254-385](https://github.com/cplusplus/nbballot/issues/960) - C++26 NB ballot comment. https://github.com/cplusplus/nbballot/issues/960
8. [P3950R0](https://wg21.link/p3950r0) - "return_value & return_void Are Not Mutually Exclusive" (Robert Leahy, 2025). https://wg21.link/p3950r0
9. [P1713R0](https://wg21.link/p1713r0) - "Allowing both co_return; and co_return value; in the same coroutine" (Lewis Baker, 2019). https://wg21.link/p1713r0
10. [P2583R3](https://isocpp.org/files/papers/P2583R3.pdf) - "Symmetric Transfer and Sender Composition" (Mungo Gill, Vinnie Falco, 2026). https://isocpp.org/files/papers/P2583R3.pdf
11. [US 246-373](https://github.com/cplusplus/nbballot/issues/948) - C++26 NB ballot comment. https://github.com/cplusplus/nbballot/issues/948
12. [LWG 4348](https://cplusplus.github.io/LWG/issue4348) - "task doesn't support symmetric transfer". https://cplusplus.github.io/LWG/issue4348
13. [P2300R10](https://wg21.link/p2300r10) - "std::execution" (Micha&lstrok; Dominiak et al., 2024). https://wg21.link/p2300r10

### Other

14. [C++ Working Draft](https://eel.is/c++draft/) - (Richard Smith, ed.). https://eel.is/c++draft/

