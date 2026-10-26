# Stage B — Authorized 152-to-20 native archive consolidation

**Authorization:** on October 26, 2026 the owner explicitly approved the exact [172-entry consolidation manifest](STAGE-B-PROPOSED-ARCHIVE-CONSOLIDATION-TARGETS.json) after verification of source-span containment: 20 retained, 152 consolidated. **Git is authoritative**; Project records are derived navigation copies. This procedure does not assert author acceptance of Miracle phases.

## Reversible execution

A fixed owner-authenticated Issue #69 GitHub Actions workflow, `stage-b-native-archive-consolidation.yml`, supports exactly three commands:

```text
/miracle-stage-b archive-plan-20261026
/miracle-stage-b archive-apply-20261026
/miracle-stage-b archive-restore-20261026
```

The workflow downloads and authenticates the exact [complete pre-migration original body artifact #12608038204](https://github.com/spwn02/Miracle/actions/runs/40217537233) with the *repository token only*, then transfers control to the preexisting Projects-scoped secret. It accepts no arbitrary Project item ID or shell statement. The exact manifest pins all 172 stable IDs, source paths, exact retained targets and native Project item node IDs.

### Plan gate

Before any write, the script verifies **all 172 actual current DraftIssue body hashes** against the pinned original source-bearing snapshots, immutable Project/DraftIssue node identities, author-approved target map, source-range containment, and all retained output sizes. It also reads the actual ProjectV2 item archival GraphQL schema. Any mismatch stops the run without mutation.

### Apply gate

1. Preserve all 172 original Project native identities and complete body snapshots in the independent backup. The repository historical catalog and its original claims remain untouched.
2. Update **only the 20 retained bodies** with source-provenance aliases for all subsumed stable IDs. For Architecture, expand the original overview from lines 3–18 to **the entire pinned 94-line source file** before archiving any other architecture chapter. Require SHA-256 post-write readback for every retained body.
3. Invoke **archiveProjectV2Item**, not deletion, for exactly 152 specified redundant native Project items. Check current Project ID, each exact item/DraftIssue identity, expected original canonical body SHA, and `isArchived=false` immediately before archiving. Independently re-read each archived node after the mutation to verify `isArchived=true` and unchanged body SHA.
4. Independently verify all 172 native item archival states and body hashes: exactly 20 visible archive drafts, 152 reversible archived drafts. The existing 18 repository Issues remain untouched. All 172 catalog entries remain as immutable historical provenance aliases in Git.

The result journal is uploaded after either success or partial failure, with original item IDs, target IDs, hashes and completed operations. This is **not** an atomic 152-item transaction: stop on failure and inspect the journal. No automatic destructive retry.

### Restoration

The exact owner-only restore command independently rechecks that each current native body is either its pinned original canonical body or the approved merged body. It reverses Project archiving with `unarchiveProjectV2Item`, restores the 20 original source-bearing snapshots, and verifies **172/172 original active identities** with their exact source-body SHA values. If any item has received an unreviewed edit, restoration stops rather than overwriting it. No recovery requires deleting DraftIssue content or making new Project identities.

**Important:** the old `stage-b-draft-rollout.py --plan-all` requires all 172 drafts to remain active, so it is no longer an appropriate post-merge readback; use the new exact 172-item archived-state verifier. This is a deliberate migration of the live Project presentation model and not a loss of canonical Git data.

Roadmap reconstruction from Git commits/tags and G0–G6 final infrastructure acceptance are **separate outstanding tasks**. Successful archive consolidation alone does not advance Issue #69 to `Verification` or `Accepted`.
