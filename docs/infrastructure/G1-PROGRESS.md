# Miracle GitHub Infrastructure — G1 Progress

**Updated:** 2026-10-20
**Workstream:** G1 — Governance & Contribution Experience
**Status:** G1.1–G1.4 implemented; live chooser/forms, community standards and policy navigation partially verified. Private vulnerability reporting ENABLED per maintainer confirmation; external reporter flow, confidential conduct reporting, submission blocking, PR prefill and acceptance pending
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

## G1.5 private vulnerability reporting — setting confirmed, detailed review deferred (2026-10-20)

The maintainer **explicitly confirmed that Private vulnerability reporting is enabled** and supplied a screenshot of the repository's **Security and quality → Security Advisories** page. The authenticated owner view displays **New draft security advisory** and advisory-state tabs. This is **owner-confirmed setting evidence** plus a live screenshot of the **maintainer-facing advisories interface**; the GitHub App connection did not independently read the private administrative toggle.

The maintainer also stated that vulnerabilities have already been documented and directed that their detailed review take place **at the finishing stage**. The screenshot's visible published-advisory view is empty; this does **not** establish whether the maintainer's documented vulnerabilities exist elsewhere, whether any confidential reports have been received or their status, and must not be interpreted as such. No vulnerability details were requested, copied, posted or changed.

**Correction to the earlier hypothesis below:** The private-reporting feature is **not assumed disabled**; the owner says it is **enabled**. The two identically named entries that were reported to navigate to `SECURITY.md` remain a **separate chooser UX observation**. The owner-facing **New draft security advisory** action is not the same as an external reporter's form. **Not verified:** an unaffiliated contributor's ability to open the private reporting form, successful confidential delivery, and the Code of Conduct confidential contact path. Do not mark these end-to-end reporting gates complete or create a synthetic advisory.

**Deferred security finishing-stage task:** revisit documented vulnerabilities with the maintainer, assess private reporting visibility and disclosure/triage procedure without posting sensitive details in public Issues. Maintain the locked G1.1 chooser label unless the maintainer approves an amendment.

## G1.5 security chooser duplicate and missing private form — user browser observation (2026-10-20)

The maintainer reports **both** similarly named **Report a security vulnerability** entries in the live GitHub New Issue chooser open `SECURITY.md`, **not** a private reporting form. The supplied screenshot depicts the two entries (first GitHub native shield icon; second our externally linked policy entry), but **does not show the destination pages**; their destination behavior is a **maintainer-reported observation**.

The custom chooser entry is deterministically configured in [`.github/ISSUE_TEMPLATE/config.yml`](../../.github/ISSUE_TEMPLATE/config.yml) with `name: Report a security vulnerability` and `url: https://github.com/spwn02/Miracle/blob/master/SECURITY.md`. That explains **one** of the redirects and redundant naming. The first entry is GitHub's native security-policy / reporting route. According to [GitHub's private vulnerability reporting setup guide](https://docs.github.com/en/code-security/how-tos/report-and-fix-vulnerabilities/configure-vulnerability-reporting/configure-for-a-repository), the private reporting form requires a **separately enabled repository setting**; when enabled, a `Report a vulnerability` button is available on the Security Advisories page. **The actual repo private-vulnerability-reporting setting was not read**: the connected integration cannot access that restricted admin endpoint. Therefore its disabled state is a **hypothesis**, not a verified fact, and no report-delivery gate has passed.

**Required owner check:** Open **Settings → Security and quality → Advanced Security → Private vulnerability reporting**. Enable if disabled; inspect the [Security Advisories](https://github.com/spwn02/Miracle/security/advisories) page for the reporting entry point, without creating/submitting a report. A separate **UX cleanup proposal** would rename the redundant custom chooser entry to **Read security reporting policy**, but its current label is explicitly prescribed by the [locked G1.1 Issue Intake Contract](G1.1-ISSUE-INTAKE-CONTRACT.md). **Do not change the contract or config without explicit owner approval.**

**Disposition:** **Public security policy route verified; confidential private-report form remains UNVERIFIED.** This is an uncovered acceptance gap, not proof GitHub's native control is broken. G1.5 and G6 remain open.

## G1.5 live security and contributing documentation navigation — screenshot evidence (2026-10-20)

The maintainer provided **two real GitHub browser screenshots**:

- [Security policy](https://github.com/spwn02/Miracle/security/policy): GitHub's **Security and quality → Security policy** view renders `SECURITY.md`, including the **Reporting a vulnerability** section. It instructs reporters not to publish details in normal GitHub issues, to use GitHub private vulnerability reporting **when available**, or to request a private channel from the maintainer through their GitHub profile. The screenshot demonstrates **policy navigation and rendering**, **not** that the private vulnerability-reporting entry is enabled or that a confidential report can be delivered.
- [Contributing guide](https://github.com/spwn02/Miracle/blob/master/CONTRIBUTING.md): GitHub renders the `CONTRIBUTING.md` Markdown preview, showing **Before contributing**, the Development Protocol link, existing Issues, Discussions, New Issue chooser, Q&A, Ideas, `SUPPORT.md`, `CODE_OF_CONDUCT.md`, `SECURITY.md` and the **Reference toolchain** heading. It proves the **guide itself opens and renders**; individual embedded links have **not** each been clicked.

**Verified:** both requested **GitHub page destinations** load correctly in the maintainer's browser, consistent with the Issue chooser's security/contributor routes. **Pending:** actual private vulnerability intake and confidential conduct-report delivery, individual embedded links as needed, PR template prepopulation, invalid-form submit blocking and formal author sign-off. No security report, issue or PR was submitted.

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
- Security and Contributing pages are now confirmed to open in GitHub; private vulnerability-reporting availability and confidential Code of Conduct contact/delivery have **not** been verified.
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
- [x] Verify GitHub browser navigation and rendering for the **Security policy** and **Contributing guide** destinations — maintainer screenshots (2026-10-20).
- [x] Confirm **Private vulnerability reporting is enabled** — explicit maintainer statement (2026-10-20); owner's Security Advisories page screenshot shows the **New draft security advisory** maintainer action.
- [ ] Verify the **external contributor's private reporting form** and confidential **conduct-reporting path** without submitting dummy or sensitive reports; actual delivery and review of existing documented vulnerabilities are deferred to final security hardening.
- [ ] Record review outcomes and explicit author acceptance.

**Next action:** G1.5 — independent user-facing verification and closeout; no fictional issue or PR is required to claim completion.
