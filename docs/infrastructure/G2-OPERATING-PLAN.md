# G2 — Miracle Development project and issue-management plan

**Program:** GitHub Infrastructure & Governance (G0–G6)
**Workstream:** G2 — Planning, Roadmap and Issue Tracking
**Status:** PROPOSED IMPLEMENTATION PLAN — repository instructions only; GitHub Project settings are not yet verified or provisioned
**Prepared:** 2026-10-17
**Authority:** [Infrastructure Contract v1.0](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Existing issue contract:** [G1.1](G1.1-ISSUE-INTAKE-CONTRACT.md)

## Purpose and separation of authority

One public **Miracle Development** GitHub Project shows what is proposed, active, under verification, or accepted. It does not supersede version-controlled product contracts, accepted implementation semantics, CI evidence or the author's release decisions.

- **Repository:** APIs, architecture, approved contracts, implementation, tests, source-of-truth roadmap documents.
- **GitHub Issues:** actionable tasks, defects and reviewable proposals; public intake is not approval.
- **GitHub Project:** cross-cutting issue status and planning views, not a separate contract.
- **Milestones:** finite and verifiable deliverables; no invented deadlines or release commitments.
- **Git tags/Releases:** actual published, verified artifacts only.

Do not create retrospective historical issues for completed work. If previously accepted work is relevant to a future task, link its real commit instead.

## 1. Project identity

| Setting | Target |
| --- | --- |
| Owner | `spwn02` (user-owned, not an imaginary organization) |
| Name | `Miracle Development` |
| Visibility | Public |
| Linked repository | `spwn02/Miracle` |
| Description | Public development and verification tracker for the Miracle C++26 foundation library |
| Default auto-add | New **issues** from `spwn02/Miracle`, not all unrelated PRs or repos |
| Canonical repository document | This plan and active phase roadmaps |

**Safety:** Do not create a second project if one with that exact title already exists. GitHub authentication to manage Projects requires the `project` permission scope; never put access tokens in tracked files.

## 2. Fields and semantics

| Field | Type | Values/semantics |
| --- | --- | --- |
| `Status` | Project built-in single-select; modify existing field, **never duplicate** | Backlog, Design, Ready, Implementing, Review, Verification, Accepted, Blocked |
| `Phase` | Text | Infrastructure `G0`…`G6`; product phase e.g. `6`. A text field avoids costly option changes for future phases |
| `Area` | Single-select | Core, Meta, Iter, Diagnostics, Range, Serde, Clap, Infrastructure, Toolchain, Documentation, Other |
| `Priority` | Single-select | P0, P1, P2, P3 |
| `Risk` | Single-select | Low, Medium, High |
| `Target release` | Text | Optional explicit release version; blank when not planned |
| `Start date` | Date | Optional actual/planned start when justified |
| `Target date` | Date | Optional **real** deadline/estimate; blank otherwise |

`P0` means immediate functional/security/release blocker, `P1` important committed work, `P2` ordinary backlog, `P3` optional improvements. Priority is assigned at triage, not automatically by the submitted form.

`Review` means the implementation/evidence is awaiting a review decision. `Verification` means the agreed checks are being run or evaluated. `Accepted` requires explicit recorded author acceptance; moving a card to Accepted does not itself satisfy that gate. `Blocked` requires a stated dependency in its issue. For multi-stage work, use sub-issues or explicit acceptance checklists; do not create a second field just to represent the same status.

**Built-in Status caveat:** GitHub often creates a Status field with default options. Use project field settings to edit its option names and order. The CLI can create *additional* fields but should not create a second Status field.

## 3. Project views

| View | Layout | Suggested filter / grouping |
| --- | --- | --- |
| All Work | Table | All items, show Status, Phase, Area, Priority, Risk, Target release |
| Active | Board | Group by Status; exclude Accepted; keep Blocked visible |
| Roadmap | Roadmap | Use optional Start date and Target date fields; items without dates remain visible elsewhere |
| Verification Queue | Table | `status:Review,Verification,Blocked` (confirm filter syntax in UI) |

The exact saved-filter expression must be checked in GitHub's own editor. Roadmap items need date or iteration values to occupy positions on the timeline; do not create fake dates to make the chart look populated.

## 4. Issue type and label policy

GitHub issue forms remain the **five G1.1 entry points**. Use a small repo-level label vocabulary for discoverability even when an issue is not added to the Project.

| Label | Meaning |
| --- | --- |
| `type:bug` | Library bug or regression |
| `type:toolchain` | Compiler, libc++, modules or GCC compatibility |
| `type:feature` | New Miracle capability |
| `type:architecture` | Contract/invariant/ownership design change |
| `type:infrastructure` | Repository, CI, tooling, operational work |
| `type:documentation` | Documentation-content problem |
| `triage:needs-info` | Missing evidence requested |
| `help-wanted` | Maintainer invites external implementation or investigation |
| `good-first-issue` | Scoped, documented and beginner-appropriate work |

Avoid redundant `priority:*`, `risk:*`, `area:*`, or `status:*` labels while those are maintained as Project fields. Do not apply a confirmed type/priority merely because a reporter chose a form; triage may reclassify.

**Security:** vulnerabilities follow private vulnerability reporting, never a normal public issue labeled `type:bug`.

## 5. Milestones

Start with **GitHub Infrastructure — G2** for provisioning, routing, verification and project-view setup. Other G-workstream milestones may be added as their scope becomes active; avoid backdating milestone completion or allocating fictional due dates. Miracle product Phase 6 milestones should be defined when Phase 6's roadmap and acceptance gate are approved.

## 6. Automation policy

Use GitHub's **built-in auto-add** workflow for repository `spwn02/Miracle`, with the exact filter **`is:issue`**. Do not constrain intake with `label:bug`, `type:*`, or `is:pr`: all genuine development issues should enter the Project and PRs are not auto-added. Projects auto-add rules depend on account plan limits; use a single concise issue rule, not a sprawling collection of label-specific workflows. The separate built-in sub-issue auto-add may remain enabled.

**Acceptance safety:** the `Auto-close issue`, `Item closed`, and `Pull request merged` default workflows must be disabled or individually proven not to set `Accepted` or close issues without explicit maintainer acceptance. `Item added to project` may initialize `Backlog` but must never initialize `Accepted`. A UI review of these workflow rules is required: merely observing that the named workflow is enabled is not enough. Treat issue closure, PR merge and completion of CI as evidence to review, not a substitute for the human acceptance gate.

Do not introduce PATs, Actions secrets, custom webhooks or a project-sync bot for initial G2 setup. If an automation requires account-level Projects permissions, it should run interactively under the maintainer's existing authenticated `gh` session.

The repository provides `scripts/github/bootstrap-g2.sh` to perform repeatable Project creation, field provisioning, repo label/milestone setup, and import of genuine outstanding issues #1–#6 without storing credentials. **Views and the built-in Status options still require interactive Project configuration.**

## 7. Delivery and acceptance

1. Confirm whether a `Miracle Development` Project already exists.
2. Provision or reuse the public, linked project and reconcile the field schema.
3. Create the four views and configure issue auto-add.
4. Establish repo labels and a finite G2 milestone.
5. Confirm real outstanding infrastructure issues #1–#6 were imported by the bootstrap script; leave Status, Priority, Risk and dates for honest human triage.
6. Move one testable real issue from Backlog through its appropriate statuses, recording evidence and review without a fake acceptance record.
7. Verify Project visibility, link from repository, view filters, workflow behavior and status transitions.
8. Record Project URL, GitHub issue references, setup commands/versions and acceptance evidence in `G2-PROGRESS.md`.

**Gate:** G2 is not complete when this document or bootstrap script is merely committed. The actual public Project and its usable workflow must be inspected before acceptance.

## Reference guidance

- [GitHub Projects documentation](https://docs.github.com/en/issues/planning-and-tracking-with-projects)
- [Project roadmap layout](https://docs.github.com/en/issues/planning-and-tracking-with-projects/customizing-views-in-your-project/customizing-the-roadmap-layout)
- [Project auto-add workflows](https://docs.github.com/en/issues/planning-and-tracking-with-projects/automating-your-project/adding-items-automatically)
- [GitHub CLI Projects commands](https://cli.github.com/manual/gh_project)
