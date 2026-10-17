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
- Corrected milestone **#1 in place** to [`GitHub Infrastructure (G0–G6)`](https://github.com/spwn02/Miracle/milestone/1). The [one-time GitHub Actions reconciliation](https://github.com/spwn02/Miracle/actions/runs/38994367818) passed with narrowly scoped `issues: write` permission. Follow-up reads of issues #1–#6 and #8 confirm their assignment to milestone #1; deferred contract migration issue #7 has no milestone. The temporary elevated workflow was removed in commit `c6f52f4`; [`scripts/github/reconcile-infrastructure-milestone.sh`](https://github.com/spwn02/Miracle/blob/master/scripts/github/reconcile-infrastructure-milestone.sh) remains a reusable maintainer utility.

## GitHub Actions permission inventory

| Workflow | Existing permissions |
| --- | --- |
| `ci.yml` | `contents: read` |
| `gcc.yml` | `contents: read` |
| `docs.yml` | `contents: read`; deploy job `pages: write`, `id-token: write` |
| `sync-gcc.yml` | `contents: write` for non-forced `gcc` update |
| `release.yml` | `contents: read`; release job `contents: write` |

The `master` and `gcc` branches were both unprotected at the start of G4 and no repository rulesets were present. Branch and tag protection have not been modified. Ruleset design must preserve the validated `sync-gcc.yml` merge-and-push behavior.

## Dependabot decision and operational policy

The maintainer approved **Dependabot-generated pull requests**, without changing the earlier instruction that AI-authored G0–G6 infrastructure implementation commits should go directly to `master`.

- **Source:** [`.github/dependabot.yml`](https://github.com/spwn02/Miracle/blob/master/.github/dependabot.yml), committed as `29c840c`.
- **Scope:** `package-ecosystem: github-actions` at directory `/`, target `master`. No `pip`, `vcpkg`, CMake, Docker, LLVM, libc++, or library-code updater has been enabled.
- **Schedule:** Monday at 09:00 in `Europe/Warsaw`; limit of three open version-update PRs.
- **Review noise:** group minor and patch updates to GitHub Actions; present major versions as independent PRs requiring explicit scrutiny.
- **Label:** `type:infrastructure` (confirmed existing repo label).
- **Security:** third-party `uses:` refs remain full-commit-SHA-pinned. GitHub Dependabot recognizes SHA-pinned Actions and version comments. The reference CI's `scripts/github/check-workflow-pins.py` must remain green.
- **Merge gate:** Dependabot may **propose** updates but cannot approve or merge them by this configuration. Review CI, pinned source provenance and synchronization effects before author approval. PR merge does not imply Miracle's human `Accepted` Project status.
- **Activation verified:** [Dependabot update run #38994801125](https://github.com/spwn02/Miracle/actions/runs/38994801125) completed successfully and immediately opened three genuine major-version update PRs: [#9 — actions/upload-pages-artifact v4→v5](https://github.com/spwn02/Miracle/pull/9), [#10 — actions/deploy-pages v4→v5](https://github.com/spwn02/Miracle/pull/10), and [#11 — actions/upload-artifact v4→v7](https://github.com/spwn02/Miracle/pull/11). All were opened by `dependabot[bot]`. These occupy all three configured open PR slots, so other version updates may queue until a review/merge/closure creates capacity. **No PR has been accepted or merged by this configuration.**

## Open decisions and checks

- [x] Action refs resolved to published Git SHAs and pin-guard/syntax steps passed on pinned reference CI; the full final-commit CI run still needs its conclusion checked.
- [ ] GitHub Pages mdBook deploy remains green with pinned actions.
- [ ] `gcc` sync and compatibility continue successfully.
- [x] Maintainer explicitly authorized Dependabot update PRs on 2026-10-17; [.github/dependabot.yml](https://github.com/spwn02/Miracle/blob/master/.github/dependabot.yml) now defines the GitHub Actions-only policy (weekly Monday 09:00 Europe/Warsaw, maximum three open PRs, minor/patch grouped, majors separate, `type:infrastructure` label). PRs target `master`; no auto-merge is configured. Verify the first actual Dependabot run and PR before considering the integration fully validated.
- [ ] Review branch protection/rulesets and protection of published tags without breaking solo-maintainer commits.
- [ ] Review private security-reporting availability and actual repository security settings, without making unsupported claims.
- [ ] Verify affected failure modes and record maintainer acceptance.

**Milestone #1 was verified. Dependabot Actions version updates are now configured to open reviewable PRs. Branch/tag rulesets and automated PR merging have not been enabled.**
