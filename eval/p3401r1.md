# WG21 Proposal Evaluation Rubric

## Paper Metadata

| Field     | Value                                                        |
| --------- | ------------------------------------------------------------ |
| Paper     | P3401R1                                                      |
| Title     | Proxy Creation Facilities: Enriching Proxy Construction for Pointer-Semantics Polymorphism |
| Type      | Library                                                      |
| Evaluator | AI (WG21_EVAL.md rubric)                                     |
| Date      | 2026-03-24                                                   |

---

## Checklist

### 1. Evidence of Need

**Applies to:** Both | **Ref:** P4133 3.1 / 4.1 | **Failure modes:** A2

**Score:** 1 / 2
**Notes:** Section 2.1.1 ("Problems addressed") enumerates five concrete
problems users face today (adaptive value factory, forced in-place
constructor, custom allocator path, compact shared ownership handle,
view wrapper) and identifies risks such as ABI drift and silent
allocation differences. The motivation is real and specific. However
the paper lacks a structured cost/benefit analysis answering the three
P4133 questions: what problem the ecosystem cannot solve on its own,
what property of standardization is required, and why the benefit
exceeds the cost of implementer burden, ABI commitment, and perpetual
maintenance. The existing discussion reads as informal motivation
rather than a dedicated evidence-of-need section.

---

### 2. Implementation

**Applies to:** Both | **Ref:** P4133 3.2 / 4.2 | **Failure modes:** A1, C2

**Score:** 0 / 2
**Notes:** The paper does not reference a standalone implementation,
link to a reference library, or present benchmarks, unit tests, or
documentation. Section 3.1 mentions that `make_proxy` "mirrors existing
practice in widely deployed implementations," but this refers to SBO
heuristics in the ecosystem generally - not to an implementation of this
paper's proposed factories. The Microsoft proxy library
(github.com/microsoft/proxy) ships some of these names, but the paper
does not cite it as a reference implementation or demonstrate test
coverage or benchmark results for the proposed API surface.

---

### 3. Teaching Story (Language Only)

**Score:** N/A (skip for library proposals)

---

### 4. Steel Man Against Standardization / Adoption

**Applies to:** Both | **Ref:** P4133 3.3 / 4.4 | **Failure modes:** D2

**Score:** 0 / 2
**Notes:** The paper does not contain a section presenting the strongest
argument for why these factories should remain in the ecosystem rather
than be standardized. The "Non-goals" subsection (2.1.2) scopes out
unrelated concerns but does not confront the central counter-argument:
that users who already use the Microsoft proxy library (or other
implementations) have working factories today, and standardization
imposes ABI commitment and maintenance burden for what may be a thin
convenience layer over `proxy`'s existing constructors. This argument
is never raised or defeated.

---

### 5. Steel Man of Competing Designs

**Applies to:** Both | **Ref:** P4133 3.4 / 4.5 | **Failure modes:** D1, D4

**Score:** 1 / 2
**Notes:** Section 3 ("Considerations and design decisions") discusses
several design choices: automatic vs explicit storage (3.1), unified
concept vs scattered constraints (3.2), compact shared handle vs
`shared_ptr` layering (3.3), and dedicated view creation (3.4). These
provide useful rationale for the chosen design. However, competing
designs are not presented at their strongest. For example, the
`shared_ptr`-based alternative is dismissed for adding "a separately
allocated control block" without quantifying the overhead or
acknowledging scenarios where `shared_ptr` interop is advantageous. The
option of doing nothing (leaving factories to the ecosystem) is never
presented as a competing design. The treatment is partial rather than
systematic.

---

### 6. Post-Adoption Metrics

**Applies to:** Both | **Ref:** P4133 3.5 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:** No success criteria are defined. The paper does not specify
how to measure whether the factories achieve their goals after
standardization - no adoption rate targets, no defect rate thresholds,
no survey plans, no benchmarks against ad-hoc alternatives.

---

### 7. Retrospective Trigger

**Applies to:** Both | **Ref:** P4133 3.6 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:** No retrospective plan is included. There is no defined
interval for revisiting the design and no explicit questions a
retrospective should answer.

---

### 8. Decision Record

**Applies to:** Both | **Ref:** P4133 3.7 | **Failure modes:** B1, B2, E1

**Score:** 1 / 2
**Notes:** Section 3 serves as a partial decision record. It explains
why automatic storage selection was chosen (3.1), why a unified concept
improves diagnostics (3.2), why compact handles are preferred over
`shared_ptr` layering (3.3), why view creation deserves a dedicated
function (3.4), and the freestanding split rationale (3.5). Missing
elements: alternatives considered are not enumerated systematically,
no dissenting views are documented, trade-offs are stated but not
weighed, and no conditions for revisiting the decisions are given.

---

### 9. Domain Coverage Attestation

**Applies to:** Both | **Ref:** P4133 3.8 | **Failure modes:** A3, D3, G1, G2

**Score:** 0 / 2
**Notes:** The paper does not mention which domains were consulted or
represented during the design review. Pointer-semantics polymorphism
affects a wide range of domains (embedded, games, HPC, finance,
networking), and none are attested as having provided input.

---

### 10. Prediction Registry

**Applies to:** Both | **Ref:** P4133 3.9 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:** The paper makes several implicit predictions - that
centralizing factories will eliminate duplicated wrappers, improve
portability, and clarify performance contracts - but none are recorded
with falsifiable criteria or revisit dates.

---

## Score Summary

| Metric     | Value       |
| ---------- | ----------- |
| Total      | 3 / 18      |
| Percentage | 17%         |
| Rating     | Needs Work  |

Rating tiers:

| Range   | Rating     | Meaning                                  |
| ------- | ---------- | ---------------------------------------- |
| 80-100% | Strong     | Paper meets the ideal model              |
| 50-79%  | Adequate   | Core requirements addressed, gaps remain |
| 0-49%   | Needs Work | Significant gaps in the evidence base    |

---

## Executive Summary

P3401R1 proposes a well-scoped set of factory functions and concepts
for the `proxy` type-erasure facility. The technical specification is
detailed and the design rationale in Section 3 shows thoughtful
engineering judgment. However, judged against the P4133 evidence
standard, the paper has significant gaps: no referenced implementation
with tests or benchmarks, no structured cost/benefit case for
standardization over ecosystem delivery, no steel man against
standardization, no post-adoption metrics, no retrospective trigger,
no domain attestation, and no prediction registry.

**Overall readiness:** Needs Work - the paper reads as a mature
technical specification but lacks the evidence infrastructure P4133
calls for.

**Strongest areas** (items scoring 2): None.

**Critical gaps** (items scoring 0): Implementation (2),
Steel Man Against Standardization (4), Post-Adoption Metrics (6),
Retrospective Trigger (7), Domain Coverage Attestation (9),
Prediction Registry (10).

---

## Improvement Tips

### 1. Evidence of Need (P4133 3.1 / 4.1)

**Scored 1.** Strengthen the existing motivation into a structured
cost/benefit analysis. Add a dedicated section answering: What problem
does this solve that the ecosystem cannot? (The Microsoft proxy library
already ships these factories - why must they be in the standard?) What
property of standardization is required? (Portability across
implementations? ABI stability? Teaching curriculum inclusion?) Why
does the benefit exceed the cost of implementer burden, ABI commitment,
and perpetual maintenance?

### 2. Implementation (P4133 3.2 / 4.2)

**Scored 0.** Produce or cite a reference implementation. If the
Microsoft proxy library already implements these factories, state that
explicitly, link to the repository, and report test coverage and
benchmark results. If it does not yet implement the full proposed
surface, fill the gaps. P4133 Section 5.1 notes that a complete
library implementation with tests, benchmarks, and documentation is
achievable in days with AI assistance.

### 4. Steel Man Against Standardization (P4133 3.3 / 4.4)

**Scored 0.** Add a dedicated section presenting the strongest
argument for leaving these factories in the ecosystem. The obvious
counter-argument: these are thin convenience wrappers over `proxy`'s
existing constructors; the Microsoft library already ships them;
standardizing them imposes ABI commitment and maintenance burden for
marginal benefit. Confront this argument and defeat it with evidence -
for example, by showing that divergent ecosystem implementations cause
real interoperability or performance problems.

### 5. Steel Man of Competing Designs (P4133 3.4 / 4.5)

**Scored 1.** Present each alternative at its strongest. Document the
advantages of using `shared_ptr` for shared ownership (ecosystem
familiarity, existing tooling support, debugger integration) before
explaining why compact handles are preferred. Add the "do nothing"
alternative as a competing design and justify the choice against it.

### 6. Post-Adoption Metrics (P4133 3.5)

**Scored 0.** Define measurable success criteria before adoption.
Examples: adoption by two major standard library implementations
within two releases, measurable reduction in ad-hoc SBO wrapper code
in surveyed codebases, benchmark parity or improvement versus the
Microsoft proxy library's existing factories.

### 7. Retrospective Trigger (P4133 3.6)

**Scored 0.** Add a mandatory look-back commitment. Define an interval
(e.g. two releases or six years) and the questions the retrospective
must answer: Did implementations ship on time? Did users migrate from
ad-hoc factories? Did the compact shared ownership handle prove
sufficient or did users still reach for `shared_ptr`?

### 8. Decision Record (P4133 3.7)

**Scored 1.** Fill the missing elements in Section 3. Enumerate
alternatives considered for each decision point, document any
dissenting views from committee or review discussions, and specify
conditions under which each decision should be revisited.

### 9. Domain Coverage Attestation (P4133 3.8)

**Scored 0.** Document which domains were represented when the design
was reviewed. Pointer-semantics polymorphism is relevant to embedded
(freestanding), games (cache sensitivity), HPC (allocator control),
networking (type-erased callbacks), and UI frameworks (polymorphic
widgets). State which domains provided input and acknowledge gaps.

### 10. Prediction Registry (P4133 3.9)

**Scored 0.** Record the paper's implicit predictions with falsifiable
criteria: "Centralizing factories will reduce duplicated SBO wrappers"
(measurable via codebase survey), "Compact shared handles will satisfy
users who currently layer shared_ptr" (measurable via adoption survey),
"Freestanding split will be accepted without contention" (measurable
at committee vote). Assign revisit dates.
