---
title: "To boldly suggest an overall plan for C++29"
document: D4162R0
date: 2026-03-27
reply-to:
  - "Vinnie Falco <vinnie.falco@gmail.com>"
audience: EWG, LEWG, DG
---

## Abstract

Twenty-nine implementers said the committee is adding features
faster than they can ship them. This plan listens.

- **Priority 1:** The Network Endeavor
  ([P4100R0](https://wg21.link/p4100r0)<sup>[1]</sup>) -
  the one major new library for C++29
- **Priority 2:** std::execution maturation - give C++26's
  most ambitious library specification room to grow
- **Priority 3:** Everything else waits
- **Process:** Feature freeze in August 2028. The final year
  is for polish, wording, and defect resolution. No new
  papers after the freeze

The committee may receive more than one plan for C++29.
That is healthy. This plan is grounded on
[P3962R0](https://wg21.link/p3962r0)<sup>[2]</sup>, on
twenty years of missing networking, and on working code.

---

## Revision History

### R0: March 2026 (pre-Croydon mailing)

- Initial publication.

---

## 1. Disclosure

The author maintains the libraries that implement the Network
Endeavor. The author has a stake in the outcome. The committee
should calibrate accordingly.

C++20 shipped coroutines in 2020. In the six years since, nobody
explored coroutine-native I/O at scale - except this team. We
built a complete networking stack: timers, sockets, DNS, TLS,
HTTP. Two libraries, three independent adopters, production
trading infrastructure.

We went further. We also built I/O on senders - not because we
believed it was the right approach, but because the committee
deserved a direct comparison. Nobody else performed that
comparison. The papers documenting what we found are in the
mailing
([P4007R0](https://wg21.link/p4007r0)<sup>[3]</sup>,
[P4014R0](https://wg21.link/p4014r0)<sup>[4]</sup>,
[P4088R0](https://wg21.link/p4088r0)<sup>[5]</sup>).

The execution model that claims universality
([P2300R10](https://wg21.link/p2300r10)<sup>[6]</sup>)
never demonstrated that universality in the domain that matters
most for networking: byte-oriented I/O. Six years. No
sender-based networking implementation shipped. No sender-based
networking proposal appeared in the mailing. The universal model
was never tested against the problem it was supposed to solve.

Networking is not on this plan because the author wants it there.
The Direction Group
([P2000R4](https://wg21.link/p2000r4)<sup>[7]</sup>)
listed networking as a priority.
[P0592R2](https://wg21.link/p0592r2)<sup>[8]</sup>,
[P0592R3](https://wg21.link/p0592r3)<sup>[8]</sup>, and
[P0592R4](https://wg21.link/p0592r4)<sup>[8]</sup>
listed networking as must-work-on-first for C++23.
[P0592R5](https://wg21.link/p0592r5)<sup>[8]</sup>
dropped it only because "no composable proposal
exists."<sup>[8]</sup> A composable proposal now exists. The
committee asked for networking. This plan delivers what the
committee asked for.

---

## 2. First things first

C++ ships on time.

C++11 shipped late, C++14 shipped on time, C++17 shipped on
time, C++20 shipped on time, C++23 shipped on time, C++26 will
ship on time, and C++29 will ship on time. But shipping on time
means shipping something that works - not shipping a
specification that implementers cannot deliver.

All good? Agreed? Right, carry on.

## 3. The implementers spoke

At the Kona 2025 WG21 meeting, twenty-nine compiler and library
implementers met and wrote
[P3962R0](https://wg21.link/p3962r0)<sup>[2]</sup>,
"Implementation reality of WG21 standardization." Their message:

> Compiler and library development operates under significant
> resource constraints. Many implementers are volunteers or are
> only partially funded for standardisation work, and there is
> often no dedicated staffing to implement all standardized
> features. As a result, full conformance to recent standards
> remains difficult in practice, with some implementations still
> working toward C++20 conformance with limited capacity to adopt
> newer standards.<sup>[2]</sup>

Implementation cost is not limited to initial development.
Ongoing maintenance, ABI stability, testing matrix growth, and
interactions with existing features all compound. New features
take time away from addressing existing defects.

> When features accumulate faster than they can be fully
> implemented, integrated, and adjusted, they stack on top of
> incomplete or inconsistent foundations, further increasing the
> risk of non-portable code.<sup>[2]</sup>

The implementers asked the committee to "consider ways of slowing
down the addition of features into the standard to allow
implementers to catch up" and to "consider longer standardization
cycles or alternating feature-focused and consolidation-focused
releases."<sup>[2]</sup>

Stroustrup observed the same pattern in
[P1962R0](https://wg21.link/p1962r0)<sup>[9]</sup>: "I
think we are trying to do too much too fast. We can do much or do
less fast. We cannot do both and maintain quality and
coherence."<sup>[9]</sup>

Voutilainen, in
[P3265R3](https://wg21.link/p3265r3)<sup>[10]</sup>,
wrote: "The motivation is to ship something that works, and is
tested to work. If we have to slow things down to achieve that,
so be it."<sup>[10]</sup>

The Direction Group's own
[P2000R4](https://wg21.link/p2000r4)<sup>[7]</sup>
states: "It is not our aim to destabilize the language with a
demand of constant change; rather to help the committee focus on
what is significant to the community as opposed to insignificant
changes and churn."<sup>[7]</sup>

**When features accumulate faster than they can be fully
implemented, they stack on top of incomplete foundations.** That
is P3962R0 in one sentence.

## 4. What this plan is for, and what it is not for

This is a focus plan and a consolidation plan. It responds
directly to
[P3962R0](https://wg21.link/p3962r0)<sup>[2]</sup>
by limiting new work to one library endeavor and devoting the
remaining bandwidth to letting C++26's major features mature.

This is not the Direction Group's
[P2000](https://wg21.link/p2000r4)<sup>[7]</sup>.
That paper covers the broad landscape. This paper decides what
the committee works on first and - equally important - what it
does not work on yet.

Having a plan does not block off-plan papers. But this plan does
something new: it proposes a feature freeze date.

## 5. High-level priority order

The priority order of handling material is thus:

1. **Networking
   ([P4100R0](https://wg21.link/p4100r0)<sup>[1]</sup>)
   and std::execution maturation.** These are the only two items
   that receive priority scheduling in LEWG and LWG. Networking
   is the one new library. std::execution gets room to grow -
   integration papers, follow-on work, and field-driven
   adjustments from the execution authors and the community
2. **In-pipeline library work.** Papers already under active
   consideration in LEWG or LWG continue to completion. No new
   papers enter the pipeline until this backlog is entirely clear
3. **New library papers.** LEWG may consider new papers only
   when its backlog is clear, and only for design review and
   polish. LEWG does not forward new work to LWG unless LWG is
   similarly out of work

**No new language features for C++29.**

- EWG may review and polish language papers if it runs out of
  other work. This helps authors improve their proposals. But
  EWG does not forward new language features to CWG
- CWG focuses exclusively on defect resolution, wording fixes,
  and working down the core issues list
- This gives implementers time to finish implementing C++26
  language features before the next wave arrives

This is the release where the committee takes a breath. The world
does not end if we pause. The implementers asked for time. This
plan gives it to them.

In every previous
[P0592](https://wg21.link/p0592r5)<sup>[8]</sup>
revision, the priority scheme determined what to work on first.
This plan determines what not to work on at all. The pipeline is
a queue, not a firehose.

## 6. The Network Endeavor

C++26 shipped execution
([P2300R10](https://wg21.link/p2300r10)<sup>[6]</sup>),
reflection, and contracts. These are genuine achievements:
structured concurrency, compile-time introspection, and
language-level preconditions. Three features the committee has
pursued for years, delivered.

C++ has been trying to standardize networking since 2005.
[P0592R5](https://wg21.link/p0592r5)<sup>[8]</sup>
dropped it from the C++26 plan because "no composable proposal
exists." That assessment was correct at the time.

A composable proposal now exists.

[P4100R0](https://wg21.link/p4100r0)<sup>[1]</sup>,
"The Network Endeavor," describes a thirteen-paper series built
on two production libraries (Capy and Corosio) with three
independent adopters (Boost.MySQL, Boost.Redis, Boost.Postgres)
and a production trading infrastructure company exploring the
stack. Published code, not a design sketch.

The foundation is the IoAwaitable protocol
([P4003R1](https://wg21.link/p4003r1)<sup>[11]</sup>):
two concepts, one type-erased executor, and a thread-local frame
allocator cache. The specification fits in six pages.

The bridge between execution models is
[P4126R0](https://wg21.link/p4126r0)<sup>[12]</sup>,
"A Universal Continuation Model." P4126R0 enables senders to
invoke any awaitable without allocating a coroutine frame. One
I/O implementation, consumed by both coroutines and senders, zero
allocation for either path. This is the number one
language-adjacent priority for C++29: the key to making
std::execution and coroutine-native I/O coexist without either
side paying a tax.

The supporting body of work:
[P4088R0](https://wg21.link/p4088r0)<sup>[5]</sup>,
[P4089R0](https://wg21.link/p4089r0)<sup>[13]</sup>,
[P4092R0](https://wg21.link/p4092r0)<sup>[14]</sup>,
[P4093R0](https://wg21.link/p4093r0)<sup>[15]</sup>,
[P4095R0](https://wg21.link/p4095r0)<sup>[16]</sup>,
[P4096R0](https://wg21.link/p4096r0)<sup>[17]</sup>,
[P4124R0](https://wg21.link/p4124r0)<sup>[18]</sup>,
[P4125R0](https://wg21.link/p4125r0)<sup>[19]</sup>.

Coroutines serve byte-oriented I/O. Senders serve compile-time
work graphs and heterogeneous dispatch. Both serve real domains.
Neither replaces the other. This plan treats them as
complementary.

[P3962R0](https://wg21.link/p3962r0)<sup>[2]</sup>
said slow down. One major new library is the responsible answer.
Networking is the one because the committee has asked for it for
twenty years and the technical substrate - C++20 coroutines - is
ready.

## 7. Letting the big features grow

std::execution is the most ambitious library specification the
committee has shipped in years. It provides structured
concurrency, sender composition, and a customization point model
that enables heterogeneous dispatch. A specification of this
scope does not reach its full potential in one cycle. It needs
time for implementers to deploy it, for users to discover what
works and what needs adjustment, and for the committee to respond
to what the field reports back.

This plan gives std::execution that time. By deprioritizing new
proposals, LEWG and LWG have bandwidth to process integration
issues, performance discoveries, and the follow-on papers that
the execution authors and the community will bring as the feature
matures in the field. It is the natural second phase of a large feature's lifecycle.

The same applies to reflection and contracts. These are
foundational additions. They will generate follow-on work as the
ecosystem builds on them. The committee needs capacity for that
work.

[P4126R0](https://wg21.link/p4126r0)<sup>[12]</sup>
lives at the boundary between execution and networking. It
enables senders to consume awaitables without frame allocation
overhead. As std::execution matures and networking arrives, this
bridge becomes the integration point. Giving both features room
to grow alongside each other is how the committee gets a coherent
async story for C++29 rather than two isolated silos.

**The best thing the committee can do for std::execution is give
it room to grow.**

## 8. The freeze

At Croydon - the final meeting for C++26 - papers were still
being updated mid-meeting. The last meeting before a standard
ships is not the place for surprises.

This plan proposes a feature freeze: in August 2028, LEWG and EWG
stop accepting new papers for C++29. The cutoff applies to new
proposals, not to revisions of papers already in progress.

After the freeze, the final year - roughly September 2028 through
2029 - is devoted to wording review, defect resolution,
integration testing, and polish. LWG and CWG have the floor.
Design groups shift to issue processing and defect triage.

[P3962R0](https://wg21.link/p3962r0)<sup>[2]</sup>
asked the committee to "consider longer standardization cycles or
alternating feature-focused and consolidation-focused
releases."<sup>[2]</sup> A freeze within the existing three-year
cycle achieves the same goal without changing the cadence.

**The last meeting before a standard ships should contain no
surprises.**

## 9. "Is this plan too aggressive? One new library?"

[P3962R0](https://wg21.link/p3962r0)<sup>[2]</sup>
asked for consolidation. One new library plus room for C++26
features to mature IS the consolidation the implementers
requested. The alternative - business as usual - ignores
twenty-nine implementers who said they cannot keep up.

## 10. "Where is std::execution integration with networking?"

Domain separation. Coroutines for byte-oriented I/O, senders for
compile-time work graphs. Complementary, not competing. SG14
published direction in February 2026 advising that networking not
be built on
[P2300R10](https://wg21.link/p2300r10)<sup>[6]</sup>.

The committee will have bandwidth to explore integration points
precisely because this plan reserves time for both features to
mature.
[P4126R0](https://wg21.link/p4126r0)<sup>[12]</sup>
is the bridge: zero-allocation awaitable consumption from sender
pipelines. Both models gain from it.

## 11. "But networking has failed before. Why is this time different?"

Previous attempts lacked coroutines. The structural properties of
C++20 coroutines - type erasure through `coroutine_handle<>`,
stackless suspension, symmetric transfer, frame as state -
resolve the specific problems that stalled prior efforts. Two
production implementations exist today with independent adopters.
The proposal is a report on working code, not a design sketch.

## 12. "What about safety profiles?"

Profiles are a priority for C++29 - when implementation
experience exists. Safety hardening that works within the
existing language - removing undefined behavior, adding erroneous
behavior, library hardening - is consolidation work and fits in
Priority 2 today.

Profiles as a language feature are carved out from the general
language freeze: once a credible implementation ships and the
committee can evaluate real deployment experience, profiles
advance to CWG. Until then, EWG continues to polish the design.
Implementations appear to be underway; the committee is better
served by waiting until they materialize before making decisions.

This plan does not deprioritize safety. It requires that safety
proposals meet the same evidence bar this plan applies to
everything else: working code before wording.

## 13. "What happens to study groups?"

Study groups continue to do what they do: research, explore,
develop proposals. The freeze applies to forwarding new features
to CWG and LWG, not to design work. SG1, SG7, SG21, SG22, SG23
all continue. Their work matures during the consolidation cycle
and arrives ready for C++32.

This is the implementation of
[P3962R0](https://wg21.link/p3962r0)<sup>[2]</sup>'s
request for "alternating feature-focused and consolidation-focused
releases."<sup>[2]</sup>

## 14. "Can off-plan proposals go into a Technical Specification?"

Yes. The freeze applies to the IS working draft. Technical
Specifications remain available for proposals that need field
testing. This is the intended escape valve.

## 15. "What about my favorite idea X, Y, foo and bar?"

This plan reserves bandwidth for exactly one new library and for
letting C++26's features mature. Bringing more new work requires
demonstrating that it does not displace that capacity. The
implementers were clear. This plan takes them seriously.

---

## References

| #    | Document                                                                                                                               |
| ---- | -------------------------------------------------------------------------------------------------------------------------------------- |
| 1    | [P4100R0](https://wg21.link/p4100r0) "The Network Endeavor: Coroutine-Native I/O for C++29," Falco et al.                             |
| 2    | [P3962R0](https://wg21.link/p3962r0) "Implementation reality of WG21 standardization," Ranns et al.                                   |
| 3    | [P4007R0](https://wg21.link/p4007r0) "Senders and Coroutines," Falco et al.                                                           |
| 4    | [P4014R0](https://wg21.link/p4014r0) "The Sender Sub-Language," Falco et al.                                                          |
| 5    | [P4088R0](https://wg21.link/p4088r0) "The Case for Coroutines," Falco et al.                                                          |
| 6    | [P2300R10](https://wg21.link/p2300r10) "std::execution," Niebler, Baker, Shoop et al.                                                 |
| 7    | [P2000R4](https://wg21.link/p2000r4) "Direction for ISO C++," Vandevoorde, Hinnant, Orr, Stroustrup, Wong                             |
| 8    | [P0592R5](https://wg21.link/p0592r5) "To boldly suggest an overall plan for C++26," Voutilainen                                       |
| 9    | [P1962R0](https://wg21.link/p1962r0) "How can you be so certain?" Stroustrup                                                          |
| 10   | [P3265R3](https://wg21.link/p3265r3) "Ship Contracts in a TS," Voutilainen                                                            |
| 11   | [P4003R1](https://wg21.link/p4003r1) "The Narrowest Execution Model for Networking," Falco et al.                                     |
| 12   | [P4126R0](https://wg21.link/p4126r0) "A Universal Continuation Model," Falco, Morgenstern                                             |
| 13   | [P4089R0](https://wg21.link/p4089r0) "On the Diversity of Coroutine Task Types," Falco et al.                                         |
| 14   | [P4092R0](https://wg21.link/p4092r0) "Consuming Senders from Coroutine-Native Code," Falco et al.                                     |
| 15   | [P4093R0](https://wg21.link/p4093r0) "Producing Senders from Coroutine-Native Code," Falco et al.                                     |
| 16   | [P4095R0](https://wg21.link/p4095r0) "Retrospective: The Basis Operation and P1525," Falco et al.                                     |
| 17   | [P4096R0](https://wg21.link/p4096r0) "Retrospective: Coroutine Executors and P2464R0," Falco et al.                                   |
| 18   | [P4124R0](https://wg21.link/p4124r0) "Combinators and Compound Results from I/O," Falco et al.                                        |
| 19   | [P4125R0](https://wg21.link/p4125r0) "Field Report: Coroutine-Native I/O at a Derivatives Exchange," Falco et al.                     |
| 20   | [P2464R0](https://wg21.link/p2464r0) "Ruminations on networking and executors," Voutilainen                                            |
