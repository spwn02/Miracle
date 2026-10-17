# Miracle GitHub Infrastructure — G3 Progress

**Updated:** 2026-10-17
**Program:** G0–G6
**Workstream:** G3 — Documentation Publishing
**Status:** Site source and build verified; deployment blocked on maintainer's GitHub Pages enablement; end-to-end publication acceptance pending
**Authority:** [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Plan:** [G3 Operating Plan](G3-OPERATING-PLAN.md)
**Tracking:** [Issue #3 — G3](https://github.com/spwn02/Miracle/issues/3) · [Issue #8 — Pages implementation](https://github.com/spwn02/Miracle/issues/8)

## Committed G3 implementation

- [`docs/index.md`](../index.md) is the public documentation landing page. It describes actual pre-1.0 features and links to the canonical technical documents without copying their frozen contracts.
- [`mkdocs.yml`](https://github.com/spwn02/Miracle/blob/master/mkdocs.yml) configures the navigation and read-only Markdown rendering from the existing `docs/` directory.
- [`.github/workflows/docs.yml`](https://github.com/spwn02/Miracle/blob/master/.github/workflows/docs.yml) builds the site with a directly pinned `mkdocs==1.6.1` on `master` pushes, documentation PRs (build only), and manual dispatch. The deploy job requests only GitHub Pages/OIDC write permissions.
- [`scripts/docs/verify-site.py`](https://github.com/spwn02/Miracle/blob/master/scripts/docs/verify-site.py) checks every canonical Markdown file produces HTML plus overview/navigation and search-index smoke checks. It is **not** an API correctness or full hyperlink accessibility test.
- Out-of-`docs/` references in editable infrastructure progress ledgers were converted to direct GitHub source URLs to avoid MkDocs treating root repository files as missing documentation pages.
- The GitHub Wiki remains an **optional index or generated mirror**, not an independently managed source of contract truth.
- No domain, Pages token, webhook, generated API reference, or change to Miracle's C++ code was introduced.

## Actual GitHub Actions evidence

The first fully populated [documentation workflow run](https://github.com/spwn02/Miracle/actions/runs/38992378865) showed:

- [x] Checkout, Python setup, MkDocs 1.6.1 installation.
- [x] `python -m mkdocs build --clean`.
- [x] `python scripts/docs/verify-site.py site`.
- [ ] `actions/configure-pages@v5` **failed:** `Get Pages site failed: Not Found`. GitHub explicitly instructs enabling Pages and setting the build source to GitHub Actions.
- [ ] Artifact upload and deployment were skipped because Pages could not be configured.

The initial build logged nine **source-link warnings** from infrastructure progress documents that link to repository-root files outside `docs/`; these links were subsequently corrected in tracked Markdown. A clean warning-free build and actual public deployment still need verification after the latest commits.

The actual documentation URL remains **unverified** until a successful Pages deployment reports its `page_url`. The typical project URL `https://spwn02.github.io/Miracle/` must not be described as live yet.

## Maintainer action: enable Pages

In [repository Pages settings](https://github.com/spwn02/Miracle/settings/pages):

1. Under **Build and deployment → Source**, select **GitHub Actions**, not branch-based publishing.
2. Do not add a custom domain or extra secrets.
3. Optionally restrict the `github-pages` deployment environment to `master` in repository environment settings.

Then run the already-committed **Documentation (GitHub Pages)** workflow from Actions (workflow dispatch), or push a genuine documentation change to `master`. Confirm the build, artifact upload, and deployment jobs succeed.

## G2 integration check with genuine G3 issue

[Issue #8](https://github.com/spwn02/Miracle/issues/8) is a real publishing-work task, rather than a fake intake test. GitHub's issue timeline recorded **two automated `added_to_project_v2` events** immediately after issue creation. At least one target is probably the separately configured Roadmap project; project-specific membership cannot be inferred from these events. For G2 acceptance, verify:

```sh
gh issue view 8 --repo spwn02/Miracle --json projectItems
```

The output must include **Miracle Development** with `Backlog` or an appropriate active state. Check that the maintainer's changed `Auto-add` workflow uses repository `Miracle`, filter `is:issue`, and acceptance-sensitive automations are disabled/verified. No completed/accepted state is implied by the bot event.

## G3 acceptance checklist

- [x] Canonical site source and navigation committed; no rewritten contracts.
- [x] Documentation build and site smoke checks passed in GitHub Actions.
- [x] Pages workflow uses no personal token and separates read-only build from privileged deploy.
- [x] Optional Wiki policy documented.
- [ ] Updated build has zero unresolved cross-tree documentation-link warnings.
- [ ] GitHub Pages enabled with GitHub Actions publishing source.
- [ ] Site deployed and real public URL recorded.
- [ ] Public landing page, navigation/search and desktop/mobile/keyboard access verified.
- [ ] Real reviewed documentation change propagates from Git to Pages automatically with no manual copy.
- [ ] Author reviews evidence and explicitly accepts G3.

**Do not close issue #3 or #8 or claim a live website until Pages actually works and acceptance evidence is recorded.**
