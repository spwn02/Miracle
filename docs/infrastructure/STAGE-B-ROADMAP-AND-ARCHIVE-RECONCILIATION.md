# Stage B — Corrective roadmap and historical archive reconciliation

**Status: IMPLEMENTATION INCOMPLETE · Proposal and verified structural inventory · 2026-10-26**

**Tracking:** [Stage B Issue #69](https://github.com/spwn02/Miracle/issues/69) and the wider [G2](https://github.com/spwn02/Miracle/issues/2), [G4](https://github.com/spwn02/Miracle/issues/4) and [G6](https://github.com/spwn02/Miracle/issues/6) acceptance gates. Normative authority is the approved [G2 Archive Fidelity & Provenance Addendum](amendments/G2-NATIVE-ARCHIVE-PROVENANCE-ADDENDUM-V1.0.md).

## What exists and what does not

The Project #3 source-body migration is complete and independently audited (172/172). **No historical draft consolidation has occurred.** An earlier `KEEP 172 / MERGE 0` review was not a fulfillment of the request for merged historical records. Prior Git/PR/Issue event ledgers are not a mapped native roadmap.

The live Project `Roadmap` filter is `is:issue is:open disposition:Planned`. It omits **every historical DraftIssue and completed Issue**, and Project #3 has **no populated Start/Target dates** for any of its 190 items (172 DraftIssues, 18 Issues) at the observed snapshot. There cannot be a source-backed history visualization through that filter alone.

## I. Verified, bounded historical archive proposal

The source-bearing backup and independent SHA snapshot allow identity-preserving consolidation, but **owner approval for exact targets remains outstanding**.

| Original source group | Current | Proposed retained | Provisional merges |
| --- | ---: | ---: | ---: |
| Foundation | 97 | 8 (7 contracts + 1 decision ledger) | 89 |
| Phase 5.4 | 29 | 1 phase/contract overview | 28 |
| Phase 5.6 | 21 | 1 phase overview + 8 checkpoints | 12 |
| Development Protocol | 16 | 1 complete contract | 15 |
| Architecture | 9 | 1 expanded overview | 8 |
| **TOTAL** | **172** | **20** | **152** |

A separate machine-readable proposed mapping enumerates all 172 immutable catalog IDs, their native Project and DraftIssue node IDs, source spans and hashes, chosen retained target, proposed disposition and prerequisites. Every proposed removed span is already contained within its retained parent source span, **except Architecture**, where the retained original `architecture:chapter:0003` must first be extended from lines 3–18 to the authoritative file's complete lines 1–94. This remains a **pre-change requirement**.

The eight Phase 5.6 checkpoints are retained as independently linkable historical checkpoint identities, even though their contract text is included in the complete Phase overview. Their preserved identity never implies engineering acceptance or completion.

The exact Phase-vs-Contract same-source duplicate pairs can be consolidated into one canonical Project presentation while recording both historic IDs and distinct semantics as aliases; deleting their original records requires the addendum's separate exact-target owner authorization. Until then, **keep all 172** and do not rewrite catalog history. A merge means *content and identity preservation plus reduced Project cardinality*, not merely copying full Markdown to 172 separate cards.

## II. Commit-backed historical roadmap, not invented scheduling

GitHub's native Roadmap supports configurable start and end **custom date fields**. The existing Project `Start date` and `Target date` describe forward planning. Under the approved addendum §2.3, they must not silently receive reconstructed Git event dates.

**Correct technical design, pending reviewed field-semantic mapping:**

1. Generate an evidence ledger from Git *reachable source and test changes*, with commit SHA, affected C++26 module/area, author time and committer time independently. The bounded `scripts/github/stage-b-reconstruct-history.py` and owner-only read-only Actions workflow implement the first pass, with no native Project writes. Commit subjects and imported `docs/phases/` files alone do not prove a historical phase's start or completion.
2. Link actual release/tag events and review-approved Phase/Checkpoint assignments explicitly, retaining unmatched commits and disputed boundaries as `unknown`. A contract's import/publication date must not stand in for an implementation date.
3. Propose separate native date fields **Observed first relevant Git event** / **Observed last relevant Git event**, with explicit `committerDate` event semantics and immutable evidence links on each retained phase/milestone item. Review and approve the mapping under §2.3 before writing any such fields.
4. Configure a separate **Historical Development** native roadmap layout using those custom date fields and a filter that includes **historical retained Phase/Checkpoint identities**, independent of the forward-looking Roadmap. GitHub's Roadmap Date fields UI supports selecting custom date fields. Do not overwrite the forward scheduling view.
5. Populate exact, reviewed historical evidence on genuine native entries only; read back dates, item identities, source coverage and view configuration. Never claim phase completion or `Accepted` solely from Git events.

**Remaining blocker:** the current connected Projects Write Pilot can update existing fields/views but does not expose creation of arbitrary date fields or the Roadmap's date-field-axis configuration. These operations require a reviewed bounded Projects GraphQL route or explicit maintainer UI setup after contract review. A table of Git timestamps is not a substitute for completed Project-native synchronization.

## III. Formal GitHub infrastructure phase closure

This cannot be conflated with closure of Stage B. Real unresolved program gates include:

- **G2 (#2):** GitHub provider's `wasAutomated=false` event-semantics discrepancy and explicit owner G2 acceptance.
- **G4 (#4):** genuine protected-tag product release verification, compatible protected `gcc` advancement, and maintainer G4 acceptance; green workflow runs are not proofs of those events.
- **G6 (#6):** genuine Q&A answer-selection and confidential reporting validation, full engineering lifecycle evidence, and explicit owner G6 acceptance.
- **Historical contract Issue #7** and related deliberate deferrals must retain their actual independent criteria; completed infrastructure does not automatically accept missing frozen source or future product work.
- **Deferred MCP Issue #60** remains independent and open.

Before declaring GitHub Phase complete: finish archive merging and real native chronological views, re-audit all actual G0–G6 requirements, explicitly disposition unresolved blockers without inventing waiver, independently inspect Project #3 and obtain scope-specific owner acceptance. A Stage B audit green check is insufficient.

**No archive has been deleted or merged by this document or the read-only Git-history generator.** It is the corrective contract-aware implementation path, not a completion certificate.
