#!/usr/bin/env python3
"""Owner-guarded Git -> Project #3 draft-body pilot; never delete/merge drafts.

--prepare: read-only backup of 172 native bodies and proposed source-bearing bodies.
--pilot: apply EXACTLY one approved stable source ID using a saved backup.
The workflow must upload the backup artifact successfully before running --pilot.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import re
import runpy
import subprocess
import sys

PROJECT = "PVT_kwHOCp2Gs84Bna-3"
ROOT = Path(".").resolve()
OUTPUT = Path(os.environ.get("STAGE_B_DRAFT_OUTPUT", "stage-b-draft-backup"))
CATALOG = ROOT / "docs/contracts/PROJECT-CATALOG.json"
PILOT_ID = "architecture:chapter:0003"
CAP = 60_000


class SyncError(RuntimeError):
    pass


def digest(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def run_gh(query: str, **variables: str) -> dict:
    command = ["gh", "api", "graphql", "-f", "query=" + query]
    for key, value in variables.items():
        command += ["-f", key + "=" + value]
    proc = subprocess.run(command, capture_output=True, text=True, check=False)
    if proc.returncode:
        raise SyncError("authenticated GitHub GraphQL request failed")
    try:
        result = json.loads(proc.stdout)
    except json.JSONDecodeError as exc:
        raise SyncError("GitHub returned malformed JSON") from exc
    if result.get("errors"):
        raise SyncError("GitHub GraphQL rejected request; no success claimed")
    return result.get("data") or {}


def desired_body(record: dict, source: dict, revision: str) -> str:
    url = ("https://github.com/spwn02/Miracle/blob/" + revision + "/"
           + source["path"] + "#L" + str(source["line_start"])
           + "-L" + str(source["line_end"]))
    head = [
        "<!-- miracle-project-catalog:" + record["id"] + " -->",
        "**Derived Git source snapshot; not authoritative.**",
        "**Canonical Git source:** " + url,
        "**Source revision:** " + revision,
        "**Git file blob:** " + source["file_git_blob_sha1"],
        "**Section lines:** " + str(source["line_start"]) + "–" + str(source["line_end"]),
        "**Source SHA-256:** " + source["source_sha256"],
        "**Group:** " + record["group"],
        "**Record type:** " + record["kind"],
    ]
    if record.get("parent"):
        head.append("**Parent record:** " + record["parent"])
    return "\n\n".join(head) + "\n\n---\n\n" + source["body"]


def preflight(pinned_revision: str | None = None) -> tuple[list[dict], dict]:
    if OUTPUT.is_absolute() and not OUTPUT.is_relative_to(ROOT):
        raise SyncError("backup destination must remain under workspace")
    account = subprocess.run(["gh", "api", "user", "--jq", ".login"],
                             capture_output=True, text=True, check=False)
    if account.returncode or account.stdout.strip() != "spwn02":
        raise SyncError("must authenticate as actual approved Project owner")
    revision = subprocess.run(["git", "rev-parse", "HEAD"],
                              capture_output=True, text=True, check=True).stdout.strip()
    if not re.fullmatch(r"[a-f0-9]{40}", revision):
        raise SyncError("Git HEAD not a full SHA")
    if pinned_revision is not None:
        if not re.fullmatch(r"[a-f0-9]{40}", pinned_revision):
            raise SyncError("malformed backed-up source commit")
        # Allow an approved backup from a previous master merge only when that
        # source commit is a genuine ancestor; source blobs are checked below.
        parent = subprocess.run(["git", "merge-base", "--is-ancestor", pinned_revision, "HEAD"],
                                capture_output=True, check=False)
        if parent.returncode:
            # Actions intentionally checks out a SHALLOW master. Failure to
            # find an ancestor locally is inconclusive, not a rejection.
            # Confirm the exact range with GitHub's authenticated compare API.
            compare = subprocess.run(
                ["gh", "api", "repos/spwn02/Miracle/compare/" + pinned_revision + "..." + revision],
                capture_output=True, text=True, check=False
            )
            if compare.returncode:
                raise SyncError("cannot authenticate source ancestry; refusing write")
            try:
                evidence = json.loads(compare.stdout)
            except json.JSONDecodeError as exc:
                raise SyncError("GitHub source ancestry response invalid") from exc
            merge = (evidence.get("merge_base_commit") or {}).get("sha")
            if evidence.get("status") not in ("ahead", "identical") or merge != pinned_revision:
                raise SyncError("backed-up source is not a verified master ancestor")
        revision = pinned_revision
    baseline = json.loads(CATALOG.read_text(encoding="utf-8"))
    if (baseline.get("projectOwner"), baseline.get("projectNumber")) != ("spwn02", 3):
        raise SyncError("target Project identity mismatch")
    importer = runpy.run_path("scripts/github/sync-miracle-project.py")
    records = importer["verify"](ROOT, baseline)
    if len(records) != 172:
        raise SyncError("noncanonical catalog size")
    audit = runpy.run_path("scripts/github/audit-project-archive.py")
    source_plan = runpy.run_path("scripts/github/stage-b-source-manifest.py")
    items = audit["collect"](audit["graphql"])
    actual = source_plan["index_drafts"](items)
    if set(actual) != {e["id"] for e in records}:
        raise SyncError("native draft identity set drift")
    cached = {}
    rows = []
    for entry in records:
        src = source_plan["source_span"](ROOT, entry, cached)
        item = actual[entry["id"]]
        content = item["content"]
        if not isinstance(content.get("id"), str) or not content["id"]:
            raise SyncError("missing actual DraftIssue ID")
        before = content["body"]
        if not isinstance(before, str) or not before.startswith(
            "<!-- miracle-project-catalog:" + entry["id"] + " -->"
        ):
            raise SyncError("native original identity marker mismatch")
        if src["source_bytes"] > CAP:
            raise SyncError("source exceeds conservative pilot limit")
        wanted = desired_body(entry, src, revision)
        if len(wanted.encode("utf-8")) > CAP:
            raise SyncError("full snapshot exceeds conservative pilot limit")
        fields = audit["field_values"](item)
        if fields.get("Artifact Type") != entry["kind"] or fields.get("Archive Group") != entry["group"]:
            raise SyncError("native group/type altered since catalog audit")
        rows.append({
            "stable_id": entry["id"], "item_id": item["id"],
            "draft_issue_id": content["id"], "title": content["title"],
            "catalog_title": entry["title"], "source_path": entry["path"],
            "source_sha256": src["source_sha256"], "source_lines": [src["line_start"], src["line_end"]],
            "source_blob": src["file_git_blob_sha1"],
            "before_sha256": digest(before), "before": before,
            "after_sha256": digest(wanted), "after": wanted,
        })
    return rows, {"project_id": PROJECT, "revision": revision, "count": len(rows),
                  "pilot_id": PILOT_ID, "mode": "backup and read-only plan"}


def prepare() -> None:
    rows, metadata = preflight()
    OUTPUT.mkdir(parents=True, exist_ok=True)
    data = {"schema": 1, **metadata, "records": rows}
    (OUTPUT / "draft-body-backup-and-plan.json").write_text(
        json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )
    changes = sum(x["before_sha256"] != x["after_sha256"] for x in rows)
    (OUTPUT / "summary.md").write_text(
        "# Stage B draft-body full backup and dry-run\n\n"
        + "- Project: " + PROJECT + "\n"
        + "- Canonical Git revision: " + metadata["revision"] + "\n"
        + "- Unique native draft identities: " + str(len(rows)) + "\n"
        + "- Drafts requiring full-text replacement: " + str(changes) + "\n"
        + "- Total original bytes: " + str(sum(len(x["before"].encode("utf-8")) for x in rows)) + "\n"
        + "- Proposed complete snapshot bytes: " + str(sum(len(x["after"].encode("utf-8")) for x in rows)) + "\n"
        + "- Fixed pilot ID: " + PILOT_ID + "\n\n"
        + "**No Project write occurs in this step. Backup artifact upload must succeed before a pilot write.**\n",
        encoding="utf-8",
    )
    print((OUTPUT / "summary.md").read_text(encoding="utf-8"))


def pilot() -> None:
    original = OUTPUT / "draft-body-backup-and-plan.json"
    if not original.exists():
        raise SyncError("must prepare and upload pre-change backup before pilot")
    plan = json.loads(original.read_text(encoding="utf-8"))
    if plan.get("count") != 172 or plan.get("project_id") != PROJECT or plan.get("pilot_id") != PILOT_ID:
        raise SyncError("invalid prepared backup")
    matches = [x for x in plan["records"] if x["stable_id"] == PILOT_ID]
    if len(matches) != 1:
        raise SyncError("ambiguous pilot selection")
    target = matches[0]
    # Require a fresh full read: no other item can acquire this stable ID.
    live, metadata = preflight(pinned_revision=plan["revision"])
    if metadata["revision"] != plan["revision"] or len(live) != 172:
        raise SyncError("Git revision or Project item count drift")
    original_by_id = {x["stable_id"]: x for x in plan["records"]}
    if len(original_by_id) != 172:
        raise SyncError("duplicate or missing backed-up stable IDs")
    for current in live:
        saved = original_by_id.get(current["stable_id"])
        if not saved or any(current[key] != saved[key] for key in
                            ("item_id", "draft_issue_id", "title", "before_sha256",
                             "source_blob", "source_sha256", "after_sha256")):
            raise SyncError("backed-up original or canonical source changed: " + current["stable_id"])
    observed = next(x for x in live if x["stable_id"] == PILOT_ID)
    if (observed["draft_issue_id"] != target["draft_issue_id"]
            or observed["item_id"] != target["item_id"]
            or observed["title"] != target["title"]
            or observed["before_sha256"] != target["before_sha256"]
            or observed["after_sha256"] != target["after_sha256"]):
        raise SyncError("stale proposed write: current Project draft differs from backup")
    if observed["before_sha256"] == observed["after_sha256"]:
        print("SKIP: pilot already equals canonical source snapshot")
        return
    query = """mutation($draft:ID!,$text:String!){
      updateProjectV2DraftIssue(input:{draftIssueId:$draft,body:$text}){
        draftIssue {id title body}
      }
    }"""
    result = run_gh(query, draft=target["draft_issue_id"], text=target["after"])
    updated = (result.get("updateProjectV2DraftIssue") or {}).get("draftIssue") or {}
    if (updated.get("id") != target["draft_issue_id"]
            or updated.get("title") != target["title"]
            or digest(updated.get("body") or "") != target["after_sha256"]):
        raise SyncError("mutation response does not match desired hash")
    # A separately authenticated query proves durable native content.
    check = run_gh(
        "query($id:ID!){node(id:$id){... on DraftIssue{id title body}}}",
        id=target["draft_issue_id"]
    ).get("node") or {}
    if (check.get("title") != target["title"]
            or digest(check.get("body") or "") != target["after_sha256"]):
        raise SyncError("post-mutation independent readback differs")
    print("PASS: fixed pilot " + PILOT_ID + " source body fully reproduced and independently read back")
    print("Before SHA-256: " + target["before_sha256"])
    print("After SHA-256: " + target["after_sha256"])
    # Never assume atomic compare-and-swap between last read and mutation.


def self_test():
    entry = {"id": "a:chapter:0001", "group": "Foundation", "kind": "Chapter",
             "parent": "a:contract:0001"}
    source = {"body": "## Example\nAny **markup**.\n", "path": "docs/example.md",
              "line_start": 1, "line_end": 2,
              "file_git_blob_sha1": "a" * 40, "source_sha256": "b" * 64}
    result = desired_body(entry, source, "c" * 40)
    assert result.startswith("<!-- miracle-project-catalog:a:chapter:0001 -->")
    assert result.endswith(source["body"])
    assert "**Parent record:** a:contract:0001" in result
    assert "blob/" + "c" * 40 in result
    assert digest("ü") != digest("u")
    assert PILOT_ID == "architecture:chapter:0003"
    print("PASS: immutable catalog identity, canonical URL, full-text preservation and Unicode digest")


def main():
    if sys.argv[1:] == ["--self-test"]:
        self_test()
    elif sys.argv[1:] == ["--prepare"]:
        prepare()
    elif sys.argv[1:] == ["--pilot"]:
        pilot()
    else:
        raise SyncError("Usage: stage-b-draft-sync.py --self-test | --prepare | --pilot")


if __name__ == "__main__":
    try:
        main()
    except (SyncError, ValueError, KeyError, OSError, subprocess.CalledProcessError) as exc:
        print("ERROR: " + str(exc), file=sys.stderr)
        raise SystemExit(1)
