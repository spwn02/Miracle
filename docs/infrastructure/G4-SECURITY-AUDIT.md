# G4 — Security Configuration Audit

**Workstream:** [G4 — Security and CI](https://github.com/spwn02/Miracle/issues/4)
**Status:** Baseline verified; administrative security feature states pending maintainer audit
**Observed:** 2026-10-17

## Verified public/repository state

- `spwn02/Miracle` is a **public** repository.
- [`SECURITY.md`](https://github.com/spwn02/Miracle/blob/master/SECURITY.md) exists and directs vulnerability reporters to **private** channels, not public Issues.
- GitHub Actions permissions are explicitly restricted by each workflow. Reference CI and mdBook publishing have passed with SHA-pinned external Actions.
- The minimal protection rulesets are **active**: [branches #25286606](https://github.com/spwn02/Miracle/rules/25286606) and [release tags #25286609](https://github.com/spwn02/Miracle/rules/25286609).
- GitHub Dependabot successfully completed its first **GitHub Actions-only** update run, producing legitimate reviewable PRs #9–#11.
- The repository's general metadata response does **not** expose a usable `security_and_analysis` settings object through the connected integration. That is **unknown**, not evidence that scanning features are disabled.

The connected GitHub integration can read repository configuration, Issues, Pull Requests, rulesets and Actions, but cannot inspect privileged security administration and alert endpoints. **No private vulnerability reports, secret-alert details, or access tokens were requested or exposed.**

## Maintainer-side, read-only security audit

A local [`audit-security.py`](https://github.com/spwn02/Miracle/blob/master/scripts/github/audit-security.py) script uses the maintainer's existing authenticated GitHub CLI to inspect feature availability without printing sensitive alert bodies or secrets.

```sh
git pull --ff-only origin master
python3 scripts/github/audit-security.py
```

It reports only safe repository setting flags and HTTP status codes for:
- Private vulnerability reporting
- Dependency vulnerability alerts (Dependabot alerts)
- Dependabot security update enablement
- Code scanning default setup and alerts availability
- Secret scanning alerts availability

**Interpretation:** HTTP 200/204 suggests that an endpoint is accessible; **it does not automatically prove full scanning configuration or alert-free operation**. HTTP 403/404 may reflect disabled features, insufficient permissions, or a plan/eligibility restriction. Do not interpret a missing field or inaccessible endpoint as definitive disabled status. Confirm the actual toggle in the repository's **Settings → Security & analysis / Advanced Security**.

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

- [ ] Maintainer supplies safe feature-state summary (without alert or secret details).
- [ ] Private reporting state and fallback method confirmed.
- [ ] Dependabot alerts/security updates state documented separately from weekly version updates.
- [ ] Secret scanning, push protection, advisory and code-scanning availability documented with limitations.
- [ ] Any agreed setting changes made explicitly and independently verified.
- [ ] G4 ledger updated with acceptance evidence.

No settings were changed by this read-only audit documentation.
