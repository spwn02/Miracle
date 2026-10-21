# G4 — Security Configuration Audit

**Workstream:** [G4 — Security and CI](https://github.com/spwn02/Miracle/issues/4)
**Status:** Python CodeQL `py/redos` alerts #1/#2 confirmed fixed by authenticated individual alert states; successful Python scan with zero findings. C++26 module interfaces remain unextracted (0/33), GCC advance and protected-tag release still outstanding.
**Observed:** 2026-10-17

## Python CodeQL ReDoS alerts #1 and #2 VERIFIED FIXED — 2026-10-21

**Latest authoritative outcome, superseding the earlier pending-rescan section below:** The maintainer performed two **authenticated, read-only GitHub Code Scanning alert-detail** requests for [alert #1](https://github.com/spwn02/Miracle/security/code-scanning/1) and [alert #2](https://github.com/spwn02/Miracle/security/code-scanning/2), both `py/redos` in `scripts/github/check-workflow-security.py`. Each actual result contains `state: fixed`, `most_recent_instance.state: fixed`, `fixed_at: 2026-10-21T15:24:55Z`; neither alert was dismissed. The `most_recent_instance.commit_sha` is still `5b6bb3b91c23013e4705577cc61a0238171a5a9d` for both, identifying the **historical source of the original finding**, not evidence the current `master` is vulnerable.

Corroborating authenticated actual analysis: [GitHub Code Scanning analysis ID `1997641994`](https://github.com/spwn02/Miracle/security/code-scanning) (`/language:python`, `refs/heads/master`, `a1e4fcf88c8ee4acc700abed1df7ef52d9258784`, created `2026-10-21T15:24:55Z`) reports `results_count: 0`, `error: ""`. The previous fixed-source merge `452780b` also reports 0 results (analysis `1997166979`); the older vulnerable source `5b6bb3b` reports 2 results (analysis `1997301220`). This supports a genuine fixed state, not a scanner job that merely completed.

**Resolution:** The two High-severity Python ReDoS findings are **fixed and the scanner agrees**; [merged PR #25](https://github.com/spwn02/Miracle/pull/25) has completed this G4 security subgate. There was no suppression or manual dismissal. This **does not** resolve the independent G4 CodeQL C++26 module coverage gap: all 33 module interfaces were excluded from successful extraction, as documented below, and [Issue #24](https://github.com/spwn02/Miracle/issues/24) remains open.

**Other live G4 evidence:** [post-merge Pages run #39625540156](https://github.com/spwn02/Miracle/actions/runs/39625540156) succeeded, including published-site content and external URL verification. [Reference C++26 CI #39625540134](https://github.com/spwn02/Miracle/actions/runs/39625540134) succeeded. [GCC candidate sync #39625540334](https://github.com/spwn02/Miracle/actions/runs/39625540334) succeeded **but `Advance gcc` was skipped**. A genuine version-tag release under the protection ruleset activated 2026-10-17 and explicit final maintainer acceptance are still outstanding. G4 must remain open.

## ReDoS remediation merged, master rescan pending — 2026-10-21

The previously observed two High-severity CodeQL `Inefficient regular expression` (Python `py/redos`, CWE-1333) findings at lines 71/82 of `scripts/github/check-workflow-security.py` concerned the `sync` and `release` job-scope checks. Their nested DOTALL regexes enabled excessive backtracking on crafted near-matching workflow text; CI exposes this guard to repository-controlled workflow-file content, not a public HTTP request.

[PR #25](https://github.com/spwn02/Miracle/pull/25) replaces both regex checks with a **linear-time, indentation-aware line scanner** that only accepts `contents: write` inside the intended job-level permissions block. It also adds regression fixtures for sibling job confusion, shell-block spoofing and a 16,000-line near-match. The complete PR-head [reference CI #39614467743](https://github.com/spwn02/Miracle/actions/runs/39614467743) passed, including **71/71 self-tests**, the three CMake consumer modes and `PASS: 5 workflow trust boundaries, 5 mutations, and 4 parser fixtures.` The PR's CodeQL Python check also completed successfully.

The maintainer explicitly approved merging. The PR is **merged as a two-parent [commit `452780b`](https://github.com/spwn02/Miracle/commit/452780b90e7b87470e435dadb5cfc38a712c2813)**, and GitHub post-merge verification confirmed that exact code on `master`. **CodeQL alert closure is pending a post-merge `master` scan and actual alert-state verification**; a passed CodeQL job on the PR does not prove that pre-existing branch alerts have closed. No alerts were manually dismissed. This Python patch does not change the separately identified CodeQL failure to extract all 33 C++26 `.ixx` module interfaces.

## CodeQL C++26 extraction scope — CSV-confirmed finding (2026-10-21)

The maintainer uploaded the genuine **CodeQL Tool Status analyzed-files CSV** (`code-scanning-files-extracted.csv`, SHA-256 `769fca48b7f156f3779df029e9afa55b38529db0dc32dfed521b3c9d05d51c05`). The available `/language:c-cpp` configuration lists **119 distinct source files**: **86 `.cxx` files all successfully extracted**, and **33 `.ixx` module interfaces all `Successfully Extracted=false`**. The set of 33 failed paths exactly matches the complete `src/public/**/*.ixx` inventory. All 8 `src/private/**/*.cxx` implementation units report successful extraction. All 33 `.ixx` `Failure Reason` fields are empty.

**Explicit security limit:** Module-interface bodies — including the compiler/reflection-heavy `src/public/meta/Reflect.ixx`, `Annotations.ixx`, and the public `Miracle.ixx` surface — are **not successfully extracted**, and CodeQL results cannot provide source-coverage assurance for those bodies. This is **demonstrated from actual per-file extraction flags**, not an estimate, a claim that everything was skipped, or a claim of a verified code vulnerability. The [CodeQL supported languages reference](https://codeql.github.com/docs/codeql-overview/supported-languages-and-frameworks/) confirms C++ modules are not supported at present and does not list `.ixx` among the supported extensions. The exact reason for each CSV `false` and the effective scanner build mode remain unknown; the CSV contains no analysis ID, commit SHA, build mode or extraction warnings.

**Validated positive scope:** All 86 conventional `.cxx` files successfully extracted; Python, JS and Actions files also extracted under their language configurations. Extraction is not equivalent to guaranteed completeness of semantic analysis, and a `results_count: 0` report is **not** proof of vulnerability absence. Do not "fix" coverage by simply relabeling extensions or weakening the pinned reference-C++26 compiler requirement.

**Residual security triage:** The owner separately shared a CodeQL UI screenshot showing two currently open *High* severity `Inefficient regular expression` alerts in `scripts/github/check-workflow-security.py` (approximately lines 71 and 82). Their exploitability and appropriate remediation must be inspected; this file and its test fixtures are relevant to the G4 workflow trust-boundary guard. Do not treat either finding as proven exploitable based solely on a scanner severity label. The Python analysis metadata `results_count: 2` is consistent with that screenshot, but should not be used as a substitute for examining actual alert contexts.

**Evidence and mitigation boundary:** [Full per-file source inventory and CSV findings](G4-CODEQL-COVERAGE-INVENTORY.md) and [G4 progress](G4-PROGRESS.md). Actual CodeQL scanner support for modules/reflection is outside the current supported matrix and no valid provider fix is available from these artifacts alone. Maintain secure CI and document the limitation rather than falsely advertising complete C++26 CodeQL coverage.

## Follow-up CodeQL analysis evidence — 2026-10-21

The maintainer executed `gh api repos/spwn02/Miracle/code-scanning/default-setup` and `gh api 'repos/spwn02/Miracle/code-scanning/analyses?per_page=10'` from an authorized account. The default setup is **configured** with the default query suite, including `c-cpp`, `python`, `actions`, and JavaScript/TypeScript aliases.

Two actual C/C++ analyses on `master` were identified: `1990589303` on commit `cf75c9c` (2026-10-20 17:53:46 UTC) and `1990547764` on `9add359` (17:47:40 UTC). Both list CodeQL, `category: /language:c-cpp`, 58 rules, 0 results and an empty `error` property. Ancillary latest analyses: JS/TS (87 rules, zero results), Actions (17 rules, zero), Python (43 rules, two analysis results). This proves real analyzer execution and record persistence **only**, not universal coverage or adjudicated findings.

[GitHub's supported-language list](https://codeql.github.com/docs/codeql-overview/supported-languages-and-frameworks/) explicitly excludes **C++20 modules**, lists C++23 as beta and does not list the `.ixx` extension among recognized C/C++ source suffixes. The authentic `master` file tree has **33 `.ixx` module interface files**; `Miracle.ixx` and `meta/Reflect.ixx` explicitly use modules and C++26 reflection respectively. GitHub documents that default C/C++ setup normally uses build-free `none` mode, but the effective mode of the observed runs was **not supplied**.

**Security assessment:** Maintain CodeQL as an ancillary analyzer of supported source/workflows, not an assertion that the current tool supports Miracle's C++26 modules or reflection. Need owner-side **Security and quality → Code scanning → Tool status** file coverage, build mode, errors/warnings, and analyzed-files CSV to establish what files are actually processed. Do not expose alert bodies in public documents. See [G4-PROGRESS.md](G4-PROGRESS.md) and tracking [Issue #24](https://github.com/spwn02/Miracle/issues/24).

## Verified public/repository state

- `spwn02/Miracle` is a **public** repository.
- [`SECURITY.md`](https://github.com/spwn02/Miracle/blob/master/SECURITY.md) exists and directs vulnerability reporters to **private** channels, not public Issues.
- GitHub Actions permissions are explicitly restricted by each workflow. Reference CI and mdBook publishing have passed with SHA-pinned external Actions.
- The minimal protection rulesets are **active**: [branches #25286606](https://github.com/spwn02/Miracle/rules/25286606) and [release tags #25286609](https://github.com/spwn02/Miracle/rules/25286609).
- GitHub Dependabot successfully completed its first **GitHub Actions-only** update run, producing legitimate reviewable PRs #9–#11.
- The connected integration does not expose a `security_and_analysis` object, but the maintainer's authenticated `gh` session **does** expose the enablement flags detailed below. This is a difference in available permissions, not inconsistent repository state.

The connected GitHub integration can read repository configuration, Issues, Pull Requests, rulesets and Actions, but cannot inspect privileged security administration and alert endpoints. **No private vulnerability reports, secret-alert details, or access tokens were requested or exposed.**

## Maintainer audit output — verified findings (2026-10-17)

The maintainer executed `python3 scripts/github/audit-security.py` using their authenticated `gh` session after fast-forwarding local `master`. Its output establishes:

| GitHub configuration / endpoint | Observed result | What is established |
| --- | --- | --- |
| Repository | `Public: True` | Public `spwn02/Miracle` |
| Dependabot security updates | `enabled`; API HTTP 200 | Security update feature enabled; **not** the same as weekly version-update PRs |
| Secret scanning | `enabled` | GitHub secret scanning enabled |
| Secret scanning push protection | `enabled` | GitHub push protection enabled |
| Non-provider secret patterns | `disabled` | Optional extra pattern detection disabled |
| Secret validity checks | `disabled` | Optional secret validity verification disabled |
| Dependabot vulnerability alerts | HTTP **204** | **Enabled**; per [GitHub's API](https://docs.github.com/en/rest/repos/repos#check-if-vulnerability-alerts-are-enabled-for-a-repository), 204 specifically means dependency vulnerability alerts are enabled |
| Private vulnerability reporting | HTTP **200** | Endpoint reachable; must inspect its returned `enabled` boolean to determine effective state |
| Code scanning default setup | HTTP **200** | Configuration endpoint reachable; must inspect its `state` to determine if CodeQL default setup is configured |
| Code scanning alerts access | HTTP **200** | Alert-list endpoint accessible; not evidence that a scan has run |
| Secret scanning alerts access | HTTP **200** | Alert-list endpoint accessible; not a statement about number or severity of alerts |

**Important distinction:** Code scanning API access is not equivalent to actual configured CodeQL analysis. Even an enabled default setup may provide incomplete coverage for Miracle's experimental C++26 reflection/modules and pinned reference compiler; require actual successful analyzer evidence before claiming protection of those semantics.

No private alert bodies, security findings, secrets, or vulnerability details were published in this audit. No feature was toggled through these checks.

### Narrow follow-up: exact boolean/status values

The read-only script has now been improved to fetch only allowlisted fields (`enabled`, `paused`, and default-setup `state`) from non-alert configuration endpoints, while **no longer requesting any alert-list endpoints**. It still prints no security alert or secret contents.

```sh
git pull --ff-only origin master
python3 scripts/github/audit-security.py
```

Use its **Allowlisted configuration values** section to resolve the private reporting and CodeQL default-setup statuses without screenshots of sensitive alert pages. This additional pass is verification only, not a request to enable CodeQL prematurely.

## Second authenticated audit — feature status resolved (2026-10-17)

The maintainer provided the actual **Allowlisted configuration values**, beyond HTTP reachability:

| Feature | Reported configuration | Conclusion |
| --- | --- | --- |
| Private vulnerability reporting | `{'enabled': True}` | **Enabled** |
| Dependabot security updates | `{'enabled': True, 'paused': False}` | **Enabled, not paused** |
| Code scanning default setup | `{'state': 'configured'}` | **Configured** |

No settings were changed. A configured CodeQL default setup does **not** prove successful scans or adequate coverage of Miracle's experimental C++26 reflection/modules and custom compiler. Those are separate analysis-evidence gates.

## Maintainer-side, read-only security audit

A local [`audit-security.py`](https://github.com/spwn02/Miracle/blob/master/scripts/github/audit-security.py) script uses the maintainer's existing authenticated GitHub CLI to inspect feature availability without printing sensitive alert bodies or secrets.

```sh
git pull --ff-only origin master
python3 scripts/github/audit-security.py
```

It reports safe repository setting flags, endpoint HTTP status codes, and allowlisted non-alert configuration booleans/state for:
- Private vulnerability reporting
- Dependency vulnerability alerts (Dependabot alerts)
- Dependabot security update enablement
- Code scanning default-setup state (without claiming reflection analysis coverage)
- No longer lists or retrieves code-scanning or secret-scanning alerts

**Interpretation:** HTTP **204** from the vulnerability-alerts enablement endpoint affirmatively means Dependabot alerts are on. HTTP 200 on the other endpoints generally demonstrates access, not full scanning configuration or an alert-free state. HTTP 403/404 may reflect disabled features, insufficient permissions, or a plan/eligibility restriction. Do not interpret a missing field or inaccessible endpoint as definitive disabled status. Confirm the actual toggle in the repository's **Settings → Security & analysis / Advanced Security**.

## Settings to inspect, without silently changing them

| Setting | What the maintainer should verify |
| --- | --- |
| Private vulnerability reporting | Enabled where offered; use the dedicated confidential reporting URL and not public Issues |
| Dependabot alerts | Enabled and reviewed by someone with repository security permissions; no contents of private alerts pasted into public Issues |
| Dependabot security updates | Separate from the approved *version-update* PRs and governed by review |
| Secret scanning and push protection | Availability and current state; report limitations where a feature requires extra plan/eligibility |
| Code scanning | Availability and actual completed analyses; if using CodeQL, C++26 module/reflection coverage must be assessed rather than assumed |
| Advisories | Confirm access to private advisory management and coordinated disclosure without manufacturing a vulnerability |

Do not add generic CodeQL C++ analysis to CI and declare coverage before verifying compatibility with Miracle's pinned compiler, modules, and reflection extensions. A failed/default scan could give misleading assurance or create false noise.

## Completion gates

- [x] Maintainer supplied safe feature-state summary (without alert or secret details) from authenticated CLI.
- [x] Private vulnerability reporting confirmed enabled; SECURITY.md documents the fallback.
- [x] Dependabot alerts and security updates confirmed enabled and distinguished from weekly version-update PRs.
- [x] Secret scanning, push protection, optional pattern/validity flags and code-scanning endpoint availability documented with limitations; actual CodeQL configuration still pending.
- [x] Relevant settings verified through authenticated configuration API; no changes requested.
- [ ] G4 ledger updated with acceptance evidence.

No settings were changed by this read-only audit documentation.
