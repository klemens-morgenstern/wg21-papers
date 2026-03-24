# WG21 Proposal Evaluation Rubric

## Paper Metadata

| Field     | Value                                              |
| --------- | -------------------------------------------------- |
| Paper     | P3086R5                                            |
| Title     | Proxy: A Pointer-Semantics-Based Polymorphism Library |
| Type      | Library                                            |
| Evaluator | AI (WG21_EVAL rubric)                              |
| Date      | 2026-03-24                                         |

---

## Checklist

### 1. Evidence of Need

**Applies to:** Both | **Ref:** P4133 3.1 / 4.1 | **Failure modes:** A2

**Score:** 1 / 2
**Notes:**

Section 3.1 presents a structured problem statement with a table
comparing existing approaches (virtual functions, ad-hoc vtable
wrappers, single-purpose type erasure) on coupling, costs, and gaps.
Section 3.6 provides detailed comparison with virtual functions
including memory layout diagrams, call sequence analysis, and
cross-architecture code generation data.

However, the paper does not explicitly answer the three questions
required for a score of 2: (1) what problem the ecosystem cannot solve -
the open-source library at github.com/microsoft/proxy already solves
the problem and ships in production; (2) what property of
standardization is required - portability, guaranteed availability,
ABI stability, or teaching curriculum inclusion are not argued; (3) why
the benefit of standardization exceeds the cost of implementer burden,
ABI commitment, teaching load, and perpetual maintenance. The
motivation section argues for the facility's value but not specifically
for its standardization over continued ecosystem delivery.

---

### 2. Implementation

**Applies to:** Both | **Ref:** P4133 3.2 / 4.2 | **Failure modes:** A1, C2

**Score:** 2 / 2
**Notes:**

The open-source implementation (github.com/microsoft/proxy) targets
C++20 and later, is continuously tested on GCC, Clang, MSVC, NVIDIA
HPC, and Intel oneAPI across Windows, Linux, and macOS. The wording
tracks version 4 of the shipping library. Section 3.6.3 provides
cross-architecture benchmarks and code generation analysis. The library
has shipped in production inside Microsoft Windows since 2022.
Production deployment, multi-compiler CI, and published benchmark data
satisfy the complete implementation criterion.

---

### 3. Teaching Story (Language Only)

**Score:** N/A _(skip for library proposals)_

---

### 4. Steel Man Against Standardization / Adoption

**Applies to:** Both | **Ref:** P4133 3.3 / 4.4 | **Failure modes:** D2

**Score:** 0 / 2
**Notes:**

The paper does not contain a dedicated section presenting the strongest
argument for why this facility should NOT be standardized. The most
obvious counter-argument - that the open-source library already ships,
works on all major compilers, and can iterate faster outside the
standard - is never articulated or confronted. Section 5 addresses
internal design trade-offs (boilerplate, compile-time cost, scoping)
but none of these subsections frame the case against standardization
itself. A committee member who raises "why not leave this in the
ecosystem?" is doing work the paper should have done.

---

### 5. Steel Man of Competing Designs

**Applies to:** Both | **Ref:** P4133 3.4 / 4.5 | **Failure modes:** D1, D4

**Score:** 1 / 2
**Notes:**

Section 3.1 identifies three existing approaches (virtual functions,
ad-hoc vtable wrappers, single-purpose type erasure) and Section 3.6
provides a thorough comparison with virtual functions including
performance data. Section 5 documents rationale for several internal
design choices (pointer semantics vs layout changes, library vs
language, naming).

However, competing library designs that solve the same problem are not
presented at their strongest. There is no mention of Boost.TypeErasure,
dyno, or other type-erasure libraries in the ecosystem. The comparison
with virtual functions is presented one-sidedly - virtual dispatch
advantages (familiarity, zero boilerplate, tooling maturity, debugger
support) are not documented. The design of doing nothing (status quo
with the existing open-source library) is not presented as a competing
option with its own advantages.

---

### 6. Post-Adoption Metrics

**Applies to:** Both | **Ref:** P4133 3.5 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:**

No measurable success criteria are defined anywhere in the paper. There
is no discussion of how to judge whether standardization succeeded -
no deployment rate targets, no adoption survey plans, no defect rate
thresholds, no teaching difficulty expectations. The paper reports past
benchmarks and production experience but defines no forward-looking
metrics against which the standardized facility can be evaluated after
adoption.

---

### 7. Retrospective Trigger

**Applies to:** Both | **Ref:** P4133 3.6 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:**

No retrospective plan is defined. There is no commitment to revisit the
facility after a defined interval (e.g., two releases or six years),
and no explicit questions the retrospective must answer. Given that the
paper explicitly defers helper facilities and facade-construction
utilities to follow-on proposals, a trigger for evaluating whether the
minimal substrate proved sufficient would be especially valuable.

---

### 8. Decision Record

**Applies to:** Both | **Ref:** P4133 3.7 | **Failure modes:** B1, B2, E1

**Score:** 1 / 2
**Notes:**

Section 5 provides substantial design rationale across 14 subsections
covering pointer semantics, naming, library-vs-language, facade model,
performance, scoping, freestanding, boilerplate, constraint levels,
substitution, views/weak ownership, diagnostics, reflection, and a
summary of trade-offs. The rationale is generally well-argued.

Missing for a complete score: (a) dissenting views from committee
review are not documented - the paper mentions "prior teachability
concerns" and "committee feedback" without recording specific
objections and how they were resolved; (b) conditions under which
decisions should be revisited are not stated; (c) alternatives rejected
at the design level (e.g., why not a value-semantic type-erasure model
like std::any, why not a Concepts-based static polymorphism approach)
are not systematically documented with reasons for rejection.

---

### 9. Domain Coverage Attestation

**Applies to:** Both | **Ref:** P4133 3.8 | **Failure modes:** A3, D3, G1, G2

**Score:** 1 / 2
**Notes:**

The acknowledgments list reviewers from WG21, Microsoft, academia
(Jilin University), and notable C++ experts. Section 3.4 documents
multi-compiler, multi-platform testing. Production use inside Microsoft
Windows is mentioned, and freestanding viability is demonstrated
(Section 5.7).

However, there is no explicit statement of which application domains
were represented in the review. Domains that would naturally exercise
this facility - embedded/real-time, game engines, high-frequency
trading, scientific computing, GUI frameworks - are not enumerated.
Coverage gaps are not acknowledged. A reader cannot determine whether
the design has been validated beyond Microsoft's internal use cases.

---

### 10. Prediction Registry

**Applies to:** Both | **Ref:** P4133 3.9 | **Failure modes:** A4

**Score:** 0 / 2
**Notes:**

The paper makes several implicit predictions - that the minimal
substrate will prove sufficient for follow-on proposals, that
compile-time cost will not be a blocker, that the facility will
interoperate with senders/receivers and ranges, that boilerplate can
be addressed by future automation. None of these are recorded with
falsifiable criteria or revisit dates. There is no prediction registry.

---

## Score Summary

| Metric     | Value      |
| ---------- | ---------- |
| Total      | 6 / 18     |
| Percentage | 33%        |
| Rating     | Needs Work |

Rating tiers:

| Range   | Rating     | Meaning                                  |
| ------- | ---------- | ---------------------------------------- |
| 80-100% | Strong     | Paper meets the ideal model              |
| 50-79%  | Adequate   | Core requirements addressed, gaps remain |
| 0-49%   | Needs Work | Significant gaps in the evidence base    |

---

## Executive Summary

P3086R5 proposes a well-engineered, production-proven library for
pointer-semantics-based runtime polymorphism. The implementation is
mature, benchmarked, multi-platform, and deployed at scale inside
Microsoft. The technical content - motivation, design rationale,
performance analysis, and formal wording - is thorough. However, the
paper scores poorly on the P4133 process requirements because it treats
the facility's technical merit as self-evident justification for
standardization without addressing the institutional questions: why
standardize rather than ship as an ecosystem library, what happens if
standardization fails to deliver, and how will success be measured.

**Overall readiness:** Strong technical proposal with significant
process gaps. The evidence base for the facility itself is excellent;
the evidence base for standardization is absent.

**Strongest areas** (items scoring 2): Implementation - mature,
multi-compiler, production-deployed, benchmarked.

**Critical gaps** (items scoring 0): Steel man against standardization,
post-adoption metrics, retrospective trigger, prediction registry.

---

## Improvement Tips

### 1. Evidence of Need (P4133 3.1 / 4.1)

**If 1:** Strengthen the existing discussion into a structured
cost/benefit analysis. Replace informal claims with specific evidence.
Quantify the cost of standardization and show concretely why the
benefit exceeds it. Specifically: the open-source library already works
on all major compilers and ships in production. What property of
standardization - guaranteed availability across all conforming
implementations, ABI stability commitments, inclusion in teaching
curricula, interoperability with other standard facilities like
senders/receivers - is required and cannot be achieved by the ecosystem
library alone?

### 4. Steel Man Against Standardization / Adoption (P4133 3.3 / 4.4)

**If 0:** Add a dedicated section presenting the strongest argument for
why this should NOT be standardized. The obvious counter-argument: the
library already ships as open-source, targets C++20, compiles on all
major platforms, iterates faster outside the standard, and avoids
locking the committee into ABI commitments for a facility whose helper
surface is still evolving. Confront this argument and defeat it with
evidence - e.g., that ecosystem delivery leads to fragmentation of
competing type-erasure approaches, that guaranteed availability across
freestanding implementations requires standard mandate, or that
interoperability with other standard facilities requires a shared
vocabulary type.

### 5. Steel Man of Competing Designs (P4133 3.4 / 4.5)

**If 1:** Present each alternative at its strongest. Document
virtual dispatch advantages honestly (zero boilerplate, universal
tooling/debugger support, decades of teaching material, no template
instantiation cost). Document competing type-erasure libraries
(Boost.TypeErasure, dyno, folly::Poly) and what they provide that
proxy does not. Present the status quo (do nothing / keep as ecosystem
library) as a competing design with its own advantages. Then explain
why proxy is preferred over each.

### 6. Post-Adoption Metrics (P4133 3.5)

**If 0:** Define measurable success criteria before adoption. Examples:
at least two major implementations ship proxy within 18 months of
standardization; at least one major open-source project migrates from
virtual-based polymorphism to proxy within three years; teaching
materials covering proxy appear in at least two university curricula
within five years; the deferred helper facilities receive follow-on
proposals within two standard cycles.

### 7. Retrospective Trigger (P4133 3.6)

**If 0:** Add a mandatory look-back commitment: a defined interval (two
releases or six years, whichever comes first) and the explicit questions
the retrospective must answer - did implementations ship on time, did
the minimal substrate prove sufficient for follow-on proposals, did
users adopt proxy over virtual functions in new code, did the
boilerplate concern materialize as a barrier, did compile-time cost
remain manageable at scale?

### 8. Decision Record (P4133 3.7)

**If 1:** Fill the missing elements. Document specific dissenting views
from LEWGI review (e.g., teachability concerns, boilerplate objections)
and how they were resolved. State conditions under which decisions
should be revisited - e.g., if reflection makes facade-construction
trivial, should the minimal substrate be extended? If compile-time cost
proves problematic at scale, what mitigations are committed?

### 9. Domain Coverage Attestation (P4133 3.8)

**If 1:** Systematize the coverage. List every affected domain
(embedded/real-time, game engines, HPC, finance, GUI frameworks,
distributed systems, kernel/driver development). State who represented
each in the review process. Acknowledge which domains have not yet
provided input. Microsoft Windows production use is one data point;
document whether embedded, gaming, or scientific computing domains have
validated the design.

### 10. Prediction Registry (P4133 3.9)

**If 0:** Record every claim the proposal makes about future outcomes
with falsifiable criteria and a revisit date. Key predictions to
register: (1) the minimal substrate will prove sufficient for follow-on
helper proposals - revisit after two standard cycles; (2) compile-time
cost will not be a deployment blocker - measure after implementations
ship; (3) interoperability with senders/receivers and ranges will be
achievable via facade definitions - revisit when those integrations are
attempted; (4) boilerplate will be addressed by future reflection-based
automation - revisit after reflection ships.
