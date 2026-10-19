# G6 — Community and End-to-End Acceptance

**Status:** IN PROGRESS; Discussions enabled and required categories configured (screenshot + repo API), Q&A live answer flow and other acceptance pending
**Authority:** [GitHub Infrastructure Contract v1.0](GITHUB-INFRASTRUCTURE-CONTRACT.md), sections 1, 4, 9–12
**Tracker:** [G6 issue #6](https://github.com/spwn02/Miracle/issues/6)
**Evidence:** [G6 progress ledger](G6-PROGRESS.md)

## Goal and constraints

G6 completes Miracle's public collaboration surface and proves a **real** trace from actionable issue, design/contract review, implementation, actual CI verification and author acceptance to truthful release documentation. It does not ship new Miracle C++ APIs, require a release/tag for infrastructure-only work, create a paid service, or automatically mark a Project item `Accepted`.

The underlying authority remains repository-tracked contracts and the maintainer's explicit decision. GitHub Issues/Projects reflect work and decisions; neither issue closure nor green CI alone grants acceptance. **No fake issues, retrospective approvals, synthetic releases or unverified checklist ticks.**

## 1. Discussion categories — owner-administered

At the initial G6 audit on **2026-10-19**, the public repository API reported `has_discussions: false`. **Later that day**, the maintainer supplied a screenshot of the live **Manage discussion categories** screen, showing **Announcements**, **Ideas**, **Q&A** (**Answers enabled**), and **Show and tell**; optional **General** and **Polls** also exist. A fresh public repository API check then returned **`has_discussions: true`**. These are independent pieces of evidence for **activation and category configuration**, not a test of a real accepted answer.

The navigational links in [Support](../../SUPPORT.md) and the GitHub issue chooser use GitHub's conventional category slugs (`q-a`, `ideas`, `announcements`, `show-and-tell`). The available read-only integration could not independently retrieve individual category pages; the maintainer subsequently **confirmed that all four advertised links open correctly** in a real browser and supplied a screenshot of the selected empty **Ideas** category. The UI check is maintainer-reported. No fabricated Discussion was created.

Owner-facing follow-up:

Maintainer checklist:

1. **Completed, verified:** [Miracle Settings](https://github.com/spwn02/Miracle/settings) → **General** → **Features**, **Discussions** enabled (`has_discussions: true`).
2. **Completed, screenshot-verified:** [Discussion categories](https://github.com/spwn02/Miracle/discussions/categories) show **Q&A** (Answers enabled), **Ideas**, **Announcements**, and **Show and tell**. General/Polls are valid optional categories, not missing or additional required gates.
3. **Category navigation completed (maintainer-reported):** All four links open correctly. The owner supplied a screenshot of the Ideas category, which currently has no matching discussions. **Still pending:** Validate Q&A answer selection using an authentic question when one exists; no meaningless test content is required.
4. **Configuration and navigation verified:** category setup screenshot + API activation + maintainer-reported click-through of all four URLs. Actual accepted-answer behavior remains **unverified**.

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

`SUPPORT.md`, `.github/ISSUE_TEMPLATE/config.yml`, `README.md`, and contributor guidance now advertise the configured categories using conventional GitHub category URL slugs; navigation was subsequently confirmed by the maintainer opening all four category links. Route open-ended questions to Discussions while keeping **actionable** bugs, proposals, compiler defects and infrastructure requests in structured Issues. Product API proposals remain subject to formal design-contract approval even if discussed publicly.

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

- [x] Discussions enabled and four required categories visually confirmed; GitHub Q&A **Answers enabled** setting visible. **Still pending:** a real accepted-answer workflow (do not conflate configuration with behavior).
- [ ] Routing to live Discussions categories updated without regressing security or actionable issue intake.
- [ ] Actual issue → reviewed PR → verified commit/CI → docs publication → explicit acceptance trail recorded in [G6 ledger](G6-PROGRESS.md).
- [ ] Contributor forms, PR prepopulation, community standards and confidentiality routes inspected in GitHub UI, distinguishing absent private moderation inbox from private vulnerability reporting.
- [ ] Project #3 membership/views/automation and published mdBook chapter verified without manufactured status changes.
- [ ] Release documentation/provenance accurately marked as applicable or **not applicable**, without fabricated tags/releases.
- [ ] Author explicitly accepts G6. Only then close [issue #6](https://github.com/spwn02/Miracle/issues/6).

**Never close G6 merely because a static check passes or a PR merges.**
