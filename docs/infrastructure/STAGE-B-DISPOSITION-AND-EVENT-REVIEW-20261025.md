# Miracle Stage B — Exact Archive/Branch Disposition and Issue Event Evidence

**2026-10-25 snapshot; review only, no destructive action authorized.** Git sources are authoritative; Project DraftIssue bodies are derived. This document is not Project/Phase acceptance, a release declaration, or authorization to delete any item/branch.

## Verified source-bearing archive

- Full guarded migration: **172/172 historical DraftIssue bodies are canonical**, 171 new writes and one prior pilot; zero pending.
- Independent read-only Project audit: [run #40225857969](https://github.com/spwn02/Miracle/actions/runs/40225857969), [artifact #12611356997](https://github.com/spwn02/Miracle/actions/runs/40225857969/artifacts/12611356997); all 172 native SHA-256 values cross-verified against backed-up expected replacements.
- Original native Markdown bodies preserved in [backup artifact #12608038204](https://github.com/spwn02/Miracle/actions/runs/40217537233/artifacts/12608038204), sourced from [run #40217537233](https://github.com/spwn02/Miracle/actions/runs/40217537233), pinned at `cd2c02b2691f32f6a624c32bfdef2f162bdf1aad`. An independent copy is retained outside ephemeral Actions artifacts.
- **Exact 172-record disposition: KEEP every identity** listed in authoritative `docs/contracts/PROJECT-CATALOG.json`. **MERGE: none. ARCHIVE/DELETE: none.** No cleanup is justified merely by identical source Markdown in different archive roles.

Two potentially confusing exact source-range overlaps (all four native item identities preserved):

| Canonical Git source | KEEP: Phase stable ID / native Project item | KEEP: Contract stable ID / native Project item |
|---|---|---|
| `docs/phases/phase-5.4-frozen-contract.md:1` | `phase-5-4:phase:0001` / `PVTI_lAHOCp2Gs84Bna-3zhCjpMU` | `phase-5-4:contract:0001` / `PVTI_lAHOCp2Gs84Bna-3zhCjpQI` |
| `docs/phases/phase-5.6-execution-contract-v1.0.md:1` | `phase-5-6:phase:0001` / `PVTI_lAHOCp2Gs84Bna-3zhCjqhs` | `phase-5-6:contract:0001` / `PVTI_lAHOCp2Gs84Bna-3zhCjql8` |

Even exact byte identity of source ranges is **not** proof that different Phase/Contract navigation roles should be consolidated. A later destructive proposal requires a specific owner-approved keep/merge/delete target manifest, recovery export and independently verified staged application.

## Git branch audit — observed 41 branch tips

Live read-only [run #40226278006](https://github.com/spwn02/Miracle/actions/runs/40226278006) generated [artifact #12610439712](https://github.com/spwn02/Miracle/actions/runs/40226278006/artifacts/12610439712) with names, exact SHA, matching merged PR head, Git compare ancestry and status. **37** are merged / zero-ahead candidates for *future review*, **none authorized for deletion**.

### Retain without further action

| Branch | Observed head SHA | Reason |
|---|---|---|
| `master` | `6dc74b9b3c9c1df11dea47f6db3d99cfe93ea59c` | protected |
| `gcc` | `2822de260944093672d3eec2462be0a4e7c2401f` | protected; intentional compiler divergence |
| `infra/living-development-workspace` | `2b2eac4f4517bcf238d0bb6f7b2c7024ebe8ae60` | PR #30 unmerged, 15 ahead |
| `infra/stage-b-reviewed-batch-write-route` | `5720572ad22e59a0939c6be0a06e88cbccf0d687` | PR #75 squash-merged but 3 ahead by ancestry; content-equivalence needs separate verification |

### Exact 37 deletion *candidates*, not permission to delete

| Branch name | Audited exact SHA | Merged PR |
|---|---|---|
| `contracts/g2-native-archive-provenance-amendment-proposal` | `fa17774a81011c4621ef796b8b36271b7299393b` | #63 |
| `docs/frozen-contract-migration` | `88f95a0dafbdd350f1a97a1d84d9bc9974239560` | #28 |
| `docs/g2-20261023-provenance-repro` | `5b311582de1cc501f085668299bb5503d7d21890` | #52 |
| `docs/g2-workspace-stage-a-scope-20261025` | `d8e4fa8392d574310f6a8984d3bf54767033fc69` | #66 |
| `docs/g6-live-community-audit` | `886392a58413a1673464bb7f45b5509804e30b49` | #54 |
| `docs/native-contract-revision-flow` | `f39ef437262152338ac6f293d14bfc1fe8b2731d` | #46 |
| `docs/oauth-project-write-stage-a-20261025` | `ddbf716f4db9b4088b4dd0a12c3a11c9d6c66b15` | #65 |
| `fix/issue-relation-post-readback` | `87b20ba313f0b8b3c7a9c245a9e246d39b8f4cdd` | #39 |
| `fix/native-issue-id-range` | `f7481d72da2792c3274c88f18151d0f66ee5805e` | #38 |
| `fix/native-roadmap-command-routing` | `2619936d36fd3bbdce86ddc1c0ba87d5d6ac0988` | #44 |
| `fix/native-workspace-resolve-obsolete-blockers` | `5c43ffcf751c78d0b3cfb7cd3581c8baa68c22cf` | #67 |
| `fix/project-progress-evidence-regex` | `cf05d2849ad396183d6953fdc63728c513d3e8fd` | #47 |
| `fix/project-view-audit-fallback` | `e97494a78bd43d75de73a94336c29b5400e7b852` | #42 |
| `fix/stage-b-shallow-ancestry-validation` | `d7184ac826e9c73a87e4332c975de80d14a919bb` | #73 |
| `fix/workspace-subissue-explicit-parent-guard` | `740f0bca8ab625ddf8c64a52affbbfea56b15d51` | #59 |
| `infra/g2-project-event-provenance` | `b6f8fba56d3b01d635b163920ca02708183181d8` | #51 |
| `infra/g6-operating-plan-evidence-reconciliation` | `02a2a7e947e0268fbc28c898816fb6369b1f3894` | #55 |
| `infra/g6-release-provenance-evidence-ledger` | `082bc13ee4399b333cad3eeb69a2c8bf0dde37f3` | #57 |
| `infra/g6-release-provenance-readonly-audit` | `3b309e9fdfe8f4c59a522d2f245f0a16b11c5fe7` | #56 |
| `infra/guarded-project-progress` | `28a038fc07e6233d49865e90e4bce6369970c3e1` | #40 |
| `infra/live-g6-community-surface` | `5a897849490a7450c8980be50674f5cc1793124f` | #53 |
| `infra/native-issue-relations` | `9afaeb65977b9d3a03ac2f31dd947c8e2b24ced1` | #37 |
| `infra/native-project-issue-intake` | `94d71e44fd5b317dbe16d4aca431909e34c9df1d` | #50 |
| `infra/native-project-operations` | `34dc577998bd7678f372026392cd6ac440538d8a` | #32 |
| `infra/native-roadmap-scope` | `6abdc91920752574061658c7686c3825f929ed8f` | #43 |
| `infra/owner-workspace29-final-acceptance` | `170e8daa4777aa554a1022e659e2e804d31cf380` | #68 |
| `infra/project-archive-integrity-audit` | `2cea35cf99d107c4892fba0a64ab8469c6a06b67` | #48 |
| `infra/project-graphql-audit` | `64f2bd8b85d33b7445ffd999869a35f476eca2fc` | #33 |
| `infra/project-graphql-reconcile` | `0a89e40d8ea31012e66228c65171034e5c627f2b` | #34 |
| `infra/project-item-control` | `2c8316d9387f3c026d7af06b3b585003e93841cd` | #35 |
| `infra/project-view-config-audit` | `cbf02ed92b45b7018f06e8c8949a969298f6236c` | #41 |
| `infra/stage-b-bounded-draft-rollout` | `2073ba3cbe39cd6e7d3903ca2cadbeb190ab9789` | #74 |
| `infra/stage-b-fixed-draft-pilot-12608038204` | `c7ea86a2f6521c1158b4bd50ef9b05e9e79864a1` | #72 |
| `infra/stage-b-guarded-draft-body-sync` | `0c9b1bc3e9c2ed77bdf53886047b5a6b3512899a` | #71 |
| `infra/stage-b-source-manifest-audit` | `92358c242d8f8a1b334153e9b2b1d91c8f7cce03` | #70 |
| `infra/workspace-direct-projects-mcp-handoff` | `d2b34361d299c2836b6cf0b3b4e03cec44f44ecb` | #61 |
| `infra/workspace-user-projects-403-auth-reconciliation` | `24f77f6728d32b5249285bb80ee21fb71637d05b` | #64 |

**Pre-deletion gate:** obtain separate explicit owner approval for the *exact list*; re-read the branch ref and verify SHA has not moved, check merged PR and compare state, confirm no open PR uses it, preserve recovery refs and stop on any drift. Do not consider an audit candidate already deleted.

## Retrospective event timeline

The already-merged [Git/PR event ledger](STAGE-B-GIT-EVENT-LEDGER-20261025.json) captures **507 commits and 52 PRs at its own earlier snapshot** with separate author, committer, PR created/merge/close timestamps. [The accompanying native Issue event ledger](STAGE-B-ISSUE-EVENT-LEDGER-20261025.json) captures **182 events across 18 non-PR Issues** observed on October 25, including unexpected Issue #60 closure at `19:05:36Z` and corrective reopening at `19:34:16Z`. These event timestamps are **historical observations**, not Phase start/completion/owner-acceptance timestamps, and are not used to backfill Project Start/Target dates.

## Remaining Stage B gates

1. Review the exact disposition proposal and separately authorize any destructive targets; with presently available evidence, **keep all 172 archives**.
2. Review unresolved repository Issues/PRs and branch anomalies, including unmerged #30 and squash-merged but non-ancestor #75; retain until proven safe and approved.
3. Confirm Project-native final acceptance only after independent readback and owner review; Issue #69 remains Open/Implementing.
4. Keep Issue #60 Open/Deferred and the G2/G4/G6 independent gates intact.
