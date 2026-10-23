# Miracle Development — Native GitHub Projects operations

**Scope:** [Miracle Development (user-owned Project #3)](https://github.com/users/spwn02/projects/3) and [`spwn02/Miracle`](https://github.com/spwn02/Miracle).  **Tracking:** [Workspace #29](https://github.com/spwn02/Miracle/issues/29).  **Status:** operating proposal/pilot, not final G2 or workspace acceptance.

This Project is Miracle's **only interactive planning workspace**. We work directly through native GitHub Projects views, Issues, Pull Requests, sub-issues, dependencies and Project fields. **No external or separately hosted whiteboard, UI, shadow database or live graph exporter** is part of this design.

The version-controlled `docs/` tree holds accepted/frozen contracts and durable evidence; Project records are the collaborative planning and proposal interface. Changes to frozen text proceed by reviewable Git pull request. A Project field edit never silently modifies an accepted contract.

## Work item taxonomy and interpretation

| Record | Canonical home | What it means |
| --- | --- | --- |
| Feature, enhancement, bug, debt, task, verification | Repository Issue included in Project #3 | Actionable or proposed engineering work; never retroactively created to manufacture progress |
| Contract revision or design decision | Issue and its review discussion; eventual Git PR | Editable working proposal, not an accepted contract until reviewed |
| Accepted/frozen specification | Git (`docs/contracts/`, `docs/phases/`, source originals) | Versioned contract authority; Project refers to its stable source and stage |
| Historical catalog entry | Pre-existing Project draft issue with `Artifact Type` and `Archive Group` | Navigation/archive; does **not** mean an unfinished engineering task |
| Implementation evidence | PR, check run, test output, acceptance record | Evidence to review; not automatic author acceptance |

The pre-existing **172 historical Project draft issues** are retained. Keep routine engineering views filtered to `is:issue` and use `is:draft` with archive fields for historical records. Do not infer completion or backlog priority from draft-item `Status=Backlog`.

**`Status`** means execution stage (Backlog, Design, Ready, Implementing, Review, Verification, Accepted, Blocked). Accepted requires recorded author approval and a scope-specific evidence boundary; a closed Issue or green CI is not by itself acceptance. **`Disposition`** (Candidate, Planned, Deferred, Declined) means scheduling and triage choice, independently of Status. **`Kind`** classifies the work, not the completion state. **`Area`**, `Phase`, release and dates are only set when justified by real planning. Never invent dates to fill a roadmap.

Example: [#31](https://github.com/spwn02/Miracle/issues/31) is a deferred research **proposal** for C++26-aware static analysis. [#24](https://github.com/spwn02/Miracle/issues/24) is the separate unresolved **current** CodeQL limitation. These records are related but not automatically `blocked by`/`blocking` each other.

## Native views and relationships

- **Development**: board for active repository Issues, grouped by Status; exclude historical drafts and preserve Blocked visibility.
- **Backlog / Deferred / Bugs**: tables filtered on genuine Issues and relevant Kind/Disposition; deferral does not imply refusal or a committed release.
- **Roadmap**: GitHub's native roadmap layout, using actual Start/Target dates or iteration fields, not guessed time boxes.
- **Contracts / Archive**: historical drafts organized by `Artifact Type` / `Archive Group`, plus separate actionable contract-revision Issues.
- **Dependencies / Blocked**: real GitHub Issue `blocked by`/`blocking` edges, native sub-issue progress, and blocked execution states where evidence exists. These are not a freeform node-and-arrow canvas.
- **Decisions / Verification**: reviewable decision Issues and real checkpoints with evidence and approvals.

The intended view names are **not themselves proof that the owner's saved views, filters or workflow automations were verified**. Audit each live tab/settings page before declaring a gate complete.

## Read-before-write agent contract

A Projects-capable agent should use the **official GitHub MCP `projects`, `issues`, `repos` and (when needed) `pull_requests` toolsets** with least-privilege credentials and the user's authorized review boundaries. The `projects` toolset exposes consolidated `projects_list`, `projects_get`, and `projects_write` operations on supported server versions. Other ChatGPT GitHub connectors may not expose those functions even though the official GitHub MCP server does.

Before any mutation, the agent MUST:

1. Resolve and confirm Project #3's immutable ID, the intended Issue's repository/number and Project item ID, and the applicable field IDs/options. Abort on missing, duplicate, archived or ambiguous identity.
2. Fetch the issue's current state, body and project values. Preserve non-targeted fields and detect changes since the plan was prepared. An unexpected current value is a conflict, not permission to overwrite.
3. Decide whether the record is a **proposal, known risk, execution task, accepted artifact, or archival draft**. Never bulk-promote archive items into repository Issues.
4. For new work, search for an existing Issue/proposal first. Create at most one deduplicated record and confirm membership/read-back after any project-add before retrying.
5. State the intended precise field/relationship changes. Require explicit maintainer approval for contract freeze/approval, release/tag changes, unexpected broad reorganizations, closing acceptance gates, or security-sensitive changes.
6. Write only authorized fields. Read back the exact item by ID and record a link to the outcome. Stop on partial failure; do not silently assume rollback. Avoid changing hidden workflow configuration as a side effect.

MCP operations and authenticated `gh` are **two clients of the same native GitHub data**. They should not maintain an independent writable mirror. For troubleshooting, `gh auth status` verifies local authentication and `gh auth refresh -s project` requests the required Projects scope; never commit tokens or credentials.

## Audited status-reconciliation pilot

The [reconciliation script](https://github.com/spwn02/Miracle/blob/201c1b2ffb8567137561ef98e649ed2237672622/scripts/github/miracle-project-reconcile-safe.sh) changes **only** the `Status` field on seven existing Issue-backed Project items after checking their Issue state, Project identity, item membership and prior field values:

| Issue | Planned status | Evidence boundary |
| --- | --- | --- |
| [#1](https://github.com/spwn02/Miracle/issues/1) | Accepted | G1/G1.5, approved by maintainer |
| [#3](https://github.com/spwn02/Miracle/issues/3) | Accepted | G3, closed/approved |
| [#5](https://github.com/spwn02/Miracle/issues/5) | Accepted | Revised Docker-only G5 scope, not Codespaces |
| [#8](https://github.com/spwn02/Miracle/issues/8) | Accepted | G3 publishing implementation subtask only |
| [#2](https://github.com/spwn02/Miracle/issues/2) | Verification | G2's `wasAutomated=false` semantics gate and author acceptance unresolved |
| [#4](https://github.com/spwn02/Miracle/issues/4) | Verification | G4 release / GCC / CodeQL and author acceptance gates pending |
| [#29](https://github.com/spwn02/Miracle/issues/29) | Implementing | Native workspace rollout; no final acceptance implied |

```bash
bash scripts/github/miracle-project-reconcile-safe.sh --plan
bash scripts/github/miracle-project-reconcile-safe.sh --apply
bash scripts/github/miracle-project-reconcile-safe.sh --verify
```

The script refuses unexpected Issue states or Project field values and performs read-after-write checks. **This is not a transactional rollback mechanism**: if a later write fails, previous successful edits remain and must be audited before retrying. It does not modify Issues, archive drafts, contracts, deadlines, milestones or automation rules.

## Live acceptance gates (still open)

- Verify the actual saved Project views, grouping, filters and owner-configured workflows. A successful script exit does not establish those UI settings.
- Verify Issue #31's real Project membership and classification, including MCP/CLI read-after-write; owner-reported pilot execution is not independent visibility from this repo.
- Confirm Project intake and status automation while retaining the distinct G2 provider-metadata concern (`wasAutomated=false`) in [#2](https://github.com/spwn02/Miracle/issues/2).
- Perform a native dependency/sub-issue pilot only with a genuine relationship; no fabricated blocker.
- Test official Projects-capable MCP reads, field updates, duplicate prevention and conflict aborts against the **same** Project #3, with scoped credentials.
- Test the proposed contract-revision Issue → reviewed Git PR → accepted Project state loop before calling sync complete.
- Record explicit maintainer acceptance under [#29](https://github.com/spwn02/Miracle/issues/29).

This document is an operational guide, **not** a new frozen product contract, automatic G2 closeout, or permission to rewrite historical artifacts.
## Automated Actions bridge (post-bootstrap, permission-gated)

The owner authorized investigating automated native Project administration **after** the original G2 bootstrap; this does not retroactively waive the G2 provider-metadata discrepancy. [Workflow](https://github.com/spwn02/Miracle/blob/201c1b2ffb8567137561ef98e649ed2237672622/.github/workflows/miracle-project-operations.yml) runs **only** a fixed allowlist of repository-reviewed Project scripts. No independent planning UI/database is created.

1. GitHub's standard repository `GITHUB_TOKEN` **cannot** read or write user-owned Projects V2. Use a separately scoped Projects credential; for this user-owned Project, GitHub documents a **classic PAT with `project` scope**, or an appropriately permissioned GitHub App token. The account settings UI also supports fine-grained PATs, but GitHub documents limitations for *user-owned Projects*, so do not assume they will work without verifying the specific token.
2. Store the credential once as the **repository Actions secret** `MIRACLE_PROJECT_TOKEN` under `spwn02/Miracle → Settings → Secrets and variables → Actions`. Give it an expiration appropriate to its purpose. Never put it in Issues, chat, scripts, the source tree, Actions logs or the repository's ordinary variables.
3. The workflow may be started via **Actions → Miracle Project Operations → Run workflow**, choosing `audit`, `reconcile-plan`, `reconcile-verify` or `reconcile-apply`. Alternatively, comment one exact command on **Issue #29**, under the maintainer account: `/miracle-project audit`, `/miracle-project reconcile-plan`, `/miracle-project reconcile-verify` or `/miracle-project reconcile-apply`. Other users and other Issues cannot invoke this workflow's privileged job.
4. The workflow always checks out `master`, with persisted checkout credentials disabled. Project credentials are passed **only** to the reviewed operation step. The ordinary repository token posts a result link on Issue #29 for comment-triggered runs. Project writes are limited to the existing, guarded seven-item reconciliation allowlist.
5. The read-only audit verifies Project identity, required fields, and #31's actual deferred classification, and enumerates saved native views and workflow names. It is **not** an automatic acceptance of those view configurations, workflow semantics, G2 `wasAutomated` provenance, or frozen contracts.
6. A nonzero exit, partial write or absent secret is a failure requiring examination. The workflow's run URL and job logs preserve the evidence. Re-running `reconcile-plan` before any apply is recommended. Future broad Project mutations require **their own reviewed code and narrow authorization contract**, not arbitrary text interpreted from an Issue.

### Credential lifecycle

Use the shortest workable expiration; revoke/rotate `MIRACLE_PROJECT_TOKEN` after suspected compromise or loss of owner access. This credential is stored and controlled by GitHub; **the assistant cannot view or install it** through the current connector. Until the secret is configured and the workflow merged, the bridge remains staged and cannot execute Projects operations on behalf of the assistant.
