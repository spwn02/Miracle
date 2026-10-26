# Stage B — Project-native historical development reconstruction

**Status:** pending verified owner-run deployment (2026-10-26). This is Git event provenance, not a claim of historical author acceptance or a phase schedule. Tracking: [Issue #69](https://github.com/spwn02/Miracle/issues/69).

The prior 172 DraftIssue history was consolidated to 20 source-complete archive cards. These cards remain **contract provenance**, not implementations or scheduled deliveries. A separate history projection is needed to display the actual chronological Git development rather than mistakenly interpreting publication of frozen Phase 5.4/5.6 contracts as their implementation dates.

## Native Project history model

Create **nine** uniquely identifiable Git-history cards using `scripts/github/stage-b-project-history.py`, with provenance-bearing Markdown and exact commit links:

| Identity | Date semantics | Evidence |
|---|---|---|
| Foundation source activity | first and last relevant **committerDate** | source/test/example Git path history |
| Diagnostics source activity | first and last relevant **committerDate** | source/test/example Git path history |
| Range source activity | first and last relevant **committerDate** | source/test/example Git path history |
| Iter source activity | first and last relevant **committerDate** | source/test/example Git path history |
| Meta source activity | first and last relevant **committerDate** | source/test/example Git path history |
| Diagnostics `v0.2.0` | referenced commit date; **not** tag-creation date | pinned lightweight tag `65759786418325047a3adffbdc1e9331fb55c490` |
| Range `v0.3.0` | referenced commit date | pinned tag `22ad01a4eeae19fa00713a23c778a5dddeb3f3cc` |
| Iter `v0.4.0` | referenced commit date | pinned tag `baa6b733b201a6eece3a3b39713d35c649b82a50` |
| Meta `v0.5.0` | referenced commit date | pinned tag `250b79d79698255ac81378cb0b16fc3b0e919fb4` |

The observed spans are **activity windows**, not dates on which work definitively began or ended. A single Git commit may be relevant to multiple areas. Unmatched commits remain in the raw evidence archive. Phase 5.4 and Phase 5.6 do **not** receive inferred start/end dates, since source-publication history alone does not establish those boundaries.

Use two **distinct native custom DATE fields**: `Observed Git first event` and `Observed Git last event`. These record historical **committer-date calendar days** only. Set each history card's native `Phase` text field to `Git History`. Existing Project `Start date`, `Target date`, Roadmap filter, Source Archive cards and all repository Issues remain untouched. This mapping explicitly separates historical Git events from planning fields and satisfies the chronology distinction required by the accepted G2 archive-provenance addendum.

Project history items are **not** automatically `Accepted`. Git milestone labels mean only that actual Git tags point to those commits; they do not certify a published GitHub Release or satisfy G4.

## Owner-only execution

On [Stage B #69](https://github.com/spwn02/Miracle/issues/69), use:

```text
/miracle-stage-b history-plan-20261026
/miracle-stage-b history-apply-20261026
/miracle-stage-b history-verify-20261026
```

The first command verifies the current complete Git history, exact four pinned tag refs, 20 historical archive + 18 Issue identity baseline, Project Date field names/types and any previous history cards; it makes no writes. The guarded apply creates at most two DATE fields and nine **distinct history-only native Project drafts**, sets 18 dates and nine `Phase` values, and independently rereads each mutation for identity, Markdown, date and label. The post-write verification rechecks **47 active Project items**, including all previous 38, and all nine expected histories. Partial work is idempotently resumed only when the exact original generated body, identity and expected existing field values still match. Every run uploads full marker/digest evidence; there is no deletion operation.

### Native Project views

Keep the existing forward-looking `Roadmap` unchanged. Create a separate `Historical Development` roadmap view with the saved filter `phase:"Git History"`. Roadmap requires date-field mapping to `Observed Git first event` and `Observed Git last event` via GitHub's **Date fields** view configuration. The available connected Projects view-mutation interface can create the roadmap layout and saved filter, but it does **not expose roadmap date-axis binding**. Verify the view's actual axes in GitHub; if they default to planning fields, **do not claim a complete timeline** until the historical axes are explicitly selected in the UI. A companion table view with both custom fields visible can show the native populated dates in the meantime.

## External completion gates

Stage B history synchronization is distinct from formal G0–G6 program acceptance. Outstanding G2 event automation provider semantics, G4 true protected-tag product release/GCC integration, G6 genuine Q&A/private vulnerability workflow and maintainer signoff remain independent. Stage B #69 remains Open/Implementing until its complete acceptance scope is verified.
