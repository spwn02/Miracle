# G4 — Branch and Tag Protection Rules

**Status:** REVIEWED PROPOSAL — not yet applied to GitHub settings
**Owner:** repository maintainer `spwn02`
**Authority:** [GitHub Infrastructure Contract v1.0](GITHUB-INFRASTRUCTURE-CONTRACT.md), section 6
**Workstream:** [G4 issue #4](https://github.com/spwn02/Miracle/issues/4)
**Repository:** `spwn02/Miracle`

## Observed baseline

At audit on 2026-10-17, the repository reports:

- `master` and `gcc` both have `protected: false`.
- `GET /repos/spwn02/Miracle/rulesets` returned `[]` (no rulesets).
- The normal CI, GitHub Pages, Dependabot and non-forced `master` → `gcc` sync have all run successfully under the prior configuration.
- The maintainer's screenshot displays GitHub's `Your master branch isn't protected` warning.

**This document is a design and executable recipe, not evidence that protection is enabled.**

## Minimal protection without breaking current development

Two source-controlled JSON recipes are proposed:

| Ruleset | Target | Rules | Allowed normal behavior |
| --- | --- | --- | --- |
| [Protect Miracle development branches](https://github.com/spwn02/Miracle/blob/master/scripts/github/rulesets/protect-development-branches.json) | Exactly `refs/heads/master` and `refs/heads/gcc` | Restrict deletions; block non-fast-forward/force pushes | Ordinary fast-forward direct commits to `master`, merges and Dependabot PR merges, non-forced `gcc` merge/push |
| [Protect Miracle release tags](https://github.com/spwn02/Miracle/blob/master/scripts/github/rulesets/protect-release-tags.json) | Pattern `refs/tags/v*` | Restrict deletions; restrict updates | Initial creation of new version tags is allowed; rewriting or deleting existing version tags is blocked |

Both specify `enforcement: active` **in their prospective recipe** and an empty bypass list. No live ruleset has been created by committing these files.

**Intentionally not enabled:** require-all-changes-via-PR, mandatory reviews, required status checks, signed commits, linear history, restrict all updates, or branch-creation restrictions. Those would change the established direct-`master` authoring or `gcc` merge flow and must be evaluated separately with the exact CI job names and recovery procedure. In particular, `sync-gcc.yml` intentionally uses `git merge --no-ff`; enabling required linear history on `gcc` would break it.

This is **basic history and ref protection**, not proof of reviewed or passing code. Dependabot's PRs are never auto-merged by this policy.

## Review and optional application

Repository ruleset administration requires the maintainer's authenticated GitHub CLI with admin-level repository permissions, or the GitHub settings UI. The connected assistant integration cannot write GitHub rulesets.

Read-only inspection:

```bash
git pull --ff-only origin master
python3 scripts/github/reconcile-rulesets.py --check
```

To enact **the precise two reviewed recipes** after approving this G4 ruleset design:

```bash
python3 scripts/github/reconcile-rulesets.py --apply
python3 scripts/github/reconcile-rulesets.py --verify
```

`--check` and `--verify` never mutate GitHub. `--apply` is **opt-in**, creates missing rulesets, refuses to overwrite or silently repair existing rules with different content, and aborts if unrelated rulesets exist that may conflict. The script is idempotent for already matching rulesets and does not store credentials. No GitHub Actions token, new personal token, or temporary write-privileged workflow is needed.

The GitHub repository UI alternative is **Settings → Rules → Rulesets**. Create one branch ruleset targeting `master` and `gcc` with *Restrict deletions* and *Block force pushes*; create a separate tag ruleset matching `v*` with *Restrict deletions* and *Restrict updates*. Keep other restriction switches off. Verify the exact target refs and bypass list.

## Post-application evidence and rollback

Record the actual GitHub ruleset IDs/links and their enforcement states, then:

1. Re-run `--verify`, verify active rulesets in GitHub's **Settings → Rules → Rulesets**, and confirm their matching refs/rules.
2. Confirm a real ordinary, non-forced direct `master` commit succeeds and the **reference C++26 CI** passes.
3. Confirm the **Sync master to gcc** action still succeeds and actually advances the `gcc` branch when the candidate is compatible; do not treat a successful no-op convergence report as proof of a pushed update.
4. Confirm a new `v*` release tag can still be created through the normal release process when legitimately publishing; do not fabricate a release tag solely to test protection.
5. Check published tags are unchanged and GitHub shows deletion/rewrites as restricted. Avoid intentionally destructive test pushes to live refs.
6. Examine Dependabot major-update PRs and required checks separately. None should be auto-merged.

**Recovery:** A repository admin can temporarily disable the affected ruleset in **Settings → Rules → Rulesets** if a documented exceptional ref repair is essential, record the change and reason, perform the minimal recovery, then restore the original recipe and verify protections. Do not force-push or rewrite a release tag merely as a convenience.

Only report **G4 protection accepted** after the repository settings and positive behavior have been independently checked, not when these JSON files are committed.
