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

## Branch and release-tag protection: configuration prepared, not applied

The maintainer supplied a screenshot that GitHub still warns **`master` is not protected** and authorized proceeding to G4's next priority. A connected read-only audit on 2026-10-17 confirmed `master` and `gcc` both report `protected: false`; repository rulesets are `[]`.

A minimal, version-controlled proposal now lives in [G4-RULESETS.md](G4-RULESETS.md):

- Branch ruleset, **`master` and `gcc` only**: prevent deletion and non-fast-forward pushes, without preventing normal author fast-forward commits, Dependabot PR merging or the `gcc` no-ff merge-and-fast-forward update.
- Tag ruleset, **`v*` only**: prevent removal and revision of already published version tags; permit normal first-time tag creation.
- Rulesets deliberately do **not** require PR-only changes, signing, linear history, branch creation restrictions, or mandatory checks at this stage.
- [`reconcile-rulesets.py`](https://github.com/spwn02/Miracle/blob/master/scripts/github/reconcile-rulesets.py) implements explicit **`--check` (read-only)**, **`--verify` (read-only)** and **`--apply` (requires maintainer's separate action)** against two reviewed [JSON recipes](https://github.com/spwn02/Miracle/tree/master/scripts/github/rulesets). It rejects drift and unknown rulesets instead of silently overwriting other policies.
- Reference CI syntax-compiles the Python script and validates the two recipes offline without GitHub administration.

**Current status: NOT APPLIED.** Source commits cannot activate repository protection. The connected GitHub integration cannot modify repository administrative rulesets, and the infrastructure contract reserves that change to the maintainer's reviewed configuration. Once the maintainer runs `--apply`, capture the actual ruleset IDs and verify that normal `master` CI and a `gcc` synchronization still succeed. A ruleset definition's `enforcement: active` does not mean GitHub is currently enforcing it.

## Live ruleset activation verified (2026-10-17)

The maintainer ran `reconcile-rulesets.py --check`, `--apply`, and `--verify` after synchronizing local `master`. The read-only preflight reported both recipes missing; the explicit apply created the rulesets, and the post-apply verification passed.

A **separate live GitHub REST read** confirmed the following actual repository state:

| Protection | GitHub rule ID | Target refs | Effective rules |
| --- | --- | --- | --- |
| [Development branches](https://github.com/spwn02/Miracle/rules/25286606) | `25286606` | `refs/heads/master`, `refs/heads/gcc` | `deletion`, `non_fast_forward` |
| [Release tags](https://github.com/spwn02/Miracle/rules/25286609) | `25286609` | `refs/tags/v*` | `deletion`, `update` |

Both live responses reported **`enforcement: active`**, an **empty `bypass_actors`** array, and `current_user_can_bypass: never`. Subsequently, GitHub's branch metadata reported `protected: true` for both `master` and `gcc`. The published G4 JSON recipes and ruleset fingerprints match those results; no extra ruleset or permissive bypass was introduced.

The preceding successful reference CI, Pages deployment and `master` → `gcc` sync occurred **before ruleset activation**, so their success is not by itself a post-protection integration test. The present ledger update is the first ordinary direct-to-`master` write used to exercise the newly protected branch. Its resulting Actions executions require independent inspection before reporting runtime validation. A `Sync master to gcc` workflow marked successful can choose *not to advance* `gcc` if the compatibility candidate is unsupported; log the actual result before claiming push compatibility.

**Release-tag enforcement:** the live ruleset configuration is proven, but no destructive tag update was attempted and no artificial release was created. The next legitimate release can verify permitted first creation without risking historical tags.

## First positive-path checks under active rulesets

A routine edit of this ledger was **accepted directly on protected `master`**, commit [`561c8c6`](https://github.com/spwn02/Miracle/commit/561c8c6079f55339b7d854bab1fc9b430e16d5a2). This proves the minimal no-deletion/no-force-push policy does not prohibit normal direct fast-forward development.

- [Post-protection documentation run #38995484861](https://github.com/spwn02/Miracle/actions/runs/38995484861): **success**; the Rust-style mdBook site built and deployed.
- [Post-protection synchronization run #38995484766](https://github.com/spwn02/Miracle/actions/runs/38995484766): **workflow success, no push**. Candidate merge, GCC validation, convergence report and upload succeeded; its log explicitly stated that the current GCC could not satisfy the master candidate. `Advance gcc` was **skipped** and `gcc` remained at `2822de260944093672d3eec2462be0a4e7c2401f`. This is an existing compatibility gate, not evidence of a ruleset rejection. A successful push *into the now-protected `gcc`* remains unproven.
- [Post-protection reference CI #38995484739](https://github.com/spwn02/Miracle/actions/runs/38995484739): at the time of this entry, infrastructure checks and toolchain installation had passed, but the self-tests/consumer build and final conclusion were still **pending**.

Actual ruleset enforcement, target refs and no-bypass policy were verified. No destructive force push or tag rewrite was attempted. The next legitimate supported GCC revision and normal new release tag can validate those remaining positive paths without manufacturing unsafe changes.

## Open decisions and checks

- [x] Action refs resolved to published Git SHAs and pin-guard/syntax steps passed on pinned reference CI; the full final-commit CI run still needs its conclusion checked.
- [ ] GitHub Pages mdBook deploy remains green with pinned actions.
- [ ] `gcc` sync and compatibility continue successfully.
- [x] Maintainer explicitly authorized Dependabot update PRs on 2026-10-17; [.github/dependabot.yml](https://github.com/spwn02/Miracle/blob/master/.github/dependabot.yml) now defines the GitHub Actions-only policy (weekly Monday 09:00 Europe/Warsaw, maximum three open PRs, minor/patch grouped, majors separate, `type:infrastructure` label). PRs target `master`; no auto-merge is configured. Verify the first actual Dependabot run and PR before considering the integration fully validated.
- [x] Live `master`/`gcc` and `v*` rulesets activated and independently verified by REST (IDs `25286606`, `25286609`); post-protection CI/direct-commit and `gcc` synchronization verification remain separate.
- [ ] Review private security-reporting availability and actual repository security settings, without making unsupported claims.
- [ ] Verify affected failure modes and record maintainer acceptance.

**Milestone #1 was verified. Dependabot Actions version updates are now configured to open reviewable PRs. Branch/tag rulesets and automated PR merging have not been enabled.**
