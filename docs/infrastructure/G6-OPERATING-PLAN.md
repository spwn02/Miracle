# G6 — Community and End-to-End Acceptance

**Status:** IN PROGRESS; repository-source checks in implementation, Discussions requires owner action
**Authority:** [GitHub Infrastructure Contract v1.0](GITHUB-INFRASTRUCTURE-CONTRACT.md), sections 1, 4, 9–12
**Tracker:** [G6 issue #6](https://github.com/spwn02/Miracle/issues/6)
**Evidence:** [G6 progress ledger](G6-PROGRESS.md)

## Goal and constraints

G6 completes Miracle's public collaboration surface and proves a **real** trace from actionable issue, design/contract review, implementation, actual CI verification and author acceptance to truthful release documentation. It does not ship new Miracle C++ APIs, require a release/tag for infrastructure-only work, create a paid service, or automatically mark a Project item `Accepted`.

The underlying authority remains repository-tracked contracts and the maintainer's explicit decision. GitHub Issues/Projects reflect work and decisions; neither issue closure nor green CI alone grants acceptance. **No fake issues, retrospective approvals, synthetic releases or unverified checklist ticks.**

## 1. Discussion categories — owner-administered

At the initial G6 audit on **2026-10-19**, the public repository API returned `has_discussions: false`. GitHub Discussions was **not enabled**. The connected integration has no repository-settings or Discussions-category write actions; this must be performed by the maintainer. Do not add links in public navigation until the feature and categories actually exist.

Maintainer checklist:

1. Navigate to [Miracle Settings](https://github.com/spwn02/Miracle/settings) → **General** → **Features**, and enable **Discussions**.
2. Open [Discussion categories](https://github.com/spwn02/Miracle/discussions/categories) and configure four distinct routes: **Q&A** (answerable; accepted-answer behavior), **Ideas**, **Announcements** (maintainer news), and **Show & Tell** (community projects/examples). Remove or leave unrelated default categories only after review; do not claim exactly four categories unless inspected.
3. Confirm the public [Discussions page](https://github.com/spwn02/Miracle/discussions) renders the categories, and validate Q&A question/answer selection through an **authentic** question when available; do not post meaningless test content in public discussion threads.
4. Report the configured category names/slugs and screenshots or command output. Until this is proven, keep the G6 Discussions gate unchecked.

Optional read-only maintainer terminal check:

```sh
gh api repos/spwn02/Miracle --jq '.has_discussions'
gh api graphql -f query='query {
  repository(owner:"spwn02", name:"Miracle") {
    discussionCategories(first:20) {
      nodes { name slug isAnswerable }
    }
  }
}'
```

Only after real confirmation, update `SUPPORT.md`, `.github/ISSUE_TEMPLATE/config.yml`, `README.md`, and contributor guidance with **specific live category links**. Route open-ended questions to Discussions while keeping **actionable** bugs, proposals, compiler defects and infrastructure requests in structured Issues. Product API proposals remain subject to formal design-contract approval even if discussed publicly.

## 2. Existing community surface

G1 provides five issue forms under `.github/ISSUE_TEMPLATE/` (bug, compiler, feature, architecture, infrastructure), a PR review template, `CONTRIBUTING.md`, `SUPPORT.md`, `SECURITY.md`, `CODE_OF_CONDUCT.md`, and `ACCESSIBILITY.md`. G6 must preserve confidential security reporting and avoid implying a verified private misconduct inbox when none exists.

The source-level [community route guard](../../scripts/github/check-community-surface.py) checks those entry points and cross-links and runs regression/negative fixtures. This is **static repository verification**; it does not prove GitHub's live issue-form renderer, security reporting settings, keyboard/screen-reader behavior, moderator tooling, or actual confidential-report delivery. Those require a separate maintainer/browser check.

Existing parent acceptance issues remain independent:
[G1.5 #1](https://github.com/spwn02/Miracle/issues/1),
[G2 #2](https://github.com/spwn02/Miracle/issues/2),
[G3 #3](https://github.com/spwn02/Miracle/issues/3), and
[G4 #4](https://github.com/spwn02/Miracle/issues/4).
G5 #5 was accepted under the Docker-only amendment. G6 cannot silently close outstanding G1–G4 gates.

## 3. Genuine end-to-end evidence chain

Use the existing **G6 implementation issue #6**, rather than introducing a fictional test issue, as the active workflow trace:

| Stage | Acceptance evidence | Boundary |
| --- | --- | --- |
| Intake | [Issue #6](https://github.com/spwn02/Miracle/issues/6) actually exists with `type:infrastructure` and defined gates | Doesn't by itself demonstrate form rendering/required-field behavior |
| Design | This operating plan, locked [infrastructure contract](GITHUB-INFRASTRUCTURE-CONTRACT.md), and recorded maintainer review | New decisions require explicit maintainer approval |
| Implementation | A real focused PR referring to #6; reviewed file diff/commit SHA | Do not close the parent issue automatically |
| Verification | Successful relevant Actions **on the exact PR revision**, including source guard and mdBook navigation | Pending is not passing; CI doesn't replace visual UI acceptance |
| Repository integration | Merged PR and documented reference to reviewed SHA, with post-merge CI | PR merge alone doesn't imply G6 phase acceptance |
| Acceptance | Author decision recorded in canonical `G6-PROGRESS.md` and issue #6 | Project `Accepted` status isn't authority |
| Release documentation | Existing `CHANGELOG.md`/release notes are inspected and linked, or explicitly say `Not applicable — infrastructure-only change, no release` | Never generate a version tag, release or release-success claim without approval |
| Project/docs | Verify the issue is in the intended **Miracle Development Project #3**; check the public mdBook chapter and navigation | Repo API doesn't expose account-owned Project workflow configuration; maintainer must inspect |

Maintainer checks for Project #3, if not already independently evidenced:

```sh
gh issue view 6 --repo spwn02/Miracle --json projectItems
bash scripts/github/audit-g2.sh
```

The results must show the actual `Miracle Development` project, valid workflow settings, and the acceptance status only when the maintainer authorizes it. **Do not equate an auto-add event with Project #3 membership**.

## 4. Definition of done

- [ ] Actual Discussions enabled, four required categories visually and/or API verified; Q&A capability tested with a real thread.
- [ ] Routing to live Discussions categories updated without regressing security or actionable issue intake.
- [ ] Actual issue → reviewed PR → verified commit/CI → docs publication → explicit acceptance trail recorded in [G6 ledger](G6-PROGRESS.md).
- [ ] Contributor forms, PR prepopulation, community standards and confidentiality routes inspected in GitHub UI, distinguishing absent private moderation inbox from private vulnerability reporting.
- [ ] Project #3 membership/views/automation and published mdBook chapter verified without manufactured status changes.
- [ ] Release documentation/provenance accurately marked as applicable or **not applicable**, without fabricated tags/releases.
- [ ] Author explicitly accepts G6. Only then close [issue #6](https://github.com/spwn02/Miracle/issues/6).

**Never close G6 merely because a static check passes or a PR merges.**
