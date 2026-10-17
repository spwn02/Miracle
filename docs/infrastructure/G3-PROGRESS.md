# Miracle GitHub Infrastructure — G3 Progress

**Updated:** 2026-10-17
**Program:** G0–G6
**Workstream:** G3 — Documentation Publishing
**Status:** mdBook 0.5.4 deployed successfully; visual/navigation acceptance and release of G3 remain pending
**Public documentation:** [The Miracle Book](https://spwn02.github.io/Miracle/)
**Authority:** [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Current plan:** [G3 Operating Plan](G3-OPERATING-PLAN.md)
**Tracking:** [#3 — G3 publishing](https://github.com/spwn02/Miracle/issues/3) · [#8 — Pages implementation](https://github.com/spwn02/Miracle/issues/8)

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

## Remaining G3 acceptance

- [x] Read-only publication from canonical `docs/`, without duplicating frozen contracts.
- [x] Native mdBook Rust-style theme, search, sidebar and chapter manifest configured.
- [x] Official mdBook v0.5.4 binary integrity verified on runner.
- [x] All 21 chapters and 20 legacy redirects validated in successful Actions builds.
- [x] Pages deployment completed and the actual Pages environment URL recorded.
- [x] MkDocs retired and README/local build guidance migrated to mdBook.
- [x] GitHub Wiki remains optional and nonauthoritative.
- [ ] Maintainer confirms dark/light theme, mobile and keyboard/sidebar navigation, in-book search and content appearance on the live site.
- [ ] A genuine canonical text edit is observed on the public deployed site without manual re-publication.
- [ ] Maintainer explicitly accepts G3 and records acceptance evidence on issues #3/#8.

**G3 has not yet been formally accepted.** Do not close tracking issues or imply browser-level accessibility and navigation acceptance from CI alone.
