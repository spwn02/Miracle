# Miracle Development — MCP agent operations (pilot)

**Status:** proposed operating playbook for [Issue #29](https://github.com/spwn02/Miracle/issues/29), not permission to modify frozen contracts or declare phases complete.  
**Identity:** user-owned `spwn02` / [Miracle Development Project #3](https://github.com/users/spwn02/projects/3) / repo `spwn02/Miracle`.

## Connecting an MCP client

The [official GitHub MCP server](https://github.com/github/github-mcp-server) ships a `projects` toolset, which is **not** in its default set. Configure the MCP client to include `projects` alongside existing repository tools. For a locally managed server:

```sh
GITHUB_TOOLSETS="default,projects" github-mcp-server stdio
```

The command assumes an installed MCP server and existing secure authentication set up **outside the repository**. Do not commit token material, console output containing tokens, or a runnable example with embedded secrets. Read-only operations require `read:project`; Project mutations require appropriate `project` permissions. Do not assume an agent environment exposes these tools merely because a GitHub repository connector is installed.

Current official Projects MCP tools:
- `projects_get`: project/item/field/view detail queries, e.g. `method=get_project` and `project_number=3`.
- `projects_list`: list projects, Project #3 items/views/fields; for item lists request `field_names=["Status","Phase","Area","Priority","Risk","Target release","Start date","Target date"]` (or explicit field IDs), otherwise the tool returns only titles.
- `projects_write`: project item changes and views; prefer resolving items through the issue's `item_owner, item_repo, issue_number` instead of mutable titles. Use `method=update_project_item`, `updated_field={"name":"Status","value":"Ready"}` **only** after actual verification/authorization. Never run example writes on an invented issue.

The official `issues` toolset includes issue intake and native `issue_dependency_read` / `issue_dependency_write` where those tools are available in the installed server version. The backup for local maintainers is `gh` + GitHub's native REST APIs. Project view edits are subject to actual permissions and require an initial read-only audit. Our currently connected ChatGPT GitHub integration can manage Issues/PRs but does not expose Project V2 mutation methods.

## Mandatory agent loop

1. **Resolve authority:** inspect `docs/infrastructure/MIRACLE-DEVELOPMENT-WORKSPACE.md`, current Git contracts, Issue #29, G2 #2 and the current Project fields/views. Identify the explicit request and active acceptance gate.
2. **Read first:** list existing Issues, including closed relevant matches; inspect labels and native blockers; get Project #3 field definitions and the exact item by stable issue reference. Search for a duplicate before any create.
3. **Plan with evidence:** propose concise Issue title, type, impact, acceptance criteria, parent/relevant contract, intended state, blocker relationships and any *real* scheduling constraints. Do not synthesize priorities or deadlines.
4. **Write only authorized mutable data:** create/update the real Issue, native blocking relation or Project field; compare fields before/after. Reject circular dependencies or updates to `Accepted` without explicit owner approval.
5. **Contracts by PR:** if changing active approved semantics, open a contract delta issue and reviewable Git PR. Verify original source hashes remain unchanged. A Project text edit cannot bypass Git provenance or rewrite history.
6. **Audit & explain:** reread the changed Issue and Project item, verify the new dependency edge, and report exact URLs, what changed, what remains, and which checks/approvals are pending. For any failed API write, do not report success.

## Supported real-world workflows

**Discovery:** `"Found a bug in meta query"` → check duplicates → create `type:bug` Issue with reproducer and acceptance criteria → existing auto-add should place it in Project #3 Backlog (verify, do not assume) → link relevant contract chapter.

**Deferral:** `"We might build our own CodeQL-like analyzer"` → capture a genuine proposal only when authorized → state why deferred and a concrete revisit trigger → keep status as Backlog unless live schema includes a verified separate planning disposition; no imaginary target release or completion status.

**Planning:** `"Feature B cannot start until A"` → resolve both genuine issues → use native GitHub Issue `blocked by` relation B←A → verify via dependency-read API → graph exporter will render A→B as a *blocks* arrow. Never use only Issue-body prose as the authoritative edge when a native relationship exists.

**Implementation:** `"Begin Phase N checkpoint"` → inspect frozen/current contract, real prerequisite state and Project phase metadata → propose implementation PR and checks → progress to Verification with evidence → request explicit maintainer acceptance before setting Accepted.

**Documentation mirror:** an accepted contract revision is committed via PR, then linked to Project; frozen originals stay untouched. Reject concurrent Project/docs changes that do not share an approved revision or recorded reconciliation.

## Offline/read-only graph verification

```sh
python3 scripts/github/export-workspace-graph.py --self-test
python3 scripts/github/export-workspace-graph.py --check
python3 scripts/github/export-workspace-graph.py --live-issues --output /tmp/miracle-live-graph.json
```

The last command **reads** native GitHub REST dependencies using a locally authenticated `gh` CLI. It does not modify Issues, Project #3, committed historical sources or roadmap statuses. The initial shipped `docs/workspace/graph.json` is catalog-only. Whiteboard positioning changes are browser-local in this pilot.

## Rollout gate

We cannot claim agentic management of **Project #3** from this GitHub connector until a Projects-capable MCP client is configured and authenticated, a real Project item is read and safely updated, and the result is independently reread. Record the toolset/version, field/schema evidence and human acceptance in Issue #29. Keep the existing G2 `wasAutomated=false` metadata discrepancy separately open under Issue #2.

Sources: [GitHub MCP official toolsets and Project tool signatures](https://github.com/github/github-mcp-server), [native Issue dependency API](https://docs.github.com/en/rest/issues/issue-dependencies).
