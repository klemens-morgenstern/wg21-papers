---
title: "Big Reputation Requires Big Responsibility"
document: P4060R0
date: 2026-03-14
reply-to:
  - "Vinnie Falco <vinnie.falco@gmail.com>"
audience: LEWG, SG1, Direction Group
---

## Abstract

The senior members of this committee have earned their standing through decades of service, sound judgment, and genuine care for the language. That standing is a gift from the community. This paper is about the responsibility that comes with the gift.

Reputation is the committee's informal currency. It is earned over decades and spent in every interaction. When reputation substitutes for evidence, the committee is doing social deference, not technical review. When reputation is withheld at a moment that demands intervention, modesty becomes abdication. This paper addresses both failure modes - not to diminish anyone's contribution, but to ask that the weight of standing be carried with the same care that earned it.

This is the second paper in a trilogy. "Big Claims Require Big Evidence" ([P4059R0](https://wg21.link/p4059r0)) addresses the committee's process. "Big Papers Require Big Humility" ([P4061R0](https://wg21.link/p4061r0)) addresses the architect's character. This paper addresses the space between: the influence that individual standing exerts on collective decisions.

---

## Revision History

### R0: March 2026 (post-Croydon mailing)

- Initial version.

---

## The Weight You Carry

*When I speak in a committee room, do people evaluate my argument or my name?*

*If a junior member made the same argument with the same evidence, would the committee reach the same conclusion?*

Reputation in WG21 is earned through years of correct judgments, successful proposals, and institutional service. It is real. It is deserved. And it distorts every interaction it touches.

When a senior member speaks, the room listens differently. The argument is heard through the filter of the member's track record. A claim that would be challenged from a newcomer is accepted from a veteran. A concern that would be investigated from a domain expert is dismissed when it contradicts a respected architect. This is not malice. It is human nature. We trust people who have been right before.

But the committee's job is not to trust. It is to verify. When reputation substitutes for verification, the committee has stopped doing its job - and the senior member whose reputation enabled the substitution bears a proportional responsibility for the outcome.

Your reputation is not yours to control. The committee gives it to you. But you are responsible for recognizing when the committee is accepting your claims on the strength of your name rather than the strength of your evidence. The weight you carry is not optional. How you carry it is.

---

## Reputation Is Not Evidence

*Have I ever seen the committee accept a claim from me that it would have challenged from someone else?*

*Did I correct the record, or did I let it stand?*

A senior member posts a small third-party library as evidence that a framework works in an adjacent domain. The library is not production-scale. A junior member posting the same library would be told: "This is a toy. Show us production evidence." The senior member is not told this. The committee accepts the library as an existence proof because the senior member posted it.

A respected chair writes a paper that reverses a long-standing direction. The paper's analysis identifies real problems. But the paper's prescription - stop all work on the alternative - goes further than the analysis supports. A junior member writing the same paper would be asked: "You have shown the problem. Where is the evidence that your proposed replacement solves it?" The respected chair is not asked this. The committee follows the prescription because the chair wrote it.

In both cases, the committee substituted reputation for evidence. The senior member did not ask for this substitution. But the senior member benefited from it. And the cost - years of lost progress, a domain expert's departure, users waiting for a facility that competing languages already provide - was paid by others.

When you see the committee accepting your claim on the strength of your name, correct the record. Say: "I have not demonstrated this. The committee should require more evidence before proceeding." That correction is the most valuable thing your reputation can buy.

---

## The Direction-Changing Paper

*Have I written a paper that killed an alternative?*

*Did I provide proportional evidence for what would replace it?*

*If nothing replaced it, do I own that cost?*

Some papers change the committee's direction. They do not propose wording. They do not add features. They argue that the committee should stop doing one thing and start doing another. These papers are among the most consequential the committee receives, and they are among the least scrutinized - because they are written by respected members whose judgment the committee trusts.

A direction-changing paper written by a senior member carries the member's full reputation. The committee reads it not as one engineer's analysis but as a verdict from a trusted authority. The paper's effect is amplified beyond what the evidence alone would support. A junior member writing the same paper would face questions the senior member does not: Where is the production evidence for the replacement? What happens to the domain if the replacement does not deliver? Who bears the cost if the alternative is removed and nothing takes its place?

The responsibility is proportional to the amplification. If your paper kills an alternative, you own what follows. If the replacement delivers, you were right. If the replacement does not deliver - if the domain ships nothing, if the domain's expert leaves, if users wait for years - the cost is partly yours. Not because you intended it. Because your reputation made the committee follow a prescription that the evidence alone would not have supported.

---

## The Amplifier Effect

*Has my "I think this is fine" ever ended a discussion that needed to continue?*

*Has my silence during a contentious poll been read as agreement?*

A senior member's opinion in a room shifts the temperature. A casual endorsement can end a debate that needed another hour. A raised eyebrow during a presentation can undermine a proposal that deserved a fair hearing. A vote from a respected member can swing a poll that was otherwise balanced.

These effects are real. The member may not be aware of them. The amplifier effect means that senior members do not have the luxury of casual opinions. Every statement carries weight. Every silence carries weight. A senior member who says nothing during a contentious discussion is not abstaining. The room reads the silence as consent - or as indifference, which is worse.

The amplifier effect is strongest in small rooms. A study group with fifteen attendees, where three are senior members, is a room where three opinions carry the weight of ten. The junior members watch the senior members before they vote. This is not cowardice. It is rational deference to experience. But when the senior members have not done the domain-specific homework, the deference is misplaced - and the senior members are responsible for the misplacement, because they accepted the deference without earning it on the specific question at hand.

General credibility does not transfer to specific claims. A member who is right about language evolution is not automatically right about networking. A member who is right about GPU dispatch is not automatically right about I/O error handling. The amplifier does not distinguish between domains. The member must.

---

## Lending Your Name

*When I co-author a paper, does my name transfer credibility the paper has not earned on its own merits?*

*Am I certain the paper deserves the weight my name gives it?*

Co-authorship is a transfer of reputation. When a respected member's name appears on a paper, the committee reads the paper differently. The member's track record becomes part of the paper's evidence. This is appropriate when the member has done the work - reviewed the design, verified the claims, tested the implementation.

It is inappropriate when the member's name is lending credibility to claims the member has not independently verified. A paper with six authors from one employer, where one author is a respected committee figure, carries that figure's reputation into every claim the paper makes. If the figure has not verified every claim, the figure's reputation is being spent without the figure's knowledge.

The risk is compounded when the same member holds an institutional role. A committee chair who co-authors a proposal and then authors the direction polls for that proposal's domain has transferred their institutional credibility twice: once to the proposal through co-authorship, and once to the polls through the chair role. The committee may not notice the double transfer. The chair should.

Lend your name deliberately. Verify what you endorse. If you cannot verify a claim, say so publicly. Your name on a paper is a promise to the committee that you stand behind its contents. Do not make that promise lightly.

---

## What Restraint Costs

This section is different from the others. Every preceding section warns about using reputation carelessly. This one warns about not using it at all.

*Is there a decision being made right now that I believe is wrong, where my intervention would change the outcome, and I am choosing not to intervene?*

*If I co-authored a direction paper that listed a priority, and the committee abandoned that priority, did I hold the committee accountable - or did I let it go?*

*Is my modesty serving the users, or is it serving my comfort?*

Modesty is a virtue in a person. It is not always a virtue in a steward. When you hold a position of unique influence - when your voice carries more weight than any other in the room - silence is not neutral. Silence is a choice. And when the committee makes a decision that harms users, the person with the most standing to challenge it bears a proportional responsibility for choosing not to.

A direction paper lists networking as a high priority. The committee abandons networking. The direction paper's author says nothing. A twenty-year library is removed from consideration. The library's author stops attending. The direction paper's author says nothing. Five years pass. Users have no standard networking. Competing languages do. The direction paper's author says nothing.

The restraint is genuine. The modesty is real. The author does not want to use their unique standing to override the committee's process. That instinct is honorable. But the users who are waiting for networking do not benefit from the author's honor. They benefit from the author's intervention. And the intervention never came.

This is the hardest failure mode to name because it looks like a virtue. The member is not doing anything wrong. They are not using their reputation carelessly. They are not lending their name to unverified claims. They are simply choosing not to act. But when you are the one person whose action would change the outcome, choosing not to act is choosing the outcome.

What restraint costs is measured not in what happened but in what did not.

---

## Tenure

Tenure is the freedom to speak without consequence. It is also the experience to know when speaking matters most. The members with the most of both use the least of either.

This paper has described two failure modes. One is using reputation where evidence should speak. The other is withholding reputation where intervention is needed. They are mirrors. Both are failures of stewardship. Both harm users. The first harms users by advancing proposals beyond their evidence. The second harms users by allowing harmful directions to proceed unchallenged.

Tenure means you have been here long enough. You have seen proposals rise and fall. You have seen domain experts arrive and depart. You have seen the committee invest years in a direction and discover too late that the direction was wrong. You have the pattern recognition to see it happening again. The question is not whether you see it. The question is what you do when you do.

The standard outlives every proposal. It outlives every committee member. The decisions made in these rooms shape what millions of programmers can and cannot do for decades. The members with the most tenure have seen the most consequences. They know what bad decisions cost. They know because they were there when the cost was paid.

Every senior member will see themselves in one section of this paper or the other. The honest ones will see themselves in both.

This paper is the second in a trilogy. "Big Claims Require Big Evidence" ([P4059R0](https://wg21.link/p4059r0)) proposes process safeguards for the committee. "Big Papers Require Big Humility" ([P4061R0](https://wg21.link/p4061r0)) addresses the architect's character. Together, the three papers address the institution, the influence, and the individual. The standard needs all three.

---

## Acknowledgments

The following individuals were asked to review a draft of this paper. If your name appears below and you consent to being listed in the published version, please let the author know. Names will be removed before publication unless consent is given.

- Bjarne Stroustrup ( Reviewed, Accepted, Declined )
- Ville Voutilainen ( Reviewed, Accepted, Declined )
