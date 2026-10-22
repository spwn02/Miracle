# Miracle Development workspace

[Open Miracle Development Project #3](https://github.com/users/spwn02/projects/3) · [Open the interactive dependency whiteboard](graph.html) · [Workspace design and authority protocol](../infrastructure/MIRACLE-DEVELOPMENT-WORKSPACE.md) · [Tracking Issue #29](https://github.com/spwn02/Miracle/issues/29)

This is an **incremental, read-only pilot**, not yet a completed or accepted development workspace. GitHub Project #3 is the operational place for issue scheduling, backlog and execution. Approved/frozen contracts remain controlled in Git; Issue relationships define actual blocking dependencies.

## Available now on this branch

The [whiteboard](graph.html) displays the 172 validated historical catalog nodes with 150 chapter-containment links. Dragging, searching, filtering, panning and zooming change only the browser view. The initial checked-in [graph manifest](graph.json) **does not contain current Issues, Project statuses, actual blocker edges or live progress**. An edge labeled `contains` does not mean `blocks`.

Use the read-only graph generator to add genuine issue nodes and native dependency arrows:

```sh
python3 scripts/github/export-workspace-graph.py --self-test
python3 scripts/github/export-workspace-graph.py                   # catalog-only
gh auth status
python3 scripts/github/export-workspace-graph.py --live-issues     # read-only GitHub REST
```

For an offline reviewed snapshot with `issues: [{number, title, state, labels, blocked_by: [{number, html_url, title}]}]`, use `--issues-json <path>`. Specify `--output <path>` to avoid overwriting the checked-in manifest. **No** invocation writes to the remote Project, creates an Issue, changes dependencies, or edits frozen contracts. The export fails on malformed or cyclic recorded blockers instead of silently producing a misleading graph.

The full workspace migration proceeds through [Issue #29](https://github.com/spwn02/Miracle/issues/29): live Project configuration inspection, appropriate Project views and a deferred-work disposition, official MCP access, validated synchronization and explicit owner approval. G2's separate `wasAutomated` verification remains [open in Issue #2](https://github.com/spwn02/Miracle/issues/2).

For contracts already published, see the [frozen contract archive](../contracts/index.md); for existing G2 status and Project configuration, see [G2 progress](../infrastructure/G2-PROGRESS.md).
