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

**Score:** 1 / 2
**Notes:**
The paper opens with a strong technical argument: C++20 coroutines
have five properties whose conjunction yields "the optimal basis for
byte-oriented I/O." Section 1 identifies four requirements every I/O
application shares (executor policy, stop signals, frame allocation,
context ownership). Section 2 presents benchmarks showing a recycling
frame allocator is 3.10x faster than `std::allocator` on MSVC and
1.28x faster than mimalloc. Section 5.2 makes a compelling ergonomic
case against `allocator_arg_t` pollution with side-by-side code
comparisons. Section 6 argues that coroutine frame allocation buys
the type erasure needed for ABI-stable compiled libraries with a
single template parameter.

However, the paper does not contain a structured cost/benefit analysis
answering the three P4133 questions: what problem the ecosystem cannot
solve (Capy and Corosio already exist on GitHub and could be
distributed via vcpkg), what property of standardization is required
(portability, guaranteed availability, ABI stability, teaching
curriculum), and why the benefit exceeds the cost (implementer burden,
ABI commitment, perpetual maintenance). The paper is written as a
research report, not a standardization proposal with a formal
evidence-of-need section. The standardization argument is implicit
rather than structured.

---

### 2. Implementation

**Applies to:** Both | **Ref:** P4133 3.2 / 4.2 | **Failure modes:** A1, C2

**Score:** 1 / 2
**Notes:**
The paper states it is "a research report drawn from working code: a
complete coroutine-only networking library." Three open-source
libraries implement the design:

- [Capy](https://github.com/cppalliance/capy) - the IoAwaitable
  protocol implementation
- [Corosio](https://github.com/cppalliance/corosio) - sockets,
  timers, TLS, DNS on multiple platforms
- [Http](https://github.com/cppalliance/http) - HTTP library built
  on Capy, shipping as a compiled library with stable ABI

The paper states both Capy and Corosio are "in active use." Frame
allocator benchmarks are provided with specific numbers on MSVC and
Apple clang. A self-contained demonstration is available on
[Compiler Explorer](https://godbolt.org/z/Wzrb7McrT). The paper
includes extensive real-world code examples: route handlers, HTTP
redirects, type-erased streams (Appendix B), and the complete
`io_awaitable_promise_base` mixin (Section 7).

However, the paper does not present evidence of unit test coverage,
documentation, or benchmarks beyond the frame allocator performance
table. The implementations appear substantial but the paper itself
does not demonstrate the full evidence set (benchmarks + unit tests +
documentation) that the rubric requires for a score of 2.

---

### 3. Teaching Story (Language Only)

**Score:** N/A _(skip for library proposals)_

---

### 4. Steel Man Against Standardization / Adoption

**Applies to:** Both | **Ref:** P4133 3.3 / 4.4 | **Failure modes:** D2

**Score:** 0 / 2
**Notes:**
The paper does not present or confront the strongest argument against
standardization. The three reference implementations (Capy, Corosio,
Http) already exist as open-source libraries and could be distributed
through package managers without incurring the costs of
standardization. The paper does not address: why an ecosystem-only
approach is insufficient; whether the ABI commitment of
standardization is justified for a design this new; whether the
implementer burden across three major standard library implementations
is proportionate to the benefit; or whether the protocol's reliance
on thread-local storage, two-call syntax, and a specific executor
model would constrain future evolution once standardized. Section 9
(Suggested Straw Polls) engages the committee process but does not
confront the case against standardization.

---

### 5. Steel Man of Competing Designs

**Applies to:** Both | **Ref:** P4133 3.4 / 4.5 | **Failure modes:** D1, D4

**Score:** 1 / 2
**Notes:**
The paper engages with specific design alternatives in focused
comparisons. Section 3.1 presents an alternative `await_suspend`
design (templated on promise type) and explains its timing problem
with protocol mismatches becoming silent runtime errors. Section 5.2
extensively compares `allocator_arg_t` parameter threading against
thread-local propagation with side-by-side code examples. Section 9
references the Kona poll on sender/receiver for networking. Companion
papers [P4007R0] and [P4014R0] are referenced for the relationship
to sender/receiver.

However, the paper lacks a dedicated section presenting each
competing design at its strongest. Missing steel men include:
sender/receiver (P2300) as the committee-adopted async framework
(its advantages: generality, composability, multi-domain coverage),
Boost.Asio's completion handler model (20+ years of deployment,
large existing codebase), pure coroutine libraries like cppcoro or
libcoro (simpler models without the IoAwaitable protocol overhead),
and the "do nothing" option (leave coroutine I/O execution models
to the ecosystem). Each alternative has genuine advantages that the
paper does not present at full strength before explaining why
IoAwaitable is preferred.

---

### 6. Post-Adoption Metrics

**Applies to:** Both | **Ref:** P4133 3.5 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:**
No success criteria are defined. The paper does not state what would
constitute successful adoption - for example: implementation by major
standard library vendors within a defined timeframe, adoption rate
among networking library authors, reduction in the number of
incompatible coroutine I/O models, teaching difficulty as reported by
educators, or performance parity with hand-tuned async code in
production deployments. The frame allocator benchmarks demonstrate
current performance but do not define target metrics for post-adoption
evaluation.

---

### 7. Retrospective Trigger

**Applies to:** Both | **Ref:** P4133 3.6 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:**
No retrospective plan. The paper does not commit to a look-back
interval or define questions for future evaluation. Given that the
design depends on thread-local storage (a mechanism with known
concerns, addressed in Section 5.5 but not yet validated at
ecosystem scale), a retrospective commitment is particularly
important. Questions that should be answered after adoption include:
did the thread-local frame allocator mechanism prove robust across
all major implementations? Did the two-call launch syntax
(`run_async(ex)(task())`) prove acceptable to users? Did the single
template parameter on `task<T>` remain sufficient as the ecosystem
grew? Did the IoAwaitable protocol achieve interoperability across
independently developed libraries?

---

### 8. Decision Record

**Applies to:** Both | **Ref:** P4133 3.7 | **Failure modes:** B1, B2, E1

**Score:** 1 / 2
**Notes:**
The paper embeds substantial design rationale throughout. Key
documented decisions include:

- Two-argument `await_suspend` over templated promise access, with
  analysis of the timing and safety trade-offs (Section 3.1)
- Thread-local frame allocator propagation over `allocator_arg_t`
  parameter threading, with extensive side-by-side comparisons
  (Sections 5.2-5.5)
- Type-erased `executor_ref` over templated executors, enabling a
  single template parameter on `task<T>` (Sections 3.3, 6.3)
- `dispatch` returning `coroutine_handle<>` for symmetric transfer
  rather than resuming inline (Section 4.1)
- `execution_context` as a base class with service management
  (Section 4.3)
- Pointer semantics for `io_env` to make ownership explicit and
  prevent accidental copies (Section 3.1)

However, the paper lacks a formal consolidated decision record.
Missing elements: alternatives considered and rejected at each major
design fork presented in one place; dissenting views from reviewers
(the acknowledgements mention feedback from Peter Dimov and Mateusz
Pusz but do not record disagreements); trade-offs consciously
accepted (the two-call syntax is acknowledged as "unfortunate" but
the broader trade-off space is not documented); and conditions under
which to revisit (e.g., if a language change to `operator new`
timing made `allocator_arg_t` viable, would the TLS approach be
reconsidered?).

---

### 9. Domain Coverage Attestation

**Applies to:** Both | **Ref:** P4133 3.8 | **Failure modes:** A3, D3, G1, G2

**Score:** 0 / 2
**Notes:**
The paper does not document which domains were represented in
review. The design was validated in networking I/O (sockets, timers,
TLS, DNS, HTTP) and the author list (Vinnie Falco, Steve Gerbino,
Mungo Gill) reflects networking expertise. The acknowledgements
mention feedback from Peter Dimov and Mateusz Pusz without
specifying the domains they represented.

Notable domains not discussed: embedded/real-time systems (where
thread-local storage may be unavailable or expensive), game engine
async models (which use custom job systems with different
constraints), database I/O (where query-completion patterns differ
from byte-stream I/O), file I/O (where the IoAwaitable model may
or may not apply), and GPU compute (where the paper explicitly
defers to sender/receiver via companion papers). The paper positions
IoAwaitable as specific to I/O rather than general-purpose async,
but does not systematically enumerate which I/O domains were
consulted or which remain unvalidated.

---

### 10. Prediction Registry

**Applies to:** Both | **Ref:** P4133 3.9 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:**
No falsifiable claims are recorded. The paper implicitly predicts
that: the IoAwaitable protocol is sufficient for all byte-oriented
I/O (sockets, TLS, DNS, HTTP, and beyond); the thread-local frame
allocator mechanism will prove robust across all major compilers and
platforms; the two-call launch syntax will be accepted by users; a
single template parameter on `task<T>` will remain sufficient as
the ecosystem grows; and the vtable cost of `executor_ref` (1-2
nanoseconds) will remain negligible relative to I/O latency. None
of these predictions are recorded with falsifiable criteria or
revisit dates.

---

## Score Summary

| Metric     | Value      |
| ---------- | ---------- |
| Total      | 4 / 18     |
| Percentage | 22%        |
| Rating     | Needs Work |

---

## Executive Summary

D4003R1 proposes the IoAwaitable protocol, a coroutine-native
execution model for I/O built on C++20 language features. The paper
is a research report drawn from working code: three open-source
libraries (Capy, Corosio, Http) that implement the protocol and
provide sockets, timers, TLS, DNS, and HTTP on multiple platforms.
The technical design is focused and well-motivated, with a small
conceptual surface (two concepts, a type-erased executor, and a
thread-local frame allocator cache) that produces a complete
networking stack. The proposed wording in Section 10 demonstrates
a lean specification footprint.

However, the paper scores low on the P4133 rubric because it is
written as a research report rather than as a standardization
proposal with the structured evidence framework P4133 requires. The
technical content is strong - the design rationale is thorough, the
implementation exists and is deployed, and the ergonomic comparisons
are compelling - but the P4133 process elements (structured
cost/benefit, steel-man arguments, metrics, retrospective
commitments, prediction registry) are absent.

**Overall readiness:** Technically mature with working implementations
and real deployment. The P4133 evidence framework is almost entirely
absent. Adding the missing process sections would strengthen the
paper significantly without requiring any design changes.

**Strongest areas** (items scoring 2): None. Items 1, 2, 5, and 8
scored 1, reflecting substantial but incomplete coverage. The
embedded design rationale (item 8) and the real implementation
(item 2) are close to the threshold for 2.

**Critical gaps** (items scoring 0): Steel man against
standardization (item 4), post-adoption metrics (item 6),
retrospective trigger (item 7), domain coverage attestation (item 9),
prediction registry (item 10). Five of nine applicable items are
absent.

---

## Improvement Tips

### 1. Evidence of Need (P4133 3.1)

**Scored 1.** Add a dedicated section answering three questions
explicitly. What problem does the ecosystem not solve (answer: Capy
and Corosio exist but a standardized IoAwaitable concept enables
interoperability between independently developed I/O libraries - an
HTTP library and a database library need to compose their async
operations through a shared protocol, which requires standardization
of the concepts and `io_env` struct). What property of
standardization is required (answer: a guaranteed-available protocol
definition that every coroutine I/O library can target without adding
a third-party dependency, enabling compiled libraries with stable ABI
across vendors). Why the benefit exceeds the cost (answer: the
specification footprint is small - two concepts, one struct, one
type-erased executor, two launch functions - so implementer burden is
proportionate; quantify the cost of incompatible coroutine I/O models
and compare it to this small standardization cost).

### 2. Implementation (P4133 3.2)

**Scored 1.** The implementations exist and are substantial. To reach
a 2, present evidence of unit test coverage, documentation, and
benchmarks beyond the frame allocator table inline in the paper.
Consider adding: a summary of test coverage for Capy and Corosio
(number of test cases, platform coverage), a performance comparison
against Boost.Asio for a standard benchmark (echo server throughput,
connection handling latency), and a link to API documentation.

### 4. Steel Man Against Standardization (P4133 3.3)

**Scored 0.** Add a dedicated section presenting the strongest
argument against standardization: "Capy, Corosio, and Http already
exist as open-source libraries. Any project that needs IoAwaitable
can depend on them directly. Standardization commits three major
implementations to supporting a protocol that relies on thread-local
storage - a mechanism that may not be available or performant on all
target platforms (embedded, GPU, WebAssembly). The two-call launch
syntax is a workaround for a language limitation that a future
coroutine language evolution could eliminate, at which point the
standardized API would be permanently burdened with the workaround.
The design is only six months old and has not been tested at ecosystem
scale." Confront each point with evidence.

### 5. Steel Man of Competing Designs (P4133 3.4)

**Scored 1.** Present each competing model at its strongest in a
single dedicated section. Sender/receiver (P2300): general-purpose,
covers GPU and I/O, already adopted into C++26, large ecosystem
investment. Boost.Asio completion handlers: 20+ years of deployment,
proven at scale, existing codebases do not need migration. Pure
coroutine libraries (cppcoro, libcoro): simpler models with no
protocol overhead, lower learning curve. The "do nothing" option: the
ecosystem already has working coroutine I/O libraries. For each,
document its advantages honestly before explaining why IoAwaitable is
the better standard choice.

### 6. Post-Adoption Metrics (P4133 3.5)

**Scored 0.** Define measurable success criteria. Examples: at least
two major standard library implementations ship a conforming
IoAwaitable within two releases; at least three independently
developed I/O libraries adopt the protocol within five years;
developer surveys show the two-call launch syntax is understood by
a defined percentage of C++ developers who use coroutines; the
thread-local frame allocator achieves the benchmarked performance on
all three major platforms.

### 7. Retrospective Trigger (P4133 3.6)

**Scored 0.** Commit to a look-back at two releases or six years
after standardization. Define the questions: did the thread-local
frame allocator mechanism prove robust? Did the two-call syntax
prove acceptable? Did the single template parameter on `task<T>`
remain sufficient? Did independently developed libraries achieve
interoperability through the protocol? Did the vtable overhead of
`executor_ref` remain negligible? What design limitations emerged
in practice?

### 8. Decision Record (P4133 3.7)

**Scored 1.** Consolidate the design rationale scattered throughout
the paper into a single section. For each major decision, document:
the alternatives considered, the advantages of the rejected
alternatives, why this design was chosen, dissenting views from
reviewers, and conditions under which the decision should be
revisited. The existing rationale is strong - it needs consolidation,
not creation.

### 9. Domain Coverage Attestation (P4133 3.8)

**Scored 0.** Document which domains were represented in review.
List the I/O domains affected by the proposal (TCP/UDP networking,
TLS, DNS, HTTP, file I/O, database I/O, serial/USB I/O, IPC) and
state which have provided input and which have not. Acknowledge
gaps explicitly - particularly embedded systems (where TLS may not
be available), file I/O (where completion patterns differ), and
database I/O (where query models differ from byte-stream models).

### 10. Prediction Registry (P4133 3.9)

**Scored 0.** Record the paper's implicit predictions with
falsifiable criteria. "The IoAwaitable protocol is sufficient for
all byte-oriented I/O domains" (testable by implementation attempts
in file I/O, database I/O, and IPC). "The thread-local frame
allocator achieves within 10% of the benchmarked performance on
all major platforms" (testable by benchmark). "The two-call launch
syntax will not be a barrier to adoption" (testable by user survey).
"The vtable overhead of executor_ref remains below 5% of total I/O
operation cost" (testable by benchmark comparison). Each claim needs
a revisit date.
