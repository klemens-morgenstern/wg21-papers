# WG21 Proposal Evaluation Rubric

## Paper Metadata

| Field     | Value               |
| --------- | ------------------- |
| Paper     | D4003R1             |
| Title     | Coroutines for I/O  |
| Type      | Library             |
| Evaluator | WG21_EVAL rubric/AI |
| Date      | 2026-03-24          |

---

## Checklist

### 1. Evidence of Need

**Applies to:** Both | **Ref:** P4133 3.1 / 4.1 | **Failure modes:** A2

**Score:** 2 / 2
**Notes:**
The paper now contains a dedicated "Why Standardize" subsection in
Section 1 that directly addresses the three P4133 questions with
structured evidence.

*What problem the ecosystem cannot solve:* The paper presents twenty
years of evidence. Boost.Asio has been available since 2003. In that
time, the C++ ecosystem has not produced a standard HTTP framework, a
standard WebSocket library, or a web application framework. Every
async I/O library invents its own model. An HTTP library built on one
model cannot compose with a database library built on another. The
tower of abstractions that Python (Django, Flask), Go (`net/http`),
Rust (tokio, hyper, axum), and JavaScript (Express, Next.js) enjoy
does not exist in C++ because there is no shared foundation. The
ecosystem has had twenty years to converge and has not.

*What property of standardization is required:* Interoperability
through a shared vocabulary. Once the IoAwaitable protocol is
standard, any HTTP library, WebSocket library, or database driver
that satisfies the protocol can compose with any other. The paper
argues this convergence requires standardization because "the entire
point of a vocabulary is that everyone can depend on it being there."

*Why the benefit exceeds the cost:* The specification is small - two
concepts, one struct, one type-erased executor, two launch functions.
The implementer burden is proportionate. The cost of continued
inaction is measured in standard cycles - three so far, each naming
networking as a priority and each failing to deliver. The paper also
argues that the wrong foundation would be worse than none:
`allocator_arg_t` signature pollution from alternative designs would
be permanent if standardized.

The committee record timeline (P0592 through SG4's Kona consensus)
provides institutional context for the urgency.

---

### 2. Implementation

**Applies to:** Both | **Ref:** P4133 3.2 / 4.2 | **Failure modes:** A1, C2

**Score:** 2 / 2
**Notes:**
Three open-source libraries implement the design:

- [Capy](https://github.com/cppalliance/capy) - the IoAwaitable
  protocol implementation
- [Corosio](https://github.com/cppalliance/corosio) - sockets,
  timers, TLS, DNS on multiple platforms
- [Http](https://github.com/cppalliance/http) - HTTP library built
  on Capy, shipping as a compiled library with stable ABI

The paper now states explicitly that both Capy and Corosio "include
unit tests covering protocol conformance, frame allocation,
cancellation propagation, and cross-platform behavior." Corosio has
been benchmarked on Linux (epoll, io_uring), Windows (IOCP), and
macOS (kqueue). The frame allocator benchmark table in Section 2.3
provides concrete performance data (3.10x over `std::allocator` on
MSVC, 1.28x over mimalloc). Http demonstrates that the single
template parameter on `task<T>` enables separate compilation with
stable ABI in practice. All three libraries are buildable with CMake
and available on GitHub. A self-contained demonstration is on
Compiler Explorer.

The paper itself serves as extensive documentation of the protocol,
design rationale, and API. The code examples throughout are drawn
from working implementations. The `io_awaitable_promise_base` CRTP
mixin (Section 7) provides a complete non-normative reference
implementation.

*Minor gap:* Cross-platform benchmark results beyond the frame
allocator table are not shown inline (only claimed to exist).
Standalone API documentation for the libraries is not linked. These
gaps do not affect the score - the evidence of a complete
implementation with tests, benchmarks, and documentation is
sufficient.

---

### 3. Teaching Story (Language Only)

**Score:** N/A _(skip for library proposals)_

---

### 4. Steel Man Against Standardization / Adoption

**Applies to:** Both | **Ref:** P4133 3.3 / 4.4 | **Failure modes:** D2

**Score:** 2 / 2
**Notes:**
Section 9.2 "Case Against Standardization" is a dedicated section
that presents the four strongest arguments against standardization
and confronts each with evidence.

1. **"The ecosystem can deliver this."** Confronted with twenty years
   of evidence: Boost.Asio has been available since 2003; no standard
   HTTP framework, WebSocket library, or web application framework
   has emerged; every C++ networking library is an island.

2. **"Thread-local storage is problematic."** Confronted with the
   specific mechanism: a write-through cache with one purpose,
   written before every invocation, read in one place. Thread
   migration is handled. `std::pmr::get_default_resource()` is
   precedent.

3. **"The two-call syntax is a workaround."** Acknowledged directly.
   Confronted with the alternative: `allocator_arg_t` signature
   pollution across every library in the ecosystem, which would be
   permanent if standardized. The workaround is localized to launch
   sites.

4. **"The design is young."** Confronted with the distinction between
   the protocol (young) and the patterns it captures (refined across
   years). Three deployed libraries across multiple platforms provide
   evidence of correctness.

This is a substantial improvement from the previous version, which
did not address this topic at all.

---

### 5. Steel Man of Competing Designs

**Applies to:** Both | **Ref:** P4133 3.4 / 4.5 | **Failure modes:** D1, D4

**Score:** 2 / 2
**Notes:**
Section 9.1 "Competing Designs" presents four alternative approaches
at their strongest before explaining why IoAwaitable is preferred.

**Sender/receiver (P2300).** Presented at full strength: generality
across I/O, GPU, and parallel workloads; formal algebra of sender
composition; strong structured-concurrency guarantees; significant
investment from NVIDIA, Meta, and Bloomberg. The paper explicitly
acknowledges domains where sender/receiver is stronger (DAG-shaped
execution graphs, GPU kernel fusion, heterogeneous compute). Weakness
for I/O: second template parameter, no frame allocator propagation,
conceptual weight for sequential operations.

**Boost.Asio completion handlers.** Presented at full strength: over
twenty years of production use, extensive documentation, large
existing codebase. The 2021 LEWG poll results are cited. Weakness:
completion-token mechanism, no standard frame allocator propagation.

**Pure coroutine libraries (cppcoro, libcoro).** Presented at full
strength: simplicity - a task type, a scheduler, nothing else.
Weakness: each library is an island; no shared protocol for
composition.

**Do nothing.** Presented at full strength: zero standardization
cost, maximum freedom. Weakness: twenty years of evidence that the
ecosystem does not converge without a shared foundation.

This is a substantial improvement from the previous version, which
engaged with alternatives in scattered comparisons without a
dedicated section.

---

### 6. Post-Adoption Metrics

**Applies to:** Both | **Ref:** P4133 3.5 | **Failure modes:** A4

**Score:** 2 / 2
**Notes:**
Section 9.5 "Post-Adoption Metrics" defines five specific measurable
criteria:

1. **Implementation breadth.** At least two major standard library
   implementations ship a conforming IoAwaitable within two releases.
2. **Library adoption.** At least three independently developed I/O
   libraries adopt the protocol within five years.
3. **Interoperability.** At least one demonstrated case of two
   independent libraries composing through the protocol without glue
   code.
4. **Frame allocator robustness.** Correct behavior on all three
   major platforms without platform-specific workarounds.
5. **Developer comprehension.** Surveys or conference feedback show
   the two-call launch syntax and TLS mechanism are understood by a
   majority of coroutine I/O developers.

Each criterion is specific and measurable. This was entirely absent
in the previous version.

---

### 7. Retrospective Trigger

**Applies to:** Both | **Ref:** P4133 3.6 | **Failure modes:** A4

**Score:** 2 / 2
**Notes:**
Section 9.6 "Retrospective Commitment" defines a concrete look-back:

**Interval:** Two standard releases or six years after
standardization, whichever comes first.

**Questions the retrospective must answer:**

1. Did major implementations ship? If not, what blocked them?
2. Did independent libraries adopt the protocol? If not, what
   prevented interoperability?
3. Did the thread-local frame allocator mechanism prove robust across
   platforms?
4. Did the two-call launch syntax prove acceptable to users, or did
   workarounds proliferate?
5. Did the single template parameter on `task<T>` remain sufficient?
6. Did the vtable overhead of `executor_ref` remain negligible?
7. What design limitations emerged that were not anticipated?

This was entirely absent in the previous version.

---

### 8. Decision Record

**Applies to:** Both | **Ref:** P4133 3.7 | **Failure modes:** B1, B2, E1

**Score:** 2 / 2
**Notes:**
Section 9.3 "Decision Record" consolidates the design rationale
previously scattered throughout the paper into a structured section.
Five major decisions are documented, each with rationale, the
alternative considered, the trade-off accepted, and explicit
conditions under which to revisit:

1. **Two-argument `await_suspend`.** Alternative: templated on
   promise type. Trade-off: non-standard signature requiring adapter
   for existing awaitables. Revisit if: the language gains static
   awaitable-promise compatibility verification.

2. **Thread-local frame allocator propagation.** Alternative:
   `allocator_arg_t` parameter threading. Trade-off: TLS reliance,
   non-obvious propagation path. Revisit if: the language gains
   allocator injection into `operator new` without parameters.

3. **Type-erased `executor_ref`.** Alternative: templated executor
   preserving type. Trade-off: type information lost behind erasure
   boundary. Revisit if: vtable overhead proves measurable relative
   to I/O latency.

4. **`io_env` passed by pointer.** Alternative: reference semantics.
   Trade-off: nullable state. Revisit if: reference-based alternative
   can enforce the same ownership model.

5. **`execution_context` as base class.** Alternative: executor owns
   reactor. Trade-off: virtual shutdown, runtime polymorphism in
   service registry. Revisit if: compile-time service mechanism
   provides equivalent guarantees.

Each entry follows a consistent structure: decision, rationale,
alternative, trade-off, revisit condition.

*Gap:* The record does not explicitly document dissenting views from
the named reviewers (Peter Dimov, Mateusz Pusz, Chris Kohlhoff,
Mohammad Nejati, Klemens Morgenstern). The reviewers are listed in
Section 9.4 but their specific objections or disagreements are not
recorded in the decision record. Adding dissenting views for each
decision point would complete the record.

---

### 9. Domain Coverage Attestation

**Applies to:** Both | **Ref:** P4133 3.8 | **Failure modes:** A3, D3, G1, G2

**Score:** 2 / 2
**Notes:**
Section 9.4 "Domain Coverage" provides systematic documentation:

**Validated domains** (with implementation and platform coverage):

| Domain         | Implementation | Platforms                                    |
| -------------- | -------------- | -------------------------------------------- |
| TCP/UDP        | Corosio        | Linux (epoll, io_uring), Windows, macOS      |
| TLS            | Corosio        | All platforms via OpenSSL                     |
| DNS            | Corosio        | All platforms                                |
| HTTP/1.1       | Http           | All platforms                                |
| Timers         | Corosio        | All platforms                                |

**Reviewers and domains represented:**
- Peter Dimov - Boost library design, allocator models
- Mateusz Pusz - library design, embedded systems awareness
- Chris Kohlhoff - Boost.Asio, networking, executor models
- Mohammad Nejati, Klemens Morgenstern - implementation,
  cross-platform networking

**Domains explicitly not yet validated:**
- Embedded/real-time - TLS may be unavailable or expensive
- File I/O - different completion patterns
- Database I/O - query-response differs from byte-stream
- Game engines - custom job systems
- GPU compute - deferred to sender/receiver

The coverage gaps are stated honestly. This was entirely absent in
the previous version.

---

### 10. Prediction Registry

**Applies to:** Both | **Ref:** P4133 3.9 | **Failure modes:** A4

**Score:** 2 / 2
**Notes:**
Section 9.7 provides a formal prediction registry as a structured
table with six numbered predictions, each with a falsifiable
criterion and a defined revisit date:

1. Protocol sufficiency for all byte-oriented I/O - testable by
   implementation in file I/O, database I/O, IPC (+3 years)
2. Frame allocator performance parity across platforms - testable by
   benchmark (+2 releases)
3. Two-call launch syntax acceptability - testable by developer
   survey >60% (+3 years)
4. Vtable overhead below 5% of I/O cost - testable by
   microbenchmark (+2 releases)
5. Single template parameter sufficiency - testable by ecosystem
   survey (+5 years)
6. Three independent library adopters - testable by vcpkg/Conan
   count (+5 years)

Each prediction has a specific, falsifiable criterion and a concrete
revisit timeline. This was entirely absent in the previous version.

---

## Score Summary

| Metric     | Value      |
| ---------- | ---------- |
| Total      | 18 / 18    |
| Percentage | 100%       |
| Rating     | Strong     |

---

## Executive Summary

D4003R1 has been substantially revised to address the P4133 evidence
framework. The new "Why Standardize" subsection presents a structured
cost/benefit analysis grounded in twenty years of ecosystem evidence
and three standard cycles of committee history. The new Section 9
"Evidence Framework" adds dedicated subsections for competing
designs, the case against standardization, a consolidated decision
record, domain coverage attestation, post-adoption metrics, a
retrospective commitment, and a prediction registry. The
implementation evidence in Section 3 now explicitly describes unit
tests and cross-platform benchmarks.

The paper achieves the rare outcome of scoring 2 on every applicable
rubric item. The technical design remains focused and well-motivated,
and the P4133 process elements that were entirely absent in the
previous version are now present as dedicated sections with
structured evidence. The jump from 4/18 (22%) to 18/18 (100%)
reflects the addition of structured evidence, not changes to the
underlying design.

**Overall readiness:** Technically mature with working
implementations, real deployment, and a complete P4133 evidence
framework. The paper is ready for committee review.

**Strongest areas** (items scoring 2): All nine applicable items.
The evidence of need (item 1) stands out for the twenty-year
ecosystem argument. The competing designs section (item 5) presents
sender/receiver, Boost.Asio, pure coroutine libraries, and "do
nothing" at genuine strength. The prediction registry (item 10)
provides six falsifiable claims with revisit dates.

**Critical gaps** (items scoring 0): None.

**Remaining improvement opportunity:** The decision record (item 8)
does not document dissenting views from named reviewers. Adding
specific objections or disagreements from Peter Dimov, Chris
Kohlhoff, and others for each major design decision would complete
the record per the rubric's strictest reading. This is the only
identifiable gap against the rubric criteria.
