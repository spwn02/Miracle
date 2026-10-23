# Candidate amendment — G2 historical Project archive fidelity and provenance

**Status:** PROPOSED / UNDER REVIEW — **NOT APPROVED, NOT NORMATIVE**
**Proposed under:** [Contract Revision #62](https://github.com/spwn02/Miracle/issues/62), genuine child of [revision protocol #45](https://github.com/spwn02/Miracle/issues/45)
**Baseline:** [GitHub Infrastructure Contract v1.0](../GITHUB-INFRASTRUCTURE-CONTRACT.md), §1, §2, §3.2 at commit `0bf1bfe3c7fe7cf295a6edb2a0962bfff273be4f`, blob `0eeaa0de5d49ceff903b9eb35d6c40bc53cfc74b`
**Authority:** the author's approved local canonical Git state and later explicit decision; this file is a review candidate, *not* a replacement for v1.0.
**No implementation authorization:** native Project history/draft mutation and branch/Issue cleanup remain Stage B, gated behind final accepted [Workspace #29](https://github.com/spwn02/Miracle/issues/29).

## 1. Purpose and compatibility

The project owner has requested a native GitHub Project #3 workspace where historical drafts display their **actual authoritative source contents**, repeated entries are consolidated, a retrospective chronology is grounded in verifiable Git history and genuinely finished repository work is closed out. Link-only Project draft bodies do not satisfy that presentation requirement.

This proposed amendment would refine G2's *Project presentation* rules. It does not rewrite the architecture/API contracts, grant normative authority to Project drafts, change G0–G6 accepted scope, modify source-original hashes, introduce a new UI/database, or authorize immediate historical cleanup.

**Preserve without exception** the existing Infrastructure Contract v1.0's authority hierarchy: designated local Git repository (not Project) governs; no independent **manually edited** contract copies become authoritative; Project Status does not confer acceptance; no fictional Issue, release, date, approval or code change.

## 2. Proposed normative rules (pending owner acceptance)

### 2.1 Source-bearing draft bodies

Retain a historical Project draft only when its relationship to canonical source is unique and reviewable. Its body must contain the **complete relevant Git-source content** for the represented contract/chapter/section, not merely a title, synopsis or link.

A non-authoritative metadata header or footer must identify: source repository/path; exact Git revision (commit and/or blob); section boundaries; identifier(s) used by the canonical historical catalog; content digest; whether the body is a **derived presentation snapshot** rather than the normative source; and a direct Git link. User-visible Markdown should preserve substantive textual content, examples and section hierarchy.

Derived copies must not become independent contract-writing surfaces. Content changes occur via reviewed Git contract PRs, then verified one-way read-only synchronization into Project. Do not silently truncate, paraphrase or omit source text. If Project draft size/API limits make a faithful copy impossible, **do not claim completion**: offer a bounded, uniquely partitioned set of nonduplicating draft records if feasible, or record the limitation and request an owner decision. The source permalink alone is not a compliant substitute for a manageable full-content record.

### 2.2 Safe archive normalization

Before any mutation, inventory all native Project item IDs, their draft bodies, immutable source links, active/archived state, and entries in `docs/contracts/PROJECT-CATALOG.json`. Produce a reviewable proposed **keep / merge / archive-delete** mapping with source ranges and hashes.

Identical representations of the same exact source revision/range may be consolidated; overlapping bodies may be merged **only if unique source material, identity and history remain traceable**. Similar titles, adjacent chapters or repeated headings alone do not justify deletion.

Destructive draft changes require explicit owner approval of the exact target-item manifest, pre-change backups/exports and a reversal plan. Do not mutate locked Git originals or rewrite the catalog's historical claims as if the originals had been different. After any approved batch, verify Project item cardinality, source coverage, retained content and canonical Git references independently. Partial failure stops the batch; no assumed transaction/rollback.

### 2.3 Historical chronology versus planning

Build the *retrospective* Project history from real, referenced Git commits, actual Issue/PR timestamps and accepted documentary evidence. Record which timestamp has which semantics:

- Commit `authorDate` (authorship recorded in Git) and `committerDate` (integration/rewrite event) must be identified separately. Neither automatically proves engineering work began or finished that day.
- GitHub PR `created_at`, `merged_at`, and Issue creation/closure/event timestamps can mark **observable recorded events only**.
- Historical contract source provenance may have separate source dates and later publication/import commits; do not conflate them.
- Missing or conflicting dates stay **unknown or disputed**, not auto-filled.

The retrospective evidence timeline is distinct from Project's **forward-looking Start/Target/Iteration scheduling**. Historical event dates must not be written into those planning fields unless a separate, reviewed mapping explicitly distinguishes their meaning and the owner authorizes it. Do not infer release deadlines, completed phases, retroactive accepted status or milestone sign-offs.

### 2.4 Verified repository closeout

After archival/timeline work has been approved for execution, review each Issue, PR and non-master branch against actual merged commits, acceptance requirements and protection policies. Close only independently verified completed work with evidence; preserve deferred/blocked/owner-signature gates. Merge only changes that are genuinely pending, compatible, reviewed and CI-verified at the expected head; never force-merge an incompatible `gcc` branch with `master`. Delete only provably redundant branches after checking all attached PRs and their merge/closure state.

### 2.5 Stage ordering (nonnegotiable)

**Stage A:** Finish the original native Workspace #29 checklist, including genuine Project workflow pilots, direct Projects-capable MCP integration, the contract-revision review/decision round trip, and explicit owner acceptance.

**Stage B:** Only after Stage A completes may the full-source draft migration, duplicate reduction, historical timeline reconstruction, and Issues/PRs/branches cleanup begin. Reviewing this candidate contract under #45 is a **Stage A governance pilot**, not permission to execute Stage B.

## 3. Review and verification obligations

Before accepting this amendment the maintainer must:

1. Compare the proposed rules to baseline v1.0 §1/§2/§3.2, current Project operating guide and the 172-entry catalog; explicitly resolve any conflict with “no independent manually edited copies.”
2. Approve or revise the concrete rules for content fidelity, draft length/partitioning, destructive deduplication, timeline event semantics and branch closeout.
3. Confirm historical frozen originals and `docs/contracts/SOURCES.json` are untouched. Run `python3 scripts/docs/verify-frozen-contracts.py --require-originals` plus documentation CI and relevant checks.
4. Record a distinct **owner decision to accept the exact normative text** in the proposal Issue and reviewed PR, not merely authorization to draft.
5. If accepted, merge as a separately versioned explicit addendum and link its exact commit/path from Project #3. The baseline v1.0 remains unchanged and retains its original approval history.

**Proposal state:** no final owner decision yet; do not merge this candidate or change Project #3 to Accepted automatically.
