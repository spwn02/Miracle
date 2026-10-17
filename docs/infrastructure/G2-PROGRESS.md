# Miracle GitHub Infrastructure — G2 Progress

**Updated:** 2026-10-17
**Program:** G0–G6
**Current workstream:** G2 — Project, Roadmap, Issue Tracking
**Status:** Project created/linked and fields/labels/milestone provisioned (owner terminal evidence); issue import and views pending
**Authority:** [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md)
**Operating plan:** [G2-OPERATING-PLAN.md](G2-OPERATING-PLAN.md)

## Completed

- Read GitHub's current Projects, roadmap, auto-add, and CLI guidance.
- Published the proposed G2 schema with explicit authority boundaries, eight Status values, seven custom fields, and four saved views.
- Added [`scripts/github/bootstrap-g2.sh`](../../scripts/github/bootstrap-g2.sh) for a user-authorized, credential-free bootstrap from the maintainer's GitHub CLI session. The script also imports real issues #1–#6 and assigns only factual Phase/Area metadata; it does **not** assign acceptance, priority, risk or due dates.
- Opened real, current infrastructure tracking issues:
  - [#1 — G1.5 interactive acceptance](https://github.com/spwn02/Miracle/issues/1)
  - [#2 — G2 Project setup](https://github.com/spwn02/Miracle/issues/2)
  - [#3 — G3 Documentation publishing](https://github.com/spwn02/Miracle/issues/3)
  - [#4 — G4 Security / CI](https://github.com/spwn02/Miracle/issues/4)
  - [#5 — G5 Development environments](https://github.com/spwn02/Miracle/issues/5)
  - [#6 — G6 Discussions / end-to-end acceptance](https://github.com/spwn02/Miracle/issues/6)
- All new Issues describe **future or unfinished work**; no artificial historical issues or acceptance records were created.

## Bootstrap execution and recovery — 2026-10-17

The maintainer ran `--check` and `--apply`. Their terminal output confirms:

- Public user-owned Project [#3 — Miracle Development](https://github.com/users/spwn02/projects/3) was created.
- Project #3 was linked to `spwn02/Miracle`.
- Seven custom Project fields were created: Phase, Area, Priority, Risk, Target release, Start date, Target date.
- Nine repository labels were created and the `GitHub Infrastructure — G2` milestone was created.
- The initial `--apply` stopped at a Bash parsing error: `unexpected EOF while looking for matching '\''`. Issue import and metadata were **not** confirmed in that run.

The script's issue-seeding tail had been accidentally corrupted by a duplicated here-document and split quoted string. It was replaced in commit `42c8e14`, retaining the already-safe creation steps, and a Bash syntax validation step was added to CI in commit `a394bb8`. The revised issue-seeding code uses GitHub's explicit Project/item/field/option IDs, avoids duplicating existing items, and sets factual Phase/Area values for issues #1–#6 without manufacturing an acceptance status or deadlines.

**Resume command:** `git pull --ff-only origin master && bash scripts/github/bootstrap-g2.sh --check && bash scripts/github/bootstrap-g2.sh --apply`

**Not yet independently verified:** Successful execution of the repaired script against Project #3, the presence of all six issues in the Project, and the live saved views/auto-add workflow.

## Tooling limitation

The connected GitHub integration can create repository files and Issues, but has **no exposed action to create/manage user-owned Projects, Project fields/views, or repository labels/milestones**. It cannot access the maintainer's authenticated local `gh` session. As a result, no Project URL, labels, milestone or actual Project board is asserted to exist yet. The bootstrap script is not proof of deployment.

GitHub's Projects interface may need manual setup of the built-in Status options, views and auto-add workflow even after running the script. A Project without configured views or with no linked issue is not accepted.

## Maintainer bootstrap checkpoint

From the root of your updated Miracle checkout:

```bash
gh auth status
gh auth refresh -s project
bash scripts/github/bootstrap-g2.sh --check
bash scripts/github/bootstrap-g2.sh --apply
```

The `--check` command is read-only and `--apply` creates or reuses the public Project and creates missing fields/labels/milestone. The script is intentionally committed as a normal text file, so invoke it with `bash` rather than assuming executable permissions.

After bootstrap, configure the existing Status field and views in GitHub. Verify that the six actual issues were linked automatically, then enable a single auto-add rule for future repository issues. Keep dates empty unless justified.

## Verification still required

- [ ] Maintainer confirms that the project exists, is public, and is linked to the correct repository; record the actual URL.
- [ ] Field names, types and Status option order checked in Project settings.
- [ ] Repository labels and finite G2 milestone verified.
- [ ] Saved views, filtering and auto-add checked against real issue state.
- [ ] Existing issues #1–#6 entered into Project without fictional status.
- [ ] At least one real issue state transition inspected, with evidence.
- [ ] Final author acceptance and any contract amendments recorded.

## Next work

Once the Project exists, update this ledger with its URL and actual verification results. Then finish G2's acceptance gate and continue G3 documentation publishing. G1.5 interactive acceptance remains open in [G1-PROGRESS.md](G1-PROGRESS.md).
