# Miracle Development — living workspace protocol (pilot v0)

**Status:** PROPOSED / NOT ACCEPTED. **Tracking:** [Issue #29](https://github.com/spwn02/Miracle/issues/29).
**Existing workspace:** [Miracle Development Project #3](https://github.com/users/spwn02/projects/3).
**Related authoritative work:** [G2 operating plan](G2-OPERATING-PLAN.md), [G2 progress](G2-PROGRESS.md), [contract archive](../contracts/index.md), [historical catalog](../contracts/PROJECT-CATALOG.json).

## Goal

Operate Miracle development **in** GitHub instead of using Projects as a passive index. A maintainer or agent can capture a bug or deferred capability, relate it to a contract and phase, record dependencies, plan it in the roadmap, implement and verify it, and preserve its decision trail. Tables, boards, a timeline and a whiteboard are **views of the same records**, not independent status stores.

This document specifies a controlled extension of G2. It neither closes Issue #2's unresolved GitHub GraphQL `wasAutomated` acceptance gate nor changes its existing Project statuses, workflows, fields or milestone. The v0 graph is a **read-only pilot**, not a claim that live Project state has been synchronized.

## Authority / no double truth

| Data | Owner | Mirrored/derived |
| --- | --- | --- |
| Proposed features, bugs, deferred ideas, tasks, discussions | GitHub repository Issues | Project #3, dependency graph |
| Current execution status, priority, risk, target dates, planning phase | Existing Project #3 fields | Agents/graph may read, never infer a missing value |
| Actual dependencies and subtask relationships | Native GitHub Issue `blocked by` / `blocking` and sub-issues | Graph directed edges |
| Approved or frozen normative contract text and provenance | Version-controlled Git source plus recorded approval | Project may show summary/link, cannot silently edit canonical text |
| Working contract revisions and architecture decisions | Issue proposal + reviewable PR | Project contract-review/decision views |
| Tests, CI outcomes, release artifacts | Git commits, workflow records, tags | Project may cite evidence; acceptance still requires explicit maintainer action |
| Historical chapter inventory | `docs/contracts/PROJECT-CATALOG.json` (172 verified entries) | Graph `contains` edges and historical reference browser |

**Immutable boundaries:** Preserve historical DOCX/Markdown byte integrity, `SOURCES.json`, the source-heading verifier, and the fact that the Phase 5.4 Markdown is a reading edition. A Project edit **never** patches a frozen file or turns a historical chapter into an implemented feature. Historical catalog IDs remain stable. Do not bulk-create 172 repository Issues from the catalog.

## Record types

A **work item** is a genuine repository Issue. Existing G1.1 issue intake types and `type:*` labels remain recognized. Newly discovered needs can enter at any time, including during implementation, without inventing a delivery date.

- **Feature / enhancement:** rationale, scope, acceptance criteria, affected API/area, source contract reference (if applicable), status and priority assigned on triage.
- **Bug / regression:** reproduction, expected/observed behavior, affected toolchain, failing test; security vulnerabilities use private reporting.
- **Deferred / research:** reason for deferral, prerequisite or re-evaluation trigger, known risks; leave unscheduled until accepted for delivery.
- **Task / checkpoint:** finite testable outcome, parent/related feature, real evidence and decision gate.
- **Contract proposal / decision:** proposed delta and alternatives, affected historical text, owner decision record and PR; frozen originals remain untouched.

For a cross-repository graph, identify work nodes by `owner/repo#number`, not only by a title. Catalog nodes use `catalog:<stable-id>`. Never match records by mutable title or line number alone.

### State dimensions

- **Execution**: reuse existing `Status` with `Backlog, Design, Ready, Implementing, Review, Verification, Accepted, Blocked`; `Accepted` is gated by explicit owner approval, not an issue-close or PR-merge event.
- **Contract**: `Proposed / In review / Approved / Frozen / Superseded`; historical freeze evidence cannot be inferred from a title.
- **Scheduling**: `Deferred` is a planning disposition, **not** a fabricated phase or a replacement for `Status`. Retain genuine blocker links, reason and revisit condition. Proposed `Disposition` field: `Unscheduled, Candidate, Scheduled, Deferred, Dropped`; add only after inspecting live Project custom-field availability and verifying no equivalent already exists.

An issue may be in `Backlog` and `Deferred`, or a contract may be `Frozen` while its implementation is still `Implementing`. These do not conflict.

## Relationships and the whiteboard

Native GitHub Issue dependencies are the authoritative **dependency** edges:
`A --blocks--> B` means B cannot proceed until A is resolved. GitHub Issue sub-issues are authoritative **work decomposition**; historical catalog parent links are **document containment**, not development dependencies.

The graph must render the types differently; it must never convert chapter ordering, phase numbering, citations, or PR linkage into a blocking dependency. Cycles in the blocker relation are invalid and should be flagged, never silently reordered. A missing dependency or inaccessible issue must be reported, not omitted as if resolved.

The v0 graph generator accepts the verified chapter catalog and an optional read-only Issue snapshot; it emits a typed `nodes/edges` JSON manifest for the separate static [graph view](../workspace/graph.html). Edges only appear when their source contains a recorded parent or blocking relationship. The graph is deliberately **not** the Project's database.

Future full whiteboard acceptance: zoom/pan, draggable layout, filtering by type/phase/area/status/disposition, real dependency arrows, issue detail navigation, blockers/critical-path inspection and explicit stale-data timestamp. Graph editing must call a permissioned API/MCP action to update the **native relationship**, not mutate an independent JSON edge.

## Views in Project #3

**Keep the four existing G2 views and eight-status field intact.** Propose additional saved views only after read-only field/view audit:

| View | Filter / purpose |
| --- | --- |
| Inbox / Backlog | New, untriaged genuine work; priorities not guessed |
| Deferred & Research | Proposed disposition `Deferred`, revisit triggers and unmet prerequisites |
| Bugs & Quality | Confirmed bug and toolchain work, area, priority, regression evidence |
| Dependencies / Blocked | Native `is:blocked` / `is:blocking` filtering, blocker triage |
| Contracts & Decisions | Proposed revisions and review/approval workflow; never auto-edit frozen originals |
| Phase Roadmap | Existing native roadmap with real start/target dates or iterations; no invented dates |
| Checkpoints / Verification | Outstanding evidence and explicit owner acceptance |

These are **target configurations, not verified live views**. GitHub Projects currently offers table, board and timeline roadmap layouts, not a free-form graph. The whiteboard therefore lives as a companion browser backed by the same Issue relationships.

## Agent access and write rules

The official [GitHub MCP Server](https://github.com/github/github-mcp-server) has a `projects` toolset (disabled by default in some configurations) alongside `issues` and `repos`. Its `projects_read/projects_write` facilities are the preferred account-authorized agent interface; the currently connected ChatGPT GitHub integration does **not** expose Project V2 methods. Local `gh` with the maintainer's `project` scope is the fallback. Do not introduce secrets or PATs into the repository.

**Read actions:** list Project items/fields/views, inspect Issues and dependencies, read catalog/contracts, export graph, audit drift. **Guarded writes:** create genuine Issue, update Project status/priority, set native dependency, propose contract delta PR, record explicitly authorized review. All write operations require validation of owner/repo/project identity, stable IDs, idempotency key or duplicate check, and provenance (actor/time/source revision). Do not allow an agent to infer `Accepted`, rewrite frozen source, invent deadlines, or bulk-migrate archive entries. No automation may close G2/#2 without its separate acceptance evidence.

**Sync protocol, after pilot:** (1) read Project/Issue source; (2) construct diff with IDs and source revisions; (3) preflight conflicts and approvals; (4) publish reviewable Git PR for active documentation changes; (5) after merge, reconcile Project derived fields/links; (6) audit parity. Conflicts pause the write and surface both versions. Do not implement unconditional bidirectional replication or history-rewriting.

## Phased rollout

1. **P0 — Pilot (this PR):** protocol, read-only catalog/Issue graph exporter, offline self-tests, linked browser page. Nothing mutates Project #3.
2. **P1 — Read-only live audit:** authenticate `gh`, inspect real Project #3 fields/views/items, inspect native dependencies and generate a real graph snapshot. Verify the snapshot and source revision with the maintainer.
3. **P2 — Operational records:** agree minimally necessary additional field(s), save filtered views, create one genuine task/dependency as a controlled demonstration. New work automatically enters Project through the existing G2 intake; check it did.
4. **P3 — MCP and editing:** enable official GitHub `projects` toolset in an authorized environment, test create/triage/relate/close on real work with approval gates. Add graph editing only when it writes native relationships.
5. **P4 — Contract/roadmap synchronization:** implement and validate pull→PR→merge→push for mutable working files; protect frozen artifacts, detect conflicts, verify rollback and obtain owner acceptance.

### Acceptance gate

Do not mark [Issue #29](https://github.com/spwn02/Miracle/issues/29) or its parent planning work completed because this proposed protocol merges. A functioning graph, live Project audit, MCP permissions, round-trip consistency, and explicit acceptance must be independently verified.

## References

- [GitHub Projects layouts](https://docs.github.com/en/issues/planning-and-tracking-with-projects/learning-about-projects/about-projects)
- [GitHub Issues dependencies](https://docs.github.com/en/issues/tracking-your-work-with-issues/using-issues/creating-issue-dependencies)
- [GitHub Issues dependency REST API](https://docs.github.com/en/rest/issues/issue-dependencies)
- [GitHub MCP Projects support announcement](https://github.blog/changelog/2025-10-14-github-mcp-server-now-supports-github-projects-and-more/)
