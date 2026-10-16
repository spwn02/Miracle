# Miracle GitHub Infrastructure — G0 Repository Audit

**Status:** Evidence gathered; G0 not yet accepted
**Audit date:** 2026-10-16
**Repository:** https://github.com/spwn02/Miracle
**Observed default branch:** `master`
**Audited remote HEAD:** `ebffbcbb99e5564d3a5e065d87f009771f38a009`
**Observed release:** `v0.5.27` (published 2026-10-16)
**Authority:** Local author-designated Git repository; remote audit is a point-in-time cross-check only.

## 1. Method and confidence

Read-only inspection used the connected GitHub integration to fetch repository metadata, tree entries, configuration files, workflow runs and selected job logs. Findings are specific to the audited remote HEAD. Local unpushed work, private configuration pages and inaccessible administration endpoints are **not** verified. This document records observations; it does not itself authorize or apply configuration changes.

### Verified

| Area | Evidence | Result |
|---|---|---|
| Repository | GitHub repo metadata | Public, Apache-2.0, default `master`, issues/projects/wiki enabled, discussions disabled; empty homepage/topics |
| Branches | GitHub branches API | `master` and `gcc` exist; both report `protected: false` |
| Rulesets | GitHub rulesets collection | No repository rulesets returned |
| Releases | GitHub releases | `v0.5.27`, `v0.1.0`, `v0.1.0-rc.1` present |
| Issues | Issues collection (all states) | No issues returned |
| CI | `.github/workflows/` | `ci.yml`, `gcc.yml`, `release.yml`, `sync-gcc.yml` |
| Tools | `.github/scripts/` | Release provenance, GCC convergence and GCC validation scripts |
| Development docs | `docs/` | Architecture, compiler support, diagnostics, feature, iter, meta, range, reference toolchain, static string, capabilities |
| Contribution and security | Root files | `CONTRIBUTING.md`, `SECURITY.md`, `README.md`, `LICENSE`, `CHANGELOG.md` present |
| Templates | `.github/` file listing | No issue forms or PR template present in inspected tree |
| Codespaces | `.devcontainer/` path | Not present at audited HEAD |
| Community docs | Root contents listing | No CODE_OF_CONDUCT.md, SUPPORT.md or ACCESSIBILITY.md in root |
| CI status | GitHub Actions runs | Latest inspected `CI` and `Sync master to gcc` runs succeeded |

## 2. Actionable findings

### G0-F01 — Dependabot has two broken ecosystems (confirmed)

`.github/dependabot.yml` currently configures `vcpkg` and `pip` updates in `/`. The most recent jobs failed with `dependency_file_not_found`, specifically `No files found in /`.

- `pip` failure: https://github.com/spwn02/Miracle/actions/runs/38948280427
- `vcpkg` failure: https://github.com/spwn02/Miracle/actions/runs/38948264968

**Recommendation:** Replace invalid root ecosystems with `github-actions` updates, and include any other ecosystem only if its manifest exists. Keep this change separate from the protocol adoption commit. Verify a subsequent Dependabot run instead of claiming success from editing YAML alone.

Candidate configuration (for later G4 review):

```yaml
version: 2
updates:
  - package-ecosystem: github-actions
    directory: /
    schedule:
      interval: weekly
```

### G0-F02 — Compiler/toolchain documentation drift (confirmed)

`CONTRIBUTING.md` refers to `spwn02/clang-p2996` as current. The README and current reference CI use `spwn02/clang-cxx26`, and CI pins `cxx26-2026.10.09.2` with source revision `040df2b05ea338d107fac7387a2e4db835d746e0`. The reference-toolchain documentation also includes stale snapshot identifiers and dates.

**Recommendation:** Reconcile prose with a single documented immutable validated snapshot, while keeping historical snapshots clearly labeled as historical. Ensure that CI and documentation describe the same snapshot. Check the complete document before editing: some values may be intentionally historical examples.

### G0-F03 — Branch security must accommodate synchronization semantics (confirmed risk)

`sync-gcc.yml` merges `master` into `gcc` using `git merge --no-ff`, validates the candidate, then performs a **non-forced** push. It may legitimately leave `gcc` behind. In addition to maintaining automation write permissions, future branch policies must not prohibit required merge commits or block the bot without a tested exemption.

**Recommendation:** Plan `master` and `gcc` rules independently. Do **not** enable a linear-history requirement on `gcc` without redesigning synchronization. Validate a real synchronization attempt before accepting new `gcc` rules.

### G0-F04 — Governance files not yet repository-authoritative (confirmed)

The approved Miracle Development Protocol v1.0 exists in the discussion but **not** in the inspected repository `docs/` directory. The new collaboration rules require repository authority.

**Recommendation:** Commit the approved text verbatim as `docs/DEVELOPMENT_PROTOCOL.md` before marking G0 accepted. Do not silently regenerate or amend approved clauses.

### G0-F05 — Community intake and presentation incomplete (confirmed)

The repo lacks issue forms, a PR template, Code of Conduct and Accessibility/Support documents in the inspected locations; Discussions is off; repository homepage and topics are unset. These are later workstreams, not blockers for G0 acceptance once recorded in the roadmap.

## 3. Access-limited / not verified

The integration received HTTP 403 for branch-protection detail APIs. It cannot audit private repository security settings, secrets, deploy keys, webhook payloads, deployment environment policies, Codespaces spending, or all admin-only configuration. `protected:false` and an empty ruleset collection are direct observations; other administrative settings must be checked by the project owner.

The existence or contents of a personal-account GitHub Project, a populated Wiki, GitHub Pages deployment, vulnerability-reporting toggle, and code-scanning default setup were **not fully verified**. A successful `CodeQL Setup` workflow run was visible, but that alone does not prove scanning of Miracle's C++26 code is effective.

## 4. Owner's manual setting audit

Record `Yes`, `No`, or `Unknown` for each item while viewing repository settings:

- [ ] Repository **Settings → General**: merge methods, auto-delete head branches, visibility, description, topics and homepage.
- [ ] **Settings → Branches / Rules → Rulesets**: existing rules, required checks and exemptions.
- [ ] **Settings → Actions → General**: permissions, workflow approvals for forks, allowed actions, default token permissions.
- [ ] **Settings → Security & analysis / Code security**: Dependabot alerts, secret scanning, code scanning, private vulnerability reporting.
- [ ] **Settings → Environments**: existing deployment targets and secret scoping.
- [ ] **Settings → Pages**: current hosting/deployment source and custom domain.
- [ ] **Settings → Deploy keys**: existing keys, access levels and need for retention.
- [ ] **Settings → Webhooks**: installed hooks, target endpoints, secret verification and necessity (do not record secret values).
- [ ] Account-owned **Projects**: whether a Miracle project already exists, visibility and repository linkage.
- [ ] **Wiki**: whether content exists, who may edit, and whether it could drift from `docs/`.
- [ ] **Codespaces**: budgets, secrets and default image/prebuild state.

Record only safe configuration conclusions in public docs. Never commit credentials, tokens, webhook secrets, private endpoint details or sensitive account data.

## 5. G0 acceptance requirements

- [ ] Author compares audited remote HEAD with the authoritative local Git tree.
- [ ] Exact approved Development Protocol v1.0 text is committed under `docs/`.
- [ ] Infrastructure contract accepted by the author, then committed with its status changed from Draft to Locked.
- [ ] Public README and CONTRIBUTING provide working links to the approved protocol.
- [ ] Findings G0-F01 through G0-F05 have assigned follow-up workstreams; the underlying G4/G1 fixes need not all land in G0.
- [ ] A compact infrastructure progress ledger records G0 acceptance status, the new commit SHA and next workstream.
- [ ] No administrative protection changes break `sync-gcc.yml` or release automation.

**Observed state:** Evidence gathering complete. **G0 remains pending**, because repository documents and acceptance checks have not yet been committed or verified.

## 6. Evidence links

- Repository: https://github.com/spwn02/Miracle
- CI: https://github.com/spwn02/Miracle/actions/workflows/ci.yml
- GCC synchronization: https://github.com/spwn02/Miracle/blob/master/.github/workflows/sync-gcc.yml
- Dependabot: https://github.com/spwn02/Miracle/blob/master/.github/dependabot.yml
- Documentation: https://github.com/spwn02/Miracle/tree/master/docs
- Current release: https://github.com/spwn02/Miracle/releases/tag/v0.5.27
