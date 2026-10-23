# G6 — Community and End-to-End Acceptance: Progress

**Updated:** 2026-10-23
**Status:** IN PROGRESS — a genuine security Issue #24 → reviewed PR #27 → passing post-merge CI/Pages → explicit scoped CodeQL risk-disposition trace is verified; G2 Project #3 event-metadata ambiguity, live Q&A accepted-answer behavior, confidential reporting and overall G6 author acceptance remain outstanding.
**Authority:** [Infrastructure Contract v1.0](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Operating plan:** [G6-OPERATING-PLAN.md](G6-OPERATING-PLAN.md)
**Tracking issue:** [#6 — community and acceptance](https://github.com/spwn02/Miracle/issues/6)

## Historical v0.5.27 release attachment — downloaded and verified (2026-10-23)

[PR #56](https://github.com/spwn02/Miracle/pull/56) introduced a **read-only, fixed-target historical provenance verifier** with negative fixtures, passed exact-head Reference C++26 CI and CodeQL, and merged into protected `master` at [`a372ff1`](https://github.com/spwn02/Miracle/commit/a372ff17a4f8035c059c8ff760eb81758390a21c). The owner-authenticated [Actions run #39902726212](https://github.com/spwn02/Miracle/actions/runs/39902726212) then actually **downloaded** the published `v0.5.27` release's `release-metadata.json` using `gh release download`. It verified the unique uploaded asset ID `645665032`, 608-byte length, byte-level SHA-256 `27a7a78fea1df167a79e6e7386bc62efe4faf257212d5fa09f8453248cba9635`, tag `v0.5.27` resolving to source commit `e1e5817c651d124d02930b86510e2b05fb7c7d2d`, and the JSON's exact `project`, `source`, `referenceToolchain`, and non-release-bearing `compilerSupport.gcc` fields against historical tagged-source expectations. The associated self-tests fail closed on mismatched identity, mutated bytes and semantic JSON.

**This supersedes only the earlier historical statement that the attachment bytes/JSON fields had not been inspected.** Neither this audit nor G6 infrastructure PRs published a new version or tested a new `v*` tag under G4's current ruleset. G2 Project event metadata, genuine Q&A accepted-answer behavior, confidential report delivery, full product release lifecycle, and final G6 owner authorization remain independently open. G6 Issue #6 remains OPEN.

## Live GitHub Discussions and public intake configuration — 2026-10-23

[PR #53](https://github.com/spwn02/Miracle/pull/53) merged at `d23b28b2b739054ec1562c74f747fe7fa76b93b6` after green Reference C++26 CI and documentation checks. The reviewed, **read-only** `/miracle-project g6-community` operation ran successfully in the owner-authenticated [GitHub Actions #39898369419](https://github.com/spwn02/Miracle/actions/runs/39898369419). The GitHub GraphQL/REST evidence confirms **real repository Discussions enabled**, repository default branch `master`, and these actual native category properties:

| Live category | Slug | `isAnswerable` |
| --- | --- | --- |
| Q&A | `q-a` | `true` |
| Ideas | `ideas` | `false` |
| Announcements | `announcements` | `false` |
| Show and tell | `show-and-tell` | `false` |

The same run checked source-controlled `SUPPORT.md`, `CONTRIBUTING.md`, the GitHub Issue chooser, `SECURITY.md`, and `CODE_OF_CONDUCT.md`. Every required live category slug had the corresponding advertised navigation link. Private-report and confidential-conduct **instructions are present**, without pretending that real private messages were delivered or handled.

**Scope:** This is an authenticated, live configuration proof beyond source-only strings. It does **not** exercise a real Discussion participant's question, posted answer, GitHub `mark answer` operation, confidential vulnerability report, confidential misconduct report, or final maintainer review. Do not create false content for those tests. [G6 Issue #6](https://github.com/spwn02/Miracle/issues/6) stays open, and the broad confidential-route acceptance checkbox stays open pending genuine delivery/access validation. The [G2 Project #3 telemetry discrepancy](G2-PROGRESS.md) also remains separate and unresolved.

The complementary [source parity audit #39892796281](https://github.com/spwn02/Miracle/actions/runs/39892796281) established the 172 historical Project↔Git records, while [published docs audit #39403645833](https://github.com/spwn02/Miracle/actions/runs/39403645833) previously confirmed 29 canonical chapters. These support checking the G6 Project/documentation source-parity criterion, not a full engineering release/contract lifecycle acceptance.

## Genuine CodeQL engineering trace — supplementary G6 evidence (2026-10-21)

**Real work, not a test artifact:** [Issue #24 — C++26 CodeQL coverage limitation](https://github.com/spwn02/Miracle/issues/24) originated from an actual security-analysis gap, with owner-supplied analyzed-files CSV proving **86/86 conventional `.cxx` files extracted and 0/33 `.ixx` module interfaces extracted**. The [G4 CodeQL coverage inventory](G4-CODEQL-COVERAGE-INVENTORY.md) and locked [infrastructure contract](GITHUB-INFRASTRUCTURE-CONTRACT.md) record the design and authority boundary. A documented [G4 security issue](https://github.com/spwn02/Miracle/issues/4) traces the cross-workstream risk and protects against falsely declaring unsupported C++26 semantics scanned.

**Implementation and review:** [PR #27](https://github.com/spwn02/Miracle/pull/27) introduced a read-only analyzed-files CSV comparator, deterministic negative parser fixtures and the required self-test invocation in CI, with no privileged token, CodeQL settings change or invented vulnerability. The maintainer **explicitly approved merging** this actual PR. GitHub created a verified two-parent [merge commit `0f49efb`](https://github.com/spwn02/Miracle/commit/0f49efb7864bb2f6012b7bc7594a7c6599b6f9b5) on protected `master`.

**Actual verification:** [Reference C++26 CI #39659546951](https://github.com/spwn02/Miracle/actions/runs/39659546951) **passed** (including the new CSV fixtures, all three consumer modes and tests). [CodeQL on master #39659546950](https://github.com/spwn02/Miracle/actions/runs/39659546950) **passed in all four configured languages**. [GitHub Pages #39659546949](https://github.com/spwn02/Miracle/actions/runs/39659546949) **deployed successfully**, including real public-site chapter parity and external-link verification. The subsequent [risk-disposition commit `be2cc72`](https://github.com/spwn02/Miracle/commit/be2cc728df11fb9347aff90b21a31ad48125570b) also passed [reference CI #39665182145](https://github.com/spwn02/Miracle/actions/runs/39665182145), [CodeQL #39665181633](https://github.com/spwn02/Miracle/actions/runs/39665181633) and [actual Pages deployment #39665182228](https://github.com/spwn02/Miracle/actions/runs/39665182228).

**Maintainer's explicit scope decision:** Continue default CodeQL for supported sources, **accept the known 33 unextracted interfaces as a bounded residual security-assurance limitation for now**, and **defer** any in-house C++26-aware static analyzer. This is explicit acceptance of the CodeQL *risk disposition*, not G6 phase acceptance, CodeQL semantic C++26 support or proof that the unscanned interfaces are vulnerability-free. Issue #24 stays OPEN as an ongoing known limitation.

**Release/provenance:** This is security-infrastructure measurement and documentation, **not a newly released Miracle C++ library version**. No new tag was created to manufacture an acceptance event. The separate genuine [v0.5.27 release](https://github.com/spwn02/Miracle/releases/tag/v0.5.27) has historical workflow/provenance verification as described below, but its publication predates the active new G4 tag-protection ruleset. G4's next genuine protected-tag release remains independent and unverified.

**Still outstanding for full G6:** this supplementary real-work trace does **not** replace the contracted issue #6-specific collaboration audit; nor does it establish a genuine Q&A answer/acceptance event, confidential-report delivery, Project #3 GraphQL `wasAutomated` clarification or **explicit final G6 maintainer approval**. Leave [G6 #6](https://github.com/spwn02/Miracle/issues/6), [G2 #2](https://github.com/spwn02/Miracle/issues/2) and [G4 #4](https://github.com/spwn02/Miracle/issues/4) open until their distinct gates are met.

## G2 upstream metadata clarification routed from Support to community (2026-10-21)

The maintainer supplied a screenshot of GitHub Support ticket `#4883332` being closed by the platform's virtual assistant with referrals to Community Discussions, Docs and Skills, **without** answering the `wasAutomated: false` GraphQL event-semantic question. This supersedes its previously Open support-ticket status, not the operational evidence. [G2 progress](G2-PROGRESS.md) and [G2 issue #2](https://github.com/spwn02/Miracle/issues/2) record the exact disposition and GitHub's official API-community escalation route. No community discussion has yet been submitted and no new bug/fix is claimed. **G2 remains open under the maintainer's strict acceptance requirement.**

**G6 remains in progress** independently, with issue→Project functionality already reproduced and provider metadata pending. Do not treat the nontechnical Support closure as an adverse functional G2 result or G6 acceptance.

## G2 operational Project #3 intake reproduced with new genuine issue — 2026-10-20

For the newly opened genuine [G4 CodeQL coverage Issue #24](https://github.com/spwn02/Miracle/issues/24), fresh Project GraphQL timeline data supplied by the maintainer confirms **GitHub Projects automation actor** added it to **Miracle Development Project #3** and assigned **Status: Backlog**, both at `2026-10-20T12:34:48Z`, four seconds after the issue was created. No manual Project assignment occurred in the issue-creation API call. GitHub's [official Projects documentation](https://docs.github.com/en/issues/planning-and-tracking-with-projects/managing-items-in-your-project/adding-items-to-your-project) identifies `@github-project-automation` as the actor for built-in workflow changes. This complements previous Project #3 `is:issue` UI-configuration evidence and real maintainer `Backlog → Implementing` transition.

**Distinct GitHub GraphQL anomaly:** Both `AddedToProjectV2Event.wasAutomated` and `ProjectV2ItemStatusChangedEvent.wasAutomated` returned `false`, as for older genuine Issue #8, despite the bot attribution. This conflicts with the documented boolean description; no authoritative GitHub explanation/fix is established. Preserve both facts; do not declare the flag fixed. The maintainer explicitly declined waiving that remaining concern as a G2 sign-off shortcut. [Canonical G2 ledger](G2-PROGRESS.md) records the full values and dates; [G2 issue #2](https://github.com/spwn02/Miracle/issues/2) remains open for upstream clarification and explicit owner acceptance.

**G6 remains in progress:** G2's tested functional issue intake does not complete its separate full collaboration, release, Q&A and final sign-off gates.

## G1.5 contributor/governance acceptance — imported milestone (2026-10-20)

The Miracle maintainer explicitly checked all five G1.5 acceptance items: issue form validation, PR template auto-prefill, private vulnerability reporting entry point, confidential conduct contact and final G1 approval. [G1.5 issue #1](https://github.com/spwn02/Miracle/issues/1) was closed as **completed** and [G1 acceptance ledger](G1-PROGRESS.md) records the final sign-off ([acceptance commit `d55e8f4`](https://github.com/spwn02/Miracle/commit/d55e8f42393a0b8afd5b07c79e53986f9b1b5c7e)). The [conduct policy](https://github.com/spwn02/Miracle/blob/master/CODE_OF_CONDUCT.md) now gives `spwn02.dev@gmail.com` for private reports; the maintainer confirmed the inbox receives email.

**Evidence precision:** G1 form submission blocking and live PR composer prefill are *maintainer-reported* user-interface checks; no newly captured browser screenshots or synthetic PR/issue were produced. The private vulnerability entry point was checked by the maintainer without submitting a confidential report; actual delivery/triage was not proved. The conduct address is published and maintainer-reported reachable, without testing an invented abuse allegation.

**G6 remains OPEN.** Its full issue → design/contract → implementation → validation → release/documentation traceability, automated intake into Project #3, real Q&A accepted-answer interaction and G6-specific final maintainer acceptance require separate evidence. Earlier G1/G6 observations below are historical snapshots and their then-pending G1.5 status is superseded by this section; no Project Status is automatically set to `Accepted`.

## G3 post-acceptance comprehensive hardening — verified (2026-10-20)

[Real G3 PR #23](https://github.com/spwn02/Miracle/pull/23) merged as [`61dfd48`](https://github.com/spwn02/Miracle/commit/61dfd483b7716b3ac143f3e6fa3d69427c9edd5d) after PR-head CI and all-chapter docs checks passed. Actual [master Pages deployment #39403645833](https://github.com/spwn02/Miracle/actions/runs/39403645833) passed: 29/29 full main-content chapter comparisons against public GitHub Pages, 66 desktop/mobile/navy Chromium AX-tree + axe WCAG tests, supplemental checks on known incomplete ARIA/theme and obscured-text contrast, 1,107 internal links, 848 fragments, and 195 of 196 external HTTP links. One owner-authenticated GitHub Settings URL is genuinely **not testable anonymously**, not a verified broken link. Hermetic contract-content mutation proved mdBook propagation while preserving frozen source. Prior human Apple VoiceOver + Safari usability is verified by the maintainer, not by CI.

This provides comprehensive **G3 documentation hardening evidence** to the larger end-to-end chain without reopening already accepted [G3 #3](https://github.com/spwn02/Miracle/issues/3). **Do not infer complete universal WCAG compliance, NVDA/Orca testing, confidential vulnerability delivery, Project #3 auto-add or G6 acceptance.** Those are separate evidence boundaries/workstreams.

## G3 formally accepted — explicit maintainer sign-off (2026-10-20)

The maintainer **explicitly approved G3 — Documentation Publishing** after confirmed Apple VoiceOver + Safari use and the Chrome AX-tree/axe remediation checks. This closes [G3 parent issue #3](https://github.com/spwn02/Miracle/issues/3) as completed; [implementation issue #8](https://github.com/spwn02/Miracle/issues/8) was already closed. Canonical [G3 evidence ledger](G3-PROGRESS.md) records the acceptance decision and its limits: 2/2/1 incomplete axe checks, representative page/link checks rather than full coverage, no independently captured screen-reader speech, and no deliberately modified frozen contract solely for deployment validation.

**G6 remains IN PROGRESS.** G3's owner acceptance does not waive G1.5 issue/PR intake, G2 automatic intake to target Project #3, G4 security/release checks, private vulnerability or conduct-reporting verification, real Q&A acceptance, or final G6 sign-off. Do not mark G6 complete or automatically assign Project `Accepted`.

## G3 real screen-reader usability — positive maintainer report (2026-10-20)

After the accessibility corrections in [G3 PR #22](https://github.com/spwn02/Miracle/pull/22) and the passing automated Chromium AX-tree / axe WCAG audit, the maintainer was expressly asked to test the live Miracle Book using a **real screen reader**, checking headings and landmarks, sidebar toggle/state, search/results, and checked/incomplete task-list announcements. The owner replied: **"Yes, it works flawlessly."**

This is **maintainer-reported real assistive-technology success**, not a captured spoken transcript or independent control of their desktop. In subsequent follow-up, the maintainer explicitly identified **Apple VoiceOver with Safari**. Exact device/OS and browser/screen-reader versions or individual spoken announcements were not supplied; do not invent them. Three representative axe samples found zero detected automated violations but reported **2/2/1 incomplete checks**, which remain documented scope limitations. **G3 #3 was subsequently closed as completed after explicit maintainer acceptance** (see the newer acceptance entry above); G6 remains **OPEN** for its separate end-to-end criteria.

## Private vulnerability reporting — enabled by maintainer confirmation; security review deferred (2026-10-20)

The maintainer **explicitly confirmed Private vulnerability reporting is enabled** for Miracle and supplied a screenshot showing the actual GitHub **Security and quality → Security Advisories** maintainer interface, including **New draft security advisory**. This resolves the previous uncertainty about whether the setting is enabled **at the level of owner-reported configuration**; the connected GitHub integration did not independently read restricted administrative settings.

The owner said existing vulnerabilities have been documented and requested that their detailed review be **deferred until the finishing stage**. The screenshot shows the **Published** tab's empty state, which neither confirms nor contradicts security findings documented outside published advisories. No report, advisory, vulnerability description or sensitive material was created or disclosed in this G6 verification pass.

The duplicate issue-chooser labels and redirects to `SECURITY.md` remain **separate UX observations**, not evidence that private vulnerability reporting is disabled. A maintainer's **New draft security advisory** action is distinct from an external contributor's private **Report a vulnerability** flow. **Unverified:** external reporter form access, confidential report delivery, actual conduct-reporting contact, triage/disclosure behavior, and any specific existing vulnerability. These stay open as end-to-end acceptance/security finishing-stage tasks; do not close G6 based on the setting alone.

## Security reporting chooser — user-observed gap (2026-10-20)

The maintainer reports that both identically labelled **Report a security vulnerability** options in the GitHub New Issue chooser redirect to `SECURITY.md`, rather than opening a private vulnerability-report form. The screenshot shows two entries, one with GitHub's native shield icon and the other an external-link icon. **The reported redirects are user observations**, not independently captured destination screenshots.

The second route is expected from our explicit [issue chooser config](https://github.com/spwn02/Miracle/blob/master/.github/ISSUE_TEMPLATE/config.yml) pointing to `SECURITY.md`; the duplicate label is part of the [locked G1.1 contract](G1.1-ISSUE-INTAKE-CONTRACT.md). The first entry is GitHub-native. GitHub documents that [private vulnerability reporting must be enabled separately](https://docs.github.com/en/code-security/how-tos/report-and-fix-vulnerabilities/configure-vulnerability-reporting/configure-for-a-repository) in **Settings → Security and quality → Advanced Security**. An enabled repository's Security Advisories page provides the private **Report a vulnerability** path. **No authenticated administrative setting was read**, so do not claim private reporting is disabled; it is a plausible cause pending confirmation.

**Acceptance next step:** Maintainer inspect/enable the **Private vulnerability reporting** setting (if appropriate), verify the Advisories reporting form is available without submitting a synthetic vulnerability, and separately approve any G1.1 contract amendment to rename the duplicate custom policy link. **Private disclosure and delivery remain unverified**; this is not a completed security gate.

## Security policy and contributor onboarding — live page verification (2026-10-20)

The maintainer supplied GitHub browser screenshots confirming both advertised destinations:

1. [GitHub Security policy](https://github.com/spwn02/Miracle/security/policy) displays repository `SECURITY.md` in **Security and quality → Security policy**, with **Reporting a vulnerability** instructions: do not publish sensitive details in normal issues, use GitHub private vulnerability reporting **when available**, or contact the owner to request a private channel first.
2. [Contributing guide](https://github.com/spwn02/Miracle/blob/master/CONTRIBUTING.md) loads in GitHub's Markdown Preview, showing contributor prerequisites, links to **Development Protocol**, Issues, Discussions, New Issue chooser, Q&A/Ideas, `SUPPORT.md`, `CODE_OF_CONDUCT.md`, `SECURITY.md` and the reference-toolchain section.

**Verified:** both top-level **destinations open and render** in the actual GitHub browser; this advances G1.5 and G6 public onboarding/security-policy navigation. **Not verified:** whether private vulnerability reporting is enabled, the creation/delivery of any confidential vulnerability or conduct report, click-through of each embedded contributor link, or PR-template prepopulation. Do not mark confidential-reporting acceptance as complete. No test report was sent.

## GitHub Community Standards — live recognition verified (2026-10-20)

The maintainer supplied a real GitHub **Insights → Community standards** screenshot. The eight visible rows each have GitHub check marks: **Description, README, Code of conduct, Contributing, License, Security policy, Issue templates, Pull request template**. These are direct evidence that GitHub recognizes the corresponding community documents/setting. The display does not independently verify browser navigation through each linked policy, private vulnerability or misconduct reporting, or whether PR-template text is automatically prefilled into a new pull request. No synthetic issue, PR, or confidential report was created.

**Acceptance:** The **Community Standards recognition** portion of G1.5 and G6 public onboarding is **verified**. The other end-to-end and confidentiality requirements remain open. [G1 progress ledger](G1-PROGRESS.md) records the same evidence.

## Live Bug Report form and inline validation — owner visual evidence (2026-10-20)

The maintainer opened [the actual Bug Report form](https://github.com/spwn02/Miracle/issues/new?template=bug_report.yml) and provided a screenshot. It shows the required **Affected revision** field empty with a red GitHub inline validation message **`Field can not be empty`**; the **Affected area** dropdown is unstarred, and **Observed behavior** is visibly required. The form includes security/reporting guidance, but the screenshot does **not** show the user following or verifying the policy link destination. No issue was submitted.

**G1/G6 verified:** one real issue form renders correctly and GitHub performs inline required-field validation. **Not yet verified:** clicking `Submit new issue` while the form remains invalid and confirming submission is blocked, other forms' validation, confidential reporting, PR template prepopulation or Project automatic intake of a new issue. The draft form's `Projects: No projects` sidebar cannot refute issue auto-add because no GitHub issue has been created.

## Live GitHub issue chooser — owner visual evidence (2026-10-20)

The maintainer supplied an actual screenshot of Miracle's **Create new issue** page. It displays the **five** structured GitHub issue forms (Architecture Proposal, Bug Report, Compiler / Toolchain Issue, Feature Proposal and Infrastructure Request), along with Contributing guide, Q&A, Ideas, a link to `SECURITY.md`, and GitHub's native private vulnerability-report entry. `Blank issue` is displayed to the owner as **Maintainers only**, consistent with the source configuration `blank_issues_enabled: false` being a public-chooser setting, not a way to prevent maintainer-authored blank issues.

**Verified at chooser stage:** forms and contact/navigation choices render together. **Subsequent screenshot:** Bug Report form opens and displays an inline error for empty required `Affected revision`. **Not yet verified:** full submit-action blocking, validation of the other forms, confidential security reporting and conduct paths, or live PR template prepopulation. GitHub displays two same-named security entries because one is its native security feature and the other is our policy link. The name of the custom link is specified by the [locked G1.1 contract](G1.1-ISSUE-INTAKE-CONTRACT.md); it has **not** been silently modified.

The same screenshot advances [G1.5 issue #1](https://github.com/spwn02/Miracle/issues/1) and the G6 public onboarding gate, **but not confidential-report delivery or final G6 acceptance**.

## G3 accessible semantics and keyboard interaction — automated audit (2026-10-20)

The maintainer requested actual screen-reader accessibility verification. [G3 PR #22](https://github.com/spwn02/Miracle/pull/22) identified five classes of genuine mdBook WCAG issues (invalid sidebar-toggle ARIA state, link contrast, small click targets, color-only prose links, unnamed disabled checklist inputs). Focused CSS/JS overrides were reviewed and merged as [`5d0bfbd`](https://github.com/spwn02/Miracle/commit/5d0bfbd512fdf2643263ee4176fe67c8cc3bf5fe). PR-head reference CI and documentation jobs passed. Post-merge [Pages #39379255036](https://github.com/spwn02/Miracle/actions/runs/39379255036) passed an **actual generated-page Chrome accessibility-tree and axe WCAG A/AA regression test**: **zero automated violations** across desktop homepage, G3 chapter, mobile homepage; sidebar button Enter/Space and expanded-state behavior passed. Axe still reported incomplete/manual-review results.

This constitutes **verified automated accessibility semantics**, **NOT a test of Orca/VoiceOver/NVDA speech/interaction**. Real screen-reader observation and author G3 sign-off remain separate open gates in [G3 ledger](G3-PROGRESS.md). Do not close G3 or G6 on this automated evidence alone.

## G3 canonical source → actual public Pages parity — CI verified (2026-10-20)

[PR #21](https://github.com/spwn02/Miracle/pull/21) merged as [`723172c`](https://github.com/spwn02/Miracle/commit/723172c63229154840219fb21d9ab8d7559c17bc) after successful PR-head reference CI and mdBook/fixture verification. The **post-merge [Pages run #39376713839](https://github.com/spwn02/Miracle/actions/runs/39376713839)** succeeded with a **real public HTTP smoke test after deployment**: canonical G3 keyboard statement and index module paragraph both matched the respective live HTML, and **three selected real Github outgoing links** appeared on the homepage and resolved successfully. This resolves the prior source-to-public paragraph and representative-link evidence gap.

**Limit:** full screen-reader/assistive-technology checks and final G3 owner acceptance remain open on [G3 #3](https://github.com/spwn02/Miracle/issues/3). The G6 end-to-end chain has stronger publication evidence, but no author G6 sign-off is inferred.

## G3 live documentation experience — owner-reported passing checks (2026-10-20)

The maintainer directly tested the public [Miracle Book](https://spwn02.github.io/Miracle/) and reported that **Tab navigation, sidebar and chapter navigation, search/results and responsive mobile-width layout all work as expected**. This adds real browser-experience evidence to the previously successful canonical mdBook/Pages deployment chain. Later post-deployment CI independently verified **representative outbound links and canonical source-to-published paragraph equality** (see newer record above). [G3 progress](G3-PROGRESS.md) retains **screen-reader semantics and full G3 author acceptance** as distinct unresolved requirements. Do not treat this positive usability report as a completed assistive-technology audit or automatically close [G3 issue #3](https://github.com/spwn02/Miracle/issues/3).

## Existing v0.5.27 release: authentic tag → CI → provenance → publication (2026-10-19 audit)

This is **real historical release evidence**, not a fabricated version to satisfy G6. The public [v0.5.27 release](https://github.com/spwn02/Miracle/releases/tag/v0.5.27), ID `414415213`, was published on **2026-10-16 00:47:37 UTC**. Its tag `refs/tags/v0.5.27` resolves via the GitHub tag API to `e1e5817c651d124d02930b86510e2b05fb7c7d2d`.

The successful [Release Actions run #38831221086](https://github.com/spwn02/Miracle/actions/runs/38831221086) ran on that exact commit. Job `116750205166` passed **71/71 CTest cases** and **3/3 CMake consumers** with the reference C++26 snapshot. Dependent release job `116751054300` passed **Validate release provenance**, **Write release provenance**, and **Create GitHub release**. Its logs report `SOURCE_REVISION=e1e5817c651d124d02930b86510e2b05fb7c7d2d`; the release workflow validates tag/version correspondence and ancestor membership in `master`.

The published `release-metadata.json` asset was uploaded by `github-actions[bot]` (asset ID `645665032`, 608 bytes, API-provided SHA-256 `27a7a78fea1df167a79e6e7386bc62efe4faf257212d5fa09f8453248cba9635`). Release notes contain a real comparison link `v0.5.26...v0.5.27`. **Historical limit (2026-10-19):** asset bytes/fields had not yet been downloaded; this limit was **superseded on 2026-10-23** by authenticated [readback #39902726212](https://github.com/spwn02/Miracle/actions/runs/39902726212), which verified the actual bytes and fields. This verifies the real tag → successful reference CI → provenance generation → GitHub publication chain. G6 infrastructure documentation is not a user-visible product release and did **not** require a new tag.

## Genuine Project #3 status transition — owner evidence (2026-10-19)

The maintainer manually moved actual issue #6 from `Backlog` to `Implementing` in Miracle Development Project #3. Owner-supplied before/after `gh issue view 6 --repo spwn02/Miracle --json projectItems` results identify status option `f75ad846` (`Backlog`) becoming `ada290ce` (`Implementing`), while the separate Roadmap Project remains `Todo`. GitHub's issue #6 timeline independently records a status-change event by `spwn02` at 2026-10-19 15:20:32 UTC. The event alone omits the target Project ID; the CLI snapshots identify the Project.

**G2/G6 verified:** genuine maintainer-authorized Project #3 status transition; no `Accepted` status implied. The previously risky automatic PR-linked `Design` transition was disabled by the maintainer. Remaining gates include causal automatic enrollment for a future real non-bug issue, live reporting and private-support checks, actual Q&A accepted-answer behavior, release provenance and final G6 author acceptance. Keep G6 issue #6 open.

## Follow-up: maintainer disables PR status regression; issue #8 Project #3 confirmed (2026-10-19)

The maintainer supplied an updated **Project #3 workflow screenshot**: `Pull request linked to issue` is **Off** (previously On, with `Set value: Status = Design`). The page counter shows **3 enabled workflows**: sub-issue auto-add, new-issue auto-add, and `Item added → Backlog`. Auto-close, Item closed and PR merged remain Off. This closes the **configuration-level risk** of a linked PR automatically resetting a later status to `Design`. Earlier notes below describe the **historical** On state and are superseded here.

The maintainer also provided real `gh issue view 8 --repo spwn02/Miracle --json projectItems` results:

- **Miracle Development Project #3:** `Backlog`.
- **Separate Roadmap Project:** `Done`.

Issue [#8](https://github.com/spwn02/Miracle/issues/8) is a **genuine completed non-bug documentation publishing issue**, not a fake test fixture. Its GitHub REST timeline records **two GitHub Projects automation-bot add events** and subsequent bot status changes. The current membership in the intended Project #3 is **verified** and the automatic intake evidence is consistent with both Projects, but the event payloads do **not** identify which Project each event targeted. **Do not claim specific causal auto-add to Project #3 is independently proven** by combining an ambiguous event with a later membership snapshot.

The issue's closed state plus Project #3's `Backlog` is useful evidence that closing this work item did not leave it automatically `Accepted` in the current snapshot. It does not verify historical transitions or constitute formal author acceptance for G2/G6. No Project status was changed. **Remaining:** real automatic intake into target Project #3 with verified causal provenance, a maintainer-approved Project status transition, live GitHub public/private intake, genuine Q&A accepted-answer behavior when available, release provenance review, and final G6 sign-off.

## Project #3 — remaining enabled workflow actions verified (2026-10-19)

The maintainer provided **three Project #3 workflow detail screenshots**:

- `Item added to project` (**On**): for an issue or PR newly added to the Project, `Set value → Status: Backlog`.
- `Pull request linked to issue` (**On**): when a PR is linked to an issue, `Set value → Status: Design`.
- `Auto-add sub-issues to project` (**On**): when a Project item has sub-issues, `Add sub-issues to the project`, without itself assigning `Accepted`.

**Configuration-level result:** All four enabled workflows (including previously reviewed issue Auto-add with filter `is:issue`) now have their visible actions inspected. None targets `Accepted`. The `Auto-close issue`, `Item closed` and `Pull request merged` workflows remain screenshot-confirmed **Off**. This checks the **direct-acceptance bypass** concern at the level of configured workflow actions.

**Historical residual risk — now mitigated:** `Pull request linked → Design` might have reset an issue from a later state. The maintainer **subsequently disabled** that workflow (see updated screenshot above); the assistant did not change settings. **Runtime** causal auto-add to target Project #3, verified status transitions and explicit maintainer acceptance remain **unverified**. See [G2 progress](G2-PROGRESS.md).

## Project #3 — Auto-add and workflow UI evidence (2026-10-19)

The maintainer supplied a screenshot from **Miracle Development → Settings → Workflows**. The `Auto-add to project` workflow is visibly **On**, restricted to repository **Miracle**, uses exactly **`is:issue`** as its filter and performs **Add the item to the project**. The screenshot also shows **Auto-add sub-issues to project**, **Item added to project**, and **Pull request linked to issue** enabled. Crucially, **Auto-close issue**, **Item closed**, and **Pull request merged** are **Off**; no default workflow among those three can automatically mark a completion by running while disabled.

This verifies **current configuration**, distinct from the prior `gh issue view 6 --json projectItems` output showing actual membership. The original workflow-overview screenshot did **not** show enabled actions, but subsequent workflow-detail screenshots now **verify** their configured targets: `Backlog`, `Design`, and add sub-issues; none sets `Accepted`. A real new-issue auto-add *result* and Project #3's acceptance-sensitive status transitions remain **unverified**. This screenshot alone does not demonstrate that a future new real issue enters Project #3 without manual intervention or that `Accepted` requires maintainer action. Refer to the [G2 verification ledger](G2-PROGRESS.md) for the independent workstream gate.

## Project #3 — real issue membership verified (2026-10-19)

The maintainer ran the read-only command `gh issue view 6 --repo spwn02/Miracle --json projectItems` and supplied its actual JSON output. It reports **two separate Project memberships** for the same real G6 issue:

| Project title returned | Status name returned |
| --- | --- |
| `Roadmap` | `Todo` |
| `Miracle Development` | `Backlog` |

This **verifies that issue #6 is a member of the intended [Miracle Development Project #3](https://github.com/users/spwn02/projects/3)** and that its current reported Project status is **Backlog**, not `Accepted`. The separate `Roadmap` Project is expected to have its own `Todo` status; do not confuse these views.

**Not established by this command alone:** how or when the issue was added, correct automatic enrollment of future issues, Project view filters, Phase/Area assignments, or a proper author-approval transition. The later workflow screenshot independently verifies the `is:issue` rule and the three disabled close/merge defaults, but does not exercise automatic intake or show the two remaining enabled workflow actions. Historical G2 evidence includes manual Project assignment for existing issues, so **membership alone must not be presented as proof of working automatic intake**. The user can inspect **Project #3 → Settings → Workflows** and use [the read-only G2 audit](https://github.com/spwn02/Miracle/blob/master/scripts/github/audit-g2.sh) for the remaining configuration gates. Since G6 implementation is already active, changing the card from `Backlog` to `Implementing` may be appropriate **only as an explicit maintainer project-management decision**; never set `Accepted` automatically.

## Discussion activation and category evidence (2026-10-19)

The maintainer provided an actual GitHub **Manage discussion categories** screenshot with the following categories: **Announcements** (restricted/locked icon), **General**, **Ideas**, **Polls**, **Q&A** (*Answers enabled*), and **Show and tell**. All four contract-required functional categories appear. **General** and **Polls** are optional defaults, not missing requirements.

A fresh public GitHub repository read after the screenshot returned **`has_discussions: true`** (previous initial audit: `false`). This establishes that Discussions is now enabled, independently of the screenshot. The category screenshot proves the configured category names and answers setting, **not** that someone posted a real Q&A question and selected an accepted answer.

[SUPPORT.md](https://github.com/spwn02/Miracle/blob/master/SUPPORT.md), [issue chooser](https://github.com/spwn02/Miracle/blob/master/.github/ISSUE_TEMPLATE/config.yml), README and contributor guidance **now link** to the configured category names using GitHub's conventional default slugs through [merged PR #17](https://github.com/spwn02/Miracle/pull/17). Newly created per-category URLs were not independently fetched/validated by the available integration (attempts to open the newly enabled pages failed due to cache availability). All four category routes were subsequently **reported opening correctly by the maintainer**; real Q&A answer selection remains a separate open check.

**Additional maintainer UI evidence (2026-10-19):** The maintainer explicitly reported that **all four advertised category links open correctly** and provided a screenshot showing the **Ideas** page selected, with `is:open category:Ideas` and `There are no matching discussions`. Empty categories are an expected initial condition; no synthetic post has been created. This is **maintainer-reported browser verification** of navigation, not a machine-verified HTTP check for every category and not a test of creating/answering a real Q&A post.

**Current accepted evidence:** Discussions enabled, four categories configured, their links confirmed working by the maintainer, and G6 issue #6 genuinely present in Miracle Development Project #3. **Still outstanding:** genuine Q&A accepted-answer experience, Project #3 automation/view safeguards and live community reporting checks.

## Discussions navigation: real PR, CI and Pages evidence (2026-10-19)

- [PR #17](https://github.com/spwn02/Miracle/pull/17) reviewed and **merged with a history-preserving merge commit** [`9c5c7e8`](https://github.com/spwn02/Miracle/commit/9c5c7e82a066c93a8992305da1be1bc433006b9c). The patch updates support/README/contributor help, the GitHub new-issue chooser, and G6 documentation and regression guard, **without modifying Issues' actionable routes or private security reporting**.
- [PR-head reference CI #39213814726](https://github.com/spwn02/Miracle/actions/runs/39213814726) **SUCCESS**: **71/71 CTest cases**, all **3/3** consumer modes, and the G6 route guard with **nine negative fixtures** passed.
- [PR-head mdBook #39213814711](https://github.com/spwn02/Miracle/actions/runs/39213814711) **SUCCESS**. [Post-merge GitHub Pages #39214184308](https://github.com/spwn02/Miracle/actions/runs/39214184308) **SUCCESS**, including rendered navigation and successful `Deploy site`; [post-merge reference C++26 #39214184313](https://github.com/spwn02/Miracle/actions/runs/39214184313) and [master-to-gcc sync #39214184363](https://github.com/spwn02/Miracle/actions/runs/39214184363) **SUCCESS**.
- **Verified after PR #17:** The maintainer reports all four category links open correctly, with screenshot evidence for the Ideas category. The `gh issue view 6 --json projectItems` output also confirms issue #6 in Project #3 with status `Backlog`. **Not yet verified:** an accepted answer on a genuine Q&A question, Project #3 workflow/auto-add acceptance, the private/community moderation channel, or author G6 sign-off. **Do not close #6.**

## Genuine G6 issue → implementation → CI → documentation trace (2026-10-19)

This is a **real, observed workflow**, not a fabricated demonstration:

1. **Intake:** existing [G6 issue #6](https://github.com/spwn02/Miracle/issues/6), with authentic infrastructure goal/criteria. The maintainer's `gh issue view 6 --json projectItems` output now **explicitly confirms Miracle Development membership and Backlog status**. Historical GitHub timeline Project-add events alone did not identify Project #3 and still do **not** prove correctly configured ongoing automation.
2. **Design/authority:** repository-locked [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md) §4/§9–§10, this [operating plan](G6-OPERATING-PLAN.md) and G6 ledger; no new product API or contract amendment was silently approved.
3. **Implementation/review:** [PR #16](https://github.com/spwn02/Miracle/pull/16) reviewed with explicit remaining gates and merged using a **history-preserving merge commit** [`43050d7`](https://github.com/spwn02/Miracle/commit/43050d70f4cbad41e65501b0428b4a60836b5b0c). PR-head SHA: `63e88c1b788d774389c17100f4e6d630758b816c`.
4. **Verification:** [PR-head reference C++26 CI #39206579960](https://github.com/spwn02/Miracle/actions/runs/39206579960) **SUCCESS**: new G6 source-route + **five negative fixtures** passed, **71/71** CTest cases and all three consumer modes passed. [PR-head mdBook #39206579929](https://github.com/spwn02/Miracle/actions/runs/39206579929) **SUCCESS**. On the merge commit, [reference CI #39206970818](https://github.com/spwn02/Miracle/actions/runs/39206970818) **SUCCESS** and [`master`→`gcc` sync #39206970799](https://github.com/spwn02/Miracle/actions/runs/39206970799) **SUCCESS**.
5. **Release-facing documentation:** [post-merge GitHub Pages #39206970782](https://github.com/spwn02/Miracle/actions/runs/39206970782) **SUCCESS**, including generated G6 operating-plan and progress chapters, navigation/search verification, and successful `Deploy site` job. Direct public CDN/browser appearance was **not** independently checked by the available integration; this is confirmed Actions deployment evidence, not a UI accessibility audit.
6. **Acceptance:** **NOT YET ACCEPTED.** Discussions activation, category configuration and maintainer-tested links are verified; issue #6's **Project #3 membership is confirmed** but ongoing Project workflows/views are not. Real Q&A accepted-answer behavior remains untested, G1.5/G2/G3/G4 acceptance issues remain independently open, and no final G6 sign-off was supplied.
7. **Release/versioning:** G6 PRs #16/#17 are infrastructure-only, so no new library release was appropriate. The genuine historical `v0.5.27` tag and public release were audited against exact-commit passing Release run #38831221086, with 71/71 tests, 3/3 consumers, provenance generation and a published metadata asset. The initial 2026-10-19 audit did not inspect internal JSON fields; authenticated [readback #39902726212](https://github.com/spwn02/Miracle/actions/runs/39902726212) subsequently verified them.

**Actual demonstrated path:** issue #6 → reviewed PR #16 → passing reference CI → merge commit → successful mdBook Pages deployment. **Subsequently verified:** category navigation, current issue #6 Project #3 membership, genuine maintainer-controlled `Backlog → Implementing`, acceptance-safe workflow configuration, and a historical v0.5.27 release provenance chain. **Not yet demonstrated:** real Q&A accepted-answer participation, causal automatic enrollment specifically into Project #3, live reporter/private conduct routes, and final G6 author acceptance.

## Baseline read-only repository audit (2026-10-19)

**Observed GitHub repository API:** `spwn02/Miracle` is public, `has_issues: true`, and **`has_discussions: false`**. G6 Discussions creation and category validation have **not** occurred. The available GitHub connection does not expose repository admin setting changes or Discussions writes, and this workstream does not authorize a workaround with personal tokens or elevated workflows.

**Source-level foundation already exists:** five YAML issue forms, an issue chooser linking security and contributing guidance, a PR traceability/verification template, community support/conduct/security/accessibility policies, GitHub Pages/mdBook, a locked infrastructure contract, and an actual G6 issue #6. Source presence is **not equivalent** to verifying the live GitHub UI or a successful confidential report.

**Current cross-workstream state (2026-10-20):** [G3 #3](https://github.com/spwn02/Miracle/issues/3) and publishing subtask #8 **closed — accepted**. [G5 #5](https://github.com/spwn02/Miracle/issues/5) **closed — accepted** in revised Docker-only scope (Codespaces declined). **Remaining open:** [G1.5 #1](https://github.com/spwn02/Miracle/issues/1), [G2 #2](https://github.com/spwn02/Miracle/issues/2), [G4 #4](https://github.com/spwn02/Miracle/issues/4) and this [G6 #6](https://github.com/spwn02/Miracle/issues/6).

**Historical release baseline:** Public releases `v0.1.0-rc.1`, `v0.1.0` and `v0.5.27` were initially observed. The later audit **independently traced v0.5.27 to its exact tagged source, passing Release workflow and public provenance asset**, as recorded above; no G6-specific library release was needed.

## Acceptance evidence matrix

| Gate | State | Evidence / follow-up |
| --- | --- | --- |
| Genuine intake issue | **Observed** | Existing [G6 issue #6](https://github.com/spwn02/Miracle/issues/6) with real acceptance criteria |
| Community source routes | **CI-VERIFIED (static)** | [#39206579960](https://github.com/spwn02/Miracle/actions/runs/39206579960) passed five negative fixtures; not a live GitHub UI test |
| Discussions enabled and four categories | **VERIFIED — configuration only** | Screenshot shows required names and Q&A Answers enabled; fresh repository API reports `has_discussions: true`. Real answered thread **not tested** |
| Public navigation after Discussions | **PASSED — maintainer-reported browser check** | [PR #17](https://github.com/spwn02/Miracle/pull/17) merged and CI/Pages passed; maintainer confirms all four category links open. Screenshot shows selected empty Ideas category. This does not validate the Q&A answer workflow |
| Design → implementation PR | **VERIFIED** | Reviewed and merged [PR #16](https://github.com/spwn02/Miracle/pull/16), commit `43050d7`; parent issue #6 stays open |
| PR-head reference CI and mdBook | **PASSED** | Reference #39206579960, docs #39206579929 on exact `63e88c1` revision |
| Merge / post-merge docs | **DEPLOYMENT PASSED** | Post-merge mdBook/Pages #39206970782; direct browser navigation still unverified |
| Project #3 membership and workflow | **REAL STATUS TRANSITION VERIFIED; AUTO-ADD CAUSALITY PENDING** | Maintainer changed issue #6 `Backlog → Implementing` in Miracle Development; before/after CLI plus owner status-change timeline event. Non-bug issue #8 remains in Project #3 as `Backlog`, separately `Roadmap / Done`. Configured `is:issue` auto-add and PR-linked workflow Off are verified, but causal automatic enrollment specifically into Project #3 is not yet established |
| Browser/UI intake and private route | **PARTIAL — PUBLIC INTAKE VERIFIED; PRIVATE REPORTING CONFIG OWNER-CONFIRMED** | Eight Community Standards checked, chooser/forms/navigation visible and inline Bug Report error confirmed. Owner says Private vulnerability reporting **enabled**; Security Advisories maintainer UI screenshot observed. **External private form, confidential delivery, conduct path, submit blocking and PR prefill remain unverified**; vulnerability review deferred to finishing stage |
| Author acceptance and G6 closure | **PENDING** | Require explicit decision |
| Release notes/provenance | **REAL HISTORICAL RELEASE CHAIN VERIFIED** | [v0.5.27 Release run #38831221086](https://github.com/spwn02/Miracle/actions/runs/38831221086): matching tag SHA, 71/71 tests, 3 consumers, provenance generation, published metadata asset. Actual release asset bytes and JSON fields verified in [#39902726212](https://github.com/spwn02/Miracle/actions/runs/39902726212); G6 infrastructure commits need no product release |

## Notes

This ledger is the evidence record, not a secondary source of product contracts. Do not mark owner-administered settings as complete until independently observed. Do not make the Project say `Accepted` simply because PR CI passes; do not create a release tag merely to exercise a workflow.
