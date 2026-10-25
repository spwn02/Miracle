# Miracle Stage B — Evidence-grounded Git/PR timeline (2026-10-25)

**Authority:** Read-only GitHub REST event observations. See the [full 507-commit and 52-pull-request evidence ledger](STAGE-B-GIT-EVENT-LEDGER-20261025.json). The ledger is pinned to protected master `af9fd4a4497c625a849e491070cbb00149e3c8eb` and contains each commit's distinct Git author and committer UTC timestamps and all observed PR create/merge/close events.

**Semantics:** Commit author time is not necessarily when the code landed. Committer time may change after rebases. PR creation/merge timestamps evidence public GitHub state transitions, **not** historical phase start, engineering completion, owner sign-off, tag or release. The Git log at this snapshot spans **2026-08-21 to 2026-10-25**, but this is not a reconstructed calendar of every earlier contract milestone. Native Issue event history is a separate follow-up. Do **not** use the retrospective dates as Project Start/Target scheduling fields.

## Recorded public pull-request events

| PR | Opened (UTC) | Outcome at snapshot | Merged / closed (UTC) | Head branch |
|---|---|---|---|---|
| [#9](https://github.com/spwn02/Miracle/pull/9) | 2026-10-17 02:25:13 | Merged | 2026-10-17 02:48:39 | `dependabot/github_actions/master/actions/upload-pages-artifact-5.0.0` |
| [#10](https://github.com/spwn02/Miracle/pull/10) | 2026-10-17 02:25:16 | Merged | 2026-10-17 02:47:33 | `dependabot/github_actions/master/actions/deploy-pages-5.0.1` |
| [#11](https://github.com/spwn02/Miracle/pull/11) | 2026-10-17 02:25:19 | Merged | 2026-10-17 02:46:50 | `dependabot/github_actions/master/actions/upload-artifact-7.0.2` |
| [#12](https://github.com/spwn02/Miracle/pull/12) | 2026-10-17 04:31:57 | Merged | 2026-10-17 05:07:53 | `g5/devcontainer-lifecycle-verification` |
| [#13](https://github.com/spwn02/Miracle/pull/13) | 2026-10-17 05:14:27 | Merged | 2026-10-17 05:19:47 | `g5/headless-nvim-clangd-acceptance` |
| [#14](https://github.com/spwn02/Miracle/pull/14) | 2026-10-19 03:00:26 | Merged | 2026-10-19 03:09:43 | `g5/docker-only-acceptance` |
| [#15](https://github.com/spwn02/Miracle/pull/15) | 2026-10-19 05:20:08 | Merged | 2026-10-19 05:25:33 | `fix/g5-cache-and-root-preflight` |
| [#16](https://github.com/spwn02/Miracle/pull/16) | 2026-10-19 05:50:05 | Merged | 2026-10-19 05:54:38 | `g6/community-acceptance-foundation` |
| [#17](https://github.com/spwn02/Miracle/pull/17) | 2026-10-19 07:06:00 | Merged | 2026-10-19 07:09:30 | `g6/discussions-navigation` |
| [#18](https://github.com/spwn02/Miracle/pull/18) | 2026-10-19 07:14:05 | Closed without merge | 2026-10-21 20:18:38 | `dependabot/github_actions/master/actions/checkout-7.0.1` |
| [#19](https://github.com/spwn02/Miracle/pull/19) | 2026-10-19 07:14:51 | Closed without merge | 2026-10-21 20:18:47 | `dependabot/github_actions/master/actions/configure-pages-6.0.0` |
| [#20](https://github.com/spwn02/Miracle/pull/20) | 2026-10-19 15:35:36 | Merged | 2026-10-20 03:56:13 | `g6/release-provenance-acceptance-audit` |
| [#21](https://github.com/spwn02/Miracle/pull/21) | 2026-10-20 05:43:51 | Merged | 2026-10-20 05:46:42 | `g3/live-published-provenance-check` |
| [#22](https://github.com/spwn02/Miracle/pull/22) | 2026-10-20 06:03:30 | Merged | 2026-10-20 06:14:32 | `g3/assistive-semantics-audit` |
| [#23](https://github.com/spwn02/Miracle/pull/23) | 2026-10-20 09:25:34 | Merged | 2026-10-20 09:50:42 | `g3/comprehensive-acceptance-hardening` |
| [#25](https://github.com/spwn02/Miracle/pull/25) | 2026-10-21 14:10:06 | Merged | 2026-10-21 14:19:25 | `g4/fix-workflow-guard-redos` |
| [#26](https://github.com/spwn02/Miracle/pull/26) | 2026-10-21 14:36:08 | Merged | 2026-10-21 14:44:42 | `g4/docs-authenticated-code-scanning-link` |
| [#27](https://github.com/spwn02/Miracle/pull/27) | 2026-10-21 18:58:48 | Merged | 2026-10-21 19:12:10 | `g4/codeql-file-coverage-audit` |
| [#28](https://github.com/spwn02/Miracle/pull/28) | 2026-10-21 21:30:07 | Merged | 2026-10-22 00:00:06 | `docs/frozen-contract-migration` |
| [#30](https://github.com/spwn02/Miracle/pull/30) | 2026-10-22 00:56:29 | Closed without merge | 2026-10-22 01:14:31 | `infra/living-development-workspace` |
| [#32](https://github.com/spwn02/Miracle/pull/32) | 2026-10-22 23:44:10 | Merged | 2026-10-23 00:07:06 | `infra/native-project-operations` |
| [#33](https://github.com/spwn02/Miracle/pull/33) | 2026-10-23 00:44:46 | Merged | 2026-10-23 00:48:48 | `infra/project-graphql-audit` |
| [#34](https://github.com/spwn02/Miracle/pull/34) | 2026-10-23 00:51:39 | Merged | 2026-10-23 00:56:13 | `infra/project-graphql-reconcile` |
| [#35](https://github.com/spwn02/Miracle/pull/35) | 2026-10-23 01:02:03 | Merged | 2026-10-23 01:06:58 | `infra/project-item-control` |
| [#37](https://github.com/spwn02/Miracle/pull/37) | 2026-10-23 01:16:43 | Merged | 2026-10-23 01:23:21 | `infra/native-issue-relations` |
| [#38](https://github.com/spwn02/Miracle/pull/38) | 2026-10-23 01:24:33 | Merged | 2026-10-23 01:28:52 | `fix/native-issue-id-range` |
| [#39](https://github.com/spwn02/Miracle/pull/39) | 2026-10-23 01:30:51 | Merged | 2026-10-23 01:34:44 | `fix/issue-relation-post-readback` |
| [#40](https://github.com/spwn02/Miracle/pull/40) | 2026-10-23 01:43:12 | Merged | 2026-10-23 01:47:14 | `infra/guarded-project-progress` |
| [#41](https://github.com/spwn02/Miracle/pull/41) | 2026-10-23 01:50:49 | Merged | 2026-10-23 01:55:34 | `infra/project-view-config-audit` |
| [#42](https://github.com/spwn02/Miracle/pull/42) | 2026-10-23 01:58:19 | Merged | 2026-10-23 02:02:21 | `fix/project-view-audit-fallback` |
| [#43](https://github.com/spwn02/Miracle/pull/43) | 2026-10-23 02:06:02 | Merged | 2026-10-23 02:09:55 | `infra/native-roadmap-scope` |
| [#44](https://github.com/spwn02/Miracle/pull/44) | 2026-10-23 02:11:45 | Merged | 2026-10-23 02:15:51 | `fix/native-roadmap-command-routing` |
| [#46](https://github.com/spwn02/Miracle/pull/46) | 2026-10-23 02:33:01 | Merged | 2026-10-23 02:43:41 | `docs/native-contract-revision-flow` |
| [#47](https://github.com/spwn02/Miracle/pull/47) | 2026-10-23 02:46:22 | Merged | 2026-10-23 02:50:08 | `fix/project-progress-evidence-regex` |
| [#48](https://github.com/spwn02/Miracle/pull/48) | 2026-10-23 02:59:00 | Merged | 2026-10-23 03:01:48 | `infra/project-archive-integrity-audit` |
| [#50](https://github.com/spwn02/Miracle/pull/50) | 2026-10-23 03:15:48 | Merged | 2026-10-23 03:19:57 | `infra/native-project-issue-intake` |
| [#51](https://github.com/spwn02/Miracle/pull/51) | 2026-10-23 03:37:21 | Merged | 2026-10-23 03:41:42 | `infra/g2-project-event-provenance` |
| [#52](https://github.com/spwn02/Miracle/pull/52) | 2026-10-23 03:44:14 | Merged | 2026-10-23 03:48:42 | `docs/g2-20261023-provenance-repro` |
| [#53](https://github.com/spwn02/Miracle/pull/53) | 2026-10-23 03:59:22 | Merged | 2026-10-23 04:04:23 | `infra/live-g6-community-surface` |
| [#54](https://github.com/spwn02/Miracle/pull/54) | 2026-10-23 04:06:04 | Merged | 2026-10-23 04:10:02 | `docs/g6-live-community-audit` |
| [#55](https://github.com/spwn02/Miracle/pull/55) | 2026-10-23 04:34:26 | Merged | 2026-10-23 04:38:07 | `infra/g6-operating-plan-evidence-reconciliation` |
| [#56](https://github.com/spwn02/Miracle/pull/56) | 2026-10-23 04:48:30 | Merged | 2026-10-23 04:53:09 | `infra/g6-release-provenance-readonly-audit` |
| [#57](https://github.com/spwn02/Miracle/pull/57) | 2026-10-23 04:55:08 | Merged | 2026-10-23 04:58:45 | `infra/g6-release-provenance-evidence-ledger` |
| [#59](https://github.com/spwn02/Miracle/pull/59) | 2026-10-23 14:56:40 | Merged | 2026-10-23 15:07:00 | `fix/workspace-subissue-explicit-parent-guard` |
| [#61](https://github.com/spwn02/Miracle/pull/61) | 2026-10-23 16:08:00 | Merged | 2026-10-23 16:11:58 | `infra/workspace-direct-projects-mcp-handoff` |
| [#63](https://github.com/spwn02/Miracle/pull/63) | 2026-10-23 17:45:16 | Merged | 2026-10-24 21:20:27 | `contracts/g2-native-archive-provenance-amendment-proposal` |
| [#64](https://github.com/spwn02/Miracle/pull/64) | 2026-10-23 17:52:24 | Merged | 2026-10-23 17:56:20 | `infra/workspace-user-projects-403-auth-reconciliation` |
| [#65](https://github.com/spwn02/Miracle/pull/65) | 2026-10-25 01:55:09 | Merged | 2026-10-25 02:09:56 | `docs/oauth-project-write-stage-a-20261025` |
| [#66](https://github.com/spwn02/Miracle/pull/66) | 2026-10-25 16:15:52 | Merged | 2026-10-25 16:19:56 | `docs/g2-workspace-stage-a-scope-20261025` |
| [#67](https://github.com/spwn02/Miracle/pull/67) | 2026-10-25 16:24:39 | Merged | 2026-10-25 16:28:55 | `fix/native-workspace-resolve-obsolete-blockers` |
| [#68](https://github.com/spwn02/Miracle/pull/68) | 2026-10-25 16:34:27 | Merged | 2026-10-25 16:38:03 | `infra/owner-workspace29-final-acceptance` |
| [#70](https://github.com/spwn02/Miracle/pull/70) | 2026-10-25 17:06:21 | Open | — | `infra/stage-b-source-manifest-audit` |

## Interpretation and omissions

The exact source record is the JSON ledger, not a hand-authored inferred phase timeline. This table is an event index. It does not claim that closed but unmerged PRs contributed code; PR merge times also do not prove a product release. Reconstructing historical Issue event dates, contract decision dates and chapter/phase provenance still requires their own authenticated evidence; do not silently fill missing values.

**No changes:** this report does not modify native Project dates, statuses, Issue histories or git refs.
