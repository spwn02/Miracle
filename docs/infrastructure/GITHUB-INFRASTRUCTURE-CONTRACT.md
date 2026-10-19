# Miracle GitHub Infrastructure Contract v1.0

**Status:** LOCKED — effective upon author acceptance and repository integration
**Program:** GitHub Infrastructure & Governance (G0–G6)
**Created:** 2026-10-16
**Reviewed:** 2026-10-17
**Repository:** `spwn02/Miracle`
**Governed by:** Approved Miracle Development Protocol v1.0
**Scope:** Repository governance, traceability, automation, documentation publishing, community intake and reproducible contributor environments. Does not change Miracle's product or C++ API contracts.

## 1. Mission and non-goals

Make GitHub a truthful, navigable operating surface for Miracle's actual development. A reader should quickly understand what Miracle does, what is currently supported, what is in progress, the evidence behind an accepted milestone, and how to contribute or ask questions.

**Invariants**

- The author's designated local Git repository is authoritative for source and contracts. Remote GitHub is the published repository state. GitHub Projects and Issues track work, but never override a locked contract.
- Avoid maintaining independent manually edited copies of contracts in Issues, Wiki or GitHub Pages.
- A status change in a Project does not by itself establish implementation or verification completion. Evidence and author acceptance are required.
- No retrospective fictitious tickets, approvals, completion dates, or verification records.
- Do not add a dependency, integration, credential or paid service without a demonstrated requirement.
- Preserve `master` as the reference-C++26 development line and `gcc` as a potentially lagging, automated compatibility line.
- Repository administration is owned by the human maintainer. Configuration recommendations are not authorization to change settings.

**Non-goals**

- Redesign Miracle's C++26 API.
- Introduce a runtime deployment environment for a library that does not need one.
- Require external PR approvals from a nonexistent maintainer team.
- Automatically build LLVM from source during every contributor setup or every GitHub Actions job.
- Imply releases or CI validate toolchains not actually tested.

## 2. Information ownership matrix

| Record | Canonical location | GitHub presentation |
|---|---|---|
| Approved contracts | `docs/contracts/`, `docs/DEVELOPMENT_PROTOCOL.md` | Linked from README, Issues and documentation site |
| Architectural decisions | `docs/adr/` | Linked from proposals/PRs |
| Roadmap and scope | `docs/phases/<N>/ROADMAP.md` | Project roadmap + GitHub milestones link to it |
| Current phase handoff | `docs/phases/<N>/PROGRESS.md` | Project progress summaries |
| Work to implement/fix | GitHub Issues | Project views, issue labels |
| Verification | CI runs + committed tests/review evidence | Linked from PRs and acceptance records |
| Release state | Git tags, GitHub Releases, CHANGELOG | README release indicator + published roadmap |
| User-facing docs | `docs/` | GitHub Pages built from Git tree (proposed default) |
| Wiki | No independent contract authority | Optional read-only index or synced derived mirror |

## 3. Project management contract (G2; proposed)

Create a public account-owned GitHub Project, provisionally named **Miracle Development**, linked to `spwn02/Miracle`.

### 3.1 Fields

| Field | Type | Values or meaning |
|---|---|---|
| Status | Single-select | Backlog, Design, Ready, Implementing, Review, Verification, Accepted, Blocked |
| Phase | Single-select/text | G0–G6 for infrastructure; numerical Miracle phases for product work |
| Area | Single-select | Core, Meta, Iter, Diagnostics, Range, Serde, Clap, Infrastructure, Toolchain, Documentation, Other |
| Priority | Single-select | P0 urgent, P1 high, P2 normal, P3 low |
| Risk | Single-select | Low, Medium, High |
| Target release | Text or iteration | Version only when actually planned |
| Target date | Date | Optional; never create fake deadlines |

Do not overload Project status with every acceptance substate; use the linked issue/PR and progress ledger for implementation-complete versus verification-complete detail when needed.

### 3.2 Views

- **All Work** — table, comprehensive.
- **Active** — board, by Status; hide accepted records unless requested.
- **Roadmap** — timeline grouped by phase or target release, dates optional.
- **Verification Queue** — review/verification/blocked items.

### 3.3 Milestones and issues

GitHub Milestones represent finite deliverables or releases. One Project item can track a feature; sub-issues or checklists track reviewable milestones. A phase can span multiple releases; do not equate each development phase with a patch release.

Recommended issue types: Bug, Feature, Architecture/Design, Documentation, Compiler/Toolchain, Infrastructure, Security report guidance. Security vulnerabilities should use private vulnerability reporting, not public issue templates.

Recommended namespaced labels: `type:*`, `area:*`, `priority:*`, `risk:*`, `status:blocked` (only where Projects cannot express this). Avoid redundant labels that merely duplicate Project fields.

**Issue invariant:** Each accepted feature issue references its governing contract and has explicit acceptance criteria. Use Discussions for open-ended ideas and Q&A.

## 4. Community and contribution contract (G1/G6; proposed)

Add or reconcile `CODE_OF_CONDUCT.md`, `SUPPORT.md`, `ACCESSIBILITY.md`, `.github/ISSUE_TEMPLATE/`, `.github/PULL_REQUEST_TEMPLATE.md`, and `CONTRIBUTING.md`.

Issue forms should collect only relevant structured evidence:

- Bug: revision/tag, reproducible steps, expected/actual results, minimized case, affected platform/toolchain, applicable test output.
- Toolchain defect: exact immutable reference snapshot, library version/branch, flags, minimal reproducer, distinction between compiler and library behavior.
- Feature proposal: motivation, public API sketch, alternatives, affected existing contract, backwards-compatibility risks.
- Infrastructure request: objective, affected workflows/settings, risk and validation criteria.

PR template: linked issue/contract, changes, test/compile evidence, compatibility impact, documentation changes, performance/ownership changes where relevant, and checklist of unresolved gaps. Template must distinguish unrun checks from passing checks.

When Discussions is enabled, initial categories should be Q&A, Ideas, Announcements, Show & Tell. Content belongs in Issues only when actionable. Code of Conduct enforcement must name a realistic moderation/reporting contact mechanism without publicly disclosing private information.

## 5. Documentation contract (G3; proposed)

Primary source: repository `docs/`. Primary published experience: GitHub Pages site built from the same tree with an automated read-only publishing workflow. GitHub Wiki is optional, and if used should contain navigation or automatically synchronized derived content rather than manually rewritten locked contracts.

Documentation navigation should distinguish:

1. Getting started and compiler support.
2. API reference and usage guides.
3. Architecture and approved Design Contracts.
4. Phase roadmap / progress.
5. Contribution and governance.
6. Versioned release notes and migration information.

**Required:** no claim of generated authoritative API documentation without validated generation. GitHub Pages and Wiki configuration await their dedicated implementation workstream.

## 6. Branch/ruleset contract (G4; proposed)

`master` and `gcc` have distinct policies:

- Both: prevent accidental deletions and force pushes, subject to tested maintainer recovery procedures.
- `master`: reliable reference-C++26 CI checks are required for PR-based changes after exact check names are verified; do not require external approvals from a solo maintainer by default.
- `gcc`: maintain `sync-gcc.yml` non-forced automation; a ruleset must either permit the exact bot integration or trigger a documented PR-based redesign. Do not require linear commit history while `git merge --no-ff` is the synchronization mechanism.
- Tags: protect published version tags against deletion/rewriting, with a documented maintenance/recovery path.
- Apply new rules progressively; test on noncritical refs first when possible.

**Invariant:** No protection is considered accepted until the existing `master` CI, `gcc` compatibility validator, release workflow and synchronization policy remain functional.

## 7. CI / dependency / security contract (G4; proposed)

Preserve existing reference-C++26 snapshot pinning, coherent libc++ provenance, reference CI, GCC compatibility and release-provenance pipeline.

Audit and progressively improve:

- Least-privilege `GITHUB_TOKEN` permissions and fork-PR workflow safety.
- Reusable workflow permission and dispatch behavior.
- Third-party actions pinning to immutable commit SHAs where practical; Dependabot updates for GitHub Actions.
- Broken `pip`/`vcpkg` Dependabot configurations (confirmed; see G0 audit).
- Available secret scanning, security advisories, dependency alerts and code-scanning settings.
- Negative CI paths: checks must fail for actual defects, not silently skip.
- Reference compiler limitations: do not treat an unproven stock Clang/GCC scan as full coverage of Miracle's C++26 modules/reflection.

No webhook, deploy key, token or Actions secret is added solely because GitHub supports that feature.

## 8. Reproducible environments (G5; proposed)

Provide a Docker-based developer image / devcontainer with a pinned, checksummed `clang-cxx26` reference snapshot and matching libc++, CMake >= 4.4, Ninja and necessary utilities.

The environment should support CLI/Neovim/clangd usage, local Docker and Codespaces, and the normal CMake presets. Codespaces prebuilds are optional and disabled initially unless user demand and budget justify them.

Use one provenance policy across local container, CI and Codespaces; never silently substitute distribution Clang for the reference compiler. Avoid embedding secrets or personal machine paths in container definitions.

### G5 maintainer-scoped amendment (2026-10-19)

The owner expressly chose **regular Docker only**, declining Codespaces because of potential charges after reporting that the prior **local interactive devcontainer worked** and approving G5 for the revised Docker-only workflow. For G5 operations, a single root Dockerfile, non-root Docker CLI/Neovim/clangd workflow, unchanged pinned reference toolchain, and reproducible build/test/consumer validation replace the previously **proposed** devcontainer/Codespaces implementation detail. Existing Dev Containers integration is retired; Codespaces is **declined/not tested**, never counted as a completed test or acceptance result.

This scoped amendment does not relax reference compiler/libc++ coherence, no-secret rules, check provenance, or §10's demand for actual build/test evidence. G5 remains open pending verification of the replacement Docker-only implementation; later Codespaces support would require a new explicit budget/scope decision.

## 9. Release and operational reporting

GitHub Releases correspond to actually tagged/validated versions. A release announcement may link to the relevant Project milestone, changelog entry and CI provenance. A phase's completion does not automatically mandate a new release; release approval remains with the author.

No automatically generated roadmap should imply that planned dates are firm promises.

## 10. Workstream gates

| Workstream | Deliverable | Exit evidence |
|---|---|---|
| G0 — Audit & Authority | Audited baseline, repository protocol, initial program contract, handoff ledger | Author confirms local-vs-remote reconciliation and documents are committed |
| G1 — Governance | Community docs, issue/PR templates, contribution rules | GitHub community files recognized; sample templates validated |
| G2 — Planning | Public Project, taxonomy, milestones and views | Test issue flows into expected Project views |
| G3 — Documentation | Repository-based docs site and Wiki policy | Sample contract edit appears in published docs without manual duplication |
| G4 — Security & CI | Rulesets and checks, repaired Dependabot, security settings | Normal CI and `gcc` sync verified; no unexpected admin bypasses |
| G5 — Dev environments | Docker/devcontainer/Codespaces documentation | Clean reproducible setup passes reference build/test criteria |
| G6 — Community and acceptance | Discussions and full end-to-end workflow | Test flow from issue → implementation → verification → acceptance → release documentation passes |

No workstream is accepted based on configuration file presence alone when actual behavior can be tested.

## 11. Deferred configuration decisions

This framework locks the stated architectural invariants, responsibility boundaries, and workstream acceptance gates. The precise configurations listed here remain **DEFERRED** until they are reviewed and locked in their respective workstreams; no unapproved detail is silently made binding.

Pending explicit discussion/adoption:

- GitHub Pages as primary public documentation, with Wiki index/mirror rather than independent editing.
- Project name, exact field/status taxonomy, issue-label convention.
- Precise `master` and `gcc` branch ruleset requirements, check names and automation bypass method.
- Community conduct policy and practical reporting channel.
- Container image distribution and Codespaces spending limits.

## 12. Adoption and current state

**LOCKED FRAMEWORK upon author acceptance and repository integration.** The mission, information ownership, architectural invariants and G0–G6 acceptance gates are binding. Where a section explicitly describes a value as *proposed*, *recommended*, *provisional* or *optional*, the detailed choice remains subject to its workstream-specific design review under section 11.

The G0 audit remains an immutable point-in-time baseline. This contract does not itself create a GitHub Project, protect a branch, activate discussions, deploy documentation, or grant any integration permissions. No workstream is accepted before its exit evidence is verified.
