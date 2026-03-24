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

| Field     | Value                          |
| --------- | ------------------------------ |
| Paper     | P3045R7                        |
| Title     | Quantities and units library   |
| Type      | Library                        |
| Evaluator | WG21_EVAL automated assessment |
| Date      | 2026-03-24                     |

---

## Checklist

### 1. Evidence of Need

**Applies to:** Both | **Ref:** P4133 3.1 / 4.1 | **Failure modes:** A2

**Score:** 1 / 2

**Notes:** The paper has an extensive Motivation chapter (Section 7)
with eight subsections making distinct arguments: vocabulary types for
cross-vendor interoperability (7.2), certification requirements in
safety-critical industries that prohibit uncertified open-source (7.3),
complexity that forces companies to write inferior in-house solutions
(7.4), and committee polling showing strong interest (7.8). Chapter 8
provides concrete code-smell examples. However, the arguments are
dispersed across narrative subsections rather than presented as a
structured cost/benefit analysis. The standardization cost discussion
(9.6) is four sentences long and says only "limit to bare minimum" - it
does not weigh costs against benefits rigorously.

---

### 2. Implementation

**Applies to:** Both | **Ref:** P4133 3.2 / 4.2 | **Failure modes:** A1, C2

**Score:** 1 / 2

**Notes:** The paper is based on the mp-units library, an actively
maintained open-source project described as having "more than 90% of all
the stars on GitHub in the field of physical units libraries for C++."
Six code examples in Chapter 12 include live Compiler Explorer links
demonstrating working, compilable code. External documentation exists at
the mp-units website. However, the paper itself does not present
benchmark results, test coverage data, or CI pipeline status. The design
goal "as fast or even faster than working with fundamental types" (9.2)
is stated without supporting benchmark evidence in the document.

---

### 3. Teaching Story (Language Only)

**Applies to:** Language | **Ref:** P4133 4.3

**Score:** N/A _(skipped - library proposal)_
**Notes:**

---

### 4. Steel Man Against Standardization / Adoption

**Applies to:** Both | **Ref:** P4133 3.3 / 4.4 | **Failure modes:** D2

**Score:** 0 / 2

**Notes:** The paper does not present any argument for why
standardization might not be needed. It does not acknowledge the
strongest counter-argument - that mp-units, Au, nholthaus/units, and
Boost.Units already exist and function in the ecosystem, which an
opponent would cite as evidence that the problem is solvable without
standardization. The certification argument (7.3) comes closest by
acknowledging that companies cannot use uncertified open-source, but
this is framed entirely as a reason FOR standardization, with no
engagement with the opposing perspective (e.g., that industry consortia
could certify an ecosystem library without burdening the standard).

---

### 5. Steel Man of Competing Designs

**Applies to:** Both | **Ref:** P4133 3.4 / 4.5 | **Failure modes:** D1, D4

**Score:** 1 / 2

**Notes:** Chapter 13 systematically compares three design paradigms:
units-only (13.1), dimension-based (13.2), and the proposed typed
quantities approach. The paper demonstrates specific failure cases -
Hz and Bq both resolving to s^-1, dimension analysis unable to
distinguish work from torque (13.2.2-13.2.3). However, the discussion
is structured as "here's why alternatives fail" rather than presenting
the strongest case someone could make for each alternative before
explaining why this design is still preferred. The competing library
authors (Au, nholthaus/units, SI library) are all co-authors, which
collapses design competition into a unified advocacy document rather
than an impartial comparison.

---

### 6. Post-Adoption Metrics

**Applies to:** Both | **Ref:** P4133 3.5 | **Failure modes:** A4

**Score:** 0 / 2

**Notes:** The paper defines no measurable success criteria. There is no
discussion of expected adoption rates, defect reduction targets,
deployment surveys, or any quantitative metric by which the
standardization could be evaluated after adoption.

---

### 7. Retrospective Trigger

**Applies to:** Both | **Ref:** P4133 3.6 | **Failure modes:** A4

**Score:** 0 / 2

**Notes:** The paper contains no retrospective plan. No section commits
to a post-adoption review at a defined interval. No explicit questions
are posed for a future look-back. The paper ends with Teachability,
Acknowledgements, and References.

---

### 8. Decision Record

**Applies to:** Both | **Ref:** P4133 3.7 | **Failure modes:** B1, B2, E1

**Score:** 1 / 2

**Notes:** Chapter 18 ("Design details and rationale") contains
substantial rationale: why multiply syntax over UDLs (18.12.2), why
units are tag types (18.7), why `std::ratio` is insufficient (18.8.1),
and comparisons with Au, Boost.Units, nholthaus/units, and Pint. The
paper includes a Richard Smith dissenting perspective on modulo semantics
(18.12.4.4) and multiple "bikeshedding" sections offering naming
alternatives. However, there are no formal conditions for revisiting any
design decision, dissenting views are not systematically collected, and
the rationale is scattered as narrative throughout design chapters rather
than structured as a decision record.

---

### 9. Domain Coverage Attestation

**Applies to:** Both | **Ref:** P4133 3.8 | **Failure modes:** A3, D3, G1, G2

**Score:** 1 / 2

**Notes:** The "About authors" section (Chapter 6) implicitly
demonstrates multi-domain representation: med-tech/safety-critical
(Berner), autonomous vehicles (Hogg at Aurora), defense/aircraft
simulation (Holthaus at Naval Air Warfare Center, MIT Lincoln Lab),
audio/DSP (Michaels at Native Instruments), astrophysics (Reverdy at
CNRS), and flight computers (Pusz via LK8000). This is a genuinely
broad author base. However, there is no explicit domain coverage
statement - no section documenting which domains reviewed the design,
which remain gaps, and how each was consulted. The coverage is implicit
through author biographies rather than systematically documented.

---

### 10. Prediction Registry

**Applies to:** Both | **Ref:** P4133 3.9 | **Failure modes:** A4

**Score:** 0 / 2

**Notes:** The paper makes no falsifiable predictions with revisit dates.
While it asserts benefits (compile-time safety, zero overhead, improved
diagnostics) and motivates with historical disaster examples (Mars
Orbiter, Ariane 5, Gimli Glider), none of these claims are recorded with
measurable success criteria or dates for re-evaluation.

---

## Score Summary

| Metric     | Value      |
| ---------- | ---------- |
| Total      | 5 / 18     |
| Percentage | 28%        |
| Rating     | Needs Work |

Rating tiers:

| Range   | Rating     | Meaning                                  |
| ------- | ---------- | ---------------------------------------- |
| 80-100% | Strong     | Paper meets the ideal model              |
| 50-79%  | Adequate   | Core requirements addressed, gaps remain |
| 0-49%   | Needs Work | Significant gaps in the evidence base    |

---

## Executive Summary

P3045R7 is a technically thorough library proposal backed by a mature
ecosystem implementation (mp-units) and an unusually broad author base
spanning safety-critical, aerospace, audio, and scientific domains. The
paper excels at technical design rationale and real-world motivation.
However, it scores 5/18 (28%) on the P4133 ideal model because it lacks
the meta-artifacts that close the feedback loop: no steel man against
standardization, no post-adoption metrics, no retrospective commitment,
and no prediction registry. The five items scoring 1 (evidence of need,
implementation, competing designs, decision record, domain coverage) all
contain good raw material that needs restructuring into dedicated,
structured sections rather than narrative scattered throughout the paper.

**Overall readiness:** The technical content is strong. The process
artifacts required by the P4133 model are almost entirely absent. The
gap is addressable - the evidence exists but is not structured as the
model requires.

**Strongest areas** (items scoring 2): None.

**Critical gaps** (items scoring 0): Steel man against standardization
(item 4), post-adoption metrics (item 6), retrospective trigger
(item 7), prediction registry (item 10).

---

## Improvement Tips

Items scoring 2 need no action. Items scoring 0 or 1 are listed below
with specific guidance for the authors.

### 1. Evidence of Need - scored 1 (P4133 3.1)

**Tip:** Consolidate the eight motivation subsections and the "low
standardization cost" paragraph (9.6) into a single structured
cost/benefit section. Answer explicitly: (a) what problem mp-units
cannot solve as an ecosystem library, (b) which property of
standardization - portability, guaranteed availability, ABI stability,
teaching curriculum inclusion - is the binding constraint, and (c) why
the benefit exceeds implementer burden, ABI commitment, teaching load,
and perpetual maintenance. The certification argument (7.3) is strong
raw material - elevate it to the center of a dedicated analysis.

### 2. Implementation - scored 1 (P4133 3.2)

**Tip:** The mp-units implementation exists and is mature. Add a section
to the paper itself presenting benchmark results (compile time and
runtime vs. raw doubles), test coverage metrics, and CI status. The
Compiler Explorer links are good evidence of functionality. What is
missing is quantitative evidence of the performance claim in 9.2 and
documentation of test breadth. A table summarizing test suite coverage
and a benchmark comparison would move this to a 2.

### 4. Steel Man Against Standardization - scored 0 (P4133 3.3)

**Tip:** Add a dedicated section presenting the strongest argument for
why the ecosystem might be enough. The core counter-argument is: mp-units
already exists, ships, and works - standardization freezes the API,
commits vendors to perpetual ABI stability, and adds teaching load. The
certification concern (7.3) could be addressed by industry certification
of an ecosystem library rather than standardization. Present this
argument at its strongest, then defeat it with evidence - for example,
that vocabulary-type interoperability across library boundaries requires
a single canonical type in the standard, which no ecosystem library can
provide.

### 5. Steel Man of Competing Designs - scored 1 (P4133 3.4)

**Tip:** Restructure Chapter 13 to present each competing paradigm at
its strongest before showing where it falls short. For units-only (Au,
nholthaus): document the simplicity and compile-time advantages an
advocate would cite. For dimension-based (Boost.Units): document the
track record and mathematical elegance. Then show why typed quantities
are needed despite these strengths. The current framing - "here is why
alternatives fail" - reads as advocacy rather than impartial analysis.

### 6. Post-Adoption Metrics - scored 0 (P4133 3.5)

**Tip:** Add a section defining measurable success criteria. Examples:
adoption by at least two major standard library implementations within
two releases of standardization; reduction in unit-related defects
reported in safety-critical codebases adopting the library; user
adoption measured by survey or package manager statistics; teaching
difficulty as reported by educators integrating quantities into
curricula. Without these, the committee cannot evaluate whether
standardization achieved its goals.

### 7. Retrospective Trigger - scored 0 (P4133 3.6)

**Tip:** Add a mandatory look-back commitment - two releases or six
years after adoption. The retrospective should ask: Did the claimed
benefits (compile-time safety, zero overhead, vocabulary-type
interoperability) materialize? Did implementers ship on time? Did
users adopt? Did the teaching story hold? What was learned?

### 8. Decision Record - scored 1 (P4133 3.7)

**Tip:** Consolidate the rationale scattered across Chapter 18 into a
structured decision record. For each major design choice (typed
quantities over dimensions-only, tag-type units, multiply syntax over
UDLs, `std::ratio` replacement), document: the decision, alternatives
considered, why they were rejected, trade-offs accepted, and conditions
under which the decision should be revisited. Collect dissenting views
(such as Richard Smith's modulo semantics concern) in one place.

### 9. Domain Coverage Attestation - scored 1 (P4133 3.8)

**Tip:** Add a dedicated section listing every affected domain
(safety-critical, aerospace, scientific computing, audio/DSP, finance,
education), who represented each in the review process, and which
domains have not yet provided input. The author biographies in Chapter 6
provide strong implicit evidence - make it explicit. Acknowledge gaps:
for example, has the financial-quantities community reviewed the design?
Have metrology standards bodies been consulted beyond individual authors?

### 10. Prediction Registry - scored 0 (P4133 3.9)

**Tip:** Record every forward-looking claim the paper makes with
falsifiable criteria and a revisit date. Examples: "Compile-time overhead
will remain within 2x of raw arithmetic for typical usage patterns -
revisit after two implementations ship." "At least three of the five
safety-critical domains represented by the author base will report
adoption within three years of standardization." "The teaching story
(Section 19) will be validated by educator feedback - survey after first
university semester using the standard library feature."
