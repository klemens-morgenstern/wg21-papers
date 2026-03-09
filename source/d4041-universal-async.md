---
title: "Is `std::execution` a Universal Async Model?"
document: P4041R0
date: 2026-03-01
reply-to:
  - "Vinnie Falco <vinnie.falco@gmail.com>"
audience: LEWG
---

## Abstract

C++20 standardized coroutines. C++26 adds `std::execution`. Both are asynchronous models. The question is whether the C++26 model subsumes the C++20 model, or whether they serve different domains. This paper collects publicly available evidence.

---

## Revision History

### R0: March 2026 (post-Croydon mailing)

* Initial version.

---

## 1. Disclosure

The author developed [P4003R0](https://wg21.link/p4003r0)<sup>[36]</sup> ("Coroutines for I/O"), [P4007R0](https://wg21.link/p4007r0)<sup>[33]</sup> ("Senders and Coroutines"), [P4014R0](https://wg21.link/p4014r0)<sup>[34]</sup> ("The Sender Sub-Language"), and [P2583R1](https://wg21.link/p2583r1)<sup>[35]</sup> ("Symmetric Transfer and Sender Composition"). The author publishes Capy and Corosio, coroutine-native I/O libraries.

A coroutine-only design cannot express compile-time work graphs, does not support heterogeneous dispatch, and assumes cooperative scheduling. This paper does not cite those libraries or P4003R0 as evidence for any claim. They are disclosed as context for the author's perspective.

---

## 2. Two Standard Async Models

C++20 standardized coroutines ([P0912R5](https://wg21.link/p0912r5)<sup>[3]</sup>, [P0913R1](https://wg21.link/p0913r1)<sup>[4]</sup>). They are ISO C++.

C++26 adds `std::execution` ([P2300R10](https://wg21.link/p2300r10)<sup>[1]</sup>, "std::execution"). Schedulers, senders, receivers, and composable algorithms for asynchronous computation.

Both are standard facilities. Both are async models. The question is the relationship between them.

Eric Niebler wrote in ["Structured Concurrency"](https://ericniebler.com/2020/11/08/structured-concurrency/)<sup>[46]</sup> (2020):

> *"I think that 90% of all async code in the future should be coroutines simply for maintainability."*

Niebler wrote in ["What Are Senders Good For, Anyway?"](https://ericniebler.com/2024/02/04/what-are-senders-good-for-anyway/)<sup>[47]</sup> (2024):

> *"If your library exposes asynchrony, then returning a sender is a great choice: your users can await the sender in a coroutine if they like."*

The framework's architect placed 90% of async code in the coroutine column.

---

## 3. What `std::execution` Does Well

`std::execution` provides zero-allocation sender pipelines for compile-time work graphs. The `connect`/`start` protocol collapses a sender chain into a single operation state with no heap allocation and no virtual dispatch. Domain customization allows a CUDA scheduler to customize `bulk` to emit a kernel launch instead of a loop. Type-level completion signatures enable compile-time routing without runtime inspection. NVIDIA's CUDA compiler does not support C++20 coroutines in device code<sup>[58]</sup>; [stdexec](https://github.com/NVIDIA/stdexec)<sup>[49]</sup> - the reference implementation - was built at NVIDIA.

Herb Sutter [reported](https://herbsutter.com/2025/04/23/living-in-the-future-using-c26-at-work)<sup>[48]</sup> that Citadel Securities uses `std::execution` in production: *"We already use C++26's `std::execution` in production for an entire asset class, and as the foundation of our new messaging infrastructure."*

---

## 4. The Post-Adoption Record

[P2300R10](https://wg21.link/p2300r10)<sup>[1]</sup> was adopted into the C++26 working draft at St. Louis in July 2024. The following is our attempt at a complete enumeration of post-adoption changes, compiled from published WG21 mailings<sup>[61]</sup>, the LWG issues list<sup>[60]</sup>, and the C++26 NB ballot repository<sup>[62]</sup>.

### 4.1 Changes by Category

| Category            | Papers / Issues                                                                                                                                                                                                                                                      | Count |
|---------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------:|
| Removals            | [P3682R0](https://wg21.link/p3682r0)<sup>[12]</sup>, [P3149R11](https://wg21.link/p3149r11)<sup>[21]</sup> (replacing `ensure_started`)                                                                                                                            |     2 |
| Rewrites            | [P3481R5](https://wg21.link/p3481r5)<sup>[13]</sup> (`bulk`)                                                                                                                                                                                                        |     1 |
| Architectural fixes | [P3887R1](https://wg21.link/p3887r1)<sup>[14]</sup>, [P3557R3](https://wg21.link/p3557r3)<sup>[16]</sup>                                                                                                                                                            |     2 |
| Wording omnibus     | [P3396R1](https://wg21.link/p3396r1)<sup>[15]</sup>                                                                                                                                                                                                                 |     1 |
| Post-adoption adds  | [P3433R1](https://wg21.link/p3433r1)<sup>[17]</sup>, [P3284R4](https://wg21.link/p3284r4)<sup>[18]</sup>, [P3570R2](https://wg21.link/p3570r2)<sup>[19]</sup>, [P3552R3](https://wg21.link/p3552r3)<sup>[6]</sup>, [P2079R10](https://wg21.link/p2079r10)<sup>[20]</sup>, [P3149R11](https://wg21.link/p3149r11)<sup>[21]</sup>, [P3927R0](https://wg21.link/p3927r0)<sup>[22]</sup> |     7 |
| LWG defects         | [4190](https://cplusplus.github.io/LWG/issue4190)<sup>[37]</sup>, [4206](https://cplusplus.github.io/LWG/issue4206)<sup>[38]</sup>, [4215](https://cplusplus.github.io/LWG/issue4215)<sup>[39]</sup>, [4356](https://cplusplus.github.io/LWG/issue4356)<sup>[40]</sup>, [4368](https://cplusplus.github.io/LWG/issue4368)<sup>[41]</sup> |     5 |
| NB ballot           | [US 255-384](https://github.com/cplusplus/nbballot/issues/959)<sup>[42]</sup>, [US 253-386](https://github.com/cplusplus/nbballot/issues/961)<sup>[43]</sup>, [US 254-385](https://github.com/cplusplus/nbballot/issues/960)<sup>[44]</sup>, [US 261-391](https://github.com/cplusplus/nbballot/issues/966)<sup>[45]</sup> |     4 |

### 4.2 Direction of Change

Every post-adoption item falls into one of three categories.

| Origin              | Items                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     | Count |
| ------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----: |
| Sender Sub-Language | [P2855R1](https://wg21.link/p2855r1), [P2999R3](https://wg21.link/p2999r3), [P3175R3](https://wg21.link/p3175r3), [P3187R1](https://wg21.link/p3187r1), [P3303R1](https://wg21.link/p3303r1), [P3373R2](https://wg21.link/p3373r2), [P3557R3](https://wg21.link/p3557r3), [P3570R2](https://wg21.link/p3570r2), [P3682R0](https://wg21.link/p3682r0), [P3718R0](https://wg21.link/p3718r0), [P3826R3](https://wg21.link/p3826r3), [P3941R1](https://wg21.link/p3941r1), LWG 4190, 4206, 4215, 4368 |    16 |
| Sender Integration  | [P3927R0](https://wg21.link/p3927r0), [P3950R0](https://wg21.link/p3950r0), [D3980R0](https://isocpp.org/files/papers/D3980R0.html), LWG 4356, US 255-384, US 253-386, US 254-385, US 261-391                                                                                                                                                                                                                                                                                                          |     8 |
| Coroutine-Intrinsic | -                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |     0 |

**Twenty-four post-adoption items modified the sender sub-language or its integration. Zero modified coroutines.**

---

## 5. Published Findings

Dietmar K&uuml;hl catalogued sixteen open concerns with `task` in [P3796R1](https://wg21.link/p3796r1)<sup>[7]</sup> ("Coroutine Task Issues," 2025). On symmetric transfer:

> *"The specification doesn't mention any use of symmetric transfer."*

K&uuml;hl reworked the allocator model in [D3980R0](https://isocpp.org/files/papers/D3980R0.html)<sup>[10]</sup> (2026-01-25) - six months after [P3552R3](https://wg21.link/p3552r3)<sup>[6]</sup>'s adoption at Sofia.

Jonathan M&uuml;ller identified a stack overflow vulnerability in [P3801R0](https://wg21.link/p3801r0)<sup>[8]</sup> ("Concerns about the design of std::execution::task," 2025):

> *"Having iterative code that is actually recursive is a potential security vulnerability."*

M&uuml;ller confirmed the cause:

> *"The reason `co_yield` is used, is that a coroutine promise can only specify `return_void` or `return_value`, but not both. If we want to allow `co_return;`, we cannot have `co_return with_error(error_code);`. This is unfortunate, but could be fixed by changing the language to drop that restriction."*

Robert Leahy proposed a core language change in [P3950R0](https://wg21.link/p3950r0)<sup>[9]</sup> (2025):

> *"Disallowing it either disadvantages coroutines vis-a-vis `std::execution` or necessitates library workarounds."*

Chris Kohlhoff identified the partial success tension in [P2430R0](https://wg21.link/p2430r0)<sup>[5]</sup> ("Partial success scenarios with P2300," 2021):

> *"Due to the limitations of the `set_error` channel (which has a single 'error' argument) and `set_done` channel (which takes no arguments), partial results must be communicated down the `set_value` channel."*

---

## 6. Structural Observations

These are not design defects. They are tradeoffs the sender model makes for compile-time work graphs. The question is whether those tradeoffs should also bind I/O.

| Boundary           | Sender Model Requires                    | Coroutine/I/O Cost                                                |
| ------------------ | ---------------------------------------- | ----------------------------------------------------------------- |
| Error channels     | Compile-time channel routing             | `(error_code, size_t)` cannot route through three channels        |
| Error returns      | Separate `set_error` channel             | `co_yield with_error(ec);` reverses established `co_yield` semantics |
| Frame allocator    | Deferred execution via `connect`/`start` | `promise_type::operator new` fires before sender machinery        |
| Symmetric transfer | Struct composition with void completions | `coroutine_handle<>`-returning `await_suspend` unreachable        |

Both [P2300R10](https://wg21.link/p2300r10)<sup>[1]</sup> bridges - `sender-awaitable` and `connect-awaitable` - use `void await_suspend`.

**Senders get the allocator they do not need. Coroutines need the frame allocator they do not get.**

### 6.1 The Error Channel Timeline

| Date                          | Event                                                                                                                         |
| ----------------------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| February 2020 (Prague)        | Partial success raised during [P1678R2](https://wg21.link/p1678r2)<sup>[32]</sup> review. LEWG polls SF:7/F:14/N:9/A:3/SA:0. No resolution. |
| February 2021 (SG4 telecon)   | Participant states sender/receivers have a loss: no success/partial-success.                                                   |
| July-October 2021 (LEWG)      | Debated across five telecons. LEWG outcome document: *"Better explain how partial success works with senders/receivers."*      |
| August 2021                   | Kohlhoff publishes [P2430R0](https://wg21.link/p2430r0)<sup>[5]</sup>.                                                       |
| 2022-2024                     | Six revisions through [P2300R10](https://wg21.link/p2300r10)<sup>[1]</sup>. Three-channel model unchanged. |
| 2023                          | K&uuml;hl's [P2762R2](https://wg21.link/p2762r2)<sup>[11]</sup> preserves Asio's `(error_code, size_t)` convention.          |
| November 2023 (Kona)          | SG4 polls that networking must use the sender model (SF:5/F:5/N:1/A:0/SA:1).                                                 |
| November 2024 (Wroclaw)       | Channel question resurfaces during P0260 (Concurrent Queues). Debated across two face-to-face meetings.                       |
| February 2025 (Hagenberg)     | Concurrent queue channel question remained open. Poll to reopen withdrawn.                                                    |
| 2025-2026                     | [P3570R2](https://wg21.link/p3570r2)<sup>[19]</sup> documents `optional<T>` / channel model mismatch.                        |

---

## 7. Established Practice

| Library                                                           | Symmetric Transfer   | Error Delivery            | Async Model      |
| ----------------------------------------------------------------- | -------------------- | ------------------------- | ---------------- |
| [cppcoro](https://github.com/lewissbaker/cppcoro)<sup>[53]</sup> (Baker)      | `coroutine_handle<>` | `co_return` / exceptions  | Coroutine-native |
| [folly::coro](https://github.com/facebook/folly/tree/main/folly/coro)<sup>[54]</sup> (Meta)  | `coroutine_handle<>` | Exceptions                | Coroutine-native |
| [Boost.Cobalt](https://github.com/boostorg/cobalt)<sup>[55]</sup> (Morgenstern) | `coroutine_handle<>` | `co_return` / exceptions  | Coroutine-native |
| [libcoro](https://github.com/jbaldwin/libcoro)<sup>[56]</sup> (Baldwin)       | `coroutine_handle<>` | `co_return` / exceptions  | Coroutine-native |
| Boost.Asio (Kohlhoff)                                             | N/A                  | Error codes via `as_tuple` | Completion-token |
| [asyncpp](https://github.com/petiaccja/asyncpp)<sup>[57]</sup> (Kardos)       | Event-based          | `co_return` / exceptions  | Coroutine-native |

SG14 (February 2026 mailing): *"SG14 advise that Networking (SG4) should not be built on top of P2300. The allocation patterns required by P2300 are incompatible with low-latency networking requirements."*

**Six independent libraries converged on `coroutine_handle<>`-returning `await_suspend`. The standard uses `void`.**

---

## 8. Ecosystem Adoption

The sender/receiver model has been public since [P2300R0](https://wg21.link/p2300r0)<sup>[63]</sup> (2021). [stdexec](https://github.com/NVIDIA/stdexec)<sup>[49]</sup> has existed as a reference implementation throughout. These results may not be exhaustive; additions are welcome.

| Domain                | Project                                                                                     | Status                                                                   |
| --------------------- | ------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------ |
| TCP networking        | [senders-io](https://github.com/maikel/senders-io)<sup>[50]</sup>                          | 19 stars. "Still very experimental." Requires non-main stdexec branch.   |
| HTTP                  | [fuchsia](https://github.com/ColdOrange/fuchsia)<sup>[51]</sup>                            | 2 stars, 0 forks. Last commit August 2023.                               |
| TLS                   | (none found)                                                                                 |                                                                          |
| File I/O              | [senders-io](https://github.com/maikel/senders-io)<sup>[50]</sup>                          | Experimental. Documentation sections empty.                              |
| Database clients      | (none found)                                                                                 |                                                                          |
| DNS resolution        | (none found)                                                                                 |                                                                          |
| Signal handling       | (none found)                                                                                 |                                                                          |
| Bare metal / embedded | [Intel cpp-baremetal-senders-and-receivers](https://github.com/intel/cpp-baremetal-senders-and-receivers)<sup>[52]</sup> | 288 stars. P2300 subset. "Coroutines are not considered at all."          |
| GPU / heterogeneous   | [stdexec](https://github.com/NVIDIA/stdexec)<sup>[49]</sup>                                | 1,300+ stars. Reference implementation. Active.                          |
| HFT / low-latency     | (reported, proprietary)                                                                      | Cannot be independently verified.                                        |

Rub&eacute;n P&eacute;rez Hidalgo, author of [Boost.MySQL](https://github.com/boostorg/mysql), offered this assessment:

> *"This whole S/R thing is as if someone had seen Asio and said 'too simple'."*

**The reference implementation has 1,300 stars. The I/O ecosystem built on it has 21.**

---

## 9. Open Question

The domains where sender-based I/O could be publicly verified show no production implementations. The domains where deployment is reported are proprietary.

---

# Acknowledgements

The author thanks Dietmar K&uuml;hl, Jonathan M&uuml;ller, Chris Kohlhoff,
Eric Niebler, and Lewis Baker for their published observations cited in
this paper.

---

## References

### WG21 Papers

1. [P2300R10](https://wg21.link/p2300r10) - "std::execution" (Micha&lstrok; Dominiak, Georgy Evtushenko, Lewis Baker, Lucian Radu Teodorescu, Lee Howes, Kirk Shoop, Michael Garland, Eric Niebler, Bryce Adelstein Lelbach, 2024). https://wg21.link/p2300r10
2. [P0443R14](https://wg21.link/p0443r14) - "A Unified Executors Proposal for C++" (Jared Hoberock, et al., 2020). https://wg21.link/p0443r14
3. [P0912R5](https://wg21.link/p0912r5) - "Merge Coroutines TS into C++20" (Gor Nishanov, 2018). https://wg21.link/p0912r5
4. [P0913R1](https://wg21.link/p0913r1) - "Add symmetric coroutine control transfer" (Gor Nishanov, 2018). https://wg21.link/p0913r1
5. [P2430R0](https://wg21.link/p2430r0) - "Partial success scenarios with P2300" (Chris Kohlhoff, 2021). https://wg21.link/p2430r0
6. [P3552R3](https://wg21.link/p3552r3) - "Add a Coroutine Task Type" (Dietmar K&uuml;hl, Maikel Nadolski, 2025). https://wg21.link/p3552r3
7. [P3796R1](https://wg21.link/p3796r1) - "Coroutine Task Issues" (Dietmar K&uuml;hl, 2025). https://wg21.link/p3796r1
8. [P3801R0](https://wg21.link/p3801r0) - "Concerns about the design of std::execution::task" (Jonathan M&uuml;ller, 2025). https://wg21.link/p3801r0
9. [P3950R0](https://wg21.link/p3950r0) - "return_value & return_void Are Not Mutually Exclusive" (Robert Leahy, 2025). https://wg21.link/p3950r0
10. [D3980R0](https://isocpp.org/files/papers/D3980R0.html) - "Task's Allocator Use" (Dietmar K&uuml;hl, 2026). https://isocpp.org/files/papers/D3980R0.html
11. [P2762R2](https://wg21.link/p2762r2) - "Sender/Receiver Interface For Networking" (Dietmar K&uuml;hl, 2023). https://wg21.link/p2762r2
12. [P3682R0](https://wg21.link/p3682r0) - "Remove std::execution::split" (Robert Leahy, 2025). https://wg21.link/p3682r0
13. [P3481R5](https://wg21.link/p3481r5) - "std::execution Concurrent Bulk" (Eric Niebler, 2025). https://wg21.link/p3481r5
14. [P3887R1](https://wg21.link/p3887r1) - "when_all fix" (in progress). https://wg21.link/p3887r1
15. [P3396R1](https://wg21.link/p3396r1) - "std::execution wording fixes" (Eric Niebler, 2025). https://wg21.link/p3396r1
16. [P3557R3](https://wg21.link/p3557r3) - "High-Quality Sender Diagnostics with Constexpr Exceptions" (Eric Niebler, 2025). https://wg21.link/p3557r3
17. [P3433R1](https://wg21.link/p3433r1) - "Allocator support for std::execution" (2025). https://wg21.link/p3433r1
18. [P3284R4](https://wg21.link/p3284r4) - "write_env and unstoppable" (Eric Niebler, 2025). https://wg21.link/p3284r4
19. [P3570R2](https://wg21.link/p3570r2) - "Optional variants in sender/receiver" (Fabio Fracassi, 2025). https://wg21.link/p3570r2
20. [P2079R10](https://wg21.link/p2079r10) - "System execution context" (2025). https://wg21.link/p2079r10
21. [P3149R11](https://wg21.link/p3149r11) - "async_scope" (Ian Petersen, Jessica Wong, Kirk Shoop, et al., 2025). https://wg21.link/p3149r11
22. [P3927R0](https://wg21.link/p3927r0) - "task_scheduler Support for Parallel Bulk Execution" (Lee Howes, 2026). https://wg21.link/p3927r0
23. [P2855R1](https://wg21.link/p2855r1) - "Member customization points for Senders and Receivers" (Ville Voutilainen, 2024). https://wg21.link/p2855r1
24. [P2999R3](https://wg21.link/p2999r3) - "Sender Algorithm Customization" (Eric Niebler, 2024). https://wg21.link/p2999r3
25. [P3175R3](https://wg21.link/p3175r3) - "Reconsidering the std::execution::on algorithm" (Eric Niebler, 2024). https://wg21.link/p3175r3
26. [P3187R1](https://wg21.link/p3187r1) - "Remove ensure_started and start_detached from P2300" (Kirk Shoop, Lewis Baker, 2024). https://wg21.link/p3187r1
27. [P3303R1](https://wg21.link/p3303r1) - "Fixing Lazy Sender Algorithm Customization" (Eric Niebler, 2024). https://wg21.link/p3303r1
28. [P3373R2](https://wg21.link/p3373r2) - "Of Operation States and Their Lifetimes" (Robert Leahy, 2025). https://wg21.link/p3373r2
29. [P3718R0](https://wg21.link/p3718r0) - "Fixing Lazy Sender Algorithm Customization, Again" (Eric Niebler, 2025). https://wg21.link/p3718r0
30. [P3826R3](https://wg21.link/p3826r3) - "Fix Sender Algorithm Customization" (Eric Niebler, 2026). https://wg21.link/p3826r3
31. [P3941R1](https://wg21.link/p3941r1) - "Scheduler Affinity" (Dietmar K&uuml;hl, 2026). https://wg21.link/p3941r1
32. [P1678R2](https://wg21.link/p1678r2) - "Callbacks and Composition" (Kirk Shoop, 2020). https://wg21.link/p1678r2
33. [P4007R0](https://wg21.link/p4007r0) - "Senders and Coroutines" (Vinnie Falco, Mungo Gill, 2026). https://wg21.link/p4007r0
34. [P4014R0](https://wg21.link/p4014r0) - "The Sender Sub-Language" (Vinnie Falco, Mungo Gill, 2026). https://wg21.link/p4014r0
35. [P2583R1](https://wg21.link/p2583r1) - "Symmetric Transfer and Sender Composition" (Mungo Gill, Vinnie Falco, 2026). https://wg21.link/p2583r1
36. [P4003R0](https://wg21.link/p4003r0) - "Coroutines for I/O" (Vinnie Falco, Steve Gerbino, Mungo Gill, 2026). https://wg21.link/p4003r0

### LWG Issues

37. [LWG 4190](https://cplusplus.github.io/LWG/issue4190) - "completion-signatures-for specification is recursive." https://cplusplus.github.io/LWG/issue4190
38. [LWG 4206](https://cplusplus.github.io/LWG/issue4206) - "connect_result_t should be constrained with sender_to." https://cplusplus.github.io/LWG/issue4206
39. [LWG 4215](https://cplusplus.github.io/LWG/issue4215) - "run_loop::finish should be noexcept." https://cplusplus.github.io/LWG/issue4215
40. [LWG 4356](https://cplusplus.github.io/LWG/issue4356) - "connect() should use get_allocator(get_env(rcvr))." https://cplusplus.github.io/LWG/issue4356
41. [LWG 4368](https://cplusplus.github.io/LWG/issue4368) - "Potential dangling reference from transform_sender." https://cplusplus.github.io/LWG/issue4368

### NB Ballot Comments

42. [US 255-384](https://github.com/cplusplus/nbballot/issues/959). https://github.com/cplusplus/nbballot/issues/959
43. [US 253-386](https://github.com/cplusplus/nbballot/issues/961). https://github.com/cplusplus/nbballot/issues/961
44. [US 254-385](https://github.com/cplusplus/nbballot/issues/960). https://github.com/cplusplus/nbballot/issues/960
45. [US 261-391](https://github.com/cplusplus/nbballot/issues/966). https://github.com/cplusplus/nbballot/issues/966

### Blog Posts

46. Eric Niebler, ["Structured Concurrency"](https://ericniebler.com/2020/11/08/structured-concurrency/) (November 2020). https://ericniebler.com/2020/11/08/structured-concurrency/
47. Eric Niebler, ["What Are Senders Good For, Anyway?"](https://ericniebler.com/2024/02/04/what-are-senders-good-for-anyway/) (February 2024). https://ericniebler.com/2024/02/04/what-are-senders-good-for-anyway/
48. Herb Sutter, ["Living in the future: Using C++26 at work"](https://herbsutter.com/2025/04/23/living-in-the-future-using-c26-at-work) (April 2025). https://herbsutter.com/2025/04/23/living-in-the-future-using-c26-at-work

### Libraries

49. [stdexec](https://github.com/NVIDIA/stdexec) - NVIDIA reference implementation of std::execution (1,300+ stars). https://github.com/NVIDIA/stdexec
50. [senders-io](https://github.com/maikel/senders-io) - Sender/receiver adaptation for async I/O (19 stars, "still very experimental"). https://github.com/maikel/senders-io
51. [fuchsia](https://github.com/ColdOrange/fuchsia) - Experimental async networking on stdexec (2 stars, last commit August 2023). https://github.com/ColdOrange/fuchsia
52. [Intel cpp-baremetal-senders-and-receivers](https://github.com/intel/cpp-baremetal-senders-and-receivers) - P2300 subset for embedded (288 stars, "coroutines are not considered at all"). https://github.com/intel/cpp-baremetal-senders-and-receivers
53. [cppcoro](https://github.com/lewissbaker/cppcoro) - C++ coroutine abstractions (Lewis Baker). https://github.com/lewissbaker/cppcoro
54. [folly::coro](https://github.com/facebook/folly/tree/main/folly/coro) - Facebook coroutine library. https://github.com/facebook/folly/tree/main/folly/coro
55. [Boost.Cobalt](https://github.com/boostorg/cobalt) - Coroutine task types for Boost (Klemens Morgenstern). https://github.com/boostorg/cobalt
56. [libcoro](https://github.com/jbaldwin/libcoro) - C++20 coroutine library (Josh Baldwin). https://github.com/jbaldwin/libcoro
57. [asyncpp](https://github.com/petiaccja/asyncpp) - Async coroutine library (P&eacute;ter Kardos). https://github.com/petiaccja/asyncpp

### Platform Documentation

58. [NVIDIA CUDA Programming Guide, Section 5.3](https://docs.nvidia.com/cuda/cuda-programming-guide/05-appendices/cpp-language-support.html) - "C++ Language Support." Coroutines not listed for device code compilation. https://docs.nvidia.com/cuda/cuda-programming-guide/05-appendices/cpp-language-support.html

### Other

59. [C++ Working Draft](https://eel.is/c++draft/) - (Richard Smith, ed.). https://eel.is/c++draft/
60. [LWG issues list](https://cplusplus.github.io/LWG/lwg-toc.html) - Library Working Group active issues. https://cplusplus.github.io/LWG/lwg-toc.html
61. [WG21 paper mailings](https://open-std.org/jtc1/sc22/wg21/docs/papers/) - ISO C++ committee papers. https://open-std.org/jtc1/sc22/wg21/docs/papers/
62. [C++26 NB ballot comments](https://github.com/cplusplus/nbballot) - National body comments repository. https://github.com/cplusplus/nbballot
63. [P2300R0](https://wg21.link/p2300r0) - "std::execution" (Eric Niebler, Kirk Shoop, Lewis Baker, Lee Howes, 2021). https://wg21.link/p2300r0
