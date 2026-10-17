# Miracle GitHub Infrastructure — G2 Progress

**Updated:** 2026-10-17
**Program:** G0–G6
**Workstream:** G2 — Project, Roadmap, Issue Tracking
**Status:** Structure and seven-item backlog VERIFIED; Auto-add filter and acceptance-unsafe default workflows require correction before G2 acceptance
**Project:** [Miracle Development — user-owned Project #3](https://github.com/users/spwn02/projects/3)
**Authority:** [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Operating plan:** [G2-OPERATING-PLAN.md](G2-OPERATING-PLAN.md)

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
- The bootstrap script is versioned at [`scripts/github/bootstrap-g2.sh`](../../scripts/github/bootstrap-g2.sh) and its shell syntax is now checked in CI.

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

The [`scripts/github/audit-g2.sh`](../../scripts/github/audit-g2.sh) script queries Project #3 using the maintainer's authenticated `gh` session. It reports current public visibility, linked repositories, fields/options, saved-view names/layouts/filters/grouping, enabled workflows, and actual issue items (including #7), plus the bot events.

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
- [ ] Correct Auto-add filter from `is:issue,pr is:open label:bug` to `is:issue` and verify a real future issue enters Project #3 automatically.
- [ ] Disable or inspect workflows that could auto-close issues or auto-assign `Accepted`; then verify a real workflow transition and explicit author acceptance.

**Status is not ACCEPTED until the remaining gates pass.** Work on later infrastructure tasks can progress in parallel without misreporting G2 completion.
