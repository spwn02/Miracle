#!/usr/bin/env python3
"""Build a read-only Miracle graph snapshot from verified catalog and genuine Issues.

No Project or Issue mutation occurs. Default: historical catalog only.
With --live-issues, GitHub CLI reads real Issues and native blocked-by relations.
"""
from __future__ import annotations

import argparse
from collections import Counter
from datetime import datetime, timezone
import json
from pathlib import Path
import subprocess
import sys

DEFAULT_CATALOG = Path("docs/contracts/PROJECT-CATALOG.json")
DEFAULT_OUTPUT = Path("docs/workspace/graph.json")
REPO = "spwn02/Miracle"


def api_pages(endpoint: str) -> list[dict]:
    """Fail closed on API errors; GitHub CLI --slurp returns an array of pages."""
    response = subprocess.run(
        ["gh", "api", "--paginate", "--slurp", endpoint],
        capture_output=True, text=True, check=True,
    )
    pages = json.loads(response.stdout)
    if not isinstance(pages, list):
        raise ValueError("Expected GitHub REST array pages")
    if pages and isinstance(pages[0], dict):
        pages = [pages]
    if any(not isinstance(page, list) for page in pages):
        raise ValueError("Unexpected GitHub CLI pagination output")
    return [value for page in pages for value in page]


def fetch_issues() -> list[dict]:
    """Read every repository Issue and its native blocked-by relationships."""
    output = []
    for raw in api_pages(f"repos/{REPO}/issues?state=all&per_page=100"):
        if "pull_request" in raw:
            continue
        if not isinstance(raw.get("number"), int):
            raise ValueError("Issue without numeric GitHub identifier")
        blockers = api_pages(
            f"repos/{REPO}/issues/{raw['number']}/dependencies/blocked_by?per_page=100"
        )
        output.append({
            "number": raw["number"],
            "title": raw["title"],
            "state": raw["state"],
            "html_url": raw["html_url"],
            "labels": raw.get("labels", []),
            "blocked_by": [
                {"number": x["number"], "html_url": x["html_url"],
                 "title": x["title"]}
                for x in blockers
            ],
        })
    return output


def safe_catalog(data: dict) -> list[dict]:
    if data.get("schemaVersion") != 1 or not isinstance(data.get("items"), list):
        raise ValueError("Unsupported or missing verified historical catalog")
    records = data["items"]
    ids = set()
    for record in records:
        ident = record.get("id")
        if not isinstance(ident, str) or not ident or ident in ids:
            raise ValueError("Duplicate/invalid catalog ID")
        ids.add(ident)
        if record.get("kind") not in ("Contract", "Chapter", "Phase", "Checkpoint", "Ledger"):
            raise ValueError(f"Invalid catalog type: {ident}")
        if not isinstance(record.get("line"), int) or record["line"] < 1:
            raise ValueError(f"Invalid source line for {ident}")
        if not isinstance(record.get("path"), str) or record["path"].startswith("/") or ".." in Path(record["path"]).parts:
            raise ValueError(f"Unsafe source path for {ident}")
    for record in records:
        if record.get("parent") and record["parent"] not in ids:
            raise ValueError(f"Unknown catalog parent: {record['id']}")
    # Containment is not allowed to be cyclic.
    parents = {r["id"]: r.get("parent") for r in records}
    for ident in parents:
        visited = set()
        cursor = ident
        while cursor:
            if cursor in visited:
                raise ValueError(f"Cyclic catalog containment: {ident}")
            visited.add(cursor)
            cursor = parents[cursor]
    return records


def build(catalog: dict, issues: list[dict] | None = None) -> dict:
    records = safe_catalog(catalog)
    nodes = []
    edges = []
    for r in records:
        nodes.append({
            "id": "catalog:" + r["id"],
            "title": r["heading"],
            "kind": r["kind"],
            "group": r["group"],
            "state": "Historical reference",
            "url": f"https://github.com/{REPO}/blob/{catalog['baseRef']}/{r['path']}#L{r['line']}",
        })
        if r.get("parent"):
            edges.append({
                "from": "catalog:" + r["parent"],
                "to": "catalog:" + r["id"],
                "kind": "contains",
            })
    warnings = []
    if issues is not None:
        numbers: set[int] = set()
        for issue in issues:
            number = issue.get("number")
            if not isinstance(number, int) or number < 1 or number in numbers:
                raise ValueError("Duplicate or invalid repository issue number")
            numbers.add(number)
            labels = [label if isinstance(label, str) else label["name"]
                      for label in issue.get("labels", [])]
            nodes.append({
                "id": f"issue:{REPO}#{number}",
                "title": str(issue["title"]),
                "kind": "Issue",
                "group": next((x for x in labels if x.startswith("type:")), "Unclassified"),
                "state": str(issue["state"]).capitalize(),  # NOT Project Status!
                "url": f"https://github.com/{REPO}/issues/{number}",
            })
        blockers_seen: set[tuple[str, str]] = set()
        for issue in issues:
            destination = f"issue:{REPO}#{issue['number']}"
            for blocker in issue.get("blocked_by", []):
                if not isinstance(blocker.get("number"), int):
                    raise ValueError("Invalid native dependency reference")
                url = blocker.get("html_url", "")
                # External references use their native GitHub URL, not a guessed local number.
                if isinstance(url, str) and url.startswith("https://github.com/"):
                    parts = url.removeprefix("https://github.com/").split("/")
                    if len(parts) >= 4 and parts[2] == "issues" and parts[3].isdigit():
                        blocker_repo = parts[0] + "/" + parts[1]
                        blocker_num = int(parts[3])
                    else:
                        raise ValueError("Invalid native blocker issue URL")
                else:
                    blocker_repo, blocker_num = REPO, blocker["number"]
                source = f"issue:{blocker_repo}#{blocker_num}"
                if source == destination:
                    raise ValueError("Issue cannot block itself")
                if source not in {n["id"] for n in nodes}:
                    nodes.append({
                        "id": source, "title": str(blocker.get("title") or source),
                        "kind": "ExternalIssue", "group": "External dependency",
                        "state": "Unknown", "url": f"https://github.com/{blocker_repo}/issues/{blocker_num}",
                    })
                    warnings.append("Dependency target outside the exported Issue set: " + source)
                key = (source, destination)
                if key not in blockers_seen:
                    blockers_seen.add(key)
                    edges.append({"from": source, "to": destination, "kind": "blocks"})
        # Blocker cycles are invalid, independent of historical containment edges.
        blockers = [(e["from"], e["to"]) for e in edges if e["kind"] == "blocks"]
        adjacency: dict[str, list[str]] = {}
        for source, dest in blockers:
            adjacency.setdefault(source, []).append(dest)
        states: dict[str, int] = {}
        def visit(node: str) -> None:
            if states.get(node) == 1:
                raise ValueError("Cyclic blocking Issue dependency: " + node)
            if states.get(node) == 2:
                return
            states[node] = 1
            for following in adjacency.get(node, []):
                visit(following)
            states[node] = 2
        for node in adjacency:
            visit(node)
    return {
        "schemaVersion": 1,
        "project": "https://github.com/users/spwn02/projects/3",
        "repo": REPO,
        "catalogSourceRevision": catalog.get("sourceRevision"),
        "coverage": {
            "catalog": len(records),
            "issues": "not fetched" if issues is None else len(issues),
            "dependencySource": "none" if issues is None else "native GitHub Issues blocked_by",
            "projectFields": "not fetched",
        },
        "nodes": nodes,
        "edges": edges,
        "warnings": sorted(set(warnings)),
    }


def self_test() -> None:
    catalog = {"schemaVersion": 1, "baseRef": "master", "items": [
        {"id": "a", "heading": "Top", "kind": "Contract",
         "group": "Foundation", "line": 1, "path": "docs/a.md"},
        {"id": "b", "parent": "a", "heading": "Child", "kind": "Chapter",
         "group": "Foundation", "line": 5, "path": "docs/a.md"},
    ]}
    issues = [
        {"number": 2, "title": "blocked", "state": "open", "blocked_by": [{"number": 1}]},
        {"number": 1, "title": "prerequisite", "state": "closed", "blocked_by": []},
    ]
    manifest = build(catalog, issues)
    assert Counter(e["kind"] for e in manifest["edges"]) == {"contains": 1, "blocks": 1}
    assert ("issue:spwn02/Miracle#1", "issue:spwn02/Miracle#2") in [
        (e["from"], e["to"]) for e in manifest["edges"] if e["kind"] == "blocks"]
    assert build(catalog)["coverage"]["issues"] == "not fetched"
    try:
        build(catalog, issues + [issues[0]])
        raise AssertionError("Duplicate issue not rejected")
    except ValueError:
        pass
    cyclic = json.loads(json.dumps(issues))
    cyclic[1]["blocked_by"] = [{"number": 2}]
    try:
        build(catalog, cyclic)
        raise AssertionError("Blocking cycle not rejected")
    except ValueError:
        pass
    malformed = json.loads(json.dumps(catalog))
    malformed["items"][0]["parent"] = "b"
    try:
        build(malformed)
        raise AssertionError("Catalog cycle not rejected")
    except ValueError:
        pass
    print("PASS: graph directions, types, duplicate handling, cycle detection, offline coverage")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--catalog", type=Path, default=DEFAULT_CATALOG)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--issues-json", type=Path,
                        help="Optional reviewed Issue fixture/snapshot (array or {issues: [...]})")
    parser.add_argument("--live-issues", action="store_true",
                        help="Read GitHub Issues/dependencies using authenticated gh CLI")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_test()
        return 0
    if args.live_issues and args.issues_json:
        parser.error("Choose live issues or a saved Issue snapshot")
    catalog = json.loads(args.catalog.read_text(encoding="utf-8"))
    issues = fetch_issues() if args.live_issues else None
    if args.issues_json:
        loaded = json.loads(args.issues_json.read_text(encoding="utf-8"))
        issues = loaded["issues"] if isinstance(loaded, dict) else loaded
        if not isinstance(issues, list):
            raise ValueError("Issue snapshot must contain an array")
    result = build(catalog, issues)
    result["generatedAt"] = datetime.now(timezone.utc).isoformat(timespec="seconds")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n",
                           encoding="utf-8")
    print("Wrote", args.output, "| nodes:", len(result["nodes"]),
          "| edges:", len(result["edges"]), "| coverage:", result["coverage"])
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ValueError, KeyError, OSError, subprocess.CalledProcessError) as error:
        print("ERROR:", error, file=sys.stderr)
        raise SystemExit(1)
