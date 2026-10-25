#!/usr/bin/env python3
"""Stage B: read-only, source-bearing Project #3 archival audit.

Uses the already-reviewed native Project GraphQL query, including DraftIssue.body.
Produces evidence files; never writes or deletes Project items or Git sources.
"""
from __future__ import annotations

from collections import Counter, defaultdict
import hashlib
import json
import os
from pathlib import Path
import re
import runpy
import subprocess
import sys

ROOT = Path(".").resolve()
CATALOG = ROOT / "docs/contracts/PROJECT-CATALOG.json"
OUTPUT = Path(os.getenv("STAGE_B_OUTPUT", "stage-b-evidence")).resolve()
ID_PATTERN = re.compile(r"<!-- miracle-project-catalog:([^ >]+) -->")
HEADING = re.compile(r"^(#{1,6})[ \t]+\S")
FENCE = re.compile(r"^[ \t]{0,3}(`{3,}|~{3,})")
PROJECT = "PVT_kwHOCp2Gs84Bna-3"
LIMIT = 60_000  # conservative staging cap; not asserted to be GitHub's API limit


class PlanError(RuntimeError):
    pass


def sha256(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def heading_ranges(lines: list[str]) -> dict[int, int]:
    """Return inclusive end line for every real Markdown ATX heading.

    Respect fenced code blocks. A section ends immediately before the next
    heading of the same or shallower depth. Code and nested subsections stay.
    """
    headings = []
    fence_char, fence_len = None, 0
    for n, line in enumerate(lines, 1):
        m = FENCE.match(line)
        if m:
            delim = m.group(1)
            if fence_char is None:
                fence_char, fence_len = delim[0], len(delim)
            elif delim[0] == fence_char and len(delim) >= fence_len:
                fence_char, fence_len = None, 0
            continue
        if fence_char is not None:
            continue
        m = HEADING.match(line)
        if m:
            headings.append((n, len(m.group(1))))
    return {
        start: next((other - 1 for other, d in headings[i + 1:]
                     if d <= depth), len(lines))
        for i, (start, depth) in enumerate(headings)
    }


def source_span(root: Path, entry: dict, cached: dict) -> dict:
    """Materialize complete source range and digest without rewriting source."""
    path = entry["path"]
    if path not in cached:
        source = root / path
        if not source.is_file() or not source.resolve().is_relative_to(root):
            raise PlanError("source path missing or escapes repository: " + path)
        content = source.read_text(encoding="utf-8")
        lines = content.splitlines()
        cached[path] = (lines, heading_ranges(lines),
                        hashlib.sha1(
                            b"blob " + str(len(content.encode("utf-8"))).encode()
                            + b"\0" + content.encode("utf-8")
                        ).hexdigest())
    lines, sections, blob = cached[path]
    start = entry["line"]
    if type(start) is not int or not (1 <= start <= len(lines)):
        raise PlanError("source anchor out of bounds: " + entry["id"])
    # Nonheading prologues (Phase 5.4) intentionally cover the full document.
    if start in sections:
        end = sections[start]
    elif start == 1:
        end = len(lines)
    else:
        raise PlanError("catalog entry points to an unsupported nonheading: " + entry["id"])
    if end < start:
        raise PlanError("invalid source range")
    body = "\n".join(lines[start - 1:end]) + "\n"
    return {"path": path, "line_start": start, "line_end": end,
            "file_git_blob_sha1": blob, "source_sha256": sha256(body),
            "source_bytes": len(body.encode("utf-8")), "body": body}


def index_drafts(project_items: list[dict]) -> dict[str, dict]:
    result = {}
    for item in project_items:
        if item.get("type") != "DRAFT_ISSUE":
            continue
        if item.get("isArchived") is not False:
            raise PlanError("archived draft in expected active inventory")
        content = item.get("content") or {}
        body = content.get("body")
        if not isinstance(body, str):
            raise PlanError("cannot read native Project draft body")
        found = ID_PATTERN.findall(body)
        if len(found) != 1 or found[0] in result:
            raise PlanError("missing or duplicate stable catalog marker in native draft")
        result[found[0]] = item
    return result


def report(root: Path, catalog: dict, items: list[dict], revision: str) -> dict:
    entries = catalog["items"]
    if len(entries) != 172 or len({e["id"] for e in entries}) != 172:
        raise PlanError("expected 172 unique catalog records")
    if not re.fullmatch(r"[0-9a-f]{40}", revision):
        raise PlanError("expected pinned 40-character Git revision")
    native = index_drafts(items)
    if len(native) != 172 or set(native) != {e["id"] for e in entries}:
        raise PlanError("native draft/canonical catalog ID-set mismatch")
    cache, records = {}, []
    for entry in entries:
        src = source_span(root, entry, cache)
        it = native[entry["id"]]
        body = it["content"]["body"]
        fields = {
            n["field"]["name"]: n["name"]
            for n in (it.get("fieldValues") or {}).get("nodes", [])
            if isinstance(n, dict) and n.get("name") is not None
            and isinstance(n.get("field"), dict) and n["field"].get("name")
        }
        if fields.get("Artifact Type") != entry["kind"] or fields.get("Archive Group") != entry["group"]:
            raise PlanError("native draft field drift: " + entry["id"])
        records.append({
            "catalog_id": entry["id"], "kind": entry["kind"], "group": entry["group"],
            "canonical_title": entry["title"], "native_title": it["content"]["title"],
            "node_id": it["id"], "source_path": entry["path"],
            "source_start_line": src["line_start"], "source_end_line": src["line_end"],
            "source_git_blob_sha1": src["file_git_blob_sha1"],
            "source_sha256": src["source_sha256"], "source_bytes": src["source_bytes"],
            "native_before_sha256": sha256(body),
            "native_before_bytes": len(body.encode("utf-8")),
            "oversize_conservative_cap": src["source_bytes"] > LIMIT,
        })
    overlap = defaultdict(list)
    for row in records:
        overlap[(row["source_sha256"], row["source_bytes"])].append(row["catalog_id"])
    duplicates = [v for v in overlap.values() if len(v) > 1]
    return {"schema": 1, "mode": "read-only pre-migration", "project_id": PROJECT,
            "project_owner": "spwn02", "project_number": 3,
            "source_commit_sha": revision,
            "count": len(records),
            "summary": {"source_bytes_total": sum(x["source_bytes"] for x in records),
                        "native_bytes_total": sum(x["native_before_bytes"] for x in records),
                        "over_cap": [x["catalog_id"] for x in records if x["oversize_conservative_cap"]],
                        "identical_source_ranges_distinct_catalog_roles": duplicates,
                        "title_spelling_drift": [x["catalog_id"] for x in records
                                                if x["canonical_title"] != x["native_title"]]},
            "records": records}


def self_test() -> None:
    lines = ["# Root", "prologue", "## One", "code", "### Child",
             "subtext", "```cpp", "## code is not heading", "```",
             "## Two", "last", "# Next"]
    ranges = heading_ranges(lines)
    assert ranges[1] == 11 and ranges[3] == 9 and ranges[5] == 9
    assert ranges[10] == 11 and ranges[12] == 12
    from tempfile import TemporaryDirectory
    with TemporaryDirectory() as p:
        root = Path(p)
        (root / "docs").mkdir()
        (root / "docs" / "sample.md").write_text("# Example\nAlpha\n## Child\nBeta\n", encoding="utf-8")
        e = {"id": "x", "path": "docs/sample.md", "line": 3}
        got = source_span(root, e, {})
        assert got["line_end"] == 4 and got["body"] == "## Child\nBeta\n"
    draft = {"id": "PVTI_demo", "type": "DRAFT_ISSUE", "isArchived": False,
             "content": {"body": "<!-- miracle-project-catalog:alpha:chapter:0001 -->\n",
                         "title": "Example"}}
    assert index_drafts([draft])["alpha:chapter:0001"]["id"] == "PVTI_demo"
    try:
        index_drafts([draft, draft])
    except PlanError:
        pass
    else:
        raise AssertionError("duplicate source identities not refused")
    print("PASS: nested heading/fence ranges, source hashes, native stable IDs and duplicate refusal")


def main() -> None:
    if sys.argv[1:] == ["--self-test"]:
        self_test()
        return
    if sys.argv[1:]:
        raise PlanError("Usage: stage-b-source-manifest.py [--self-test]")
    audit = runpy.run_path("scripts/github/audit-project-archive.py")
    importer = runpy.run_path("scripts/github/sync-miracle-project.py")
    doc = json.loads(CATALOG.read_text(encoding="utf-8"))
    importer["verify"](ROOT, doc)
    login = subprocess.run(["gh", "api", "user", "--jq", ".login"],
                           text=True, capture_output=True, check=False)
    if login.returncode or login.stdout.strip() != "spwn02":
        raise PlanError("Project credential not authenticated as approved owner")
    revision = subprocess.run(["git", "rev-parse", "HEAD"],
                              text=True, capture_output=True, check=True).stdout.strip()
    items = audit["collect"](audit["graphql"])
    plan = report(ROOT, doc, items, revision)
    OUTPUT.mkdir(parents=True, exist_ok=True)
    (OUTPUT / "source-manifest.json").write_text(
        json.dumps(plan, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    (OUTPUT / "summary.md").write_text(
        "# Stage B Source/Draft Read-Only Manifest\n\n"
        f"- Exact repository commit: `{revision}`\n"
        f"- Matched canonical/native drafts: **{plan['count']}**\n"
        f"- Full source bytes across records: **{plan['summary']['source_bytes_total']}**\n"
        f"- Current native body bytes across records: **{plan['summary']['native_bytes_total']}**\n"
        f"- Conservative 60 KiB source-sized candidates: {plan['summary']['over_cap']}\n"
        f"- Identical range groups (no deletion inferred): {plan['summary']['identical_source_ranges_distinct_catalog_roles']}\n"
        f"- Native/canonical title spelling differences: {plan['summary']['title_spelling_drift']}\n\n"
        "No GitHub Project drafts, issue fields, dates, branches, or sources were changed.\n",
        encoding="utf-8")
    print((OUTPUT / "summary.md").read_text(encoding="utf-8"))


if __name__ == "__main__":
    try:
        main()
    except (PlanError, ValueError, OSError, KeyError, subprocess.CalledProcessError) as exc:
        print("ERROR: " + str(exc), file=sys.stderr)
        raise SystemExit(1)
