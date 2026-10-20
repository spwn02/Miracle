# Miracle GitHub Infrastructure — G1 Progress

**Updated:** 2026-10-20
**Workstream:** G1 — Governance & Contribution Experience
**Status:** G1.1–G1.4 implemented; owner screenshots verify chooser, Bug Report inline validation, and GitHub recognition of eight visible Community Standards. Actual submission blocking, PR template prefill, policy-link behavior, confidential contact and final acceptance pending
**Authority:** [Infrastructure Contract v1.0](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Design baseline:** [Development Protocol v1.0](../DEVELOPMENT_PROTOCOL.md)
**Integration policy for this infrastructure work:** Author explicitly authorized AI-authored DevOps files and direct pushes to `master`; this exception does not amend product-development protocol requirements.

## Milestones and evidence

| Milestone | Published artifacts | State | Outstanding acceptance evidence |
| --- | --- | --- | --- |
| G1.1 — Issue intake | `.github/ISSUE_TEMPLATE/{bug_report,compiler_issue,feature_proposal,architecture_proposal,infrastructure_issue,config}.yml`; [locked contract](G1.1-ISSUE-INTAKE-CONTRACT.md) | **Live chooser verified by maintainer screenshot**; source structure reviewed | Verify required/optional field behavior and security policy link interaction inside the live forms |
| G1.2 — Pull request intake | [PR template](https://github.com/spwn02/Miracle/blob/master/.github/PULL_REQUEST_TEMPLATE.md) | Implementation complete | Verify template pre-population in GitHub's live PR interface without falsely asserting a test PR exists |
| G1.3 — Community governance | [Code of Conduct](https://github.com/spwn02/Miracle/blob/master/CODE_OF_CONDUCT.md), [Support](https://github.com/spwn02/Miracle/blob/master/SUPPORT.md), [Accessibility](https://github.com/spwn02/Miracle/blob/master/ACCESSIBILITY.md) | **Community Standards recognition visually verified** (Code of conduct, security policy, templates etc.) | Follow policy links and assess confidential moderation contact; Accessibility is not separately listed in the screenshot |
| G1.4 — Contributor documentation | [Contributing](https://github.com/spwn02/Miracle/blob/master/CONTRIBUTING.md), [reference toolchain](../reference-toolchain.md), [README](https://github.com/spwn02/Miracle/blob/master/README.md) | Documentation implemented; source links and CI identity reviewed | Final review of compiler documentation against CI, Markdown links, and user-facing onboarding |
| G1.5 — Acceptance | Evidence-based review of the complete contribution workflow | Pending | Perform live GitHub UI verification; track issues discovered and author acceptance |

## G1.5 live Community Standards recognition — screenshot evidence (2026-10-20)

The maintainer supplied an actual screenshot from the repository's GitHub **Insights → Community → Community standards** view. Each of the **eight visible checklist entries displays GitHub's check mark**: **Description**, **README**, **Code of conduct**, **Contributing**, **License**, **Security policy**, **Issue templates**, and **Pull request template**. This establishes GitHub's **live recognition** of these community files/settings, not merely their presence in repository source.

**Verified:** GitHub's actual Community Standards dashboard recognizes all eight displayed entries, including policies, issue templates and the PR template. The screenshot ends at the Pull request template row; no claims are made about any content below the visible capture. **Not established:** whether the linked pages were clicked and inspected, whether the PR template prepopulates a newly opened PR, whether private vulnerability/conduct reporting reaches an appropriate maintainer, or explicit overall G1 sign-off. No artificial PR or confidential report was created.

## G1.5 live Bug Report form validation — screenshot evidence (2026-10-20)

The maintainer opened the genuine [Bug Report issue form](https://github.com/spwn02/Miracle/issues/new?template=bug_report.yml) without creating a new issue. The live screenshot shows the form title, security/credential-redaction guidance, **Affected revision** (marked required), **Affected area** (unstarred dropdown showing `None`), and **Observed behavior** (marked required, with a Markdown editor). The **Affected revision** input is empty and GitHub renders the red inline validation message **`Field can not be empty`**. No dummy issue was submitted.

**Verified:** Live Bug Report form rendering, presence of required/optional field indicators, and inline empty-required-field validation on `Affected revision`. The warning appears in the browser interface rather than being inferred from committed YAML.

**Not yet verified:** Whether the **Submit new issue** action is actually blocked while mandatory fields remain empty, whether each other issue form enforces its required fields, security/contributing URL click-through, PR template prepopulation or private-report delivery. A draft's sidebar showing no Project association is not evidence against later Project auto-add (which occurs after issue creation).

## G1.5 real New Issue chooser screenshot — verified (2026-10-20)

The maintainer supplied a screenshot from the **live GitHub Create new issue chooser**. It shows all **five contract-required structured forms**: Architecture Proposal, Bug Report, Compiler / Toolchain Issue, Feature Proposal and Infrastructure Request. It also displays the contribution guide, Q&A and Ideas community entry points, the repository-specific security policy link, and GitHub's own **Report a security vulnerability** entry. The `Blank issue` control is visibly **Maintainers only**; its presence for the owner does not establish that the public can bypass the configured `blank_issues_enabled: false` restriction.

**UI finding (nonblocking):** GitHub's native private vulnerability-report entry and the separately configured `SECURITY.md` policy link have identical **Report a security vulnerability** display names. These are two **different routes**, not two vulnerability issue forms. The [locked G1.1 intake contract](G1.1-ISSUE-INTAKE-CONTRACT.md) explicitly specifies the custom policy link label, so this audit **does not rename it without a contract decision**.

**Verified at the initial chooser stage:** all five custom forms and contact routes render. Subsequent screenshots confirm that **Bug Report** opens with inline required-field validation and that **GitHub Community Standards recognizes eight visible entries**. **Still open:** other forms, actual submission gating, security/contribution link destinations, private-report delivery, PR template prepopulation and final G1 acceptance. No dummy issue, security report or PR was created.

## Compiler/toolchain consistency

Miracle reference CI [`.github/workflows/ci.yml`](https://github.com/spwn02/Miracle/blob/master/.github/workflows/ci.yml) pins:

- Snapshot: `cxx26-2026.10.09.2`
- Source revision: `040df2b05ea338d107fac7387a2e4db835d746e0`
- Asset identity: `clang-cxx26-2026.10.09.2-linux-x86_64`

G1.4 aligns contributor and reference-toolchain documentation to this verified **CI-pinned** identity. The mutable `cxx26` development branch and release-specific provenance remain separate concepts. Older release metadata is not rewritten by documentation updates.

## G1.5 source-level verification — 2026-10-17

The published `master` copies of all six Issue Template YAML files were inspected directly:

- All five issue form names, the exact ordered field IDs, and required-field counts match the approved G1.1 contract.
- No duplicate field IDs and no unapproved label, assignee, Issue Type, or Project properties were found.
- Each form contains the security-reporting guidance; `config.yml` disables blank issues and links to the existing security and contribution policies without assuming Discussions exists.
- The PR template contains contract/ADR references and a verification-evidence matrix.
- The contributor guide, reference toolchain documentation, and README no longer represent the obsolete `clang-p2996` fork or `cxx26-2026.10.08` as the current pinned toolchain.

These checks establish **source-level contract conformance**, not GitHub's live form renderer behavior or semantic validation of user input. The connected interface cannot access the live New Issue chooser or GitHub Community Standards page; interactive acceptance remains open.

## Outstanding operational work

- The maintainer screenshot now proves **inline empty-required-field validation** in the live Bug Report form; source checks alone could not. Actual submission blocking, other forms' behavior, and PR template pre-population remain unverified.
- Security/Code of Conduct reporting uses existing GitHub routes; a verified confidential maintainer contact is not yet established.
- `G0-PROGRESS.md` records pending **author-side local repository reconciliation**, which cannot be inferred from successful remote pushes.
- G2 will introduce formal labels, issue types/project automation only after their configuration has been reviewed.

## G1 acceptance checklist

- [x] Five structured issue forms and chooser configuration committed.
- [x] Pull request review template committed.
- [x] Conduct, support, accessibility policies committed.
- [x] Contributor guide uses the `clang-cxx26` fork and explicitly pinned CI snapshot.
- [x] Reference toolchain document reconciled with CI source and snapshot.
- [x] README includes project-governance/community entry points.
- [x] Verify live GitHub New Issue chooser displays exactly five custom forms and expected contact routes — owner screenshot (2026-10-20).
- [x] Open the live Bug Report form and observe **inline required-field validation**: empty `Affected revision` shows red `Field can not be empty` (owner screenshot, 2026-10-20).
- [ ] Verify actual **submission blocking** with incomplete required fields, remaining issue forms and security/contributing link destinations without publishing a dummy issue or report.
- [ ] Verify actual PR template pre-population when creating a PR.
- [x] Verify **GitHub Community Standards recognition** of all eight visible entries (description, README, Code of Conduct, contributing, license, security policy, issue templates, PR template) — screenshot 2026-10-20.
- [ ] Verify the policy/documentation **link destinations** in the browser and the separate confidential reporting path.
- [ ] Record review outcomes and explicit author acceptance.

**Next action:** G1.5 — independent user-facing verification and closeout; no fictional issue or PR is required to claim completion.
