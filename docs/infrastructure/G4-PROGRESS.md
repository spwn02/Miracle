# Miracle GitHub Infrastructure — G4 Progress

**Updated:** 2026-10-17
**Status:** Initial Actions hardening implemented; live validation and repository settings review pending
**Project:** [Miracle Development](https://github.com/users/spwn02/projects/3)
**Tracker:** [G4 issue #4](https://github.com/spwn02/Miracle/issues/4)
**Authority:** [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md), sections 6–7

## Implemented

- Resolved publisher tag refs for checkout v6, setup-python v6, upload-artifact v4, configure-pages v5, upload-pages-artifact v4, deploy-pages v4.
- Pinned every matching third-party Actions reference in the five existing GitHub workflows to full 40-character commit SHAs; labels in comments are informational only.
- Added [`scripts/github/check-workflow-pins.py`](https://github.com/spwn02/Miracle/blob/master/scripts/github/check-workflow-pins.py) to reject version-tag refs, with a positive/negative self-test.
- Added the pin guard and syntax checks for the program-wide milestone reconciliation script to reference CI.
- Preserved the existing reference-toolchain pin, Pages deploy permissions, release provenance and `master` → `gcc` non-forced synchronization logic.
- G2's milestone was incorrectly scoped to G2 alone. Added [`scripts/github/reconcile-infrastructure-milestone.sh`](https://github.com/spwn02/Miracle/blob/master/scripts/github/reconcile-infrastructure-milestone.sh) to **rename milestone #1** to `GitHub Infrastructure (G0–G6)` and associate issues #1–#6 and #8. Issue #7 remains an unscheduled separate documentation task. Account-side reconciliation requires the maintainer's authenticated `gh` session; the script itself is not proof that the rename was applied.

## GitHub Actions permission inventory

| Workflow | Existing permissions |
| --- | --- |
| `ci.yml` | `contents: read` |
| `gcc.yml` | `contents: read` |
| `docs.yml` | `contents: read`; deploy job `pages: write`, `id-token: write` |
| `sync-gcc.yml` | `contents: write` for non-forced `gcc` update |
| `release.yml` | `contents: read`; release job `contents: write` |

The `master` and `gcc` branches were both unprotected at the start of G4 and no repository rulesets were present. Branch and tag protection have not been modified. Ruleset design must preserve the validated `sync-gcc.yml` merge-and-push behavior.

## Open decisions and checks

- [ ] Pin-guard and complete reference CI green on final commit.
- [ ] GitHub Pages mdBook deploy remains green with pinned actions.
- [ ] `gcc` sync and compatibility continue successfully.
- [ ] Decide whether Dependabot may open automated GitHub Actions update PRs. The maintainer requested no PRs for direct infrastructure implementation; do not silently enable update bots.
- [ ] Review branch protection/rulesets and protection of published tags without breaking solo-maintainer commits.
- [ ] Review private security-reporting availability and actual repository security settings, without making unsupported claims.
- [ ] Verify affected failure modes and record maintainer acceptance.

**No branch/tag rulesets or Dependabot bots have been enabled by this ledger.**
