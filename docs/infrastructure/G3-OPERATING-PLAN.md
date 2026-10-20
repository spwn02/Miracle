# G3 — Documentation Publishing: Operating Plan

**Workstream:** G3 — repository-backed documentation
**Status:** mdBook content and browser AX/axe checks passed; owner reports real screen-reader usability works (software/browser unspecified). Explicit G3 approval and recorded accessibility scope limits remain open
**Authority:** [GitHub Infrastructure Contract v1.0](GITHUB-INFRASTRUCTURE-CONTRACT.md), §5 and §10
**Tracker:** [G3 parent issue #3](https://github.com/spwn02/Miracle/issues/3) · [Pages issue #8](https://github.com/spwn02/Miracle/issues/8)

## 1. Canonical documentation model

- The sole canonical content tree is [`docs/`](../index.md). GitHub commits retain the authority and history of every approved contract and C++ guide.
- [`book.toml`](https://github.com/spwn02/Miracle/blob/master/book.toml) defines the mdBook site configuration. It points the book source directly to `docs/` and writes generated HTML into ignored `site/`.
- [`docs/SUMMARY.md`](https://github.com/spwn02/Miracle/blob/master/docs/SUMMARY.md) is the human-maintained Rust Book-style sidebar and chapter manifest. It includes all existing documents, including infrastructure evidence, without copying or changing the technical contract content.
- The rendered book provides native mdBook sidebar, table of contents, in-book search, section navigation, edit-source links, and light/dark theme selection. The default `rust` theme and `navy` dark theme match Rust's familiar documentation conventions; no proprietary CSS framework is introduced.
- Reference and API pages are hand-maintained prose: **no generated authoritative C++ API reference is claimed**.
- The provenance-preserving migration of historically frozen Miracle contracts remains **separate, unscheduled issue [#7](https://github.com/spwn02/Miracle/issues/7)**.

## 2. Deployment and dependencies

```text
master: docs/**/*.md + docs/SUMMARY.md + book.toml
                         |
                     GitHub Actions
                    docs.yml (build)
                         |
                    mdBook v0.5.4
                  SHA-256-verified binary
                         |
                    site/*.html + search
                         |
                  GitHub Pages artifact
                         |
               deploy-pages (OIDC, master)
                         |
              https://spwn02.github.io/Miracle/
```

[The publishing workflow](https://github.com/spwn02/Miracle/blob/master/.github/workflows/docs.yml) runs on documented changes to `master`, PRs (build only), and workflow dispatch. It fetches the immutable [mdBook v0.5.4](https://github.com/rust-lang/mdBook/releases/tag/v0.5.4) `x86_64-unknown-linux-gnu` binary and verifies archive SHA-256:

```text
3f28de05dafca9d0f2eab99c662116b0e37b89b1d96a08f8f430b9eeae958cd7
```

No `cargo install`, LLVM rebuild, personal access token, GitHub Pages publishing branch, additional service, or custom domain is required in CI.

The build job requests read-only repository access. Only the `master` Pages deployment job receives `pages: write` and `id-token: write` for GitHub's managed publishing environment.

## 3. Rust Book navigation and legacy URLs

The chapter tree begins with Introduction, then Getting Started, Library Guides, Architecture & Contracts, and Infrastructure & Governance. The `SUMMARY.md` manifest must list every canonical Markdown document exactly once; `scripts/docs/verify-site.py` checks both coverage and generated nonempty chapter pages. New documentation pages require a deliberate chapter entry.

**URL migration:** mdBook uses `chapter.html` instead of MkDocs' previous `chapter/index.html`. The native `[output.html.redirect]` table in `book.toml` generates redirect pages for all earlier chapter paths, keeping existing shared MkDocs URLs working when readers follow the site's links.

This is a static documentation site. GitHub Wiki stays **optional** and may only index or automatically mirror canonical text; it must not contain independently edited copies of locked contracts.

**Real screen-reader report (2026-10-20):** After being asked to verify landmarks/headings, sidebar and toggle announcements, search/results and checklist state using a real screen reader, the maintainer replied **"Yes, it works flawlessly."** This is **maintainer-reported positive assistive-technology usability evidence** rather than independently recorded speech output. Product, OS, browser and exact announcements were not supplied. Axe's three-page/viewport scan still reported **2/2/1 incomplete checks**. Record these limitations and obtain explicit phase sign-off separately.

## 4. Verification and acceptance

The workflow runs mdBook compilation plus [the site verifier](https://github.com/spwn02/Miracle/blob/master/scripts/docs/verify-site.py). The verifier detects missing chapter source, incomplete sidebar listings, empty generated pages, broken legacy redirects, and absent search assets. It does **not** constitute full hyperlink crawling, screen-reader acceptance, or verification of C++ APIs.

G3 can be accepted only after all of the following:

- [ ] The pinned mdBook binary is checksum-verified and builds from a clean GitHub runner.
- [ ] Every chapter and legacy redirect is present; in-book search index is produced.
- [ ] GitHub Pages deployment succeeds and the public URL is observed in the actual deployment log.
- [x] Author reports correct Rust Book-style reading experience and actual responsive navigation/search results, including keyboard Tab and sidebar interaction (2026-10-20).
- [x] Verify representative external links and live canonical Markdown paragraphs by **post-deployment public HTTP** in [Pages run #39376713839](https://github.com/spwn02/Miracle/actions/runs/39376713839); 2/2 paragraphs and 3/3 outbound links passed.
- [x] Verify generated mdBook's **browser AX tree and axe-core WCAG A/AA samples** (PR #22, post-merge Pages #39379255036; zero automated violations, incomplete/manual-review items remain).
- [x] Maintainer reports successful **real screen-reader interaction** in response to targeted headings, sidebar, search, and checklist checks (2026-10-20). Assistive-tech product/browser not specified, no speech transcript or exhaustive coverage; theme switch was not newly tested.
- [ ] A real, reviewed canonical documentation edit appears on Pages automatically; no parallel manual Wiki edit was required.
- [ ] The removal of MkDocs configuration is reflected in README, workstream ledger and local preview guidance.
- [ ] Author explicitly accepts G3, and issues #3 and #8 are closed only with supporting evidence.

See [G3-PROGRESS.md](G3-PROGRESS.md) for current evidence. Historical MkDocs runs are recorded as migration history, not represented as current technology.

**Public-site verification:** [`scripts/docs/verify-live-site.py`](../../scripts/docs/verify-live-site.py) runs offline comparison/negative fixtures on pull requests. Following a successful `master` Pages deployment, it GETs the **public** G3 chapter and homepage, compares two source-derived paragraphs with rendered HTML, checks that three source-declared external URLs occur in the homepage HTML and verifies they resolve. The check passed in [run #39376713839](https://github.com/spwn02/Miracle/actions/runs/39376713839). Its scope is representative, not a full crawl or a screen-reader audit.

## 5. Local workflow

Install mdBook **v0.5.4** through a trusted package source or its official release (match the version, verify release integrity when using the archive), then run:

```sh
mdbook serve --open
```

For the same build verification that CI performs:

```sh
mdbook build
python3 scripts/docs/verify-site.py site
```

`site/` is generated output and must never be committed. Changes to an approved contract still require the ordinary Miracle development/acceptance process.
