# Miracle GitHub Infrastructure — G3 Progress

**Updated:** 2026-10-20
**Program:** G0–G6
**Workstream:** G3 — Documentation Publishing
**Status:** ACCEPTED & POST-ACCEPTANCE HARDENING VERIFIED (2026-10-20): all 29 canonical public chapter bodies matched, whole-site internal link/fragment check passed, 66 accessibility samples and supplementary axe checks passed, isolated contract mutation proved. Owner-only external HTTP route and universal AT coverage remain inherent evidence bounds; G3 #3 and #8 remain closed completed
**Public documentation:** [The Miracle Book](https://spwn02.github.io/Miracle/)
**Authority:** [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Current plan:** [G3 Operating Plan](G3-OPERATING-PLAN.md)
**Tracking:** [#3 — G3 publishing](https://github.com/spwn02/Miracle/issues/3) · [#8 — Pages implementation](https://github.com/spwn02/Miracle/issues/8)

## G3 comprehensive hardening batch — implemented, merged and verified (2026-10-20)

Following the maintainer's explicit request to work on **all six previously recorded limitations in one batch**, genuine [PR #23](https://github.com/spwn02/Miracle/pull/23) merged with history-preserving commit [`61dfd48`](https://github.com/spwn02/Miracle/commit/61dfd483b7716b3ac143f3e6fa3d69427c9edd5d). On the exact PR head, [reference C++26 CI #39403163691](https://github.com/spwn02/Miracle/actions/runs/39403163691) and [full documentation audit #39403163724](https://github.com/spwn02/Miracle/actions/runs/39403163724) both passed. **Actual post-merge public Pages [run #39403645833](https://github.com/spwn02/Miracle/actions/runs/39403645833) succeeded**, including all post-deploy checks.

| Original limitation | New evidence and disposition |
| --- | --- |
| 1. Unresolved axe `incomplete` results | The audit now **identifies every incomplete rule**, rejects unknown kinds, verifies the theme menu's real `aria-controls` target in the DOM, and independently computes foreground/background contrast for obscured sidebar/content elements. **All 66 supplementary evaluations passed**, with **zero detected WCAG violations**. Axe may still *label* these geometric/hidden-menu cases incomplete; our separate checks establish their known conditions without pretending axe's raw label changed. |
| 2. Limited page, viewport and theme coverage | Automated Chromium AX-tree/axe WCAG A/AA audit now checks **all 29 canonical chapters** at desktop and mobile widths, and **four representative chapters in navy theme** at both sizes: **66 successful audits**. Fixed discovered dark-theme active sidebar contrast and scrollable code/table keyboard focus. Not a claim of every possible browser/theme combination or exhaustive WCAG conformance. |
| 3. Limited human assistive-technology coverage | The maintainer previously completed the prescribed **Apple VoiceOver + Safari** real speech/navigation usability check successfully. The automated checks now cover every chapter's semantics in Chromium, but **no NVDA/Orca hardware session was conducted**. Such cross-platform human tests cannot be truthfully inferred or considered an undiscovered failure. |
| 4. Limited documentation links | Full crawler checked **52 rendered HTML files**, **1,107 internal references**, and **848 fragments**: **all resolved**. All **196 distinct external URLs** were attempted; **195 returned successful public HTTP responses**. One GitHub **owner-authenticated Settings** URL returned anonymous 404 and is explicitly **INDETERMINATE**, not broken or independently verified. Repaired broken GitHub-source-relative links in G1/G3/G5/G6 documentation and public Discussions navigation. |
| 5. Only two published paragraphs checked | The actual post-merge deployment rebuilds pinned mdBook at the **same exact source revision** and compares the **full main-content text, structural tokens and link hrefs of every one of the 29 canonical chapter bodies** against live public GitHub Pages. **29/29 matched** in the [post-deploy run](https://github.com/spwn02/Miracle/actions/runs/39403645833). |
| 6. No sample frozen-contract edit | The new [isolated contract-propagation test](https://github.com/spwn02/Miracle/blob/master/scripts/docs/verify-contract-propagation.py) mutates only a **temporary copy** of a frozen infrastructure contract, builds with pinned mdBook, proves the sentinel appears in generated contract HTML, and verifies the original contract SHA-256 is **byte-for-byte unchanged**. Real live publication is separately demonstrated by genuine non-contract documentation changes. An actual modified frozen contract was intentionally **not** committed or published. |

**Disposition:** The six **actionable documentation/pipeline regression gaps** are now covered by permanent CI tests or narrowly documented, contract-preserving equivalents. **No G3 code defect or failed check from this batch remains outstanding.** Global absence of all accessibility defects, other screen-reader platforms, and owner-authenticated GitHub URLs cannot be certified from this evidence, so they are recorded as **coverage boundaries**, not claimed to have passed. The maintainer's earlier formal G3 acceptance remains valid; [G3 #3](https://github.com/spwn02/Miracle/issues/3) and [#8](https://github.com/spwn02/Miracle/issues/8) remain **CLOSED — completed**. G1/G2/G4/G6 remain independent.

## G3 FINAL ACCEPTANCE — maintainer-approved (2026-10-20)

**Decision:** The maintainer explicitly responded **"Yes, I approve it"** to the request to formally approve **G3 — Documentation Publishing**, accepting the stated and recorded limits. This approval **applies to G3 only**, including closure of [parent issue #3](https://github.com/spwn02/Miracle/issues/3). [Implementation issue #8](https://github.com/spwn02/Miracle/issues/8) was already closed. It does **not** approve G1/G2/G4/G6, the status of any GitHub Project items, or any product release.

**Acceptance evidence:** canonical `docs/` → immutable mdBook v0.5.4 GitHub Pages publication, chapter navigation/search and legacy redirects, real documentation-edit auto-publishing, [source-to-public Pages matching and 3/3 external links](https://github.com/spwn02/Miracle/actions/runs/39376713839), maintainer keyboard/sidebar/search/mobile checks, the [post-merge Chrome AX-tree/axe WCAG sample audit](https://github.com/spwn02/Miracle/actions/runs/39379255036), and maintainer-reported **Apple VoiceOver + Safari** usability for the requested screen-reader tasks. The audit detected and fixed accessibility faults in [PR #22](https://github.com/spwn02/Miracle/pull/22); its final sample had zero *detected* violations. No newly manufactured change to an authoritative frozen product contract was required to test Pages publishing: real non-contract canonical documentation edits served the equivalent deployment check.

**Limits explicitly accepted with the G3 sign-off:** the axe samples returned **2/2/1 inconclusive/manual-review items**; only representative pages/viewports and three external links were tested, not all pages/links/browsers/themes; no independent screen-reader speech capture or exact VoiceOver/Safari/OS version was provided; full WCAG conformance and generated C++ API-reference coverage are **not claimed**. These are documented coverage limitations, not falsely marked test successes.

**Disposition:** **G3 ACCEPTED**; close [issue #3](https://github.com/spwn02/Miracle/issues/3) as completed after recording this approval. Documentation and accessible-theme regression checks remain maintained in the usual CI workflow.

## Real screen-reader usability — maintainer-reported passing result (2026-10-20)

After the focused accessibility fixes merged in [PR #22](https://github.com/spwn02/Miracle/pull/22), the maintainer was asked to test the live [Miracle Book](https://spwn02.github.io/Miracle/) with a **real screen reader** and check headings/landmarks, sidebar toggle/expanded-state announcements, search/results, and completed/incomplete checklist semantics. The maintainer replied: **"Yes, it works flawlessly."**

**Evidence classification:** A positive **maintainer-reported screen-reader usability check** in direct response to those test instructions. The maintainer subsequently identified the exact combination as **Apple VoiceOver with Safari**. This confirms the **screen reader and browser**, not the precise device, operating-system version, Safari/VoiceOver release versions or exact spoken announcements. No independently captured speech/audio transcript exists; this is not an exhaustive accessibility or WCAG compliance certification.

Separately, [post-merge Pages/axe run #39379255036](https://github.com/spwn02/Miracle/actions/runs/39379255036) reported **zero detected automated WCAG violations** across three representative browser/page/viewport samples, but **2/2/1 incomplete checks** remain outside a positive automated result. These are documented residual review limitations. No G3 final sign-off was requested or supplied in this response.

**Disposition at test time:** **Apple VoiceOver + Safari** interaction was reported working, with version/transcript/audit scope limitations. **Superseded by the final G3 approval recorded above**; both G3 parent #3 and publishing subtask #8 are now completed.

## Maintainer visual acceptance and continued implementation

After reviewing the published Rust Book-style site, the maintainer responded **“Amazing, this is gorgeous! Let's continue now.”** This provides positive acceptance evidence for the site's overall appearance and the decision to retain mdBook, but it is **not** a documented screen-reader, mobile, keyboard-only, or exhaustive search test. Those checks remain explicit before formal G3 workstream acceptance.

Current publication was preserved while the next infrastructure workstream (G4) started. GitHub milestone [#1 — GitHub Infrastructure (G0–G6)](https://github.com/spwn02/Miracle/milestone/1) now covers both G3 issues (#3 and #8) as part of one program-level deliverable.

## Current implementation: mdBook (Rust Book style)

The maintainer requested replacing the initial MkDocs site with **mdBook**, using Rust documentation conventions. The active deployment now consists of:

- [`book.toml`](https://github.com/spwn02/Miracle/blob/master/book.toml): mdBook source `docs/`, output `site/`, native `rust` theme and `navy` dark preference, repository/edit links, search and original source provenance.
- [`docs/SUMMARY.md`](https://github.com/spwn02/Miracle/blob/master/docs/SUMMARY.md): complete sidebar organizing **21 canonical Markdown chapters**, with all previous documentation present exactly once.
- [`docs/index.md`](../index.md): introductory page and truthful guide links; product architecture/contracts remain in canonical Git Markdown, unchanged.
- [`docs.yml`](https://github.com/spwn02/Miracle/blob/master/.github/workflows/docs.yml): prebuilt official **mdBook v0.5.4 Linux GNU** binary with published archive SHA-256 `3f28de05dafca9d0f2eab99c662116b0e37b89b1d96a08f8f430b9eeae958cd7`; no `cargo install` or LLVM build in CI.
- [`scripts/docs/verify-site.py`](https://github.com/spwn02/Miracle/blob/master/scripts/docs/verify-site.py): requires every `SUMMARY.md` chapter to exist, be rendered and indexed, detects missing search assets and checks legacy redirects.
- [`README.md`](https://github.com/spwn02/Miracle/blob/master/README.md): links the public Rust-style book and canonical documents.

**Legacy link compatibility:** The old MkDocs layout used `/topic/` directory URLs. mdBook uses `/topic.html`. `book.toml` now provides **20 native redirect pages** from the old directory-based URLs to the equivalent chapters. Generated redirects are verified in CI; they are not independent documentation copies.

The old `mkdocs.yml` was removed once mdBook had passed both rendering and deployment. `site/` remains ignored build output; no extra branch, paid service, API token, deploy key or custom domain is required.

## Observed validation and deployment

- [Successful mdBook build + GitHub Pages deployment, run #38993575669](https://github.com/spwn02/Miracle/actions/runs/38993575669): archive integrity check, mdBook build, chapter/search verifier, artifact upload and Pages deployment all passed.
- [Successful full redirect smoke test + Pages deployment, run #38993669295](https://github.com/spwn02/Miracle/actions/runs/38993669295): mdBook build, **21 canonical chapters**, **20 legacy redirects**, navigation/search checks, upload, deployment all passed.
- [Successful updated documentation deployment, run #38993715168](https://github.com/spwn02/Miracle/actions/runs/38993715168): latest operating-plan update built and published.
- Deployment logs reported the environment URL `https://spwn02.github.io/Miracle/`. The maintainer previously confirmed GitHub Pages publishing worked on the same repository.
- Newer changes to the README and removal of `mkdocs.yml` do not change the book source, binary, or render pipeline. The final workflow build will be checked after updating this progress ledger.

**Limit of verification:** CI checks generated pages and redirects. It does not independently test every link, screen reader behavior, mobile layouts, browser theme persistence, or whether a particular newly edited paragraph appears on the public CDN. Those remain explicit acceptance checks.

## Historical MkDocs baseline (superseded; retained for provenance)

Before the mdBook request, G3 initially shipped a MkDocs 1.6.1 configuration. The [strict-build run #38992585203](https://github.com/spwn02/Miracle/actions/runs/38992585203) passed `mkdocs build --clean --strict` and the original smoke check. It originally failed at GitHub Pages configuration until the maintainer enabled **Settings → Pages → Source: GitHub Actions**. The subsequent [Pages run #38993190545](https://github.com/spwn02/Miracle/actions/runs/38993190545) succeeded and established initial publication.

This historical evidence does **not** describe the current renderer. The old `mkdocs.yml` remains available only in earlier Git commits, for example [the last pre-migration baseline](https://github.com/spwn02/Miracle/blob/0f22f90074b8f69a82c9ecfebe6589f746abbdc1/mkdocs.yml).

## G2 integration check

A real publishing task, [issue #8](https://github.com/spwn02/Miracle/issues/8), triggered two automatic `added_to_project_v2` events on creation. The issue's Project membership and its practical status in **Miracle Development Project #3**, not merely the separate **Roadmap** Project, should be confirmed via:

```sh
gh issue view 8 --repo spwn02/Miracle --json projectItems
```

Issue #7 (historic frozen-contract migration) remains unscheduled and no development contracts are silently rewritten.

## Publishing implementation closeout evidence (2026-10-17)

The maintainer confirmed that the GitHub Pages site **works** and later explicitly approved the Rust Book-style visual presentation (“Amazing, this is gorgeous! Let's continue now.”). The site has remained automatically published from canonical `docs/` edits without any duplicate Wiki-editing procedure.

After publication, further real canonical edits (including this G3 ledger, new G4 book chapters and security updates) triggered successful builds and deployments through the same `docs.yml` workflow. Most recently the site was rebuilt and deployed on [commit `2ec2473`](https://github.com/spwn02/Miracle/commit/2ec2473aac07d23501a179a37ee1e699db5226dd) in [run #38999347401](https://github.com/spwn02/Miracle/actions/runs/38999347401). GitHub's deployment result and the maintainer's observed live site support that publishing is functional, not only configured. The existing verifier covers canonical Markdown chapters, navigation inventory, search artifact and historical MkDocs URL redirects.

**Publishing subtask [#8](https://github.com/spwn02/Miracle/issues/8) was closed `completed`** on that evidence. This does **not** imply complete G3 program acceptance: we have not independently demonstrated a full keyboard/screen-reader/mobile regression, every outbound link's validity, or a client-observed render of a particular most-recent paragraph. Those stricter experience checks remain with parent [#3](https://github.com/spwn02/Miracle/issues/3), instead of inventing a browser validation result.

## Assistive semantics audit and fixes — real Chrome/axe validation (2026-10-20)

The maintainer requested a **screen-reader accessibility check**. The connected execution environment cannot operate or hear the maintainer's **Orca, VoiceOver, or NVDA** desktop session, so **actual screen-reader speech and interaction have NOT been verified**. Rather than mislabeling a static HTML analysis as an assistive-technology test, [PR #22](https://github.com/spwn02/Miracle/pull/22) added a permanent, independently executed **Chrome accessibility-tree and axe-core WCAG semantics regression audit** for built mdBook pages.

**Initial failing findings, genuinely observed:** invalid `aria-expanded` on mdBook's sidebar `<label>`, insufficient Rust-theme link contrast, undersized navigation/header click targets, links distinguished only by color, and unlabeled read-only task-list checkboxes. Corrective `theme/accessibility.css` and `theme/accessibility.js` overrides (loaded through `book.toml`) now provide contrasting/underlined links, larger targets, an accessible/keyboard-operable sidebar button, and descriptive names for checked/incomplete documentation task states. Canonical Miracle C++ contract content was not rewritten.

**Evidence:** Reviewed [PR #22](https://github.com/spwn02/Miracle/pull/22) merged as [`5d0bfbd`](https://github.com/spwn02/Miracle/commit/5d0bfbd512fdf2643263ee4176fe67c8cc3bf5fe). [PR-head documentation run #39379034043](https://github.com/spwn02/Miracle/actions/runs/39379034043) and [reference CI #39379034086](https://github.com/spwn02/Miracle/actions/runs/39379034086) were **SUCCESS**. The actual post-merge [Pages run #39379255036](https://github.com/spwn02/Miracle/actions/runs/39379255036) was also **SUCCESS**: built pages passed **0 axe-core A/AA violations** across desktop homepage (**29 passing rule checks**), representative G3 chapter (**29**), and mobile homepage (**28**). The Chrome Accessibility Tree contained main/navigation regions, headings, named controls and links. A real Chromium keyboard test confirmed that **Enter and Space** operate the sidebar toggle and update `aria-expanded`; existing public Pages source/link parity checks passed after deployment.

**Scope limit:** Axe reported **2 incomplete/manual-review results** for each desktop page and **1** for mobile; `0 violations` is *not* a comprehensive proof of WCAG compliance. Chrome's accessibility tree tests semantics, but **does not listen to speech output**, establish usability with real assistive technology, or test every chapter/theme/browser. The G3 screen-reader and author sign-off gates stay **UNCHECKED**.

**Required final manual screen-reader observation:** On the live [Miracle Book](https://spwn02.github.io/Miracle/), use a real screen reader to navigate **landmarks/headings, table of contents and sidebar, link/button names, Enter/Space state announcements, search and search result announcements, and checked/incomplete checklist semantics**. Record device, browser, screen reader and actual observed announcements/issues; do not fabricate successful outcomes.

## Public Pages source parity and outbound links — GitHub Actions VERIFIED (2026-10-20)

The genuine [G3 post-deployment probe PR #21](https://github.com/spwn02/Miracle/pull/21) was reviewed and merged with a **history-preserving merge commit** [`723172c`](https://github.com/spwn02/Miracle/commit/723172c63229154840219fb21d9ab8d7559c17bc). Its [PR-head mdBook job #39376457444](https://github.com/spwn02/Miracle/actions/runs/39376457444) and [reference C++26 CI #39376457502](https://github.com/spwn02/Miracle/actions/runs/39376457502) passed; the offline source/HTML negative fixtures passed in the mdBook job.

**Crucially, the actual post-merge [GitHub Pages run #39376713839](https://github.com/spwn02/Miracle/actions/runs/39376713839) succeeded**. Its **Publish GitHub Pages** job `118476429713` executed the HTTP checks **after** successful deployment, not merely against a generated build artifact. Job logs explicitly confirm:

- `PASS: live canonical source content /infrastructure/G3-PROGRESS.html` — an actual paragraph about keyboard navigation was derived from canonical `docs/infrastructure/G3-PROGRESS.md` and found in the public HTML response.
- `PASS: live canonical source content /` — the module/target paragraph derived from `docs/index.md` was present in the public homepage HTML. The rendered page also contained the three expected outgoing links defined in that source.
- `PASS: live outbound link .../tree/master/docs`
- `PASS: live outbound link .../blob/master/CONTRIBUTING.md`
- `PASS: live outbound link .../issues/new/choose`
- `PASS: public Pages source-to-render comparison and 3 external links`.

The probe uses only Python's standard library, a cache-busting query and bounded retries, and runs under the existing **master Pages workflow**. It independently verifies **real network-visible publication and representative link resolution**; it is **not** an exhaustive link crawler, comprehensive browser accessibility test or screen-reader audit. Its success does not imply every chapter, every external destination or every environment behaves correctly.

**Acceptance:** The specifically requested **canonical-source-to-live-paragraph comparison and representative external link spot-check are verified**. [G3 #3](https://github.com/spwn02/Miracle/issues/3) remains OPEN solely for the remaining accessibility assessment/known limitations and explicit maintainer full-G3 acceptance; child publishing issue #8 stays completed.

## Live user-experience verification — maintainer report (2026-10-20)

The maintainer independently used the deployed [Miracle Book](https://spwn02.github.io/Miracle/) and explicitly reported: **“Tab, sidebar, search, resize to a mobile width, navigation and search results all work as expected”**. This is **firsthand owner-reported browser verification**, not a synthetic CI assertion. It covers:

- **Tab / keyboard navigation:** reported working.
- **Sidebar and chapter navigation:** reported working.
- **Search interaction and actual search results:** reported working.
- **Responsive/mobile-width layout and navigation after resizing:** reported working.

The report makes no claim about a specific screen reader or assistive technology, alternate browser/OS, every interactive control at every viewport, exhaustive outbound links, or byte-for-byte agreement of a newly updated rendered paragraph with the canonical Markdown source. The site verifier and prior deployment jobs independently cover source chapter generation, search artifacts and redirects, but cannot substitute for those remaining manual checks.

**Acceptance:** Mark the owner-tested keyboard, sidebar, search and responsive checks **PASSED**. **Do not** mark a full screen-reader accessibility audit, link crawl, or final G3 approval complete. [Parent issue #3](https://github.com/spwn02/Miracle/issues/3) stays open pending the separate checks and explicit maintainer G3 sign-off.

## Remaining G3 acceptance

- [x] Read-only publication from canonical `docs/`, without duplicating frozen contracts.
- [x] Native mdBook Rust-style theme, search, sidebar and chapter manifest configured.
- [x] Official mdBook v0.5.4 binary integrity verified on runner.
- [x] All 21 chapters and 20 legacy redirects validated in successful Actions builds.
- [x] Pages deployment completed and the actual Pages environment URL recorded.
- [x] MkDocs retired and README/local build guidance migrated to mdBook.
- [x] GitHub Wiki remains optional and nonauthoritative.
- [x] Maintainer confirms live site works and approves the Rust Book visual design; theme/sidebar/search generation is tested in CI.
- [x] **Maintainer-reported hands-on pass:** Tab/keyboard navigation, sidebar and chapters, actual search results, and mobile-width responsive behavior (2026-10-20).
- [x] Automated **Chromium accessibility tree + axe-core WCAG** audit on 3 desktop/mobile samples, **0 automated violations** and keyboard sidebar state check (post-merge Pages run #39379255036); small number of incomplete manual-review results reported.
- [x] **Real screen-reader interaction — maintainer-reported pass:** following explicit headings/landmarks, sidebar, search/results, and task-status checklist, owner replied "Yes, it works flawlessly" (2026-10-20). **Apple VoiceOver with Safari** confirmed in maintainer follow-up; exact device/OS and verbatim spoken announcements not recorded; coverage is bounded.
- [x] **Real public HTTP spot-check passed after deployment:** two source-derived paragraphs matched live Pages HTML, and **3/3** selected external links resolved in [run #39376713839](https://github.com/spwn02/Miracle/actions/runs/39376713839) (2026-10-20).
- [x] Genuine canonical docs edits automatically generated and deployed new Pages versions, without duplicate editing; post-deployment HTTP runner also confirmed specific published text against canonical source (run #39376713839).
- [x] Maintainer explicitly **approved G3** and accepted the recorded limited axe coverage, representative-link/site checks and lack of exhaustive WCAG certification (2026-10-20). Parent #3 is closed completed; publishing subtask #8 was already closed.

**G3 IS FORMALLY ACCEPTED (2026-10-20).** The maintainer approved the deployed mdBook documentation, practical Apple VoiceOver + Safari interaction and bounded automated/manual evidence, with the listed accessibility/scope limits retained. Both G3 issues #3 and #8 are completed. This does not indicate complete WCAG conformance or acceptance of G6.
