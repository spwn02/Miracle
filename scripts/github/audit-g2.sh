#!/usr/bin/env bash
# Read-only audit of spwn02's Miracle Development GitHub Project #3.
# Requires authenticated gh (project scope) and Python 3; makes no changes.
set -euo pipefail

for tool in gh python3; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "Missing required command: $tool" >&2
    exit 1
  fi
done
gh auth status >/dev/null

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

# Collect raw JSON. An empty/invalid response or GraphQL error is never
# silently interpreted as a successful audit.
gh project view 3 --owner spwn02 --format json > "$tmp/project.json"
gh project field-list 3 --owner spwn02 --limit 100 --format json > "$tmp/fields.json"
gh project item-list 3 --owner spwn02 --limit 100 --format json > "$tmp/items.json"
gh issue view 7 --repo spwn02/Miracle --json projectItems > "$tmp/issue7.json"
gh api "repos/spwn02/Miracle/issues/7/events?per_page=100" > "$tmp/events.json"

query="$(cat <<'GRAPHQL'
query {
  user(login: "spwn02") {
    projectV2(number: 3) {
      title
      url
      public
      repositories(first: 20) {
        nodes { nameWithOwner }
      }
      views(first: 30) {
        nodes { name number layout filter }
      }
      workflows(first: 30) {
        nodes { name enabled }
      }
    }
  }
}
GRAPHQL
)"
gh api graphql -f query="$query" > "$tmp/metadata.json"

python3 - "$tmp" <<'PY'
import json
from pathlib import Path
import sys

root = Path(sys.argv[1])
issues_expected = set(range(1, 8))
expected_status = (
    "Backlog", "Design", "Ready", "Implementing",
    "Review", "Verification", "Accepted", "Blocked",
)
expected_views = {"All Work", "Active", "Roadmap", "Verification Queue"}
problems = []

def read_json(name):
    path = root / (name + ".json")
    raw = path.read_text(encoding="utf-8")
    if not raw.strip():
        problems.append(f"{name}: CLI returned empty output")
        return None
    try:
        return json.loads(raw)
    except json.JSONDecodeError as error:
        problems.append(f"{name}: invalid JSON: {error}")
        return None

def show(name, value):
    print(f"\n===== {name} =====")
    print(json.dumps(value, indent=2, ensure_ascii=False))

project = read_json("project")
fields_data = read_json("fields")
items_data = read_json("items")
issue7_data = read_json("issue7")
events_data = read_json("events")
graphql_data = read_json("metadata")

if isinstance(project, dict):
    show("PROJECT IDENTITY", {
        k: project.get(k) for k in ("number", "title", "url", "public", "closed")
    })
    if project.get("public") is not True:
        problems.append("Project is not confirmed public")

fields = fields_data.get("fields", []) if isinstance(fields_data, dict) else []
if not isinstance(fields, list):
    fields = []
    problems.append("Field list has unexpected schema")
show("PROJECT FIELDS", [
    {"name": f.get("name"), "type": f.get("type") or f.get("dataType"),
     "options": [o.get("name") for o in f.get("options", [])
                 if isinstance(o, dict)]}
    for f in fields if isinstance(f, dict)
])
status_fields = [f for f in fields if isinstance(f, dict)
                 and f.get("name") == "Status"]
if len(status_fields) != 1:
    problems.append(f"Expected one Status field, found {len(status_fields)}")
else:
    names = tuple(o.get("name") for o in status_fields[0].get("options", [])
                  if isinstance(o, dict))
    if names != expected_status:
        problems.append(f"Status options/order differ: {names}")

if isinstance(items_data, dict) and isinstance(items_data.get("items"), list):
    items = items_data["items"]
    issue_items = {}
    for item in items:
        url = (item.get("content") or {}).get("url", "")
        prefix = "https://github.com/spwn02/Miracle/issues/"
        if url.startswith(prefix):
            try:
                number = int(url.removeprefix(prefix))
            except ValueError:
                continue
            issue_items[number] = {
                "url": url, "status": item.get("status"),
                "phase": item.get("phase"), "area": item.get("area"),
            }
    show("ACTUAL MIRACLE ISSUE ITEMS", issue_items)
    for number in sorted(issues_expected - set(issue_items)):
        problems.append(f"Issue #{number} absent from Project #3 item list")
else:
    problems.append("Item-list response missing .items array")

if isinstance(issue7_data, dict):
    show("ISSUE #7 DIRECT PROJECT MEMBERSHIP", issue7_data.get("projectItems"))
else:
    problems.append("Issue #7 projectItems response could not be checked")

if isinstance(events_data, list):
    show("ISSUE #7 PROJECT AUTOMATION EVENTS", [
        {"event": e.get("event"), "actor": (e.get("actor") or {}).get("login"),
         "at": e.get("created_at")}
        for e in events_data if e.get("event") in
        ("added_to_project_v2", "project_v2_item_status_changed")
    ])

if isinstance(graphql_data, dict):
    if graphql_data.get("errors"):
        problems.append("GraphQL errors: " + json.dumps(
            graphql_data["errors"], ensure_ascii=False
        ))
    node = ((graphql_data.get("data") or {}).get("user") or {}).get("projectV2")
    if not isinstance(node, dict):
        problems.append("GraphQL response has no data.user.projectV2 object")
    else:
        names = [
            x.get("nameWithOwner")
            for x in ((node.get("repositories") or {}).get("nodes") or [])
        ]
        views = [
            {"name": x.get("name"), "layout": x.get("layout"),
             "filter": x.get("filter"), "number": x.get("number")}
            for x in ((node.get("views") or {}).get("nodes") or [])
        ]
        workflows = [
            {"name": x.get("name"), "enabled": x.get("enabled")}
            for x in ((node.get("workflows") or {}).get("nodes") or [])
        ]
        show("LINKED REPOSITORIES", names)
        show("PROJECT VIEWS", views)
        show("PROJECT WORKFLOWS", workflows)
        if "spwn02/Miracle" not in names:
            problems.append("Project does not appear linked to spwn02/Miracle")
        found_views = {v["name"] for v in views}
        missing_views = expected_views - found_views
        if missing_views:
            problems.append(f"Missing views: {sorted(missing_views)}")
        if not any(w["enabled"] and "auto" in
                   (w["name"] or "").lower() and "add" in
                   (w["name"] or "").lower() for w in workflows):
            problems.append("Auto-add workflow not confirmed enabled")

print("\n===== AUDIT CONCLUSION =====")
if problems:
    for problem in problems:
        print(f"UNVERIFIED / ACTION NEEDED: {problem}")
    print("G2 acceptance NOT established.")
    sys.exit(1)

print("Machine-readable gates passed. Check view filtering, column grouping")
print("and exact auto-add repository rule manually before formal G2 acceptance.")
PY
