# Miracle GitHub Infrastructure — G2 Progress

**Updated:** 2026-10-19
**Program:** G0–G6
**Workstream:** G2 — Project, Roadmap, Issue Tracking
**Status:** G2 Auto-add and all four enabled workflow actions screenshot-reviewed; author-controlled Accepted preserved by configured actions. Actual target-Project automatic intake, real status transition and author sign-off pending
**Project:** [Miracle Development — user-owned Project #3](https://github.com/users/spwn02/projects/3)
**Authority:** [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Operating plan:** [G2-OPERATING-PLAN.md](G2-OPERATING-PLAN.md)

## Remaining enabled Project workflow actions — screenshot audit (2026-10-19)

The maintainer provided **three live Project #3 → Settings → Workflows screenshots**, establishing the configured triggers/actions for the three previously uninspected enabled workflows:

| Enabled workflow | Observed trigger | Observed action | Interpretation |
| --- | --- | --- | --- |
| `Item added to project` | An **issue or pull request** is added to Project #3 | `Set value → Status: Backlog` | Appropriate initial triage status, never `Accepted` |
| `Pull request linked to issue` | A PR is linked to an issue | `Set value → Status: Design` | **No direct acceptance bypass**, but potentially moves an issue **backward** from `Implementing`, `Review`, or `Verification` to `Design`; current-state conditional behavior was **not** demonstrated |
| `Auto-add sub-issues to project` | A Project item has sub-issues | `Add sub-issues to the project` | No direct Status write; the separately enabled `Item added` workflow may initialize new sub-issues to `Backlog` |

Together with the earlier screenshot (**Auto-add to project: On**, repo `Miracle`, filter `is:issue`; **Auto-close issue**, **Item closed** and **Pull request merged** all Off), this **completes the UI inspection of enabled workflow actions**. **No inspected enabled action sets `Accepted`**, and the three specifically risky defaults remain disabled. The screenshot inspection is **configuration evidence**, not a runtime proof that a real issue transitions exactly as expected, nor proof of successful automatic enrollment of a new non-bug issue.

**Recommendation, not an action taken:** disable `Pull request linked to issue → Design` to avoid silently regressing an in-progress issue's status when a PR is linked. Author-controlled transitions to `Design`, `Implementing`, `Review`, `Verification`, and `Accepted` better preserve the [Project status semantics](G2-OPERATING-PLAN.md). The existing workflow has **not** been disabled or edited by the assistant. If the maintainer keeps it, verify on a genuine issue whether it overrides a later stage and document that decision.

**Still required for G2 acceptance:** observe a real non-bug issue automatically entering target Project #3 (not solely the separate Roadmap), verify at least one real status transition without inappropriate reclassification, and obtain explicit maintainer G2 acceptance. The issue remains OPEN.

## Latest Project #3 workflow UI verification (2026-10-19)

The maintainer supplied a **live screenshot of Miracle Development Project #3 → Settings → Workflows → Auto-add to project**. It shows the workflow **On**, repository selection **Miracle**, and exact filter **`is:issue`** (no `label:bug`, `is:pr` or other restrictive terms). The workflow's action is **Add the item to the project**. This independently resolves the previously unverified **UI configuration** of the broad issue-intake rule.

The workflow navigation shows **four enabled workflows**: `Auto-add sub-issues to project`, `Auto-add to project`, `Item added to project`, and `Pull request linked to issue`. It shows `Auto-close issue`, `Item closed`, and `Pull request merged` **Off**, satisfying the specific instruction to disable those three potentially unsafe defaults. Also displayed Off: `Auto-archive items`, `Code changes requested`, `Code review approved`, `Item reopened`.

**The original overview screenshot did not expose the enabled actions**, which the maintainer subsequently supplied in three detailed screenshots (see above). These actions are now UI-reviewed and none sets `Accepted`. Still unverified: whether a **new real non-bug issue** automatically enters Project #3 and an actual Project #3 status transition that respects maintainer-controlled acceptance. The user previously provided `gh issue view 6 --repo spwn02/Miracle --json projectItems` identifying `Miracle Development / Backlog` and a separate `Roadmap / Todo`: current membership is verified, **but issue #6 may have been manually added**, so this does not independently validate future automatic intake. No status was changed by the assistant; formal G2 acceptance is still pending.

Historical sections below retain the **older** incorrect `label:bug` UI configuration as historical evidence, not a description of the current state.

## Current issue disposition

The program-wide milestone remains **GitHub Infrastructure (G0–G6)**. Genuine publication subtask [#8](https://github.com/spwn02/Miracle/issues/8) was closed **completed** after mdBook publishing succeeded, without closing broader G3 acceptance parent #3. This is an implementation completion record, **not** proof that issue #8's automatic Project intake targeted Project #3, or that Project workflow settings prevented premature `Accepted` assignments.

[G2 issue #2](https://github.com/spwn02/Miracle/issues/2) remains open in verification. Confirm the precise `gh issue view 8 --json projectItems` output, the UI Auto-add rule and acceptance-sensitive workflows, and a real status transition before formal acceptance.

## Cross-workstream milestone reconciliation

The maintainer clarified that milestones represent **complete deliverables**, not individual G-subphases. Existing GitHub milestone #1 was renamed **in place** from `GitHub Infrastructure — G2` to [`GitHub Infrastructure (G0–G6)`](https://github.com/spwn02/Miracle/milestone/1) by [verified one-time workflow](https://github.com/spwn02/Miracle/actions/runs/38994367818). The historical issue milestone ID is preserved; no duplicate milestone was created.

Issues **#1–#6 and #8** were verified with milestone #1, while unscheduled, deferred frozen-contract migration **#7** intentionally remains outside the milestone. The elevated one-time workflow was deleted after successful verification. The updated bootstrap calls an idempotent reconciliation script on future maintainer runs.

## Subsequent automation check — G3 issue #8 (2026-10-17)

After correcting the auto-add rule and acceptance-sensitive workflows, the maintainer reported that configuration was complete and authorized continuing to G3.

A **genuine G3 documentation-publishing issue** [#8](https://github.com/spwn02/Miracle/issues/8) was created with `type:documentation` to track actual publishing work. The GitHub issue timeline records two `added_to_project_v2` events by `github-project-automation[bot]` at 01:46:17 UTC, followed by status updates at 01:46:18 and 01:46:19 UTC. This proves Project automation is running for **non-bug** issues, unlike the previous `label:bug`-restricted filter.

Because the maintainer also has a separate account-owned `Roadmap` Project with automatic intake, the issue timeline does **not** identify which of the two Project-add events belongs to `Miracle Development` (#3). Before final G2 sign-off, the maintainer can confirm actual membership via:

```bash
gh issue view 8 --repo spwn02/Miracle --json projectItems
```

The output must include `Miracle Development`. The maintainer also reports adjusting the workflow filters and acceptance-safe settings; their **current live** configuration has not yet been re-inspected in a post-change screenshot. All other G2 structural fields and four saved views were independently verified in the earlier complete audit.

**G2 is operationally usable and subsequent work may proceed. Formal acceptance of G2 (closing issue #2) still awaits project-specific issue #8 membership and explicit maintainer sign-off**, without fabricating approval.

## Latest maintainer acceptance audit — 2026-10-17

The maintainer supplied the complete successful output of `scripts/github/audit-g2.sh` and screenshots of both Auto-add workflows.

**Verified:**
- Project #3 is public, open, linked exclusively to `spwn02/Miracle`.
- The **existing** Status field has exactly `Backlog, Design, Ready, Implementing, Review, Verification, Accepted, Blocked` in the agreed order.
- Custom `Phase`, `Area`, `Priority`, `Risk`, `Target release`, `Start date`, and `Target date` fields are present; Area/Priority/Risk options match the contract.
- `All Work` is an unfiltered table; `Active` is a board with `-status:Accepted`; `Roadmap` is an unfiltered roadmap layout; `Verification Queue` is a table with `status:Review,Verification,Blocked`.
- Issues #1–#6 have appropriate Phase and Area and `Backlog` status. Issue #7 is **now present** in Project #3 with `Backlog`; its Phase and Area are unassigned, correctly preserving an unscheduled phase (Area should be classified as Documentation during triage).
- `gh issue view 7 --json projectItems` exposes **two different user-owned Projects**, `Roadmap` with `Todo` status and `Miracle Development` with `Backlog`. The earlier automated addition appears to have belonged to the separate Roadmap Project; the later manual Project #3 addition occurred at 01:33:08 UTC.
- GitHub workflows `Auto-add to project` and `Auto-add sub-issues to project` are both enabled.

**Remaining acceptance blocker shown in actual Project #3 screenshot:**
- The **Auto-add to project** rule is configured for repository `Miracle` but filtered as `is:issue,pr is:open label:bug`. This excludes most development work, including `type:feature`, `type:documentation` and `type:infrastructure`, so it does **not** satisfy G2's general issue-intake contract.
- Change the filter to **`is:issue`** (no `label:bug`, no `is:pr`). Save and keep the workflow on.
- `Auto-add sub-issues to project` is enabled and its screenshot shows no problematic criteria. It can stay enabled.
- `Auto-close issue`, `Item closed`, and `Pull request merged` are currently enabled. GitHub defaults can auto-close issues or move statuses to `Done` when an issue closes/PR merges. Since the historical `Done` choice was renamed `Accepted`, these automations risk bypassing the explicit author-acceptance gate. **Disable these three until their target statuses and acceptance implications have been individually reviewed.** Inspect `Pull request linked to issue` as well, but leave it only if it never assigns `Accepted`.
- `Item added to project` may stay enabled when it sets newly added issues to `Backlog`.

**G2 not formally accepted:** The filter correction and acceptance safeguards have to be confirmed from the current Project UI. Then verify normal new-issue intake through a *genuine* future issue (not a fabricated dummy) and record an explicit author acceptance decision.

The audit originally printed `Machine-readable gates passed` despite this incorrect Auto-add filter because GitHub's public Project workflow GraphQL fields expose enablement, not the repository-specific filter rule. Treat browser workflow inspection as a required independent gate; machine-readable success alone is insufficient.

## Provisioned and observed

- The maintainer's terminal output confirmed Project #3 was created and linked to `spwn02/Miracle`.
- Seven custom fields were provisioned: Phase, Area, Priority, Risk, Target release, Start date and Target date.
- Nine repository labels were provisioned.
- The finite `GitHub Infrastructure — G2` milestone was created. Its attachment to [issue #2](https://github.com/spwn02/Miracle/issues/2) was independently verified through the GitHub repository API after the maintainer reran the repaired bootstrap.
- The maintainer reported completing the repaired `--apply` command. Its precise Project item-import and field-edit results remain unverified through this integration, which lacks access to GitHub user-owned Project administration.
- The `type:infrastructure` label was assigned to the six genuine tracking issues directly and verified by the connected GitHub integration.
- The bootstrap script is versioned at [`scripts/github/bootstrap-g2.sh`](https://github.com/spwn02/Miracle/blob/master/scripts/github/bootstrap-g2.sh) and its shell syntax is now checked in CI.

## Real outstanding issues

| Issue | Scope |
| --- | --- |
| [#1](https://github.com/spwn02/Miracle/issues/1) | G1.5 — contributor intake and community recognition verification |
| [#2](https://github.com/spwn02/Miracle/issues/2) | G2 — Project setup, views, automation and acceptance |
| [#3](https://github.com/spwn02/Miracle/issues/3) | G3 — documentation publishing |
| [#4](https://github.com/spwn02/Miracle/issues/4) | G4 — security, rulesets, CI and dependency updates |
| [#5](https://github.com/spwn02/Miracle/issues/5) | G5 — Docker/devcontainer/Codespaces |
| [#6](https://github.com/spwn02/Miracle/issues/6) | G6 — Discussions and full workflow acceptance |

No retrospective historical tickets, fake due dates, automatic acceptance statuses or fictitious approvals were created.

## Live auto-add evidence for issue #7 — 2026-10-17

- GitHub issue [#7](https://github.com/spwn02/Miracle/issues/7) was created as a deferred, unscheduled contract migration task with `type:documentation` and no fabricated deadline.
- Its GitHub issue-event timeline records `added_to_project_v2` by `github-project-automation[bot]` at **01:04:54 UTC**, followed by `project_v2_item_status_changed` from the same bot at **01:04:56 UTC**.
- This establishes that GitHub Projects **auto-add and status automation fired** for the new issue. The repository-facing REST response does not identify the Project number or resulting status option. Confirmation of membership in *Project #3* and its effective status requires authenticated Project inspection.
- An issue can be successfully added while not appearing in a filtered view; `All Work` should be used to inspect unfiltered current membership.

### Final read-only Project audit

The [`scripts/github/audit-g2.sh`](https://github.com/spwn02/Miracle/blob/master/scripts/github/audit-g2.sh) script queries Project #3 using the maintainer's authenticated `gh` session. It reports current public visibility, linked repositories, fields/options, saved-view names/layouts/filters/grouping, enabled workflows, and actual issue items (including #7), plus the bot events.

```bash
git pull --ff-only origin master
bash -n scripts/github/audit-g2.sh
bash scripts/github/audit-g2.sh
```

It performs no writes and does not print tokens. The exact auto-add workflow **repository and filter configuration** and the saved views' practical usability still require browser verification.

## Second maintainer audit and current blocker (2026-10-17)

The maintainer ran the revised `audit-g2.sh` and supplied the raw console output. It confirmed actual Project #3 entries for issues #1–#6, with:
`#1 G1 Infrastructure Backlog`, `#2 G2 Infrastructure Backlog`, `#3 G3 Documentation Backlog`, `#4 G4 Infrastructure Backlog`, `#5 G5 Infrastructure Backlog`, and `#6 G6 Infrastructure Backlog`.

However, the section titled **G2 PROJECT CONFIGURATION** printed nothing, so none of the following were proven by that run: actual Status option inventory, four views and saved filters, public visibility, linked repository, or enabled workflow configuration.

The unfiltered Project #3 issue-items report contained only #1–#6, and the filtered search for issue [#7](https://github.com/spwn02/Miracle/issues/7) returned nothing. The older `added_to_project_v2` event does not specify Project #3, so membership in the intended Project is **not established**. This is a real acceptance gap rather than merely a hidden card in a Project view.

The script previously used `gh api graphql --jq` directly, allowing an empty output to pass without an error. Commit `aacda14` replaces this with **raw JSON responses, explicit nonempty/JSON/GraphQL-error checks, and a nonzero audit result** if any required gate cannot be verified. The GraphQL query was simplified to supported Project, repository, view and workflow fields. Interactive Project settings still require manual verification.

If #7 remains absent, add it directly without creating a duplicate issue:

```bash
gh project item-add 3 --owner spwn02 \
  --url https://github.com/spwn02/Miracle/issues/7

gh project item-list 3 --owner spwn02 --limit 100 --format json \
  --jq '.items[] | select(.content.url == "https://github.com/spwn02/Miracle/issues/7")'
```

This manually repairs the current membership; it does **not** prove that the ongoing Auto-add workflow is correctly scoped. Check the Project #3 **Settings → Workflows → Auto-add** repository/filter rule to prevent recurrence.

## Follow-up audit failure and corrective action (2026-10-17)

The maintainer ran `bash scripts/github/audit-g2.sh` and received `cannot iterate over: null` before the issue-item section. The cause was an **audit jq filter bug**, not evidence of an invalid Project: nested `fields`, `views`, `workflows`, and `repositories` were resolved against the GraphQL response root instead of the Project object captured as `$p`.

The four selectors were corrected in commits `b18f4e0` and `06d7534`. The revised filter was checked against representative GraphQL Project data. A new read-only `gh issue view 7 --json projectItems` check was added for direct membership verification.

The maintainer also ran a CLI filter for issue [#7](https://github.com/spwn02/Miracle/issues/7) under Project #3 and received **no matching items**. That proves only that the filter did not find a matching `.content.url` entry in the returned items; it does **not** override the historical GitHub `added_to_project_v2` event or identify which Project received it. The corrected audit must inspect the actual issue's `projectItems` metadata and the complete Project item listing.

To re-run after pulling:

```bash
git pull --ff-only origin master
bash scripts/github/audit-g2.sh
```

If `gh issue view 7 --json projectItems` does not identify **Miracle Development Project #3**, check the Project's Auto-add workflow repository filter; only then add issue #7 manually with `gh project item-add 3 --owner spwn02 --url https://github.com/spwn02/Miracle/issues/7` as appropriate. Do not assert that an Auto-add event alone proves successful membership in Project #3.

## Owner's view setup report (2026-10-17)

The maintainer reports that the G2 Project configuration steps are complete and observes multiple views containing the same six issues, plus a redundant default `View 1` tab. GitHub views are alternate layouts or filters of the **same** Project items, not copies of issues; repeating an issue across views is correct. Deleting the unused default `View 1` tab is appropriate when the named `All Work` view provides an equivalent comprehensive table. Deleting a view does not remove its issues.

The report does **not** independently prove the saved status options, view filters, workflow configuration or Phase/Area assignment. Project Settings and saved views require a UI check; this integration cannot retrieve the user-owned Project's configuration. The maintainer's `gh project item-list` attempt exposed an error in a previously published verification command: `--field` and `--format json` cannot be combined. The corrected alternatives are recorded below.

## Recovery from initial bootstrap syntax failure

The initial `--apply` created Project #3, its fields, labels and milestone, then stopped at a corrupted here-document in the script's issue-import tail. The bug was repaired in commit `42c8e14` and a `bash -n` CI check was introduced in `a394bb8`. The corrected script is non-destructive on reruns, uses GitHub Project/field/option/item IDs, and assigns only factual Phase/Area metadata when importing real issues.

## Finish configuration in the GitHub Project UI

1. In [Project #3](https://github.com/users/spwn02/projects/3), open **Settings → Fields → Status**. Modify the **existing** Status field rather than creating another. Use the exact options: `Backlog`, `Design`, `Ready`, `Implementing`, `Review`, `Verification`, `Accepted`, `Blocked`.
2. Create/synchronize the saved views:
   - **All Work**: table containing all items, with Status/Phase/Area/Priority/Risk/Target release visible.
   - **Active**: board with columns grouped by Status; omit Accepted items and retain Blocked.
   - **Roadmap**: roadmap timeline using Start date and Target date; undated items are expected to be absent from the timeline, **not** assigned fictional dates.
   - **Verification Queue**: table filtered to Review, Verification and Blocked.
3. Under **Settings → Workflows**, configure **Auto-add** to import new issues from `spwn02/Miracle`, without also accepting unrelated repos or automatically marking items Accepted.
4. Verify real issues #1–#6 appear; set initial Status according to present reality (for example, G2 may be `Verification` once implementation is finished). Do not claim other workstreams are Implementing until that is true.
5. Verify at least one real state transition and record the evidence before G2 acceptance.

### Read-only verification from the maintainer's CLI

```bash
gh project view 3 --owner spwn02 --format json
gh project field-list 3 --owner spwn02 --format json
# Human-readable table with selected columns
gh project item-list 3 --owner spwn02 --limit 20 \
  --field Status --field Phase --field Area

# Full machine-readable item data (without --field)
gh project item-list 3 --owner spwn02 --limit 20 --format json
```

Project view configuration, saved filters and the Auto-add rule must still be checked in the GitHub UI. CLI output alone does not prove interactive behavior.

## Acceptance checklist

- [x] Actual Project URL recorded.
- [x] Author terminal evidence for project creation/linking and seven custom fields.
- [x] Required repository labels created; current six issues classified.
- [x] Milestone created and issue #2 association verified.
- [x] Author reported repaired bootstrap run complete.
- [x] Independently inspect Project #3 items and Phase/Area values for issues #1–#6 (issue #7 remains correctly unscheduled).
- [x] Verify that new issue #7 triggered built-in Project auto-add and a bot status update (GitHub issue timeline).
- [x] Confirm #7 resides in Project #3 with `Backlog` and is also a member of the separate `Roadmap` Project.
- [x] Verify the existing Status field has all eight required options, in the expected order.
- [x] Verify the four saved view names, layouts and saved filter strings (column/grouping presentation not independently inspected).
- [x] Confirm Project #3 Auto-add is **On** with repository `Miracle` and exact filter `is:issue` — maintainer screenshot on 2026-10-19.
- [ ] Verify a **new real non-bug issue** enters target Project #3 automatically (rather than only the separate `Roadmap` Project).
- [x] Verify `Auto-close issue`, `Item closed`, and `Pull request merged` are **Off** in the Project #3 workflow UI.
- [x] Inspect all enabled workflow **actions**: `Item added` sets `Backlog`, `PR linked` sets `Design`, and sub-issue workflow adds items; **none directly sets `Accepted`**. `PR linked → Design` may regress later statuses; mitigation remains recommended.
- [ ] Verify a real workflow status transition and explicit author acceptance.

**Status is not ACCEPTED until the remaining gates pass.** Work on later infrastructure tasks can progress in parallel without misreporting G2 completion.
