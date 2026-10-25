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

## Direct GitHub Projects MCP — integration handoff (original Workspace #29 gate)

**Live integration status (2026-10-25):** [Workspace #60](https://github.com/spwn02/Miracle/issues/60) remains an open child of [#29](https://github.com/spwn02/Miracle/issues/29). The **original** GitHub Projects connector exposed the official `projects_list` / `projects_get` / `projects_write` operations but returned **HTTP 403 `Resource not accessible by integration`** when reading user-owned Project #3 on its original authorization path. The maintainer subsequently selected distinct **Miracle Projects OAuth Read Pilot** and **OAuth Write Pilot** connections. They are callable from this agent host and have successfully read Project #3 and exercised a reversible **direct MCP write** on genuine Issue #60. This verifies the new pilots' authorization path, **not** repair of the original connector or completion of #60's other safety gates. See the live checkpoint below and [write evidence on #60](https://github.com/spwn02/Miracle/issues/60#issuecomment-6392137873).

**Supported upstream capability:** GitHub's [MCP Projects announcement](https://github.blog/changelog/2025-10-14-github-mcp-server-now-supports-github-projects-and-more/) and [2026 consolidated Projects update](https://github.blog/changelog/2026-01-28-github-mcp-server-new-projects-tools-oauth-scope-filtering-and-new-features/) document the three official Projects tools. The `projects` toolset is **not part of the default toolsets**; see [GitHub's discussion #2275](https://github.com/github/github-mcp-server/issues/2275) and its current server configuration. Enabling tools in a **different MCP host** is insufficient: this original acceptance gate requires them in the agent/host that will actually manage Project #3. Repository commits and Actions secrets cannot reconfigure this conversation's ChatGPT connector. Verify actual tool discovery after a user-consented host connection; do not assume availability, permissions, or a supported URL configuration from public documentation alone.

**Provider-specific authentication limitation:** GitHub's [REST Projects reference](https://docs.github.com/en/rest/projects/projects) says the **user-owned Project** list/get endpoints **do not work with GitHub App user-access tokens, GitHub App installation tokens or fine-grained PATs**. The [user-owned Project fields reference](https://docs.github.com/en/rest/projects/fields) documents the same restriction. The **original connector's** `403` is consistent with an incompatible integration token or insufficient authorization, but the assistant has **not inspected that connector's credential type**, so it cannot conclusively distinguish them. A GitHub Classic PAT with appropriate `project` scope or a user OAuth token/host with compatible Projects authorization may be necessary; **confirm actual support in the chosen ChatGPT connector** rather than assuming a reinstall or increased repository permission will fix it. An upstream [remote-MCP ChatGPT 403 report](https://github.com/github/github-mcp-server/issues/1325) concerns organization Projects and is only analogous evidence, not proof of this user-owned cause. Never place a token or secret in the repository, Issues, ChatGPT messages, shared logs or commits.

**Host integration and least-privilege contract:**

1. The owner chooses/configures a host connection supporting the official GitHub MCP `projects` toolset and explicitly authorizes Projects access. Use the host's supported OAuth/scoped credentials; **never paste a PAT into a GitHub Issue, PR, chat, shell command transcript, source file, or Actions log**. Do not rotate/change the working Actions `MIRACLE_PROJECT_TOKEN` merely to make this test pass.
2. Discover actual `projects_list`, `projects_get`, `projects_write` tool schemas and permissions. If absent, **stop**: an Actions workflow invoking `gh api graphql` is a tested fallback, not direct MCP. Read and pin unique `spwn02 / Miracle Development #3` identity, field options, and genuine Issue #60's existing unarchived Project item before requesting any write.
3. Exercise direct MCP **read → authorized single-item write → independent read-back** only on a genuine open engineering Issue. Use explicit old/new field values and no inferred approval. Repeat the intended value to test idempotence; attempt a deliberately stale expected value to demonstrate refusal (where the API lacks atomic compare-and-swap, this check must be enforced in the client and the residual concurrent-editor race recorded). An insufficient-permission test must fail closed without credentials or partial changes.
4. Exercise direct **add-existing** only if a genuinely missing, approved Issue is encountered. Never delete/recreate an Issue or temporarily strip membership merely to force an acceptance test. Preserve the native auto-add workflow and refuse duplicate or archived Project membership.
5. Confirm the agent cannot use a generic Project field write to set `Accepted`, a contract freeze, review authorization, dates, release tags or historical archive items. This requires an explicit *owner* decision and independently verified Git authority; the GitHub API itself does not supply Miracle's owner-acceptance semantics.
6. Attach exact host/tool identity, live calls, native item IDs, run/PR links where applicable, negative results and post-write snapshot to #60. **Only** then review #60 for owner acceptance, remove its genuine blocker edge from #29 by an owner-approved operation, and assess the remaining #45 normative-amendment gate. Direct MCP setup by itself never accepts #29 or starts Stage B.

**Fallback boundary:** The current `/miracle-project` GitHub Actions operations already demonstrate real Project #3 inspection, field classification, forward status progress, no-op retries and stale-value refusal. Preserve these working controls while direct MCP is unavailable. They may be documented as operational, **not relabeled as direct Projects-capable MCP**. Never add an arbitrary-GraphQL command or shadow Project database to bypass the missing host integration.

## Maintainer-approved operational scope revision — 2026-10-25

The maintainer explicitly approved changing the original Stage A delivery gate: **direct GitHub Projects MCP completeness is now a deferred enhancement, not a prerequisite for accepting native Workspace #29**. The separate [G2 Workspace Stage A Scope Addendum v1.0](amendments/G2-WORKSPACE-STAGE-A-SCOPE-ADDENDUM-V1.0.md) specifies the new positive operational criteria and preserves prior evidence. The original GitHub Projects connector's 403, the missing write-token insufficiency negative test, and non-atomic external-editor race exposure remain **unresolved**, not silently counted as passes.

The Project #3 operational baseline is the user-authorized OAuth Read Pilot; demonstrated reversible direct Write Pilot; native GitHub Project auto-intake, Views and Issue relationships; and reviewed bounded owner-only GitHub Actions writes. No independent UI/database is authorized. [Issue #60](https://github.com/spwn02/Miracle/issues/60) remains OPEN as a **Deferred** enhancement; it is not `Accepted`. [Contract revision #45](https://github.com/spwn02/Miracle/issues/45) and [proposal #62](https://github.com/spwn02/Miracle/issues/62) already have genuine closed/accepted and accepted Project evidence.

**Stage boundary:** accept #29 explicitly under revised scope, verify native Project `Accepted` and dependency reconciliation, **then** separately plan Stage B according to the approved [Archive Fidelity Addendum v1.0](amendments/G2-NATIVE-ARCHIVE-PROVENANCE-ADDENDUM-V1.0.md). Stage B requires complete source text in retained drafts, a reviewed exact-item deduplication manifest, a retrospective evidence timeline distinct from planning fields, and cautious branch/Issue closeout. No unreviewed destructive migration.

## Direct OAuth pilots — verified Stage A write checkpoint (2026-10-25)

The maintainer-selected **OAuth Read Pilot** and **OAuth Write Pilot** independently read the user-owned `spwn02` / **Miracle Development #3** (Project node `PVT_kwHOCp2Gs84Bna-3`). The Read Pilot paginated **189 visible Project items** (including **172 historical drafts**) and inspected **19 saved views**. The Write Pilot successfully called the official direct `projects_write(update_project_item)` on the existing, OPEN engineering Issue [#60](https://github.com/spwn02/Miracle/issues/60), unique Project item `281181947` / `PVTI_lAHOCp2Gs84Bna-3zhDCfvs`.

| Direct MCP check | Observed result | Evidence boundary |
| --- | --- | --- |
| `Priority` unset → `P3` (option `84562fd6`) | Write succeeded; both independent OAuth pilots read `P3` | Real field mutation; no Status change |
| Repeat `P3` → `P3` | Succeeded; independent readback remained `P3` | Same-value retry, not concurrency control |
| Restore `Priority` `P3` → unset | Succeeded; both clients confirmed original field snapshot | No persistent test value or archive mutation |
| Invalid field ID / invalid Priority option | Both returned HTTP 400; readback unchanged | Schema validation, not permission refusal |
| Stale expected Priority=unset while live value was `P3` | Client **refused before invoking** `projects_write`; original value then restored | Preflight comparison only; not atomic compare-and-swap |

**Required direct-MCP guard protocol (agent responsibility, not GitHub's authorization policy):**

1. Pin the **exact** Project node/owner/number and unique OPEN Issue-backed item ID. Reject duplicate, missing, archived, draft, historical-catalog or accepted-artifact targets. Verify current repository Issue identity/state and option IDs. Do not fabricate Issues or toggle membership solely to test enrollment.
2. Require a narrow **explicit field allowlist** with recorded old/new values. Reject unexpected current values, unrecognized options and missing permission evidence; read the field **immediately before** sending the write. On ambiguous API outcomes, re-read before retrying.
3. Treat **`Status=Accepted`**, frozen contracts/revision ratification, release/tag changes, **Start date / Target date**, historical archive edits and dependency-gate changes as **protected operations**. Require separately evidenced explicit owner authorization, relevant reviewed Git authority and scope-specific validation. Possessing a generic OAuth `projects_write` method does **not** confer Miracle acceptance authority.
4. Call only the approved MCP operation. Independently read back the exact item (ideally through the separate Read Pilot) and verify **changed and untouched fields**. Record immutable IDs, old/new values, owner approval reference, results and limitations; never log tokens.
5. This MCP `update_project_item` schema provides **no expected-old-value/server-side CAS**. Preflight validation and even serialized single-agent calls cannot exclude an external-editor race. The owner-only Actions fallback's expected-value checks are also not proof of atomic CAS. On contested fields, abort and reconcile; do not replay stale writes blindly.
6. **Insufficient-permission write refusal remains untested**: the legacy connector's 403 and the Read Pilot's lack of a write tool do not prove Write Pilot token permission denial. Test only with a separately consented restricted credential; verify no partial mutation. Keep credentials out of chat, code, Issues and logs.

**Acceptance boundary:** Direct write/retry/rollback and client-side stale refusal evidence is recorded in [#60](https://github.com/spwn02/Miracle/issues/60), but #60 remains **OPEN / Implementing** until safeguards, credential-permission negative testing, fallback reconciliation and **explicit owner acceptance** complete. #29 remains **OPEN / Implementing**; accepted #45/#62 work does not automatically accept #29. **Stage B remains prohibited** until Stage A and #29 owner sign-off. No archive deduplication, historical draft editing, retrospective dates or branch cleanup was performed.

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

## Agent-managed item inspection and bounded triage

The same owner-only Issue #29 Actions bridge now supports two additional commands. An agent using the connected GitHub Issue API can post the commands under **Issue #29**; the workflow executes the versioned, reviewed [Project field controller](https://github.com/spwn02/Miracle/blob/57fc85cdf7f59b158f1071bc10435c1e3326f0b8/scripts/github/project-item-control.py) with `MIRACLE_PROJECT_TOKEN` and returns a run-link receipt. The Project itself remains the only planning UI and system of record.

```text
/miracle-project inspect 31
/miracle-project update {"issue":29,"field":"Kind","expected":null,"value":"Task"}
```

**Read before writing.** The `inspect` command returns the authentic Issue state, native item ID, and its selected field values. For `update`, `expected` is the last observed option name or `null` for an unset field; `value` is a real, unique option in the live Project schema. Whitespace and multiword labels are supported inside JSON values. The command rejects changed values rather than overwriting concurrent owner work; if the desired value is already present, it makes no new mutation.

Only the single-select fields **`Kind`, `Disposition`, and `Area`** are editable through this new general command. It refuses archived/duplicate/missing Project items, nonexistent/closed repository Issues, unknown field options, unexpected credential owners, extra JSON keys, oversized commands, and non-owner invocations. The script checks the Project's immutable ID and reads back the result after a direct `updateProjectV2ItemFieldValue` mutation.

**Important concurrency limit:** GitHub Projects' field mutation lacks an atomic compare-and-swap input. The script rechecks immediately before the write, but another editor could race between read and mutation; record and investigate any detected conflict. Do not treat the bridge as a transactional database. No generic status acceptance, timeline modification, dependency-graph editing, archive reclassification, contract freeze, release or Issue closure is authorized by this command.

`python3 scripts/github/project-item-control.py --self-test` validates parsing and negative fixtures without GitHub credentials. Agent-mediated read/write acceptance still requires a live inspect followed by a small, justified Project field update and read-back.

## Native Issue relationships (owner-only, GitHub-hosted)

[Real implementation task #36](https://github.com/spwn02/Miracle/issues/36) tracks the relationship controller and its acceptance. GitHub's official **sub-issues** and **issue dependencies** REST APIs are the native source; no custom graph storage or website.

The same owner-only Issue #29 command bridge accepts:

```text
/miracle-project relations 29
/miracle-project add-child {"parent":29,"child":36}
/miracle-project add-blocker {"issue":<blocked-issue-number>,"blocked_by":<prerequisite-number>}
```

Unlike user-owned Project field operations, native Issue relationships use the workflow's separate, **job-scoped `issues:write` repository `GITHUB_TOKEN`**, not `MIRACLE_PROJECT_TOKEN`. Untrusted comment content is accepted only via an exact command grammar and Python's JSON parser (never executed as shell). The controller verifies both real repository Issue identities and that write targets are open, rejects self-links, current incompatible parents and incomplete lists, refuses `replace_parent`, and checks the canonical parent and child endpoints after a mutation.

For **add-child**, the child Issue must declare its parent on an explicit standalone line, e.g. `Parent: #29`, `Parent work: #29`, or `Parent: [Workspace #29](https://github.com/spwn02/Miracle/issues/29)`. The controller accepts one unambiguous declaration only, with any linked label and destination numbers matching inside this repository; an incidental mention of `#29` elsewhere, negation or duplicate/contradictory parent statements do **not** authorize a new hierarchy mutation. Already-linked genuine children still use idempotent read-back. For **add-blocker**, the blocked Issue must explicitly document `Blocked by #N` before the agent may add that dependency, and a reverse dependency is prohibited. This guard prevents equating unrelated-but-linked Issues with actual prerequisites. No fabricated dependency should ever be created solely to demonstrate the API.

Native Issue relationships are distinct from Project `Status` and `Disposition`: neither hierarchy nor a dependency creates an acceptance decision, schedule, or release commitment. This operation cannot edit contracts, close Issues, change fields, or modify Project draft archives.

The REST API is not a transaction: GitHub performs validation of deeper hierarchy or dependency constraints. Post-write read-back is mandatory. If a network/API error occurs after a mutation, inspect the live relation before retrying; do not assume rollback.

## Forward-only execution progress (agent-owned, guarded)

The native Project command bridge also supports a distinct **execution** command:

```text
/miracle-project progress {"issue":36,"expected":"Backlog","value":"Implementing"}
```

This is **not** the general `update` command. `Status` cannot be set by generic field editing. The progress command is limited to forward planning/execution transitions (`Backlog → Design/Ready/Implementing`, `Design → Ready/Implementing`, `Ready → Implementing`, `Implementing → Review/Verification`, `Review → Verification`), always with a read-before-write expected value and post-write read-back. The existing G2 precedent for a deliberate `Backlog → Implementing` move is retained.

Only **open Issue-backed, uniquely enrolled, nonarchived** Project #3 work may progress. The Issue must already have `Kind` and `Area`, **`Disposition=Planned`**, documented acceptance criteria or delivery gates, and (for Review/Verification) a linked Miracle PR or CI execution record in its body. Missing classifications, deferred/candidate/declined items, regressive transitions or unapproved stages are rejected.

**No agent-controlled transition to `Accepted` or `Blocked` is exposed.** `Accepted` remains a human authorization/evidence gate; `Blocked` requires a genuine documented GitHub dependency and a separately approved operation. Moving a Project field neither closes the Issue nor alters any contract, PR, tag, CI or release. The GitHub Project GraphQL API does not provide atomic compare-and-swap, so concurrent edits between read and write remain a limitation. The bridge serializes **executions**, not all possible human UI edits.

The first live pilot uses the real [Issue #36](https://github.com/spwn02/Miracle/issues/36), whose native parent #29, acceptance checklist, and triage fields have been verified. A legitimate `Backlog → Implementing` change indicates remaining implementation/verification work, **not** acceptance or completion.

## Native view configuration verification

The standard `/miracle-project audit` command reads the **real Project #3 view configuration**, including each saved name, layout, exact filter, native grouping, vertical grouping and visible field names. It assesses the canonical work-oriented views separately from the historical archive views, reporting missing/mismatched filters as warnings rather than silently changing any UI configuration.

The Development board must group by `Status` to function as a true execution board. `Backlog`, `Deferred`, `Bugs`, `Blocked`, `Contract Reviews`, `Decisions`, `Overview`, and `Archive` must remain native saved views with intentional filters. The Roadmap must use GitHub's actual **ROADMAP_LAYOUT**. A present Roadmap tab is **not sufficient to claim an effective schedule**: Project timeline bars additionally require genuinely assigned date/iteration fields and meaningful date values. No arbitrary dates are to be fabricated to populate the chart.

A base Project-identity audit may succeed while warning about incomplete view grouping or scheduling. Such warnings are *actionable findings*, not implicit approval of Project presentation. A separately reviewed, non-destructive native Project view operation or explicit user-side settings adjustment is required before claiming full visual acceptance. GraphQL's exposed visible fields and grouping options do not establish every browser-only display setting (such as some roadmap axis/zoom preferences).

## Scoped native Roadmap filter

The verified 2026-10-23 Project audit found the native **Development** board already uses **vertical Status columns**; the earlier horizontal-grouping warning was a false positive. The same audit confirmed the existing **Roadmap** was a genuine `ROADMAP_LAYOUT` but had an **empty filter**, inadvertently mixing historical Project draft records and deferred proposals into the timeline's item source.

The owner-only Actions bridge therefore offers three fixed operations against **the existing Roadmap view only**:

```text
/miracle-project roadmap-plan
/miracle-project roadmap-apply
/miracle-project roadmap-verify
```

The only authorized change is **empty filter → `is:issue is:open disposition:Planned`**. The guarded controller authenticates as `spwn02`, verifies Project #3's immutable ID, requires exactly one `Roadmap` in `ROADMAP_LAYOUT`, refuses any nonempty/conflicting owner filter, makes the dedicated `updateProjectV2View` GraphQL mutation, and independently reads the view back. Repeated application is idempotent. No new view, changed layout, Group By, draft issues, execution status, dates/iterations, milestones or contracts are touched.

A valid Roadmap filter does **not** imply scheduled bars exist. Deliberately unplanned/datetimeless work remains in Backlog/Development; genuine dates must be set through a separate reviewed planning decision, not generated merely to populate a visual timeline.

## Historical archive integrity audit (read-only)

The 172 historical navigation drafts must preserve both their **identity** in Project #3 and their exact canonical **Git-source provenance**; having Archive view tabs is not by itself sufficient.

An owner-only command on Issue #29 triggers a read-only, paginated live GraphQL comparison:

```text
/miracle-project archive-audit
```

The audit validates the repository catalog against its checked-in headings, confirms exactly 172 catalog entries, enumerates **all active Project #3 items** with bounded cursor pagination and stable immutable Project identity, and checks each `[Archive/...]` draft for:

- A unique matching title and catalog ID embedded in a `miracle-project-catalog` marker
- Correct Git source path and line link, group, type, optional parent ID, and source-reference body
- Matching native `Artifact Type` and `Archive Group` single-select field values
- Unexpected extra Archive drafts, duplicate titles or absent items, archived records or incomplete Project field values

The job prints individual mismatches and fails without making any Project or Git changes. It never interprets archive `Status` as implementation completion. It is intentionally **not an importer or a bidirectional sync**: corrections require a reviewed provenance decision, especially for frozen-source identity. GitHub's Project item pagination is explicitly bounded; a concurrent Project update during enumeration causes a fail-closed retry rather than a false pass.

A complete technical match is **necessary but not sufficient** for the maintainer to accept the historical migration. Unknown approvals and missing independent source versions remain separate questions.

## Native Issue intake (open work only)

The [Project #3 intake checkpoint](https://github.com/spwn02/Miracle/issues/49) establishes an owner-only, **idempotent existing-Issue enrollment** path. The GitHub Issues connector remains responsible for creating a *genuine* feature, bug or task after duplicate search; GitHub's own auto-add rule is preferred, and **no shadow tracker** is created.

On [Workspace Issue #29](https://github.com/spwn02/Miracle/issues/29), an authorized agent can issue:

```text
/miracle-project intake-inspect 49
/miracle-project intake-add 49
```

`intake-inspect` reads the exact native Issue ID/state and unique Project #3 membership. `intake-add` first checks the same live identities, and **only if absent** enrolls the **existing**, described, OPEN, nonarchive repository Issue through `addProjectV2ItemById`, followed by a separately executed GraphQL read-back. A previously enrolled Issue returns `SKIP` without writing. Archived/mismatched/duplicate Project items, nonexistent/empty Issues, and unexpected Project credentials fail closed.

Enrollment alone **does not grant `Planned` disposition, dates, `Accepted` status, PR review, or contract approval**; classifications remain deliberate guarded field edits. This operation cannot create new repository Issues, touch historical draft catalog entries, close Issues, or mutate unrelated Project fields. A concurrent GitHub auto-add can race the read and write; inspect fresh membership after any ambiguous failure rather than assuming rollback or creating another issue.

## Target-Project G2 event provenance (read-only)

The final G2 [provider metadata discrepancy](https://github.com/spwn02/Miracle/issues/2) is *not* the same as operational auto-add correctness. GitHub's public GraphQL documentation defines `wasAutomated` as whether the event resulted from workflow automation; Project #3 has bot-attributed, functioning auto-add/Backlog transitions for genuine Issues #8 and #24, yet their events return `wasAutomated=false`.

The owner-only Issue #29 command:

```text
/miracle-project g2-provenance
```

performs read-only, target-Project-specific GraphQL timeline queries on **existing real Issues #8, #24 and #49**. It prints event type, Project number, timestamp, actor, the literal Boolean and status changes; excludes events from the separate Roadmap Project #2; rejects incomplete timeline pagination; and makes no Project/Issue write. The result is diagnostic evidence only, not an alternative Project auto-add workflow or a claim that GitHub changed its GraphQL semantics. If results remain inconsistent, preserve them and seek provider clarification via the public developer community. Never fabricate an Issue to trigger a favorable event or quietly waive the G2 maintainer gate.

The script intentionally reports `false` without converting it to `true` based on the `github-project-automation` actor. Neither successful Project field changes nor this audit can supply a missing GitHub provider statement.

## Live G6 GitHub community configuration audit

The G6 [community acceptance workstream](https://github.com/spwn02/Miracle/issues/6) requires **runtime** proof that support/category links and Q&A behavior correspond to GitHub's saved settings, not just strings in source files. Through the existing owner-only Issue #29 bridge:

```text
/miracle-project g6-community
```

A read-only, fixed-script Actions job checks the actual public repository identity, default branch, `has_discussions` setting, live GraphQL Discussion category slugs and whether the Q&A category has `isAnswerable=true`. It compares real category slugs against the checked-out `SUPPORT.md`, `CONTRIBUTING.md` and native Issue chooser links. It also requires published SECURITY/Code of Conduct instructions to exist. The command neither posts a Discussion nor accesses private reports or sends confidential emails.

**Evidence boundary:** A passing configuration check demonstrates that the **Q&A answer-selection feature is enabled**, not that a real participant asked a question or that an answer was actually accepted. Likewise, a published security/conduct route does not confirm **confidential delivery or triage**. Those remaining G6 gates require a real interaction or later owner-verified experience. Do not create fake questions, vulnerabilities, or conduct allegations solely to pass G6.
