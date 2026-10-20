# Miracle GitHub Infrastructure — G4 Progress

**Updated:** 2026-10-17
**Status:** G4 implementation consolidated and validated; security settings confirmed, full acceptance held for actual release/GCC/CodeQL evidence
**Project:** [Miracle Development](https://github.com/users/spwn02/projects/3)
**Tracker:** [G4 issue #4](https://github.com/spwn02/Miracle/issues/4)
**Authority:** [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md), sections 6–7

## Security analysis coverage audit scoped — 2026-10-20

A genuine, independently actionable [G4 issue #24 — verify CodeQL coverage for experimental reference-C++26 modules and reflection](https://github.com/spwn02/Miracle/issues/24) was opened under the existing program milestone #1, labeled `type:infrastructure`, and linked to [G4 parent issue #4](https://github.com/spwn02/Miracle/issues/4). This is a **real remaining security-hardening task**, not a completed analysis or disposable Project-integration fixture. Scope: inspect actual CodeQL run outcome, analyzed source/build paths and C++26 parsing limitations, then publish grounded findings without exposing private vulnerabilities. No CodeQL findings, scan coverage or runtime success are claimed yet.

This legitimate new non-bug item additionally supplied fresh GitHub Projects bot event evidence for G2. See the independent [G2 verification ledger](G2-PROGRESS.md) for exact event IDs and limitations. No artificial ticket, Project setting change, access token or paid integration was introduced.

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

## Final post-protection CI and G4 security-audit preparation

**Reference C++26 CI now confirmed green**, as reported by the maintainer and independently observed in [run #38995484739](https://github.com/spwn02/Miracle/actions/runs/38995484739) on `561c8c6`; subsequent [run #38995614569](https://github.com/spwn02/Miracle/actions/runs/38995614569) on `be6ca58` also passed. [mdBook publishing #38995614555](https://github.com/spwn02/Miracle/actions/runs/38995614555) succeeded under protection. GCC workflow runs remained successful but the actual `Advance gcc` step was skipped because the current compiler cannot validate the candidate; do not claim an actual post-protection `gcc` push was tested.

**Pull request history policy:** GitHub's current repository settings allow merge commits, squash merging, and rebase merging; no auto-merge. [G4-PR-WORKFLOW.md](G4-PR-WORKFLOW.md) establishes preservation-first review comments and recommends **Create a merge commit** or an explicit local `git merge --ff-only` when ancestry allows. Neither requires rewriting existing commits. Any restriction of the repository's available UI merge methods remains a separate maintainer setting decision.

**Security availability audit:** Current public metadata and the connected GitHub integration do not expose authoritative private vulnerability, Dependabot alert, secret scanning, or code scanning settings. Those states remain **unknown**, not assumed disabled. [G4-SECURITY-AUDIT.md](G4-SECURITY-AUDIT.md) and [`audit-security.py`](https://github.com/spwn02/Miracle/blob/master/scripts/github/audit-security.py) provide an administrator-authenticated, read-only settings/endpoint audit that prints no sensitive alert payloads. No scanning feature was turned on or claimed supported without verification of C++26 modules/reflection.

## Maintainer security-settings audit and PR merge verification (2026-10-17)

The maintainer ran the account-authenticated read-only `audit-security.py` and supplied its output. The full interpretation is in [G4-SECURITY-AUDIT.md](G4-SECURITY-AUDIT.md).

**Verified enabled:**
- `security_and_analysis.dependabot_security_updates` = `enabled`.
- `security_and_analysis.secret_scanning` = `enabled`.
- `security_and_analysis.secret_scanning_push_protection` = `enabled`.
- Dependency vulnerability alerts endpoint returns HTTP **204**, the documented positive enabled status.

**Verified disabled optional additions:** `secret_scanning_non_provider_patterns` and `secret_scanning_validity_checks`. These are genuine off settings, not proof that the core secret scanner or push protection is off.

**Still unknown:** private vulnerability reporting endpoint returned HTTP 200, but this alone does not identify its `enabled` boolean. Code scanning default setup and alerts endpoints also returned HTTP 200, but no CodeQL `state`, successful analyses, or C++26 compiler/reflection coverage has been established. The improved [read-only audit script](https://github.com/spwn02/Miracle/blob/master/scripts/github/audit-security.py) now prints only safe allowlisted configuration state/boolean fields and avoids listing any alert details.

**Dependabot PR lifecycle:** GitHub confirms three first-run major-update PRs **#11, #10 and #9 were actually merged** into `master` as normal **two-parent merge commits** `cc26fbb`, `de1a2d0`, and `d20f599`, respectively. This preserves the original proposed commit identities; the PRs were not squashed or rebased. Their merged versions affect artifact upload and Pages Actions, so reference CI, Pages publishing and GCC compatibility require green checks at the merged `master` head. No repo setting for merging was silently changed.

**Validation:** `master` was observed at `d20f599` immediately after the three PR merges. Its mdBook publishing run [#38996369465](https://github.com/spwn02/Miracle/actions/runs/38996369465) passed; the reference CI [#38996369479](https://github.com/spwn02/Miracle/actions/runs/38996369479) was still in progress at that checkpoint. Do not treat the PRs' earlier branch CI checks as conclusive validation of subsequent combined merges.

## Least-privilege checkout and workflow regression checks (2026-10-17)

After the maintainer confirmed running the second security audit (without sharing its `Allowlisted configuration values` output), G4 continued on repository-level controls that can be verified independently.

**Committed implementation:**
- `ci.yml`, `gcc.yml`, `docs.yml` and `release.yml` use `actions/checkout` with `persist-credentials: false`. Their checkout steps only require public repository files; the release creation step obtains its separate GitHub token explicitly. PR-triggered workflows no longer retain the checkout credential for later checked-out code execution.
- `sync-gcc.yml` now has read-only **root** permissions, with `contents: write` scoped explicitly to its synchronization job. Its checkout credential remains available because the job performs the legitimate, non-forced `gcc` push.
- [`scripts/github/check-workflow-security.py`](https://github.com/spwn02/Miracle/blob/master/scripts/github/check-workflow-security.py) statically verifies these **Miracle-specific** invariants: root `contents: read`, read-only checkouts, no `pull_request_target` / `workflow_run` escalation, Pages deployment restricted to `master` and never PRs, and write permissions limited to the intentional sync/release/Pages jobs. Four **negative mutation fixtures** ensure prohibited input variants cause the guard to fail.
- The guard is integrated into `ci.yml` before installing the expensive compiler toolchain. This is an additional regression check, **not** a general YAML or workflow-security analyzer.

**Observed GitHub test evidence:** The first run after integration, [reference CI #38998543324](https://github.com/spwn02/Miracle/actions/runs/38998543324), successfully completed `Validate infrastructure shell syntax`, including the SHA pin guard and new workflow-trust policy checker. The full compiler/consumer test outcome remains independently trackable in that run.

**Security-feature state remains bounded by evidence:** The maintainer said they ran the revised read-only security script, but did not supply its configuration-value section. Thus **private vulnerability reporting `enabled`** and **CodeQL default setup `state`** remain unverified. No change was made to either feature, and we must not claim that C++26 reflection/modules have effective CodeQL analysis without successful configured scans.

## Security final-settings evidence and scripts-directory consolidation

The maintainer supplied the follow-up read-only audit: **private vulnerability reporting enabled**, **Dependabot security updates enabled and unpaused**, and **Code scanning default setup configured**. See [G4 security audit](G4-SECURITY-AUDIT.md). The latter is a setup confirmation, not evidence that CodeQL accurately analyzes experimental C++26 reflection/modules.

The maintainer also identified `.github/scripts/` alongside `scripts/` as ambiguous and requested one canonical root. The three historical GitHub CI helpers were moved **unchanged** to `scripts/ci/`; all five active YAML references were rewritten atomically. See [scripts/README.md](https://github.com/spwn02/Miracle/blob/master/scripts/README.md) for the final organization (`ci/`, `github/`, `docs/`). `scripts/ci/test-ci-helpers.py` tests layout and release/GCC report fixtures; the workflow security guard rejects legacy helper paths. GCC validator root resolution is unchanged because the original and new locations are both two directory levels below the repository root.

**Migration gate:** the files and workflow references move together in one Git commit. Follow-up reference CI, GCC compatibility/synchronization and release-provenance paths still require live validation. Do not fabricate a release tag for testing.

## Canonical script migration — post-commit CI acceptance evidence

Atomic commit [`00c5c87`](https://github.com/spwn02/Miracle/commit/00c5c8755b0fdf36a7f070ec044385b2258a6e58) was independently inspected through GitHub's commit/tree APIs. GitHub classifies each of the three helper relocations as a **rename**, with **exactly the same Git blob SHA** as its old location; `.github/scripts/` no longer exists.

**Live GitHub Actions validation of that exact commit:**

| Workflow | Run | Result |
| --- | --- | --- |
| Reference C++26 CI | [#38999112561](https://github.com/spwn02/Miracle/actions/runs/38999112561) | **Success** — infrastructure checks (including helper smoke tests and legacy-path negative guard), self-tests/examples, `add_subdirectory`, `FetchContent`, and installed-package consumers all passed |
| GitHub Pages mdBook | [#38999112521](https://github.com/spwn02/Miracle/actions/runs/38999112521) | **Success** — build and publication passed |
| Sync master to gcc | [#38999112551](https://github.com/spwn02/Miracle/actions/runs/38999112551) | **Success** — candidate creation and GCC validation completed, but `Advance gcc` was **skipped** by the existing compatibility gate; no new GCC-branch ref write was tested |

The script relocation and its live reference-CI/build validation are **complete**. The only still-unverified positive path is a future compatible update *into* protected `gcc`; do not force a branch write to fabricate that evidence.

## Maintainer local smoke test and closeout triage

The maintainer reported a successful local fast-forward to [`2ec2473`](https://github.com/spwn02/Miracle/commit/2ec2473aac07d23501a179a37ee1e699db5226dd) and ran `python3 scripts/ci/test-ci-helpers.py` locally, obtaining **PASS: unified scripts/ci layout, release metadata and GCC convergence fixtures**.

On that same Git commit, the full [reference CI #38999347402](https://github.com/spwn02/Miracle/actions/runs/38999347402), [mdBook Pages #38999347401](https://github.com/spwn02/Miracle/actions/runs/38999347401), and [GCC sync workflow #38999347406](https://github.com/spwn02/Miracle/actions/runs/38999347406) all completed **successfully**. A successful sync workflow must not be reported as successful protected-`gcc` advancement when its `Advance gcc` step was skipped.

The original G4 issue is [kept open for verification](https://github.com/spwn02/Miracle/issues/4): a genuine release run for an approved `v*` tag, a future GCC-compatible branch advancement, actual CodeQL analysis limitations for experimental C++26 semantics, and explicit author phase acceptance. All planned code/script consolidation is complete; no fictitious release or destructive branch test was made.

## Open decisions and checks

- [x] Action refs pinned and guard passed; independent reference CI runs #38995484739 and #38995614569 completed successfully.
- [x] Read-only checkout credentials disabled where possible, GCC write permission job-scoped, workflow-trust guard/negative fixtures executed successfully in CI early step (run #38998543324).
- [x] GitHub Pages mdBook deploy passed after branch protection (run #38995614555).
- [x] GCC sync job completes safely with the compatibility gate (actual protected-branch push still unproven because `Advance gcc` was skipped).
- [x] Maintainer explicitly authorized Dependabot update PRs on 2026-10-17; [.github/dependabot.yml](https://github.com/spwn02/Miracle/blob/master/.github/dependabot.yml) now defines the GitHub Actions-only policy (weekly Monday 09:00 Europe/Warsaw, maximum three open PRs, minor/patch grouped, majors separate, `type:infrastructure` label). PRs target `master`; no auto-merge is configured. Verify the first actual Dependabot run and PR before considering the integration fully validated.
- [x] Live `master`/`gcc` and `v*` rulesets activated and independently verified by REST (IDs `25286606`, `25286609`); post-protection CI/direct-commit and `gcc` synchronization verification remain separate.
- [x] Authenticated security baseline checked: Dependabot alerts/updates, secret scanning and push protection confirmed enabled; optional pattern/validity checks confirmed off.
- [x] Private vulnerability reporting enabled; Dependabot security updates active/unpaused; CodeQL default setup configured. Experimental C++26 analysis coverage remains unverified.
- [ ] Verify affected failure modes and record maintainer acceptance.

**G4 remains open.** Milestone #1, SHA-pinned Actions, Dependabot PR handling, active branch/tag protection, reference CI/Pages baseline, and most security toggles are verified. A compatible protected-`gcc` push, meaningful CodeQL C++26 coverage, post-migration workflow verification and final maintainer acceptance remain outstanding.
