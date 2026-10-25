# Stage B — Exact Owner-Approved Branch Cleanup Runbook (2026-10-25)

**Authorization boundary:** The owner explicitly approved deletion of the **37 named branch refs with exact head SHAs and merged PR mappings** in [the reviewed historical disposition](STAGE-B-DISPOSITION-AND-EVENT-REVIEW-20261025.md) and the independent backup manifest from owner-authenticated [Actions #40232523562](https://github.com/spwn02/Miracle/actions/runs/40232523562), artifact ID `12613503581`. This approval does **not** extend to another branch, a different SHA, any Project draft or Issue, source files, releases, protected refs, or recovery tags.

## Fixed execution

The one-off workflow `.github/workflows/stage-b-approved-branch-cleanup.yml` accepts exactly one owner-authored Issue #69 comment:

```text
/miracle-stage-b delete-approved-37-20261025
```

It requires a repository/actor/Issue identity match and uses no arbitrary branch-name inputs, shell interpolation of comments or generalized ref deletion endpoint. Its GitHub Actions token requires `contents:write` and `pull-requests:read` **only for this restricted job**; it does **not** receive the separate native Project-scoped token. No workflow-dispatch entry point is exposed.

The pinned `scripts/github/stage-b-approved-37.json` contains exactly 37 approved triples (ref name, head commit SHA and merged PR), matches the prior complete read-only branch inventory and pins canonical reviewed disposition blob `1930abd1b385fec082eeaf686c8a95d5e244c089`.

The executable performs these steps, stopping on any unexpected state:

1. Preflight **all** approved branch tips against exact SHA, protection status, exact merged PR head, no open PR, and Git `master` ancestry. Verify both protected branches and three pre-existing retained branches exist; verify `master` descends from the owner approval baseline.
2. Create and independently read back **37 permanent-until-separately-approved recovery tags** `refs/tags/stage-b-recovery-20261025/01` to `.../37`, each pointing to its originally approved commit SHA. If a name already exists, it must point to the exact same SHA.
3. Re-run full preflight **after** recovery refs exist. Before each deletion, repeat per-ref/PR/ancestry/open-PR checks and submit a Git push with `--force-with-lease=refs/heads/<branch>:<approved_sha>` so a moved remote tip causes refusal rather than deletion. Immediately independently verify that the branch ref is gone and that its recovery tag remains.
4. Re-enumerate all branches and require exactly **37 deletions**, all five previously retained branches still present, and all recovery refs verified. Upload the full plan/outcome JSON with every target and recovery tag even if the run fails part-way through.

Deletion is **not** a global GitHub API transaction. If any batch fails, previous exact-lease deletions may have succeeded; inspect the workflow's saved JSON before manual retry. No script auto-retries an incomplete collection. Any unexpected ref movement is a conflict requiring new review.

## Recovery

Recovery tags are real Git refs, not merely an expiring Actions artifact. To restore an approved deleted branch, use its pinned original name/SHA from `stage-b-approved-37.json` and its matching tag, re-verify both, then create `refs/heads/<original-name>` pointing at the same commit. The recovery tags must **not** be deleted in this Stage B operation. Their later cleanup requires separate explicit owner authorization.

The original 172 Project archive bodies and their full Markdown backups, `master`, `gcc`, unmerged PR #30, and the two content-equivalent squash-merged work branches are outside this cleanup.

**Post-review bookkeeping:** Because the one-off execution workflow itself requires a review branch, `infra/stage-b-approved-37-prune` is *new* and **not** among the 37 owner-authorized deletions. It is retained unless independently reviewed and separately authorized for later removal. Successful cleanup therefore reduces the 42-branch approval baseline by 37, while adding this one implementation branch; expect 6 surviving branch refs, not 5, assuming no other concurrent additions.

This runbook records narrowly scoped authorization, not Stage B owner acceptance or any G2/G4/G6 milestone.
