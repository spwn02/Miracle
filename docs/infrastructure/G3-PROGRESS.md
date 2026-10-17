# Miracle GitHub Infrastructure — G3 Progress

**Updated:** 2026-10-17
**Program:** G0–G6
**Workstream:** G3 — Documentation Publishing
**Status:** Publishing subtask #8 CLOSED (completed); G3 parent #3 OPEN for independent accessibility/navigation acceptance
**Public documentation:** [The Miracle Book](https://spwn02.github.io/Miracle/)
**Authority:** [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Current plan:** [G3 Operating Plan](G3-OPERATING-PLAN.md)
**Tracking:** [#3 — G3 publishing](https://github.com/spwn02/Miracle/issues/3) · [#8 — Pages implementation](https://github.com/spwn02/Miracle/issues/8)

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

## Remaining G3 acceptance

- [x] Read-only publication from canonical `docs/`, without duplicating frozen contracts.
- [x] Native mdBook Rust-style theme, search, sidebar and chapter manifest configured.
- [x] Official mdBook v0.5.4 binary integrity verified on runner.
- [x] All 21 chapters and 20 legacy redirects validated in successful Actions builds.
- [x] Pages deployment completed and the actual Pages environment URL recorded.
- [x] MkDocs retired and README/local build guidance migrated to mdBook.
- [x] GitHub Wiki remains optional and nonauthoritative.
- [x] Maintainer confirms live site works and approves the Rust Book visual design; theme/sidebar/search generation is tested in CI.
- [ ] Full mobile/keyboard/screen-reader interaction and representative live search behavior still need explicit hands-on verification.
- [x] Genuine canonical docs edits automatically generated and deployed new Pages versions through Actions, with no manual mirrored edits; direct client-observed verification of an individual updated paragraph remains untested.
- [ ] Maintainer explicitly accepts **full G3** experience and records remaining browser accessibility/navigation checks on parent #3; publishing implementation #8 is complete.

**Full G3 is not formally accepted.** Close completed implementation subtask #8 only, while retaining parent #3 for outstanding accessibility/navigation verification and explicit G3 acceptance; do not infer screen-reader/keyboard coverage from CI.
