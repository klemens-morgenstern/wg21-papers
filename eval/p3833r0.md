# WG21 Proposal Evaluation Rubric

## Paper Metadata

| Field     | Value                |
| --------- | -------------------- |
| Paper     | P3833R0              |
| Title     | std::multi_lock      |
| Type      | Library              |
| Evaluator | WG21_EVAL rubric/AI  |
| Date      | 2026-03-24           |

---

## Checklist

### 1. Evidence of Need

**Applies to:** Both | **Ref:** P4133 3.1 / 4.1 | **Failure modes:** A2

**Score:** 1 / 2
**Notes:**
Section 1 (Motivation) identifies the gap in the mutex-wrapper family
with a clean 2x2 table showing `multi_lock` as the missing quadrant.
Four use-case examples demonstrate the pattern. However, the argument
is structural ("completes the table") rather than evidentiary. There is
no structured cost/benefit analysis answering: what problem the
ecosystem cannot solve by shipping this on vcpkg/Conan, what property
of standardization is required, and why the benefit of perpetual ABI
commitment and implementer burden exceeds the cost.

---

### 2. Implementation

**Applies to:** Both | **Ref:** P4133 3.2 / 4.2 | **Failure modes:** A1, C2

**Score:** 1 / 2
**Notes:**
Section 7 links to a Beman-project implementation on GitHub
(bemanproject/timed_lock_alg) and states it has been tested with
multiple mutex types. The paper does not reproduce or summarize
benchmarks, unit-test coverage, or documentation. The reader must
leave the paper to assess implementation quality. Partial credit for
having a public implementation; full credit requires evidence of
benchmarks, tests, and documentation presented in the paper or
clearly summarized.

---

### 3. Teaching Story (Language Only)

**Score:** N/A _(skip for library proposals)_

---

### 4. Steel Man Against Standardization / Adoption

**Applies to:** Both | **Ref:** P4133 3.3 / 4.4 | **Failure modes:** D2

**Score:** 0 / 2
**Notes:**
The paper does not contain a section addressing why the ecosystem might
be sufficient. A developer can already compose multiple `unique_lock`
objects with `std::lock()` today - the paper acknowledges this is
"verbose and error-prone" but never presents the strongest version of
the counter-argument (e.g., a small helper template in user code, a
Boost component, or a vcpkg package could fill this gap without
standardization cost). No counter-argument is confronted.

---

### 5. Steel Man of Competing Designs

**Applies to:** Both | **Ref:** P4133 3.4 / 4.5 | **Failure modes:** D1, D4

**Score:** 1 / 2
**Notes:**
Section 6 discusses one alternative (container-based / `std::span`
interface) and rejects it for losing heterogeneous mutex support. The
rejection is reasonable but brief. Missing: the "do nothing" design
(status quo with manual `unique_lock` composition), a
policy/strategy-based design, or an approach that extends
`scoped_lock` rather than adding a new type. Only one alternative is
steel-manned.

---

### 6. Post-Adoption Metrics

**Applies to:** Both | **Ref:** P4133 3.5 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:**
No success criteria are defined. The paper does not state what would
constitute successful adoption (e.g., shipped by N implementations
within M years, measured reduction in deadlock-related bugs, adoption
rate in codebases that currently use manual `unique_lock` composition).

---

### 7. Retrospective Trigger

**Applies to:** Both | **Ref:** P4133 3.6 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:**
No retrospective plan. No defined interval for reviewing whether
`multi_lock` achieved its goals or whether the design needs revision
after field experience.

---

### 8. Decision Record

**Applies to:** Both | **Ref:** P4133 3.7 | **Failure modes:** B1, B2, E1

**Score:** 1 / 2
**Notes:**
Section 4 documents four design decisions (try-lock return value, timed
locking, exception safety, deadlock avoidance) with rationale. Section
6 documents one rejected alternative. Missing: dissenting views or
objections raised during design review, conditions under which the
committee should revisit the design, and trade-offs that were
consciously accepted (e.g., single `bool owns` for all mutexes means
no per-mutex ownership tracking).

---

### 9. Domain Coverage Attestation

**Applies to:** Both | **Ref:** P4133 3.8 | **Failure modes:** A3, D3, G1, G2

**Score:** 0 / 2
**Notes:**
No mention of which domains were consulted. Concurrent programming
patterns vary across domains (embedded, HPC, financial systems, game
engines). The paper does not state which domains informed the design or
acknowledge coverage gaps.

---

### 10. Prediction Registry

**Applies to:** Both | **Ref:** P4133 3.9 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:**
No falsifiable claims recorded. The paper implicitly predicts that
developers will prefer `multi_lock` over manual `unique_lock`
composition, but this claim is not recorded with criteria or a revisit
date.

---

## Score Summary

| Metric     | Value     |
| ---------- | --------- |
| Total      | 4 / 18    |
| Percentage | 22%       |
| Rating     | Needs Work |

---

## Executive Summary

P3833R0 proposes `std::multi_lock`, a variadic RAII lock wrapper that
fills the gap between `unique_lock` (flexible, single mutex) and
`scoped_lock` (rigid, multiple mutexes). The motivation is clearly
articulated and the technical specification is thorough, with complete
proposed wording relative to N5014. However, the paper focuses almost
entirely on the "what" and "how" while leaving the P4133 evidence
requirements largely unaddressed.

**Overall readiness:** The technical design is well-developed but the
supporting evidence framework expected by P4133 is almost entirely
absent. The paper would benefit significantly from a revision that adds
the missing sections before requesting committee time.

**Strongest areas** (items scoring 2): None.

**Critical gaps** (items scoring 0): Steel man against standardization
(item 4), post-adoption metrics (item 6), retrospective trigger
(item 7), domain coverage attestation (item 9), prediction registry
(item 10).

---

## Improvement Tips

### 1. Evidence of Need (P4133 3.1)

**Scored 1.** Strengthen the existing motivation into a structured
cost/benefit analysis. Answer explicitly: why must this be in the
standard rather than a vcpkg/Conan package? What property of
standardization (portability, guaranteed availability, ABI stability,
teaching curriculum inclusion) is required? Quantify the cost
(implementer burden, ABI commitment, perpetual maintenance) and show
concretely why the benefit exceeds it.

### 2. Implementation (P4133 3.2)

**Scored 1.** Summarize benchmarks, test coverage, and documentation
in the paper itself. The reader should not need to clone a GitHub
repository to assess implementation quality. Show representative
benchmark results (e.g., lock acquisition latency vs. manual
composition), state the number and scope of unit tests, and describe
the documentation available.

### 4. Steel Man Against Standardization (P4133 3.3)

**Scored 0.** Add a dedicated section presenting the strongest
argument for why the ecosystem is sufficient. The obvious counter: a
small utility template wrapping multiple `unique_lock` objects with
`std::lock()` can be shipped as a header-only library without
standardization cost. Confront this argument and defeat it with
evidence - e.g., the fragility of manual composition, the frequency of
deadlock bugs in real codebases, or the teaching benefit of a standard
vocabulary type.

### 5. Steel Man of Competing Designs (P4133 3.4)

**Scored 1.** Add the "do nothing" design as a first-class
alternative. Consider whether extending `scoped_lock` (e.g., adding
`try_lock` and deferred locking to it) would be preferable to a new
type. Present each alternative at its strongest before explaining why
`multi_lock` as a new type is the better choice.

### 6. Post-Adoption Metrics (P4133 3.5)

**Scored 0.** Define measurable success criteria before adoption.
Examples: shipped by at least two major implementations within two
releases of standardization, measurable reduction in manual
multi-`unique_lock` patterns in surveyed codebases, adoption in at
least one major open-source project within three years.

### 7. Retrospective Trigger (P4133 3.6)

**Scored 0.** Add a mandatory look-back commitment. Suggest two
releases or six years, whichever comes first. Define the questions:
did developers adopt `multi_lock` over manual composition? Did
implementations ship on schedule? Were the deadlock-avoidance
guarantees sufficient in practice?

### 8. Decision Record (P4133 3.7)

**Scored 1.** Document dissenting views or objections encountered
during design. Add conditions under which the design should be
revisited (e.g., if P3832 is not adopted, the timed-locking members
may need redesign). Acknowledge the trade-off of a single `bool owns`
for all mutexes - this prevents per-mutex ownership queries, which may
matter for some use cases.

### 9. Domain Coverage Attestation (P4133 3.8)

**Scored 0.** List the domains affected by multi-mutex locking
(embedded, HPC, financial, game engines, server infrastructure) and
state which have provided input on the design. Acknowledge any gaps
and explain plans to fill them.

### 10. Prediction Registry (P4133 3.9)

**Scored 0.** Record the paper's implicit predictions with falsifiable
criteria. Examples: "Developers will prefer `multi_lock` over manual
`unique_lock` composition when locking 2+ mutexes with deferred or
timed semantics" (testable via usage surveys post-adoption),
"Implementation complexity is comparable to `scoped_lock`" (testable
by implementation effort measurement).
