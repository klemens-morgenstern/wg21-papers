# WG21 Proposal Evaluation Rubric

## Scoring Scale

| Score | Meaning                                                  |
| ----- | -------------------------------------------------------- |
| 0     | Absent - the paper does not address this element         |
| 1     | Partial - mentioned informally or addressed incompletely |
| 2     | Complete - dedicated section with structured evidence    |

- Library proposals: items 1-2 and 4-10 apply (skip item 3). Max 18.

---

## Paper Metadata

| Field     | Value                                         |
| --------- | --------------------------------------------- |
| Paper     | P3832R0                                       |
| Title     | Timed lock algorithms for multiple lockables  |
| Type      | Library                                       |
| Evaluator | AI (WG21_EVAL rubric)                         |
| Date      | 2026-03-24                                    |

---

## Checklist

### 1. Evidence of Need

**Applies to:** Both | **Ref:** P4133 3.1 / 4.1 | **Failure modes:** A2

**Score:** 1 / 2
**Notes:** The introduction states that users who need timeout-based
locking of multiple mutexes must write their own deadlock-avoidance
algorithm, calling this "error-prone, verbose, and inconsistent with
the existing standard library facilities." This is a reasonable
motivation but it remains informal. There is no dedicated cost/benefit
section answering what property of standardization is required (e.g.
portability, guaranteed availability, ABI stability, teaching
curriculum inclusion) or why the benefit exceeds the cost of
implementer burden, ABI commitment, and perpetual maintenance.

---

### 2. Implementation

**Applies to:** Both | **Ref:** P4133 3.2 / 4.2 | **Failure modes:** A1, C2

**Score:** 1 / 2
**Notes:** Section 6 ("Implementation Experience") references the gcc
implementation of `std::lock` and describes the modification needed
(replacing `m.lock()` with `m.try_lock_until(tp)`). A Compiler
Explorer link is provided. However, there is no complete library with
benchmarks, unit tests, or documentation. The implementation
experience amounts to a sketch of the delta from existing code rather
than a standalone, tested artifact.

---

### 3. Teaching Story (Language Only)

**Applies to:** Language | **Ref:** P4133 4.3

**Score:** N/A _(skip for library proposals)_
**Notes:**

---

### 4. Steel Man Against Standardization / Adoption

**Applies to:** Both | **Ref:** P4133 3.3 / 4.4 | **Failure modes:** D2

**Score:** 0 / 2
**Notes:** The paper does not include a section presenting the
strongest argument for why this should not be standardized. There is
no discussion of why the ecosystem (e.g. Boost, standalone utility
headers) might already serve this need adequately, nor any
confrontation of such an argument with evidence.

---

### 5. Steel Man of Competing Designs

**Applies to:** Both | **Ref:** P4133 3.4 / 4.5 | **Failure modes:** D1, D4

**Score:** 1 / 2
**Notes:** The Design Rationale section touches on several design
choices - free functions vs. other forms, parameter-pack vs.
tuple/range, `int` return value vs. alternatives - but these are
internal design axis discussions, not competing overall designs for
solving the same problem. The option of "do nothing" is not presented
or defeated. Alternative approaches (e.g. a `timed_scoped_lock` RAII
wrapper, a range-based interface, or extending `std::scoped_lock`
with timeout parameters) are not explored as competing designs with
their own strengths documented.

---

### 6. Post-Adoption Metrics

**Applies to:** Both | **Ref:** P4133 3.5 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:** No success criteria are defined. There is no discussion of
how to measure whether the feature achieved its goals after adoption -
no deployment-rate targets, adoption surveys, defect-rate tracking, or
workaround-frequency measurements.

---

### 7. Retrospective Trigger

**Applies to:** Both | **Ref:** P4133 3.6 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:** No retrospective plan is included. There is no defined
interval or set of questions for a future look-back to assess whether
the feature delivered its promised benefits.

---

### 8. Decision Record

**Applies to:** Both | **Ref:** P4133 3.7 | **Failure modes:** B1, B2, E1

**Score:** 1 / 2
**Notes:** The Design Rationale section provides partial rationale for
several choices: free-function form, parameter-pack interface, `int`
return type mirroring `std::try_lock`, and unspecified deadlock-
avoidance strategy. However, the record is incomplete - it does not
document dissenting views, does not present rejected alternatives with
their trade-offs, and does not state conditions under which these
decisions should be revisited.

---

### 9. Domain Coverage Attestation

**Applies to:** Both | **Ref:** P4133 3.8 | **Failure modes:** A3, D3, G1, G2

**Score:** 0 / 2
**Notes:** The Acknowledgments section names individuals from the
std-proposals mailing list and a code reviewer, but there is no
systematic documentation of which domains were consulted. Domains
that rely on timed multi-mutex locking (real-time systems, embedded,
high-frequency trading, database engines) are not identified, and no
coverage gaps are acknowledged.

---

### 10. Prediction Registry

**Applies to:** Both | **Ref:** P4133 3.9 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:** No falsifiable claims are recorded. The paper implicitly
predicts that users will adopt these algorithms over hand-rolled
solutions, that the implementation burden is minimal, and that the
interface generalizes well - but none of these are stated with
falsifiable criteria or revisit dates.

---

## Score Summary

| Metric     | Value      |
| ---------- | ---------- |
| Total      | 4 / 18     |
| Percentage | 22%        |
| Rating     | Needs Work |

Rating tiers:

| Range   | Rating     | Meaning                                  |
| ------- | ---------- | ---------------------------------------- |
| 80-100% | Strong     | Paper meets the ideal model              |
| 50-79%  | Adequate   | Core requirements addressed, gaps remain |
| 0-49%   | Needs Work | Significant gaps in the evidence base    |

---

## Executive Summary

P3832R0 proposes a straightforward library extension - timed versions
of multi-lockable algorithms - that fills a genuine gap in the
standard mutex toolkit. The motivation is sound and the proposed
wording is concise. However, measured against P4133's ideal model,
the paper is thin on process evidence: it has no steel-man case
against standardization, no competing-design analysis beyond internal
parameter choices, no post-adoption metrics, no retrospective
trigger, no domain attestation, and no prediction registry.

**Overall readiness:** The technical proposal is clear and the
implementation delta is small, but the paper lacks the structured
evidence P4133 calls for. A revised draft addressing items 4, 6-7,
and 9-10 would substantially strengthen the case.

**Strongest areas** (items scoring 1): Evidence of Need,
Implementation, Competing Designs (internal choices), Decision Record
(partial).

**Critical gaps** (items scoring 0): Steel Man Against
Standardization, Post-Adoption Metrics, Retrospective Trigger, Domain
Coverage Attestation, Prediction Registry.

---

## Improvement Tips

### 1. Evidence of Need (P4133 3.1 / 4.1)

**If 1:** Strengthen the existing discussion into a structured
cost/benefit analysis. Replace informal claims with specific evidence.
Quantify the cost of standardization and show concretely why the
benefit exceeds it.

### 2. Implementation (P4133 3.2 / 4.2)

**If 1:** Fill the gaps in the existing implementation. Add missing
unit tests, benchmarks, documentation, or edge-case coverage. The
implementation does not need to be production-ready - it needs to
demonstrate that the design works.

### 4. Steel Man Against Standardization / Adoption (P4133 3.3 / 4.4)

**If 0:** Add a section presenting the strongest argument for why this
should NOT be standardized. Address whether a standalone header or
Boost component would suffice. Then confront that argument and defeat
it with evidence.

### 5. Steel Man of Competing Designs (P4133 3.4 / 4.5)

**If 1:** Present each alternative at its strongest. Document its
advantages honestly before explaining why this design is preferred. A
one-sentence dismissal is not a steel man.

### 6. Post-Adoption Metrics (P4133 3.5)

**If 0:** Define measurable success criteria before adoption. Examples:
deployment rate across major implementations within two releases, user
adoption survey results, defect rates, teaching difficulty reported by
educators, frequency of workarounds in user code.

### 7. Retrospective Trigger (P4133 3.6)

**If 0:** Add a mandatory look-back commitment: a defined interval (two
releases or six years, whichever comes first) and the explicit
questions the retrospective must answer.

### 8. Decision Record (P4133 3.7)

**If 1:** Fill the missing elements. A decision record that explains
the choice but omits alternatives, dissenting views, or revisit
conditions is incomplete.

### 9. Domain Coverage Attestation (P4133 3.8)

**If 0:** Document which domains were represented when the design was
reviewed. List the domains affected by the proposal and state which
have provided input. Acknowledge gaps explicitly.

### 10. Prediction Registry (P4133 3.9)

**If 0:** Record every claim the proposal makes about future outcomes
with falsifiable criteria and a revisit date.
