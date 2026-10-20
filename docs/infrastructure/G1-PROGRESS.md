# Miracle GitHub Infrastructure — G1 Progress

**Updated:** 2026-10-20
**Workstream:** G1 — Governance & Contribution Experience
**Status:** **G1 ACCEPTED** — G1.1–G1.4 delivered; G1.5 five-part browser acceptance confirmed by maintainer and explicitly signed off on 2026-10-20. Browser-only checks are maintainer-attested, not independently reproduced by the GitHub integration. Actual confidential report delivery and end-to-end G6 tests were not asserted.
**Authority:** [Infrastructure Contract v1.0](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Design baseline:** [Development Protocol v1.0](../DEVELOPMENT_PROTOCOL.md)
**Integration policy for this infrastructure work:** Author explicitly authorized AI-authored DevOps files and direct pushes to `master`; this exception does not amend product-development protocol requirements.

## Milestones and evidence

| Milestone | Published artifacts | State | Outstanding acceptance evidence |
| --- | --- | --- | --- |
| G1.1 — Issue intake | `.github/ISSUE_TEMPLATE/{bug_report,compiler_issue,feature_proposal,architecture_proposal,infrastructure_issue,config}.yml`; [locked contract](G1.1-ISSUE-INTAKE-CONTRACT.md) | **Accepted:** five-form chooser and source structure checked; submit-blocking of incomplete required inputs attested by maintainer | No G1 blockers; optional field semantics remain governed by GitHub form schema |
| G1.2 — Pull request intake | [PR template](https://github.com/spwn02/Miracle/blob/master/.github/PULL_REQUEST_TEMPLATE.md) | **Accepted:** actual PR composer prefill maintainer-attested | No synthetic PR created or claimed |
| G1.3 — Community governance | [Code of Conduct](https://github.com/spwn02/Miracle/blob/master/CODE_OF_CONDUCT.md), [Support](https://github.com/spwn02/Miracle/blob/master/SUPPORT.md), [Accessibility](https://github.com/spwn02/Miracle/blob/master/ACCESSIBILITY.md) | **Accepted:** eight Community Standards rows verified; private conduct contact published and maintainer-confirmed receiving | No independent confidential-report delivery or appeals body claimed |
| G1.4 — Contributor documentation | [Contributing](https://github.com/spwn02/Miracle/blob/master/CONTRIBUTING.md), [reference toolchain](../reference-toolchain.md), [README](https://github.com/spwn02/Miracle/blob/master/README.md) | **Accepted:** published source and visible onboarding routes reviewed; compiler identity reconciled to CI | Does not claim every nested external link was independently clicked |
| G1.5 — Acceptance | Evidence-based review of the complete contribution workflow | **ACCEPTED by maintainer 2026-10-20** | Evidence scope and boundaries documented below |

## G1.5 final author acceptance — 2026-10-20 (supersedes earlier pending entries)

The repository maintainer explicitly submitted the following five checked confirmations in the G1.5 closeout conversation, following instructions to test the live GitHub user interface:

1. **[x] Issue form validation** — accepted as maintainer confirmation of required-field submit-blocking across the five configured forms. The earlier Bug Report screenshot independently documented inline validation on one required field; new five-form browser results were **not accompanied by screenshots or automation output**. No dummy issues are claimed.
2. **[x] Pull request template** — maintainer confirms automatic prepopulation in the real PR composer. No test PR or captured composer screenshot is claimed.
3. **[x] Private vulnerability reporting** — maintainer confirms access to the external/non-admin reporter form without submitting a confidential report. Owner-confirmed setting enablement was recorded separately; actual confidential report delivery/triage was **not tested**.
4. **[x] Confidential conduct contact** — the published [Code of Conduct](https://github.com/spwn02/Miracle/blob/master/CODE_OF_CONDUCT.md) names `spwn02.dev@gmail.com` after [approved commit `3aa90f1`](https://github.com/spwn02/Miracle/commit/3aa90f10cb4f127b8c0a33cb46ea55cf47bbb33b); maintainer confirms inbox receives messages. No synthetic conduct allegation was sent.
5. **[x] Final G1 acceptance** — the maintainer explicitly signs off on G1.5 and thus completes the G1 contribution/governance workstream.

**Disposition: G1 / G1.5 ACCEPTED.** This is **maintainer-attested browser acceptance** with earlier independent GitHub file audits and selected owner screenshots, not machine-observed tests of all five form submissions or the PR composer. Earlier sections below retain the chronological *then-pending* findings; the final sign-off above supersedes those historical statuses without rewriting evidence. No invented issues, PRs, confidential reports or vulnerability adjudication. Detailed security-hardening, Project #3 automatic intake, live Q&A accepted-answer behavior, release/end-to-end validation and final G6 acceptance remain separate workstreams. [G1.5 issue #1](https://github.com/spwn02/Miracle/issues/1) is the tracking record.

## G1.5 conduct contact published and reporter entry confirmed by maintainer — 2026-10-20

The maintainer approved using **spwn02.dev@gmail.com** as the private Code of Conduct reporting address and confirmed that the mailbox receives messages. The approved amendment was published to `master` as [commit `3aa90f1`](https://github.com/spwn02/Miracle/commit/3aa90f10cb4f127b8c0a33cb46ea55cf47bbb33b), updating only `CODE_OF_CONDUCT.md` in that commit. It names the contact, provides subject-line and minimal-evidence guidance, distinguishes conduct from security reports, identifies alternative GitHub reporting routes, and avoids promises of anonymity, absolute confidentiality or a response-time guarantee. The maintainer's reachability statement is user confirmation, **not an independently observed outside-in delivery test**. `SECURITY.md` and the G1.1 issue form contract were not modified.

The maintainer additionally stated that they completed **G1.5 Step 3**, previously described as opening the **non-admin/external reporter-facing private vulnerability reporting form without submission**. Record this as **maintainer-reported live browser verification**, not as independently inspected screenshots, simulated report delivery or a security finding review. No confidential report was submitted and detailed vulnerability review remains deferred.

**Historical checkpoint (before final sign-off):** the live submit-blocking, PR prefill and author-approval gates were pending at this earlier checkpoint. The later maintainer-attested five-item acceptance record above supersedes their pending status. No synthetic issues or PRs were created to fill the gap.

## G1.5 independent read-only recheck — 2026-10-20

A fresh read of the published `master` Issue Template YAML files rechecked all five contract-defined forms: **37 editable field IDs, 20 required declarations** (Bug Report 8/5, Compiler / Toolchain 9/7, Feature Proposal 7/4, Architecture Proposal 7/5, Infrastructure Request 6/3). All IDs remain unique within their forms; no unapproved top-level labels, assignees, Projects or Issue Types appear. This is a **source-structure check**, not YAML-schema execution, a GitHub browser test, or proof of actual submit blocking.

A fresh read of the repository PR template confirms the expected traceability, test-results and risk sections; actual recent PR bodies, including [PR #17](https://github.com/spwn02/Miracle/pull/17), contain author-written content, so they **cannot establish browser-side automatic template prefill**. GitHub's Community Standards recognition screenshot from earlier on 2026-10-20 remains valid but is likewise not a PR composition test.

GitHub's [current private-reporting documentation](https://docs.github.com/en/code-security/how-tos/report-and-fix-vulnerabilities/report-privately) distinguishes **owner-created draft advisories** from a **non-admin security researcher's Report a vulnerability form**. GitHub's [2026-10-01 changelog](https://github.blog/changelog/2026-10-01-structured-forms-for-private-vulnerability-reports/) also describes the new default structured reporter form, so the precise UI should be checked as an external contributor, not inferred from owner screenshots. Owner-confirmed enablement remains recorded; no private report, conduct allegation, or advisory was submitted or inspected.

**Gates not yet evidenced (do not mark complete):**

1. Read-only live validation of **submit blocking** for missing form-required inputs on each of the five form types, without publishing a synthetic issue.
2. Actual **PR body prepopulation** in the genuine GitHub PR authoring UI, ideally during the next real PR; a committed template and Community Standards checkmark are not enough.
3. **External/non-admin reporter entry** to the private vulnerability form without submitting a synthetic report; no inference from the owner's `New draft security advisory` action.
4. A **usable private contact channel for conduct concerns**. The current `CODE_OF_CONDUCT.md` explicitly says that Miracle does not advertise a verified confidential project-specific reporting inbox; the GitHub-profile request path is documented fallback guidance, not proof of private contact/delivery.
5. **Explicit maintainer G1.5 acceptance** after actual evidence is reviewed. A request to work on closeout is not evidence of passing unrun checks.

**Disposition:** G1.5 **remains open**. The accessible GitHub connection exposes repository files, PRs and Issues, but not the live browser composer or a non-admin reporter session. Do not fabricate UI results, tests, confidential reports, PRs or acceptance to close this gate. This recheck introduces no change to the locked G1.1 design, GitHub settings, issue forms, reporting routes or product source. Detailed review of the maintainer's documented security findings stays deferred to the finishing security-hardening stage.

## G1.5 private vulnerability reporting — setting confirmed, detailed review deferred (2026-10-20)

The maintainer **explicitly confirmed that Private vulnerability reporting is enabled** and supplied a screenshot of the repository's **Security and quality → Security Advisories** page. The authenticated owner view displays **New draft security advisory** and advisory-state tabs. This is **owner-confirmed setting evidence** plus a live screenshot of the **maintainer-facing advisories interface**; the GitHub App connection did not independently read the private administrative toggle.

The maintainer also stated that vulnerabilities have already been documented and directed that their detailed review take place **at the finishing stage**. The screenshot's visible published-advisory view is empty; this does **not** establish whether the maintainer's documented vulnerabilities exist elsewhere, whether any confidential reports have been received or their status, and must not be interpreted as such. No vulnerability details were requested, copied, posted or changed.

**Correction to the earlier hypothesis below:** The private-reporting feature is **not assumed disabled**; the owner says it is **enabled**. The two identically named entries that were reported to navigate to `SECURITY.md` remain a **separate chooser UX observation**. The owner-facing **New draft security advisory** action is not the same as an external reporter's form. **Not verified:** an unaffiliated contributor's ability to open the private reporting form, successful confidential delivery, and the Code of Conduct confidential contact path. Do not mark these end-to-end reporting gates complete or create a synthetic advisory.

**Deferred security finishing-stage task:** revisit documented vulnerabilities with the maintainer, assess private reporting visibility and disclosure/triage procedure without posting sensitive details in public Issues. Maintain the locked G1.1 chooser label unless the maintainer approves an amendment.

## G1.5 security chooser duplicate and missing private form — user browser observation (2026-10-20)

The maintainer reports **both** similarly named **Report a security vulnerability** entries in the live GitHub New Issue chooser open `SECURITY.md`, **not** a private reporting form. The supplied screenshot depicts the two entries (first GitHub native shield icon; second our externally linked policy entry), but **does not show the destination pages**; their destination behavior is a **maintainer-reported observation**.

The custom chooser entry is deterministically configured in [`.github/ISSUE_TEMPLATE/config.yml`](https://github.com/spwn02/Miracle/blob/master/.github/ISSUE_TEMPLATE/config.yml) with `name: Report a security vulnerability` and `url: https://github.com/spwn02/Miracle/blob/master/SECURITY.md`. That explains **one** of the redirects and redundant naming. The first entry is GitHub's native security-policy / reporting route. According to [GitHub's private vulnerability reporting setup guide](https://docs.github.com/en/code-security/how-tos/report-and-fix-vulnerabilities/configure-vulnerability-reporting/configure-for-a-repository), the private reporting form requires a **separately enabled repository setting**; when enabled, a `Report a vulnerability` button is available on the Security Advisories page. **The actual repo private-vulnerability-reporting setting was not read**: the connected integration cannot access that restricted admin endpoint. Therefore its disabled state is a **hypothesis**, not a verified fact, and no report-delivery gate has passed.

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

- G1.5 final acceptance (above) records the maintainer's positive live submit-blocking and PR-prefill checks. The earlier screenshot independently establishes only Bug Report inline validation; the other browser checks remain maintainer-attested.
- Security and Contributing pages are confirmed to open in GitHub. The maintainer reports verifying the non-admin vulnerability form entry point and receiving mail at the newly published private conduct contact; confidential report delivery itself was not tested.
- G0's local fast-forward reconciliation was previously accepted in [G0-PROGRESS.md](G0-PROGRESS.md); later GitHub commits naturally require subsequent local synchronization.
- G2 Project #3 automated intake and final G6 workflow validation remain separate acceptance gates; G1 sign-off does not automatically accept either.

## G1 acceptance checklist

- [x] Five structured issue forms and chooser configuration committed.
- [x] Pull request review template committed.
- [x] Conduct, support, accessibility policies committed.
- [x] Contributor guide uses the `clang-cxx26` fork and explicitly pinned CI snapshot.
- [x] Reference toolchain document reconciled with CI source and snapshot.
- [x] README includes project-governance/community entry points.
- [x] Verify live GitHub New Issue chooser displays exactly five custom forms and expected contact routes — owner screenshot (2026-10-20).
- [x] Open the live Bug Report form and observe **inline required-field validation**: empty `Affected revision` shows red `Field can not be empty` (owner screenshot, 2026-10-20).
- [x] **Required-field submission blocking for all five forms** — maintainer-reported real GitHub UI verification (2026-10-20); no new screenshots supplied, no dummy issue or vulnerability report created. The Security policy and Contributing destinations were independently evidenced by prior owner screenshots.
- [x] **Automatic PR template prepopulation** — maintainer-reported real PR composer verification (2026-10-20); no synthetic PR or screenshot claimed.
- [x] Verify **GitHub Community Standards recognition** of all eight visible entries (description, README, Code of Conduct, contributing, license, security policy, issue templates, PR template) — screenshot 2026-10-20.
- [x] Verify GitHub browser navigation and rendering for the **Security policy** and **Contributing guide** destinations — maintainer screenshots (2026-10-20).
- [x] Confirm **Private vulnerability reporting is enabled** — explicit maintainer statement (2026-10-20); owner's Security Advisories page screenshot shows the **New draft security advisory** maintainer action.
- [x] **External reporter entry point:** maintainer reports checking the non-admin private vulnerability form (2026-10-20); no screenshot independently assessed or synthetic report sent; actual delivery and vulnerability review deferred.
- [x] **Conduct contact:** [Code of Conduct](https://github.com/spwn02/Miracle/blob/master/CODE_OF_CONDUCT.md) publishes `spwn02.dev@gmail.com` (commit `3aa90f1`), and maintainer confirmed mailbox receives messages; no invented confidential report.
- [x] **Explicit final G1/G1.5 author acceptance** (2026-10-20), recorded at top of this ledger; signed off on the five checked items.

**Next action:** G1 complete. Continue outstanding G2/G4/G6 milestones independently; do not equate G1 sign-off with end-to-end acceptance or confidential report delivery.
