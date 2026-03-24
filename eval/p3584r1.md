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

| Field     | Value                                        |
| --------- | -------------------------------------------- |
| Paper     | P3584R1                                      |
| Title     | Proxy Facade Builder                         |
| Type      | Library                                      |
| Evaluator | AI (WG21_EVAL rubric)                        |
| Date      | 2026-03-24                                   |

---

## Checklist

### 1. Evidence of Need

**Applies to:** Both | **Ref:** P4133 3.1 / 4.1 | **Failure modes:** A2

**Score:** 1 / 2
**Notes:** Section 3 (Motivation and scope) describes the problem -
manual facade authoring is error-prone and obscures intent - and
lists five concrete goals. However, the paper does not provide a
structured cost/benefit analysis answering the three standardization
questions: what problem the ecosystem cannot solve, what property of
standardization is required, and why the benefit exceeds the cost.
The builder already ships in microsoft/proxy, so the question of why
it must live in the standard library rather than remain a
high-quality ecosystem component is left unanswered.

---

### 2. Implementation

**Applies to:** Both | **Ref:** P4133 3.2 / 4.2 | **Failure modes:** A1, C2

**Score:** 1 / 2
**Notes:** Section 4.6 states the defaults "have shipped in
production deployments," indicating a mature implementation exists
(microsoft/proxy). The paper does not link to or describe that
implementation, its test suite, benchmarks, or documentation. No
benchmark data is presented. A reader unfamiliar with the proxy
library cannot verify the claim from the paper alone.

---

### 3. Teaching Story (Language Only)

**Score:** N/A _(skip for library proposals)_

---

### 4. Steel Man Against Standardization / Adoption

**Applies to:** Both | **Ref:** P4133 3.3 / 4.4 | **Failure modes:** D2

**Score:** 0 / 2
**Notes:** The paper does not present or confront the strongest
argument against standardizing this facility. The obvious
counter-argument - that the builder is a convenience DSL layer over
core proxy that already ships as an ecosystem library with no
portability barrier, and standardizing it locks the committee into
maintaining a specific fluent API indefinitely - is never raised or
addressed.

---

### 5. Steel Man of Competing Designs

**Applies to:** Both | **Ref:** P4133 3.4 / 4.5 | **Failure modes:** D1, D4

**Score:** 1 / 2
**Notes:** Section 4 documents several design choices (fluent DSL,
explicit constraint parameters, opt-in substitution, skills pattern)
and provides rationale for each. However, alternatives are described
only in terms of what was replaced (e.g. the single aggregate
constraints struct), not steel-manned at their strongest. The "do
nothing" alternative - leaving the builder in the ecosystem - is not
considered. No alternative composition models (e.g. CRTP mixins,
concept-constrained tag dispatch, reflection-based inference) are
presented.

---

### 6. Post-Adoption Metrics

**Applies to:** Both | **Ref:** P4133 3.5 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:** No success criteria are defined. The paper does not state
how adoption, usability, or correctness would be measured after
standardization.

---

### 7. Retrospective Trigger

**Applies to:** Both | **Ref:** P4133 3.6 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:** No retrospective plan is proposed. There is no defined
interval or set of questions for a post-adoption look-back.

---

### 8. Decision Record

**Applies to:** Both | **Ref:** P4133 3.7 | **Failure modes:** B1, B2, E1

**Score:** 1 / 2
**Notes:** Section 4 provides substantial design rationale
(Sections 4.1-4.10) and Section 1.1 documents changes from R0,
showing the design evolved in response to review. Missing elements:
alternatives considered and why rejected (presented at their
strongest), dissenting views from committee or community feedback,
and explicit conditions under which the design should be revisited.

---

### 9. Domain Coverage Attestation

**Applies to:** Both | **Ref:** P4133 3.8 | **Failure modes:** A3, D3, G1, G2

**Score:** 0 / 2
**Notes:** No mention of which domains were consulted. Section 4.6
references "production deployments" but does not identify the
domains (embedded, gaming, cloud, HPC, etc.) those deployments
represent or whether any domains were not consulted.

---

### 10. Prediction Registry

**Applies to:** Both | **Ref:** P4133 3.9 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:** The paper makes several implicit predictions - that the
builder will reduce authoring errors, that defaults are empirically
balanced, that skills will encourage facade modularity - but none
are recorded with falsifiable criteria or revisit dates.

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

P3584R1 proposes a well-designed fluent builder DSL for the proxy
facade system, with thorough internal design rationale and clear
technical specifications. However, it treats the design as
self-evidently necessary and omits most of the process and evidence
infrastructure that P4133 calls for. The paper reads as a
specification addendum rather than a standalone proposal that
justifies its own standardization.

**Overall readiness:** Needs Work - the technical design is mature
but the evidentiary framework is largely absent.

**Strongest areas** (items scoring 2): None.

**Critical gaps** (items scoring 0): Steel man against
standardization, post-adoption metrics, retrospective trigger,
domain coverage attestation, prediction registry.

---

## Improvement Tips

### 1. Evidence of Need (P4133 3.1 / 4.1)

**If 1:** Strengthen the existing discussion into a structured
cost/benefit analysis. Replace informal claims with specific evidence.
Quantify the cost of standardization and show concretely why the
benefit exceeds it. In particular: why must this builder be in the
standard library rather than remain in microsoft/proxy or a similar
ecosystem library? What property of standardization (portability,
guaranteed availability, ABI stability, teaching curriculum inclusion)
is required?

### 4. Steel Man Against Standardization / Adoption (P4133 3.3 / 4.4)

**If 0:** Add a dedicated section presenting the strongest argument
for why this should NOT be standardized. The obvious argument: the
builder is a convenience layer that already ships in an ecosystem
library; standardizing it locks the committee into a specific fluent
API design, increases implementer burden across all standard library
vendors, and adds perpetual maintenance for a facility that can
evolve faster outside the standard. Confront that argument and
defeat it with evidence.

### 5. Steel Man of Competing Designs (P4133 3.4 / 4.5)

**If 1:** Present each alternative at its strongest. Document its
advantages honestly before explaining why this design is preferred.
Consider at minimum: (a) do nothing - leave the builder in the
ecosystem, (b) CRTP-based mixin composition, (c) concept-constrained
tag-dispatch builders, (d) future reflection-based facade inference
(acknowledged in Non-Goals but not compared). A one-sentence
dismissal is not a steel man.

### 6. Post-Adoption Metrics (P4133 3.5)

**If 0:** Define measurable success criteria before adoption. Examples:
adoption rate of the builder API vs. manual facade construction in
codebases using std::proxy within two releases, reduction in facade
authoring defects reported to implementers, teaching difficulty
reported by educators, frequency of workarounds in user code.

### 7. Retrospective Trigger (P4133 3.6)

**If 0:** Add a mandatory look-back commitment: a defined interval (two
releases or six years, whichever comes first) and the explicit questions
the retrospective must answer - did the builder reduce authoring errors,
did defaults prove suitable across domains, did skills composition see
adoption, did implementers ship on time?

### 8. Decision Record (P4133 3.7)

**If 1:** Fill the missing elements. Document: alternatives considered
and why they were rejected (with the alternatives presented at their
strongest), dissenting views from committee or community feedback,
and conditions under which the design should be revisited (e.g. if
reflection makes fluent builders obsolete).

### 9. Domain Coverage Attestation (P4133 3.8)

**If 0:** Document which domains were represented when the design was
reviewed. The "production deployments" referenced in Section 4.6
should identify which domains those deployments serve. List the
domains affected by the proposal and state which have provided
input. Acknowledge gaps explicitly.

### 10. Prediction Registry (P4133 3.9)

**If 0:** Record every claim the proposal makes about future outcomes -
"defaults are empirically balanced," "skills will encourage facade
modularity," "the builder will reduce authoring errors" - with
falsifiable criteria and a revisit date.
