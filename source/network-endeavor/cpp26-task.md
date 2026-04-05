# C++26 `std::execution::task` - Post-Croydon Inventory

Collected: 2026-04-05. Covers all approved changes to `std::execution::task` from initial plenary approval through the Croydon meeting.

## Timeline

- **2025-02 Hagenberg:** SG1 forwards P3552R0 to LEWG (7-0-0-1-0). Name changed to `task`.
- **2025-02 Hagenberg:** LEWG names it `std::execution::task` (10-13-1-0-0). Approves `affine_on`, non-exception errors.
- **2025-04 telecon:** LEWG approves design of P3552R1 (5-6-6-1-0).
- **2025-05 telecon:** Forwarded to LWG for C++29, weak consensus for C++26 if possible (5-3-4-1-0).
- **2025-06 Sofia:** LWG poll "Put P3552R3 into C++26" - 10 F, 0 N, 1 A. Plenary: 77-11 (29 abstain). **Approved for C++26.**
- **2025-07:** PR [#8032](https://github.com/cplusplus/draft/pull/8032) merged. P3552R3 is in the working draft. 867 additions, 3 files.
- **2025-08 telecon:** LEWG reviews P3796R1 + P3801R0 together. Stack overflow: no consensus. Dangling refs: consensus against fixing. Two minor fixes approved (noexcept, rvalue qual). Remaining items deferred.
- **2025-09 telecon:** LEWG approves dropping scheduler arg from `affine_on` (2-6-1-0-0). Encourages solving affine_on design issues for C++26 (4-6-0-0-0). Allocator source: strictly neutral (0-0-5-0-0).
- **2025-10:** NB comments filed. P3796R1 and P3801R0 labeled `nb-comment`.
- **2026-03 Croydon:** C++26 finalized. 411 NB comments resolved. Motions 28-38 affect execution/task. All approved at closing plenary.

## Croydon Motions Affecting Task

### Motion 28: P3826R5 - Fix Sender Algorithm Customization

- **Paper:** [P3826R5](https://wg21.link/p3826r5) (Eric Niebler)
- **Draft issue:** [#8862](https://github.com/cplusplus/draft/issues/8862)
- **NB comments:** [RO 4-395](https://github.com/cplusplus/nbballot/issues/970) (partial)
- **Status:** Open, milestone post-2026-03

**What it does:** Rewrites the sender algorithm customization mechanism. The core problem: P3718R0 left gaps in how domains dispatch sender algorithms. The fix introduces a two-phase dispatch in `connect` where the "starting domain" and "completing domain" are determined separately, and `transform_sender` is called with the completing domain's tag as a discriminator.

**Impact on task:**
- Changes how `task`'s `as_awaitable` locates customizations of nested senders
- Changes how `affine_on` dispatches through domains
- Introduces `get_completion_domain` query
- Removes `write_env` uses from `on(sndr, sch, closure)` (consistency with P3941)
- All sender algorithms that task interacts with (`then`, `let_value`, `bulk`, etc.) have their customization points rewritten

**Cost/tradeoff:** This is infrastructure. It does not change what task *does* but changes *how* task's sender interactions dispatch through domains. Libraries that customize sender algorithms (including NVIDIA's nvexec) must update their domain implementations.

---

### Motion 29: P3980R1 - Task's Allocator Use

- **Paper:** [P3980R1](https://wg21.link/p3980r1) (Dietmar Kuhl)
- **Draft issue:** [#8863](https://github.com/cplusplus/draft/issues/8863)
- **Draft PR:** [#8918](https://github.com/cplusplus/draft/pull/8918) (open, 45 add / 44 del)
- **Also fixes:** [#8917](https://github.com/cplusplus/draft/issues/8917) (`operator new` signature mismatch)
- **NB comments:**
  - [US 254-385](https://github.com/cplusplus/nbballot/issues/960) - Constrain `allocator_arg` to first position
  - [US 253-386](https://github.com/cplusplus/nbballot/issues/961) - Allow arbitrary allocators for coroutine frame
  - [US 255-384](https://github.com/cplusplus/nbballot/issues/959) - Use allocator from receiver's environment
  - [US 261-391](https://github.com/cplusplus/nbballot/issues/966) - Bad specification of parameter type
- **Status:** PR open, being applied

**What it does:** Two changes:

1. **`allocator_arg` must be first argument.** The previous spec allowed `allocator_arg` anywhere in the coroutine function's parameter list. Now it must be the first argument (or first after the implicit `this` for member functions). This aligns with `std::generator` and other standard library types that use `allocator_arg_t`.

2. **Environment allocator comes from the receiver, not the coroutine frame.** Previously, task used the *same* allocator for both the coroutine frame allocation and the child sender environment. Now these are separated:
   - Coroutine frame allocator: provided via `allocator_arg` at the coroutine call site (or default-constructed)
   - Child environment allocator: obtained from `get_allocator(get_env(rcvr))` when the task is `connect`ed

**The allocator API (post-P3980R1):**

```cpp
// promise_type synopsis (after P3980R1)
class task<T, Environment>::promise_type {
public:
    task get_return_object() noexcept;
    // NO constructor taking args - removed
    unspecified get_env() const noexcept;

    // Frame allocation: allocator_arg must come first
    void* operator new(size_t size);
    template<class Alloc, class... Args>
      void* operator new(size_t size,
        allocator_arg_t, Alloc alloc, Args&&... args);
    template<class This, class Alloc, class... Args>
      void* operator new(size_t size,
        const This&, allocator_arg_t, Alloc alloc, Args&&...);
    void operator delete(void* pointer, size_t size) noexcept;

private:
    // alloc member REMOVED - no longer stored in promise
    stop_source_type  source;
    stop_token_type   token;
    optional<T>       result;       // if !is_void_v<T>
    error-variant     errors;
};
```

```cpp
// How the user provides a frame allocator:
task<int> no_alloc(int x) { /* default allocator */ }

task<int> with_alloc(allocator_arg_t, auto a, int x) {
    /* frame allocated with 'a' */
}

// Member function form:
task<int> Foo::bar(allocator_arg_t, auto a, int x) {
    /* implicit 'this' before allocator_arg is fine */
}
```

```cpp
// get_env behavior change:
// BEFORE (P3552R3): env.query(get_allocator) returns alloc (stored member)
// AFTER  (P3980R1): env.query(get_allocator) returns
//   allocator_type(get_allocator(get_env(RCVR(*this))))
//   if well-formed, else allocator_type()
```

```cpp
// connect gains a Mandates clause:
template<receiver Rcvr>
  state<Rcvr> connect(Rcvr&& recv) &&;
// Mandates: at least one of these is well-formed:
//   allocator_type(get_allocator(get_env(rcvr)))
//   allocator_type()
```

**Cost/tradeoff:**
- Frame allocator and environment allocator are now independent - good for flexibility
- The `promise_type` constructor is **removed entirely** - allocator no longer stored in the promise
- The environment allocator is lazy - obtained from the receiver at query time, not cached
- `allocator_arg` first-position requirement matches `generator` but removes the "flexible position" convenience (trailing `auto&&...` trick no longer works for optional allocator)
- Any code that relied on the promise constructor receiving coroutine args will break

---

### Motion 33: P3941R4 - Scheduler Affinity

- **Paper:** [P3941R4](https://wg21.link/p3941r4) (Dietmar Kuhl)
- **Draft issue:** [#8867](https://github.com/cplusplus/draft/issues/8867)
- **NB comments:**
  - [US 232-366](https://github.com/cplusplus/nbballot/issues/941) - Customize `affine_on` when scheduler unchanged
  - [US 233-365](https://github.com/cplusplus/nbballot/issues/940) - Clarify `affine_on` vs `continues_on`
  - [US 234-364](https://github.com/cplusplus/nbballot/issues/939) - Remove scheduler parameter from `affine_on`
  - [US 235-363](https://github.com/cplusplus/nbballot/issues/938) - `affine_on` should not forward stop token
  - [US 236-362](https://github.com/cplusplus/nbballot/issues/937) - Specify default implementation of `affine_on`
- **LWG issues:** LWG4329, LWG4330, LWG4331, LWG4332, LWG4344
- **Status:** Open, milestone post-2026-03

**What it does:** Rewrites scheduler affinity from scratch. Five major changes:

1. **`affine_on` becomes unary.** Was `affine_on(sndr, sched)`, now `affine_on(sndr)`. The scheduler is obtained from `get_start_scheduler(get_env(rcvr))` at `connect` time. No more passing the scheduler explicitly.

2. **New query: `get_start_scheduler`.** Distinct from `get_scheduler`. Returns the scheduler an operation was *started on*, not the scheduler to schedule *new* work on. `task` propagates `get_start_scheduler` instead of `get_scheduler`. The type is renamed from `scheduler_type` to `start_scheduler_type`.

3. **Infallible scheduler requirement.** `affine_on` requires the scheduler to be infallible - `schedule(sched)` must complete with `set_value()` only (no `set_error`, no `set_stopped`) when given an `unstoppable_token`. This is checked statically. `inline_scheduler`, `task_scheduler`, and `run_loop::scheduler` are made infallible. `parallel_scheduler` probably cannot be.

4. **`change_coroutine_scheduler` removed.** The `co_await change_coroutine_scheduler(sch)` mechanism is removed entirely. It caused problems: local variables constructed before the change are destroyed on the wrong scheduler, and task must unconditionally store state for the possibility of a scheduler change. Alternative: `co_await starts_on(s, nested_task)`.

5. **`affine_on` customization via `transform_sender`.** Senders that don't change execution agent (e.g., `just`, `read_env`, `write_env`) can customize `affine_on` to avoid rescheduling. Implementation-specific property, with recommendation that standard senders define it.

**Cost/tradeoff:**
- `change_coroutine_scheduler` removal is a breaking API change from P3552R3
- `start_scheduler_type` replaces `scheduler_type` in the Environment
- Every algorithm that uses `get_scheduler` for affinity now uses `get_start_scheduler`
- `let_*`, `starts_on`, `sync_wait` all updated to propagate `get_start_scheduler`
- Libraries using `affine_on(sndr, sched)` must update to `affine_on(sndr)`
- `parallel_scheduler` cannot be used directly with `affine_on` / `task` (must be adapted)

---

### Motion 35: P3927R2 - `task_scheduler` Bulk Execution

- **Paper:** [P3927R2](https://wg21.link/p3927r2) (Eric Niebler)
- **Draft issue:** [#8869](https://github.com/cplusplus/draft/issues/8869)
- **Status:** Open, milestone post-2026-03

**What it does:** Makes `task_scheduler` support parallel `bulk` execution. Currently, wrapping a `parallel_scheduler` in a `task_scheduler` loses the parallelism - bulk work runs sequentially. Fix: `task_scheduler` now inherits from `parallel_scheduler_backend` instead of a plain `void`, gaining access to `schedule_bulk_chunked` and `schedule_bulk_unchunked`.

**Impact on task:**
- `task_scheduler`'s internal `sch_` changes from `shared_ptr<void>` to `shared_ptr<parallel_scheduler_backend>`
- `task_scheduler` gains a `ts-domain` with `transform_sender` that intercepts `bulk_chunked` and `bulk_unchunked` senders
- New exposition-only class `backend-for<Sch>` inherits `parallel_scheduler_backend`
- Implemented in NVIDIA stdexec and CCCL

**Cost/tradeoff:** Pure improvement for GPU/parallel use cases. No breaking changes to the task API itself. Adds complexity to `task_scheduler` internals.

---

### Motion 36: P4151R1 - Rename `affine_on`

- **Paper:** [P4151R1](https://wg21.link/p4151r1) (Robert Leahy)
- **Draft issue:** [#8870](https://github.com/cplusplus/draft/issues/8870)
- **LEWG poll (Croydon):** Forward to LWG for C++26: 7 SF, 7 F, 0 N, 0 A, 0 SA
- **Status:** Open, milestone post-2026-03

**What it does:** Renames `affine_on` to `affine`. Rationale: after P3941R4 made `affine_on` unary (no scheduler parameter), "on" no longer makes sense as a preposition - "affine on *what*?" All other `*_on` algorithms in `std::execution` take a scheduler parameter.

**Renames:**
- `std::execution::affine_on` -> `std::execution::affine`
- `std::execution::affine_on_t` -> `std::execution::affine_t`
- Member function `affine_on` -> `affine`

Applied after all other Croydon papers except P4159R0 and P4154R0.

---

### Motion 37: P4159R0 - Make `sender_in`/`receiver_of` exposition-only

- **Paper:** [P4159R0](https://wg21.link/p4159r0) (Tim Song et al.)
- **Draft issue:** [#8871](https://github.com/cplusplus/draft/issues/8871)
- **Status:** Open, milestone post-2026-03

**What it does:** Makes `sender_to` and `receiver_of` concepts exposition-only (`sender-to`, `receiver-of`). Response to LWG4361.

**Impact on task:** Indirect. These concepts constrain how senders connect to receivers. Task is a sender. The concepts still exist but are no longer part of the public API surface.

---

### Motion 38: P4154R0 - Renaming Various Execution Things

- **Paper:** [P4154R0](https://wg21.link/p4154r0) (Tim Song, Ruslan Arutyunyan, Arthur O'Dwyer)
- **Draft issue:** [#8872](https://github.com/cplusplus/draft/issues/8872)
- **NB comments:** [US 205-320](https://github.com/cplusplus/nbballot/issues/895), [RO 4-395](https://github.com/cplusplus/nbballot/issues/970) (partial)
- **Status:** Open, milestone post-2026-03

**What it does:** Bulk renames in clause 33 [exec]:
- `sender_t` -> `sender_tag`
- `scheduler_t` -> `scheduler_tag`
- `operation_state_t` -> `operation_state_tag`
- `receiver_t` -> `receiver_tag`
- `system_context_replaceability` namespace -> `parallel_scheduler_replacement`
- `sysctxrepl` in stable names -> `parschedrepl`

Applied last, after all other Croydon papers.

**Impact on task:** `task_scheduler` uses `scheduler_t` which becomes `scheduler_tag`. All concept tag references in task implementation change.

---

## Open Bugs

### #8917: `operator new` signature mismatch (P1-Important)

- **Issue:** [#8917](https://github.com/cplusplus/draft/issues/8917) (filed 2026-04-04)
- **Problem:** Definition in [task.promise] takes `const Args&...` but synopsis shows `Args&&...`
- **Fix:** Being resolved in PR #8918 (P3980R1 application)
- **Kuhl's comment:** "I didn't notice that the WP was already inconsistent and the mark-up in the paper doesn't show the change."

---

## Related Papers Not in Croydon Motions

### P3796R1 - Coroutine Task Issues (Kuhl)

- **Tracking:** [cplusplus/papers#2402](https://github.com/cplusplus/papers/issues/2402)
- **Labels:** LEWG, LWG, SG1, nb-comment, C++26
- **Milestone:** 2026-telecon
- **Status:** Open. The approved fixes (noexcept `unhandled_stopped`, rvalue qualification) were incorporated into P3941R4. Remaining items (stack overflow, dangling refs) deferred.

### P3801R0 - Concerns about `std::execution::task` (Muller)

- **Tracking:** [cplusplus/papers#2405](https://github.com/cplusplus/papers/issues/2405)
- **Labels:** LEWG, nb-comment, C++29
- **Milestone:** 2026-telecon
- **Status:** Open. Stack overflow (3.1): no consensus it's an issue. Dangling refs (3.3): consensus against fixing. Author suggested remaining topics for C++29.

---

## Summary: What Task Looks Like Post-Croydon

After all motions are applied, `std::execution::task` differs from P3552R3 in these ways:

| Aspect | P3552R3 (Sofia) | Post-Croydon |
| ------ | ---------------- | ------------ |
| Scheduler affinity mechanism | `affine_on(sndr, sched)` (binary) | `affine(sndr)` (unary, scheduler from `get_start_scheduler`) |
| Scheduler query | `get_scheduler` / `scheduler_type` | `get_start_scheduler` / `start_scheduler_type` |
| `change_coroutine_scheduler` | Present | **Removed** |
| Frame allocator position | `allocator_arg` anywhere | `allocator_arg` must be first |
| Environment allocator source | Same allocator as frame (stored in promise) | From `get_allocator(get_env(receiver))` at query time |
| `promise_type` constructor | Takes coroutine args, extracts allocator | **Removed** |
| `promise_type::alloc` member | Present (exposition-only) | **Removed** |
| `task_scheduler` backend | `shared_ptr<void>` | `shared_ptr<parallel_scheduler_backend>` |
| `task_scheduler` bulk | Sequential only | Parallel `bulk_chunked` / `bulk_unchunked` |
| Infallible scheduler req | Not specified | Required for `affine` (`set_value()` only with unstoppable token) |
| Concept tags | `sender_t`, `scheduler_t`, etc. | `sender_tag`, `scheduler_tag`, etc. |
| `sender_to` / `receiver_of` | Public concepts | Exposition-only |
| Algorithm customization | P3718R0-based | Rewritten (two-phase domain dispatch) |

**What did NOT change:** The `Environment` template parameter. The open query protocol. The `queryable = destructible` base. None of the Croydon motions address the structural interoperability risk documented in P4089.

---

## Links Index

### Papers
- [P3552R3](https://wg21.link/p3552r3) - Add a Coroutine Task Type (Kuhl, Nadolski)
- [P3826R5](https://wg21.link/p3826r5) - Fix Sender Algorithm Customization (Niebler)
- [P3927R2](https://wg21.link/p3927r2) - `task_scheduler` Bulk Execution (Niebler)
- [P3941R4](https://wg21.link/p3941r4) - Scheduler Affinity (Kuhl)
- [P3980R1](https://wg21.link/p3980r1) - Task's Allocator Use (Kuhl)
- [P4151R1](https://wg21.link/p4151r1) - Rename `affine_on` (Leahy)
- [P4154R0](https://wg21.link/p4154r0) - Renaming Various Execution Things (Song, Arutyunyan, O'Dwyer)
- [P4159R0](https://wg21.link/p4159r0) - Make `sender_in`/`receiver_of` exposition-only (Song et al.)
- [P3796R1](https://wg21.link/p3796r1) - Coroutine Task Issues (Kuhl)
- [P3801R0](https://wg21.link/p3801r0) - Concerns about `std::execution::task` (Muller)

### Draft PRs and Issues
- [#8032](https://github.com/cplusplus/draft/pull/8032) - P3552R3 merged (Sofia)
- [#8862](https://github.com/cplusplus/draft/issues/8862) - Motion 28: P3826R5
- [#8863](https://github.com/cplusplus/draft/issues/8863) - Motion 29: P3980R1
- [#8867](https://github.com/cplusplus/draft/issues/8867) - Motion 33: P3941R4
- [#8868](https://github.com/cplusplus/draft/issues/8868) - Motion 34: P3856R8 (reflection, not task)
- [#8869](https://github.com/cplusplus/draft/issues/8869) - Motion 35: P3927R2
- [#8870](https://github.com/cplusplus/draft/issues/8870) - Motion 36: P4151R1
- [#8871](https://github.com/cplusplus/draft/issues/8871) - Motion 37: P4159R0
- [#8872](https://github.com/cplusplus/draft/issues/8872) - Motion 38: P4154R0
- [#8917](https://github.com/cplusplus/draft/issues/8917) - Bug: `operator new` signature mismatch
- [#8918](https://github.com/cplusplus/draft/pull/8918) - PR: P3980R1 application

### NB Comments (private repo, links for reference)
- [US 205-320](https://github.com/cplusplus/nbballot/issues/895) - Concept tag naming
- [US 232-366](https://github.com/cplusplus/nbballot/issues/941) - `affine_on` customization
- [US 233-365](https://github.com/cplusplus/nbballot/issues/940) - `affine_on` vs `continues_on`
- [US 234-364](https://github.com/cplusplus/nbballot/issues/939) - Remove scheduler from `affine_on`
- [US 235-363](https://github.com/cplusplus/nbballot/issues/938) - Stop token forwarding
- [US 236-362](https://github.com/cplusplus/nbballot/issues/937) - `affine_on` default impl
- [US 253-386](https://github.com/cplusplus/nbballot/issues/961) - Arbitrary frame allocators
- [US 254-385](https://github.com/cplusplus/nbballot/issues/960) - `allocator_arg` position
- [US 255-384](https://github.com/cplusplus/nbballot/issues/959) - Receiver env allocator
- [US 261-391](https://github.com/cplusplus/nbballot/issues/966) - Parameter type spec
- [RO 4-395](https://github.com/cplusplus/nbballot/issues/970) - Namespace naming

### Committee Tracking
- [cplusplus/papers#2200](https://github.com/cplusplus/papers/issues/2200) - P3552 (closed, plenary-approved)
- [cplusplus/papers#2402](https://github.com/cplusplus/papers/issues/2402) - P3796R1 (open)
- [cplusplus/papers#2405](https://github.com/cplusplus/papers/issues/2405) - P3801R0 (open)
- [cplusplus/papers#2633](https://github.com/cplusplus/papers/issues/2633) - P3980R1

### LWG Issues
- [LWG4329](https://cplusplus.github.io/LWG/issue4329) - `affine_on` for other senders
- [LWG4330](https://cplusplus.github.io/LWG/issue4330) - `affine_on` semantics
- [LWG4331](https://cplusplus.github.io/LWG/issue4331) - `affine_on` shape
- [LWG4332](https://cplusplus.github.io/LWG/issue4332) - `affine_on` vs stop token
- [LWG4344](https://cplusplus.github.io/LWG/issue4344) - `affine_on` default spec
- [LWG4361](https://cplusplus.github.io/LWG/issue4361) - `awaitable-receiver::set_value` Mandates
