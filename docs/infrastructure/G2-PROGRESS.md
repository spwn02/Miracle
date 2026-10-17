# Miracle GitHub Infrastructure — G2 Progress

**Updated:** 2026-10-17
**Program:** G0–G6
**Workstream:** G2 — Project, Roadmap, Issue Tracking
**Status:** Bootstrap reported successful; UI configuration and final acceptance pending
**Project:** [Miracle Development — user-owned Project #3](https://github.com/users/spwn02/projects/3)
**Authority:** [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Operating plan:** [G2-OPERATING-PLAN.md](G2-OPERATING-PLAN.md)

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
gh project item-list 3 --owner spwn02 --limit 20 \
  --field Status --field Phase --field Area --format json
```

Project view configuration, saved filters and the Auto-add rule must still be checked in the GitHub UI. CLI output alone does not prove interactive behavior.

## Acceptance checklist

- [x] Actual Project URL recorded.
- [x] Author terminal evidence for project creation/linking and seven custom fields.
- [x] Required repository labels created; current six issues classified.
- [x] Milestone created and issue #2 association verified.
- [x] Author reported repaired bootstrap run complete.
- [ ] Independently inspect Project item-import results and Phase/Area values.
- [ ] Verify the existing Status field has all eight required options.
- [ ] Verify the four saved views and filter/column behavior.
- [ ] Enable and inspect issue Auto-add.
- [ ] Verify real workflow transition and explicit author acceptance.

**Status is not ACCEPTED until the remaining gates pass.** Work on later infrastructure tasks can progress in parallel without misreporting G2 completion.
