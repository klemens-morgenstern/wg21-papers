# WG21 Proposal Evaluation Rubric

Derived from P4133R0 "What Every Proposal Must Contain." Copy this
template for each paper under review. Fill in the metadata, score
each applicable item 0-2, tally the result, write the executive
summary, and consult the improvement tips for any item scoring
below 2.

## Scoring Scale

| Score | Meaning                                                  |
| ----- | -------------------------------------------------------- |
| 0     | Absent - the paper does not address this element         |
| 1     | Partial - mentioned informally or addressed incompletely |
| 2     | Complete - dedicated section with structured evidence    |

- Library proposals: items 1-2 and 4-10 apply (skip item 3). Max 18.
- Language proposals: all 10 items apply. Max 20.

---

## Paper Metadata

| Field     | Value                 |
| --------- | --------------------- |
| Paper     | _e.g. P1234R0_        |
| Title     |                       |
| Type      | _Library or Language_ |
| Evaluator |                       |
| Date      |                       |

---

## Checklist

### 1. Evidence of Need

**Applies to:** Both | **Ref:** P4133 3.1 / 4.1 | **Failure modes:** A2

Library criteria:

- **0** - No justification for standardization over ecosystem delivery
- **1** - Informal mention of standardization value without structured
  cost/benefit analysis
- **2** - Dedicated section answering: what problem the ecosystem cannot
  solve, what property of standardization is required, and why the
  benefit exceeds the cost

Language criteria:

- **0** - No demonstration that existing language facilities are
  insufficient
- **1** - Claims current solutions are painful without real-world code
  or quantified evidence
- **2** - Real-world code showing current pain and proposed relief, with
  argument that library alternatives are insufficient

**Score:** \_\_ / 2
**Notes:**

---

### 2. Implementation

**Applies to:** Both | **Ref:** P4133 3.2 / 4.2 | **Failure modes:** A1, C2

Library criteria:

- **0** - No implementation, or pseudocode / API sketches only
- **1** - Partial implementation - compiles but lacks tests, benchmarks,
  or documentation
- **2** - Complete library with benchmarks, unit tests, and documentation

Language criteria:

- **0** - No implementation
- **1** - Implementation exists but incomplete - missing edge cases,
  diagnostics, or interaction analysis with existing features
- **2** - Compiler fork or branch demonstrating the feature works, edge
  cases considered, interactions with existing features explored

**Score:** \_\_ / 2
**Notes:**

---

### 3. Teaching Story (Language Only)

**Applies to:** Language | **Ref:** P4133 4.3

- **0** - No teaching story
- **1** - Brief mention of learnability without addressing error messages
  or mental model interactions
- **2** - Structured section covering first-encounter experience, error
  messages on misuse, and which existing mental models are reinforced
  or broken

**Score:** \_\_ / 2 _(skip for library proposals)_
**Notes:**

---

### 4. Steel Man Against Standardization / Adoption

**Applies to:** Both | **Ref:** P4133 3.3 / 4.4 | **Failure modes:** D2

Library criteria:

- **0** - Does not address why the ecosystem might be enough
- **1** - Acknowledges the alternative without presenting the strongest
  counter-argument or confronting it with evidence
- **2** - Dedicated section: strongest argument against standardization,
  confronted and defeated with evidence

Language criteria:

- **0** - Does not address complexity costs or reasons against adoption
- **1** - Acknowledges concerns informally without structured analysis
- **2** - Dedicated section: strongest case against adoption (complexity,
  teaching burden, surprising interactions), confronted with evidence

**Score:** \_\_ / 2
**Notes:**

---

### 5. Steel Man of Competing Designs

**Applies to:** Both | **Ref:** P4133 3.4 / 4.5 | **Failure modes:** D1, D4

- **0** - No mention of alternative designs
- **1** - Alternatives mentioned but not fairly represented or advantages
  not documented
- **2** - Strongest case for each alternative presented, advantages
  documented, choice justified against each

**Score:** \_\_ / 2
**Notes:**

---

### 6. Post-Adoption Metrics

**Applies to:** Both | **Ref:** P4133 3.5 | **Failure modes:** A4

- **0** - No success criteria defined
- **1** - Vague mention of expected outcomes without measurable criteria
- **2** - Specific measurable criteria defined before adoption (e.g.
  deployment rate, adoption surveys, defect rates, teaching difficulty,
  workaround frequency)

**Score:** \_\_ / 2
**Notes:**

---

### 7. Retrospective Trigger

**Applies to:** Both | **Ref:** P4133 3.6 | **Failure modes:** A4

- **0** - No retrospective plan
- **1** - Informal suggestion of future review without defined interval
  or questions
- **2** - Defined interval (e.g. two releases or six years) and explicit
  questions the retrospective must answer

**Score:** \_\_ / 2
**Notes:**

---

### 8. Decision Record

**Applies to:** Both | **Ref:** P4133 3.7 | **Failure modes:** B1, B2, E1

- **0** - No record of why this design was chosen over alternatives
- **1** - Some rationale provided but missing alternatives considered,
  dissenting views, or conditions for revisiting
- **2** - Complete record: design rationale, alternatives considered and
  why rejected, trade-offs acknowledged, dissenting views documented,
  conditions under which to revisit

**Score:** \_\_ / 2
**Notes:**

---

### 9. Domain Coverage Attestation

**Applies to:** Both | **Ref:** P4133 3.8 | **Failure modes:** A3, D3, G1, G2

- **0** - No mention of which domains were consulted or represented
- **1** - Some domains mentioned but coverage not systematically
  documented
- **2** - Explicit statement of which domains were represented in review,
  how they were consulted, and any coverage gaps acknowledged

**Score:** \_\_ / 2
**Notes:**

---

### 10. Prediction Registry

**Applies to:** Both | **Ref:** P4133 3.9 | **Failure modes:** A4

- **0** - No falsifiable claims recorded
- **1** - Claims made but without falsifiable criteria or a revisit
  schedule
- **2** - Claims recorded with falsifiable success/failure criteria and
  defined revisit dates

**Score:** \_\_ / 2
**Notes:**

---

## Score Summary

| Metric     | Value |
| ---------- | ----- |
| Total      | __ / __ |
| Percentage | __% |
| Rating     |     |

Rating tiers:

| Range   | Rating     | Meaning                                  |
| ------- | ---------- | ---------------------------------------- |
| 80-100% | Strong     | Paper meets the ideal model              |
| 50-79%  | Adequate   | Core requirements addressed, gaps remain |
| 0-49%   | Needs Work | Significant gaps in the evidence base    |

---

## Executive Summary

_Write 3-5 sentences summarizing the evaluation._

**Overall readiness:**

**Strongest areas** (items scoring 2):

**Critical gaps** (items scoring 0):

---

## Improvement Tips

Consult the tips below for each item that scored 0 or 1. Each tip
references the P4133 section where the requirement is defined and
describes what the author should add.

### 1. Evidence of Need (P4133 3.1 / 4.1)

**If 0:** Add a dedicated section answering three questions. For
library proposals: What problem does this solve that the ecosystem
cannot? What property of standardization (portability, guaranteed
availability, ABI stability, teaching curriculum inclusion) is
required? Why does the benefit exceed the cost of implementer burden,
ABI commitment, teaching load, and perpetual maintenance? For language
proposals: show real-world code demonstrating the pain of existing
facilities and the relief the feature provides.

**If 1:** Strengthen the existing discussion into a structured
cost/benefit analysis. Replace informal claims with specific evidence.
Quantify the cost of standardization and show concretely why the
benefit exceeds it.

### 2. Implementation (P4133 3.2 / 4.2)

**If 0:** Produce an implementation before requesting committee time.
For library proposals: a complete library with benchmarks, unit tests,
and documentation - achievable in days with AI assistance (P4133
Section 5.1). For language proposals: a proof-of-concept compiler fork
demonstrating the feature works (P4133 Section 5.2).

**If 1:** Fill the gaps in the existing implementation. Add missing
unit tests, benchmarks, documentation, or edge-case coverage. The
implementation does not need to be production-ready - it needs to
demonstrate that the design works.

### 3. Teaching Story (P4133 4.3)

**If 0:** Add a section covering: How does a student encounter this
feature for the first time? What does the error message look like on
misuse? Which existing mental models does it reinforce, and which does
it break?

**If 1:** Expand to cover error messages and mental model interactions.
A teaching story that describes only the happy path is incomplete.

### 4. Steel Man Against Standardization / Adoption (P4133 3.3 / 4.4)

**If 0:** Add a section presenting the strongest argument for why this
should NOT be standardized (library) or NOT be added to the language
(language). Then confront that argument and defeat it with evidence.
The committee member who raises this objection in the room is doing
work the author should have done in the paper.

**If 1:** Strengthen the counter-argument. Present it at its strongest
before refuting it. If the refutation relies on assertion rather than
evidence, add evidence.

### 5. Steel Man of Competing Designs (P4133 3.4 / 4.5)

**If 0:** Add a section documenting alternative designs that solve the
same problem. For each alternative: what does it provide that this
design does not? What are its advantages? Why was this design chosen
over it? Include the design of doing nothing.

**If 1:** Present each alternative at its strongest. Document its
advantages honestly before explaining why this design is preferred. A
one-sentence dismissal is not a steel man.

### 6. Post-Adoption Metrics (P4133 3.5)

**If 0:** Define measurable success criteria before adoption. Examples:
deployment rate across major implementations within two releases, user
adoption survey results, defect rates, teaching difficulty reported by
educators, frequency of workarounds in user code. A feature adopted
without success criteria cannot be evaluated after adoption.

**If 1:** Replace vague expectations with specific, measurable criteria.
"Users will find it useful" is not a metric. "Adopted by two major
implementations within 18 months of standardization" is.

### 7. Retrospective Trigger (P4133 3.6)

**If 0:** Add a mandatory look-back commitment: a defined interval (two
releases or six years, whichever comes first) and the explicit questions
the retrospective must answer - did the benefits materialize, did
predictions hold, did users adopt, did implementers ship on time, what
was learned?

**If 1:** Make the commitment concrete. Specify the interval and the
questions. An informal "we should revisit this someday" is not a
trigger.

### 8. Decision Record (P4133 3.7)

**If 0:** Add a section documenting: why this design and not
alternatives, what was traded away, what alternatives were considered
and why they were rejected, and under what conditions the decision
should be revisited.

**If 1:** Fill the missing elements. A decision record that explains
the choice but omits alternatives, dissenting views, or revisit
conditions is incomplete.

### 9. Domain Coverage Attestation (P4133 3.8)

**If 0:** Document which domains were represented when the design was
reviewed. List the domains affected by the proposal and state which
have provided input. Acknowledge gaps explicitly.

**If 1:** Systematize the coverage. List every affected domain, state
who represented each, and document any domains that have not yet been
consulted.

### 10. Prediction Registry (P4133 3.9)

**If 0:** Record every claim the proposal makes about future outcomes -
"this will enable X," "this will be implementable in N months," "this
covers domain Y" - with falsifiable criteria and a revisit date.

**If 1:** Add falsifiable criteria and revisit dates to existing claims.
A claim without a way to test it is not a prediction - it is an
assertion.
