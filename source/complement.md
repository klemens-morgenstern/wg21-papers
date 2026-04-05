# Complement, Not Coexist

## The Change

Replace "coexist" framing with "complement" framing across all
Network Endeavor papers.

"Coexist" implies reluctant tolerance - two things sharing space
because neither could displace the other. "Complement" says each
model makes the other more valuable. The papers already demonstrate
complementary behavior; the word should match.

## The Canonical Line

The Disclosure section of nearly every paper contains a variation of:

> Coroutine-native I/O and `std::execution` address different
> domains and should coexist in the C++ standard.

Replace with:

> Coroutine-native I/O and `std::execution` are complementary.
> Each serves the domain where its design choices pay off.

## Why "Complement" Is Correct

The papers already make the complementary case:

- The design fork (P4088R0 Section 10) shows two columns of
  strengths where neither is a subset of the other
- The bridge papers (P4092R0, P4093R0) show the two models
  working together, not merely alongside
- P4126R0 proposes one I/O implementation consumed by both
  models at zero allocation cost - partnership, not tolerance
- P4003R1 already uses the word "companion"
- P4014R1 Section 12 interleaves sender pipelines and coroutine
  tasks in the same system
- `std::execution::task` is itself both a coroutine and a sender

## Where "Complement" Applies and Where It Does Not

"Complement" describes the relationship between the two *models*
at the domain level:

- Senders serve GPU dispatch, heterogeneous execution,
  compile-time work graphs
- Coroutines serve byte-oriented I/O, type-erased streams,
  separate compilation, ABI stability
- The bridges (P4092, P4093, P4126) connect them at the boundary
- Each model makes the other more valuable

"Complement" does NOT mean accepting `std::execution::task` as
the coroutine side of the partnership. The papers document real
structural costs when the sender model tries to own coroutines:

- Environment parameter fragments task interoperability (P4089)
- AS-EXCEPT-PTR converts routine error_code to exceptions
  (P4007, P4090)
- No symmetric transfer under the sender protocol (P2583)
- Allocator timing mismatch (P4007)
- Compound I/O results do not fit the three-channel model
  without data loss (P4090, P4091)

The coroutine-native task (`task<T>`, one parameter) is ours.
The sender pipeline is theirs. The bridges connect them. Be
generous about the models complementing each other. Hold firm
on the documented costs when the sender model absorbs coroutines
through `std::execution::task`.

## Vocabulary

Preferred words, in order of frequency:

- **complement** / **complementary** - the default. The models
  complement each other
- **companion(s)** - when the sentence needs a noun. "Both
  companions provide structured concurrency." "C++26 ships
  both companions." "Three companions - each contributing
  what the others cannot"
- **contribute** - when describing what each model brings.
  "One contributes optimization visibility. The other
  contributes structural type erasure"

## Kill List

| Kill | Replace with |
| :--- | :----------- |
| coexist / coexistence | complement / complement each other |
| domain split | complementary specializations |
| domain-specific (execution models) | complementary (execution models) |
| separation / the measure of the separation | (rewrite - bridges connect, they do not separate) |
| each serving its domain | each contributing what the others cannot |

## The Task Friction Pattern

The models complement each other. `std::execution::task`
trying to serve both models in one type does not.

Serving both models in one type is where friction originates -
two template parameters, sixteen open issues, constraints that
neither model alone requires. The companion approach accepts
the design fork: each model does what it does best, and bridges
connect them at ~10-14 ns with zero allocations.

**Narrative rule:** Do not lead with this observation. The
paper goes about its business documenting coroutine strengths.
The task friction surfaces when it arises naturally - typically
in a Q&A or comparison section. The posture is incidental
("and by the way"), not prosecutorial.

## Instructions

When editing a paper:

1. Find every instance of "coexist" (including "coexistence",
   "should coexist", "can coexist") and every instance of
   "domain split"
2. Replace with the complementary framing. Match the surrounding
   tone and sentence structure. Not every replacement is the
   canonical line - some are mid-paragraph and need to flow
   naturally. Use the Vocabulary and Kill List sections above
3. Do not weaken the claim. "Complement" is stronger than
   "coexist", not softer. The two models are partners that
   complete each other's capabilities
4. Do not change the meaning. The papers never argue that
   `std::execution` should be removed or modified. That stance
   is unchanged. The word is changing, not the position
5. Do not soften documented findings about `std::execution::task`.
   The complement framing is about the models, not about
   accepting the sender task as equivalent to a coroutine-native
   task
6. Let the task friction observation surface naturally when
   `task` comes up in context. Do not lead with it
7. Preserve the ASCII-only rule (no Unicode, no em-dashes)
