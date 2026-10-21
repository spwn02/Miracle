# G4 — Security Configuration Audit

**Workstream:** [G4 — Security and CI](https://github.com/spwn02/Miracle/issues/4)
**Status:** Maintainer-authenticated security baseline confirmed; active CodeQL default-setup analyses verified 2026-10-20, while experimental C++26 module/reflection extraction and file coverage remain unverified
**Observed:** 2026-10-17

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
