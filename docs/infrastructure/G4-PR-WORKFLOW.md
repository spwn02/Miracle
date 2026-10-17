# G4 — Pull Request Reviews and History-Preserving Merges

**Status:** Operating guidance for Miracle maintainers, not an automatic acceptance rule
**Workstream:** [G4 — Security and CI](https://github.com/spwn02/Miracle/issues/4)
**Applies to:** contributor and Dependabot PRs targeting `master`

## Review the actual change

Open the PR's **Conversation**, **Files changed** and **Checks** tabs. For Actions update PRs, inspect the exact version, SHA-pinned `uses:` replacement, changelog/breaking changes, permissions, and impact on reference CI, mdBook, and `gcc` sync.

Use comments with an explicit intention:
- `question:` request an explanation.
- `suggestion:` propose an optional improvement.
- `issue:` identify incorrect or unsafe behavior, including a reproduction.
- `blocking:` describe the specific condition that must pass before merge.
- `nit:` minor, nonblocking style issue.
- `praise:` highlight an important improvement.

Leave line-specific feedback in **Files changed**, or submit a collected review via **Finish your review → Comment / Approve / Request changes**. GitHub's inline ```suggestion``` blocks support a directly applicable replacement.

A PR's merge does **not** automatically mean its Project card is `Accepted`. Author acceptance follows the governing technical contract and evidence.

## Select a merge strategy that does not rewrite existing commits

Miracle's repository settings currently permit all three GitHub UI merge methods: merge commit, squash, and rebase. **Recommended:** choose **Create a merge commit** in the GitHub UI (or `gh pr merge <number> --merge`). GitHub preserves existing PR commit identities and records a merge point. Do not choose **Rebase and merge** or **Squash and merge** when the original commit history matters.

A **true fast-forward** is different: `git merge --ff-only` moves the `master` ref onto an existing descendant commit and makes no new commits. GitHub's normal PR merge button uses `--no-ff`; the UI does **not** offer true fast-forward-only merging.

Only attempt a local FF after review and checks:

```sh
gh pr checks 9 --repo spwn02/Miracle
gh pr checkout 9
# Note the actual local branch name created by gh.
git status --short
git log --oneline --graph -12

git switch master
git pull --ff-only origin master

# Replace <reviewed-local-branch> with the branch checked out above.
git merge --ff-only <reviewed-local-branch>
git push origin master
```

If `merge --ff-only` fails, it safely leaves `master` unchanged. **Do not** respond with `--force`, `git reset --hard`, `git push --force-with-lease`, or an unreviewed `rebase`.

Branches created earlier than the latest `master` may have diverged. A linear-history fast-forward is then impossible **without changing the graph**. The history-preserving default is a normal merge commit:

```sh
gh pr view 9 --repo spwn02/Miracle --json headRefOid
gh pr checks 9 --repo spwn02/Miracle
gh pr merge 9 --repo spwn02/Miracle --merge \
  --match-head-commit <reviewed-pr-head-sha>
```

`--match-head-commit` prevents merging if the PR head changed since review. Recheck failed/pending jobs rather than force-bypassing them. If conflicts occur, preserve history by merging the current `master` into the topic branch (and rerunning checks) or request a reviewed conflict resolution; do not silently rebase the branch.

If you absolutely require a fast-forward despite divergence, the PR branch can **first merge the current `master` into itself** using a normal merge commit; after review and branch CI this restores an ancestry path for `ff-only`. That preserves existing commit SHAs but introduces a merge commit *inside the PR branch*, which is later reachable from `master`. It does not yield a perfectly linear history.

## Current repository controls

- Active rulesets prohibit deletion and force-pushing to `master`/`gcc`, while allowing ordinary forward commits and reviewed merge commits.
- A merge commit is **not** a destructive history rewrite. Rebase/squash creates replacement commits; avoid these when preserving commit identity is important.
- Existing reference CI and specialized Pages/GCC workflows must be examined for changed files; branch protection currently does not mandate a GitHub check by name.
- Dependabot update PRs are authorized. Automatic PR **merges** are not.
- For safer UI defaults, the maintainer can turn **off** "Allow squash merging" and "Allow rebase merging" in **Settings → General → Pull Requests**, retaining "Allow merge commits". Do not change repository settings silently.

GitHub reference: [Pull request merge methods](https://docs.github.com/en/pull-requests/reference/pull-request-merges).
