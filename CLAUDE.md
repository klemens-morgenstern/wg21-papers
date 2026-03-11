# WG21 Paper Style Rules

Rules for all work on WG21 papers in this repository.

## Formatting

1. Markdown files must be ASCII only - no Unicode characters
2. Diacritics in personal names must never be omitted -
   represent them using HTML character references. Prefer
   named character references (`&uuml;`) over numeric
   character references (`&#252;`),
   e.g. `Dietmar K&uuml;hl`, `Micha&lstrok; Dominiak`
3. Single dashes (`-`) for all dashes - no em-dashes, no double dashes
4. Human-written tone - not wordy, no AI-characteristic phrasing
5. No git operations - user handles all git manually
6. The `archive/` folder is restricted for human use only - do
   not read, write, or modify any file in that folder
7. Padded tables with aligned pipe characters in every table on every rewrite
8. WG21-appropriate style
9. `[P####R#](https://wg21.link/p####r#)` link convention for all paper references
10. Paper titles on first mention
11. Backticks for code identifiers in headings

## Front Matter

12. Every paper begins with a valid YAML front matter block
    delimited by `---`. Required and optional fields:

    ```yaml
    ---
    title: "Paper Title"
    document: D0000R0
    date: YYYY-MM-DD
    reply-to:
      - "Author Name <author@example.com>"
    audience: SG1, LEWG
    ---
    ```

    - `title` - required, double-quoted string
    - `document` - required, unquoted paper number
      (e.g. D4007R0)
    - `date` - required, unquoted ISO 8601 date
    - `reply-to` - required, YAML list of double-quoted
      strings in the form `"Name <email>"`
    - `audience` - required, unquoted comma-separated
      target audience (e.g. SG1, LEWG)
13. Do not add front matter fields that the Pandoc template
    (`tools/wg21.html5`) does not consume

## Language and Grammar

14. American English unless otherwise specified
15. Follow the Chicago Manual of Style unless otherwise specified
16. Use the Oxford comma
17. Avoid dangling "This" - every sentence beginning with "This" must
    have a clear, unambiguous antecedent. If "This" could refer to
    more than one thing, replace it with a specific noun phrase
18. Do not end sentences with a preposition in formal papers -
    e.g. "worth building upon" not "worth building on"
19. "bound" = attached/tied to; "bounded" = limited -
    e.g. "an allocator bound to that tenant's quota"
20. Possessive before a gerund -
    e.g. "coroutines' interacting" not "coroutines interacting"
21. "may" = permitted/possible; "might" = hypothetical/enabled but
    not currently supported. Use precisely
22. Prefer "deeper" or "more thorough" over "full" for appendix
    cross-references. Understatement is better than hyperbole
23. Headings must not contain a dangling "This" - replace with a
    specific noun phrase. e.g. "ABI Lock-In Makes the Architectural
    Gap Permanent" not "ABI Lock-In Makes This Permanent"
24. Transitive verbs must not leave their object implicit when the
    referent is not immediately obvious. Supply the object explicitly.
    e.g. "forgetting to forward the allocator" not "forgetting to
    forward"
25. Avoid ambiguous prepositional attachment. When a prepositional
    phrase could modify either a nearby noun or the main verb,
    restructure to make the attachment clear.
    e.g. "The allocator - along with its state - must be
    discoverable" not "The allocator - with its state - must be
    discoverable" (where "with its state" could attach to
    "discoverable")
26. Place "only" immediately before the word, phrase, or clause it
    modifies to avoid ambiguity.
    e.g. "provides only one mechanism" not "only provides one
    mechanism"
27. Do not use contractions (it's, they're, don't, etc.) in formal
    papers. Use the expanded form

## Code Examples

28. Consistent comment style within a code block - either all
    comments are sentences (capitalized, with terminating punctuation)
    or all are fragments (lowercase, no period). Do not mix
29. Consistent capitalization in groups of code comments that
    summarize arithmetic or data
30. Align trailing comment columns when consecutive lines have
    trailing comments
31. Fenced code blocks must not exceed 90 characters per
    line. This rule does not apply to mermaid blocks -
    never reformat mermaid source. When wrapping is
    needed, follow the conventions for the block's
    language:

    **C++** - break at a natural syntactic boundary and
    indent the continuation 4 spaces from the construct
    it belongs to:
    - Function signatures: break after the return type,
      or after the opening parenthesis
    - Template parameter lists: break after a comma,
      align with the first parameter or indent 4 spaces
    - Chained expressions (pipe `|`, `<<`, etc.): break
      before the operator, align operators vertically
    - Trailing comments: shorten the gap between code and
      comment rather than wrapping the code, but keep
      grouped comments aligned with each other (rule 30)
    - If a trailing comment still exceeds 90 characters
      after tightening the gap, wrap the comment text to
      a second line. The continuation `//` must align
      with the `//` on the first line. Never move a
      trailing comment above the code line - keep all
      comments in a contiguous block on the right

    **Prose in code fences** (markdown, plain text) -
    wrap at word boundaries with no continuation indent

## Lists

32. Always insert a blank line before a bullet or numbered list.
    Pandoc does not recognise a list that immediately follows a
    prose line without a separating blank line - the items render
    as inline text in both HTML and PDF

## Structure

33. Sections with subsections should have at least one introductory
    sentence between the section heading and the first subsection
    heading - do not leave an empty section heading
34. Every paper must contain a `## Revision History` section,
    placed immediately after the Abstract's trailing `---` and
    before Section 1. Each revision is an H3 subheading in the
    form `### R<n>: <Month> <Year> (<pre-Meeting mailing>)`
    followed by a bullet list of changes
35. References and citations - for readers of printed copies:
    - Every hyperlink in the document body must also appear
      in the References section
    - Every hyperlink in the document body must have a
      superscripted citation marker giving the reference
      number, e.g. `<sup>[7]</sup>`
    - Every entry in the References section must include a
      readable URL (not hidden behind markdown link text)
    - Group related links under a single reference entry -
      e.g. one "C++ Working Draft" entry for all
      `eel.is/c++draft/` sections, one entry for the NB
      ballot repository covering individual issue links
    - Use `wg21.link` short URLs for WG21 papers - do not
      use `open-std.org` long-form URLs (see rule 9)
    - Links inside markdown tables are exempt from citation
      markers (tables are dense reference lists where
      superscripts would harm readability)
    - Links inside the Acknowledgements section are exempt
      from citation markers
    - Superscripted citation numbers in the body must always
      match the corresponding entry number in the References
      section - update all citations when references are
      added, removed, or renumbered

## Proposed Wording Sections

36. Proposed wording uses Pandoc fenced divs and HTML
    `<ins>`/`<del>` elements styled by `tools/paperstyle.css`.
    Three div classes are available:

    - `:::wording` - unchanged spec text (gray box, gray
      left border). Inline code inherits neutral color
    - `:::wording-add` - purely additive text (green box,
      green left border, green text)
    - `:::wording-remove` - purely removed text (red box,
      red left border, red strikethrough text)

    For mixed changes (unchanged text with inline additions
    or deletions), wrap the block in `:::wording` and mark
    individual changes with `<ins>` (green underline) or
    `<del>` (red strikethrough).

    Do not use the dual-blockquote pattern (separate
    "Current text:" and "Proposed text:" blockquotes).
    Do not use separate "Current:" and "Proposed:" fenced
    code blocks for code changes. Always use the inline
    `<ins>`/`<del>` convention described here instead

37. Fenced div formatting rules:

    - No space between `:::` and the class name:
      `:::wording` not `::: wording`
    - Blank line after the opening `:::wording*` marker
    - Blank line before the closing `:::`
    - Do not use blockquote `>` syntax inside wording
      divs - use plain paragraphs. The div provides
      visual framing
    - Markdown formatting (`*italic*`, `` `code` ``)
      works normally inside wording divs

38. Code diffs in wording sections use raw HTML
    `<pre><code>` blocks (not fenced `` ``` `` blocks)
    inside a `:::wording` div, with `<ins>` and `<del>`
    for inline changes. Angle brackets in code must be
    escaped as `&lt;`/`&gt;`. Example:

    ```
    :::wording

    <pre><code><del>void</del><ins>coroutine_handle&lt;&gt;</ins> await_suspend(
        coroutine_handle&lt;Promise&gt;) noexcept
    {
        <del>start(state);</del>
        <ins>auto h = start(state);</ins>
        <ins>return h ? h : noop_coroutine();</ins>
    }</code></pre>

    :::
    ```

## Tone

39. Do not present options as predetermined conclusions. When
    recommending alternatives to a committee, present them as options
    to contemplate, not dictated outcomes
40. Avoid politically charged comparisons - do not invoke other
    contentious features as analogies unless the comparison is
    structurally precise. If the structures being compared are
    fundamentally different, the analogy will be perceived as
    political
