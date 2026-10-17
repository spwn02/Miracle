# G3 — Documentation Publishing: Operating Plan

**Workstream:** G3, repository-backed documentation
**Status:** Implementation baseline, pending live Pages deployment and contract-change propagation verification
**Authority:** [GitHub Infrastructure Contract v1.0](GITHUB-INFRASTRUCTURE-CONTRACT.md), §5 and §10
**Tracker:** [G3 parent issue #3](https://github.com/spwn02/Miracle/issues/3) · [G3 publishing issue #8](https://github.com/spwn02/Miracle/issues/8)

## One canonical source

- Human-authored API usage, architecture, tooling and governance documents live in [`docs/`](../index.md).
- [`mkdocs.yml`](https://github.com/spwn02/Miracle/blob/master/mkdocs.yml) defines navigation and uses MkDocs **1.6.1**, a pinned direct build dependency. MkDocs is a documentation-only tool and does not become a Miracle C++ runtime/build dependency.
- The GitHub Pages site is **generated** in CI from `docs/`, not a separate editable body of contracts.
- The [landing page](../index.md) is editorial navigation and describes the actual documentation; it never replaces an approved contract.
- No generated C++ API reference is claimed; the existing API pages are hand-maintained guides whose claims require ordinary code/tests review.
- Frozen contracts must not be rewritten, backfilled, or silently upgraded. Their separate provenance-preserving migration is [issue #7](https://github.com/spwn02/Miracle/issues/7).

## Publishing architecture

```text
Git master: docs/**/*.md  +  mkdocs.yml
                |
          GitHub Actions
        docs.yml (read-only build)
                |
          MkDocs 1.6.1
                |
       site/ HTML + search index
                |
     artifact (not a git branch)
                |
    deploy-pages / github-pages
                |
      public documentation site
```

`docs.yml` builds on documentation-changing pushes to `master`, matching pull requests (build only), and explicit dispatch. Only an authorized `master` job uploads to and deploys on GitHub Pages. Deployment uses built-in GitHub OIDC, `pages: write` and `id-token: write` in the deployment job, without a long-lived token, deploy key, webhook, or separate publishing branch.

The generated static website is expected at `https://spwn02.github.io/Miracle/` once GitHub Pages is enabled. This address is a **predicted default**, not a verified live deployment.

`scripts/docs/verify-site.py` checks that **every** Markdown file under `docs/` produces a nonempty HTML page and that the navigation/search index exists. It does not pretend to validate compiler APIs or every hyperlink. Existing cross-tree Markdown references must be audited when polishing the site.

## Administration needed once

The maintainer must select **Repository → Settings → Pages → Build and deployment → Source: GitHub Actions** (not `Deploy from a branch`). GitHub Pages is a public publishing surface. The `github-pages` environment will be used for deployment; restricting it to `master` in repository environment protection is recommended. No custom domain, CNAME, paid provider or DNS change is necessary for G3.

After initial setup, commits under `docs/` should publish automatically without a manual export step. `git push` is the only required source-update action.

## Navigation and Wiki policy

The primary hierarchy is:

1. Overview and getting started.
2. Compiler/toolchain support and build capabilities.
3. API guides (Meta, Iter, Range, Diagnostics, Static string, Feature).
4. Architecture, approved development protocol and infrastructure contract.
5. Development Project/issues.
6. Links to contribution, security and real release/change history in Git.

The GitHub Wiki remains **optional**. It must not become an independent editable copy of locked contracts. A later index-only or generated read-only Wiki could link back to canonical pages; do not enable an empty wiki or manually duplicate docs just to satisfy a cosmetic checkbox.

## G3 acceptance gates

- [ ] The documentation build completes from a clean runner, and the smoke verifier succeeds.
- [ ] GitHub Pages is configured to deploy from **GitHub Actions**.
- [ ] The `github-pages` deployment job succeeds with least privilege and provides a real URL.
- [ ] Site is publicly accessible, navigation/search and links work, and mobile/keyboard browsing is usable.
- [ ] A genuine, reviewed canonical documentation edit appears on the published site automatically, with no duplicate manual edit.
- [ ] GitHub Wiki policy recorded and README/source-navigation links remain accurate.
- [ ] Maintainer verifies evidence and explicitly accepts the workstream.

The appearance of `docs.yml`, a workflow run, or a `site/` artifact alone does **not** establish G3 completion.

## Local preview

With Python 3.12 and an isolated environment:

```sh
python3 -m venv .venv-docs
. .venv-docs/bin/activate
python -m pip install "mkdocs==1.6.1"
mkdocs serve
```

To check output without publishing:

```sh
mkdocs build --clean
python scripts/docs/verify-site.py site
```

The rendered `site/` and local `.venv-docs/` are build artifacts, not new sources of truth; do not commit them.
