#!/usr/bin/env python3
"""Read-only integrity audit: canonical 172-record catalog ↔ native Project #3.

Uses direct authenticated GraphQL pagination. Does not create, edit, archive,
move or approve Project items or frozen contract files.
"""
from __future__ import annotations

from collections import Counter, defaultdict
import json
from pathlib import Path
import re
import runpy
import subprocess
import sys

PROJECT_ID = "PVT_kwHOCp2Gs84Bna-3"
CATALOG_PATH = Path("docs/contracts/PROJECT-CATALOG.json")
PAGE_SIZE = 20
MAX_ITEMS = 1000

QUERY = """
query($cursor:String) {
  user(login:"spwn02") {
    projectV2(number:3) {
      id
      items(first:20, after:$cursor) {
        totalCount
        pageInfo { hasNextPage endCursor }
        nodes {
          id
          type
          isArchived
          content {
            ... on DraftIssue { title body }
          }
          fieldValues(first:30) {
            totalCount
            nodes {
              ... on ProjectV2ItemFieldSingleSelectValue {
                name
                field { ... on ProjectV2SingleSelectField { name } }
              }
            }
          }
        }
      }
    }
  }
}
"""


class AuditError(RuntimeError):
    pass


def graphql(cursor: str | None) -> dict:
    args = ["gh", "api", "graphql", "-f", "query=" + QUERY]
    if cursor is not None:
        args += ["-f", "cursor=" + cursor]
    proc = subprocess.run(args, capture_output=True, text=True, check=False)
    if proc.returncode != 0:
        raise AuditError("GraphQL query failed; inspect GitHub Actions runner and token validity")
    try:
        response = json.loads(proc.stdout)
    except json.JSONDecodeError as exc:
        raise AuditError("GraphQL returned malformed JSON") from exc
    if response.get("errors"):
        # Avoid surfacing potentially sensitive GraphQL error details.
        raise AuditError("GitHub GraphQL reported a schema or permission error")
    project = (((response.get("data") or {}).get("user") or {}).get("projectV2") or {})
    if project.get("id") != PROJECT_ID:
        raise AuditError("Project identity mismatch")
    conn = project.get("items") or {}
    page = conn.get("pageInfo") or {}
    if (type(conn.get("totalCount")) is not int or conn["totalCount"] > MAX_ITEMS or
            not isinstance(conn.get("nodes"), list) or len(conn["nodes"]) > PAGE_SIZE or
            type(page.get("hasNextPage")) is not bool):
        raise AuditError("Invalid or truncated Project items page")
    if page["hasNextPage"] and not page.get("endCursor"):
        raise AuditError("Truncated page missing a continuation cursor")
    return conn


def collect(get_page) -> list[dict]:
    cursor, seen, items, total = None, set(), [], None
    while True:
        data = get_page(cursor)
        if total is None:
            total = data["totalCount"]
        if total != data["totalCount"]:
            raise AuditError("Project item count changed during audit; rerun on stable state")
        items.extend(data["nodes"])
        if len(items) > MAX_ITEMS:
            raise AuditError("Unbounded Project enumeration")
        page = data["pageInfo"]
        if not page["hasNextPage"]:
            if len(items) != total:
                raise AuditError("Project list length does not match totalCount")
            break
        next_cursor = page["endCursor"]
        if next_cursor in seen or next_cursor == cursor:
            raise AuditError("GraphQL cursor repeated")
        seen.add(next_cursor)
        cursor = next_cursor
    ids = [i.get("id") for i in items]
    if any(not isinstance(i, str) or not i for i in ids) or len(set(ids)) != len(ids):
        raise AuditError("Duplicate/missing Project item IDs")
    return items


def field_values(item: dict) -> dict[str, str]:
    connection = item.get("fieldValues") or {}
    count = connection.get("totalCount")
    entries = connection.get("nodes")
    if type(count) is not int or count > 30 or not isinstance(entries, list) or len(entries) != count:
        raise AuditError("Project item field values are incomplete")
    fields = {}
    for entry in entries:
        if not isinstance(entry, dict) or not isinstance(entry.get("name"), str):
            continue
        name = (entry.get("field") or {}).get("name")
        if isinstance(name, str):
            if name in fields:
                raise AuditError("Duplicate select value for field " + name)
            fields[name] = entry["name"]
    return fields


def compare(catalog: list[dict], project_items: list[dict], source_body) -> list[str]:
    messages = []
    expected = {entry["title"]: entry for entry in catalog}
    if len(expected) != len(catalog):
        raise AuditError("Canonical catalog has duplicate titles")

    archive = defaultdict(list)
    for item in project_items:
        content = item.get("content") or {}
        title = content.get("title")
        if item.get("type") == "DRAFT_ISSUE" and isinstance(title, str) and title.startswith("[Archive/"):
            archive[title].append(item)

    for title in sorted(archive.keys() - expected.keys()):
        messages.append("EXTRA archive draft not in catalog: " + title)

    for entry in catalog:
        title = entry["title"]
        matches = archive.get(title, [])
        if len(matches) != 1:
            messages.append(f"{entry['id']}: expected exactly one draft item, observed {len(matches)}")
            continue
        item = matches[0]
        if item.get("isArchived") is not False:
            messages.append(entry["id"] + ": draft unexpectedly archived")
        content = item.get("content") or {}
        body = content.get("body")
        if not isinstance(body, str):
            messages.append(entry["id"] + ": body absent")
            continue
        desired = source_body(entry, "master")
        required = [
            "<!-- miracle-project-catalog:" + entry["id"] + " -->",
            "**Canonical Git source:** https://github.com/spwn02/Miracle/blob/master/"
                + entry["path"] + "#L" + str(entry["line"]),
            "**Group:** " + entry["group"],
            "**Record type:** " + entry["kind"],
            "**Historical reference, not a task or approval.**",
        ]
        if entry.get("parent"):
            required.append("**Parent record:** " + entry["parent"])
        for value in required:
            if value not in body:
                messages.append(entry["id"] + ": missing source/provenance marker: " + value[:75])
        if body.strip() != desired.strip():
            # Annotations may be legitimate, but are not identical to the canonical importer body.
            messages.append(entry["id"] + ": draft body differs from canonical import template")
        try:
            fields = field_values(item)
        except AuditError as exc:
            messages.append(entry["id"] + ": " + str(exc))
            continue
        for name, value in (("Artifact Type", entry["kind"]), ("Archive Group", entry["group"])):
            if fields.get(name) != value:
                messages.append(f"{entry['id']}: {name}={fields.get(name)!r}, expected {value!r}")
    return messages


def self_test() -> None:
    entry = {"title":"[Archive/Chapter] Example", "id":"example:chapter:0001",
             "group":"Foundation", "kind":"Chapter", "path":"docs/a.md", "line":1}
    body = ("<!-- miracle-project-catalog:example:chapter:0001 -->\n"
            "**Canonical Git source:** https://github.com/spwn02/Miracle/blob/master/docs/a.md#L1\n\n"
            "**Group:** Foundation\n\n**Record type:** Chapter\n\n"
            "**Historical reference, not a task or approval.**\n")
    item = {"id":"PVTI_example", "type":"DRAFT_ISSUE", "isArchived":False,
            "content":{"title":entry["title"],"body":body},
            "fieldValues":{"totalCount":2, "nodes":[
              {"name":"Chapter","field":{"name":"Artifact Type"}},
              {"name":"Foundation","field":{"name":"Archive Group"}}]}}
    assert compare([entry], [item], lambda e,r:body) == []
    wrong = json.loads(json.dumps(item))
    wrong["fieldValues"]["nodes"][0]["name"] = "Contract"
    assert any("Artifact Type" in x for x in compare([entry],[wrong],lambda e,r:body))
    assert any("expected exactly one" in x for x in compare([entry],[],lambda e,r:body))
    assert any("expected exactly one" in x for x in compare([entry],[item,item],lambda e,r:body))
    assert any("EXTRA" in x for x in compare([], [item], lambda e,r:body))
    pages = [
        {"totalCount":1, "nodes":[item], "pageInfo":{"hasNextPage":False,"endCursor":None}},
    ]
    assert collect(lambda c:pages.pop(0)) == [item]
    print("PASS: archive identifier, provenance, field, missing/extra/duplicate and pagination fixtures")


def main() -> None:
    if sys.argv[1:] == ["--self-test"]:
        self_test()
        return
    if sys.argv[1:]:
        raise AuditError("Usage: audit-project-archive.py [--self-test]")
    auth = subprocess.run(["gh","api","user","--jq",".login"],
                          capture_output=True,text=True,check=False)
    if auth.returncode or auth.stdout.strip() != "spwn02":
        raise AuditError("Projects credential not authenticated as expected owner")
    catalog_data = json.loads(CATALOG_PATH.read_text(encoding="utf-8"))
    if (catalog_data.get("projectOwner"),catalog_data.get("projectNumber")) != ("spwn02",3):
        raise AuditError("Unexpected catalog Project target")
    sync = runpy.run_path("scripts/github/sync-miracle-project.py")
    entries = sync["verify"](Path(".").resolve(), catalog_data)
    if len(entries) != 172:
        raise AuditError(f"Expected 172 catalog source records, found {len(entries)}")
    print("Source catalog: 172 validated entries; types=" + str(dict(Counter(x["kind"] for x in entries))))
    items = collect(graphql)
    print("Project #3 active items observed: " + str(len(items)))
    problems = compare(entries, items, sync["source_body"])
    if problems:
        for line in problems[:200]:
            print("MISMATCH: " + line)
        print(f"FAIL: {len(problems)} historical catalog differences (read-only, no writes).")
        raise SystemExit(1)
    print("PASS: 172/172 historical Project drafts uniquely match canonical source IDs, links, bodies and archive fields.")
    print("PASS: No extra draft with an [Archive/] title was detected.")
    print("No frozen source, Project item, Issue, view, date or status was modified.")


if __name__ == "__main__":
    try:
        main()
    except (AuditError, ValueError, OSError, KeyError) as exc:
        print("ERROR: " + str(exc), file=sys.stderr)
        raise SystemExit(1)
