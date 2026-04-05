# Disclose

Point this at any WG21 paper to add, update, or reconcile its disclosure section. The disclosure section is the author's maker's mark - who they are, where they stand, how the paper was made. It appears before the first technical argument. It uses the same language every time so the reader learns the structure once and reads on autopilot thereafter.

This file is the single source of truth for disclosure paragraphs. Each rule has an ID, a **Text:** field containing the canonical text, and a **When:** condition. The **Text:** is replicated verbatim across papers. Deviations from the canonical text are corrected during enforcement.

The disclosure section is assembled from five groups in fixed order: A, B, C, D, Z. Rules within each group appear in number order.

---

<!-- HARD DIRECTIVE: Group A MUST be the first content in the disclosure section. Nothing precedes it. One sentence. One item. This directive survives any tool change, any process change, any refactoring. -->

## Group A: Opening Posture

Always present. Always first. One sentence. One item.

**Text:** The author provides information and serves at the pleasure of the committee.

---

## Group B: Identity

Who the author is, what they maintain, what they have built.

### B1. Network Endeavor Membership

**Text:** This paper is part of the [Network Endeavor](https://wg21.link/p4100r0) ([P4100R0](https://wg21.link/p4100r0)), a project to bring coroutine-native I/O to C++.

**When:** The paper is in `source/network-endeavor/`.

**Specialize:** Only if the paper's existing disclosure already lists companion papers, preserve that listing after the canonical text. Do not import companion paper references from the body of the paper into the disclosure.

### B2. Capy/Corosio Maintainership

**Text:** The author developed and maintains [Capy](https://github.com/cppalliance/capy) and [Corosio](https://github.com/cppalliance/corosio) and believes coroutine-native I/O is the correct foundation for networking in C++.

**When:** The paper references, depends on, or analyzes Capy or Corosio.

**Specialize:** Only name additional libraries (e.g. [Http](https://github.com/cppalliance/http)) if the paper's subject is that library or the paper directly depends on it for its proposed wording or protocol. A paper that merely mentions Http in passing does not trigger this. If only some authors maintain the named libraries, name them explicitly: "Falco and Gerbino developed and maintain..."

---

## Group C: Position

Where the author stands on contested questions.

### C1. Complement with `std::execution`

**Text:** Coroutine-native I/O and `std::execution` are complementary. Each serves the domain where its design choices pay off.

**When:** The paper touches the relationship between coroutine-based I/O and sender/receiver.

### C2. Competing Proposals

**Text:** The author maintains proposals in the coroutine I/O space that address the same problem domain as sender-based networking.

**When:** The paper is a process or governance paper where the author's competing proposals create a conflict of interest.

**Specialize:** Name the specific proposals if the paper discusses them directly.

### C3. Boost Ecosystem Stake

**Text:** The author has published Boost libraries and has a stake in the project's success.

**When:** The paper examines standard library components alongside Boost alternatives.

### C4. Profiles Stake

**Text:** The author believes profiles is a reasonable direction for C++ safety and deserves real-world use before the committee judges the design.

**When:** The paper concerns C++ safety profiles.

---

## Group D: Method and Context

How the paper was made. Historical framing, AI usage. Paper-specific freeform disclosures (not covered by any rule) also go in this group, after the numbered rules.

### D1. Historical Framing

**Text:** This paper examines the published record. That effort requires re-examining consequential papers, including papers written by people the author respects.

**When:** The paper is a historical analysis of committee decisions.

### D2. AI Usage

**Text:** This paper uses AI.

**When:** AI tools produced analysis or predictions in the paper.

**Specialize:** If the methodology is novel or central to the paper's argument, append a sentence describing it: "The analysis in Section N was produced by [tool/method]."

### D3. Public Domain Dedication

**Text:** All original content in this paper is dedicated to the public domain under [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/). This dedication does not affect the non-exclusive rights already granted to ISO/IEC and INCITS through the author's participation in standards development. Anyone may freely reuse, adapt, or republish this material - in whole or in part - as tutorials, documentation, or other teaching materials, with or without attribution.

**When:** The paper title is prefixed with "Tutorial:" or "Tool:".

**Abstract footer:** When D3 is present, also insert the following sentence at the bottom of the Abstract section (after the brutal-summary sentence and any following content, as the final line before the section ends):

> The author dedicates all original content in this paper to the public domain under [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/). It may be freely reused as the basis of tutorials, documentation, and other teaching materials.

If the abstract already ends with that sentence verbatim, leave it. If it ends with a different wording of the same dedication, replace it with the canonical text above.

---

<!-- HARD DIRECTIVE: Group Z MUST be the last content in the disclosure section. Nothing follows it. One sentence. One item. Inform-papers only. Absent for ask-papers. This directive survives any tool change, any process change, any refactoring. -->

## Group Z: Closing Posture

Inform-papers only. Always last. One sentence. One item. Absent for ask-papers.

**Text:** This paper asks for nothing.

---

## Enforcement: Running Disclose on a Document

Point this file at any paper to add, update, or reconcile its disclosure section.

**Paper type.** Determine whether the paper is an ask-paper or an inform-paper. An ask-paper contains "Ask:" in the title or has straw polls requesting committee action. If unclear, ask the user.

### Step 1: Scan the paper

Read the paper's content and front matter. For each rule in Groups B, C, and D, evaluate the **When** condition. Produce two lists: rules that should be present, and rules that should not.

### Step 2: Read the existing disclosure section

If the paper has a `## 1. Disclosure` section, parse it. Identify which groups and rules are already present. Match existing paragraphs to rules by semantic intent - not by exact string match. A paragraph that expresses the same meaning as a rule's canonical text in different words is a match for that rule.

### Step 3: Reconcile

For each rule that should be present:
- **Missing entirely:** Insert the canonical text at the correct position per the group order.
- **Present but different wording:** Replace with the canonical text. The canonical text is the single source of truth.
- **Present and matches:** Leave it.

For each rule that should NOT be present (condition no longer met):
- **Present in the disclosure:** Flag via AskQuestion. Do not auto-remove. Ask: "Rule [ID] ([name]) is present in the disclosure but its condition is no longer met. Remove it?"

For paragraphs that do not match any rule:
- These are paper-specific freeform disclosures. Preserve them in Group D, after the numbered rules.
- When a paragraph contains both rule-matching content and freeform content (e.g., an organizational affiliation interleaved with a library disclosure), extract the freeform content before replacing the rule-matching portion. If the agent cannot cleanly separate the two, flag the paragraph via AskQuestion rather than silently dropping the freeform content.

**Abstract footer.** If D3 is active, apply the abstract footer rule described under D3 after reconciling the disclosure section.

**Formatting.** Canonical text carries its own links where applicable. For citation superscripts (`<sup>[N]</sup>`), the agent applies them from the paper's existing reference apparatus to match the paper's citation style. The agent does not invent links or superscripts that are absent from both the canonical text and the paper.

### Step 4: Reorder

After all insertions and replacements, reorder the disclosure section to match the group order: A, then B-rules in order, then C-rules in order, then D-rules in order, then freeform, then Z.

### Step 5: Verify bookends

- Group A must be the first content. If missing, insert it.
- Group Z must be the last content (inform-papers only). If missing on an inform-paper, insert it. If present on an ask-paper, flag via AskQuestion.

### No disclosure section exists

Create one from scratch: insert `## 1. Disclosure` after the revision history (or after the front matter if no revision history), then assemble all applicable rules per the group order.

---

## Adding a New Rule

When a new paper introduces a conflict of interest, affiliation, methodology, or position that is not covered by an existing rule and is likely to recur in future papers:

1. Determine which group the new rule belongs to: Identity (B), Position (C), or Method/Context (D). Groups A and Z are fixed and never gain new rules.
2. Choose the position within the group that creates the best logical and emotional reading order - earlier items establish context for later items. Assign the sequential number for that position (e.g., B3, C6, D3). Renumber existing rules if the new rule slots between them. Two positions are absolute: Group A is always first and Group Z is always last in the disclosure.
3. Write the canonical text as a single paragraph. Keep it short - one to three sentences. State facts and positions affirmatively. No ghosts (negations that summon what they deny).
4. Write the **When** condition. It must be mechanically testable by reading the paper's content and front matter - no judgment calls.
5. Add a **Specialize** field only if the canonical text needs per-paper adaptation. If every paper uses it verbatim, omit the field.
6. Renumber subsequent rules within the group if needed to maintain sequential numbering.
