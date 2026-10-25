#!/usr/bin/env python3
"""Guarded, resumable batches of Git -> native Project draft bodies.

Requires the original complete 172-draft artifact from run #40217537233,
downloaded into stage-b-draft-backup/ before any operation.
--plan-batch is read-only. --apply-batch and --restore-batch are intentionally
NOT connected to the owner-only Actions dispatcher in this preparatory PR.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import runpy
import sys

sync = runpy.run_path("scripts/github/stage-b-draft-sync.py")
SyncError = sync["SyncError"]
digest = sync["digest"]
run_gh = sync["run_gh"]
PROJECT = sync["PROJECT"]
OUTPUT = sync["OUTPUT"]
BATCH_SIZE = 10
EXPECTED_COUNT = 172


def choose(records: list[dict], index: int) -> list[dict]:
    if not isinstance(index, int) or not 0 <= index < (EXPECTED_COUNT + BATCH_SIZE - 1) // BATCH_SIZE:
        raise SyncError("invalid fixed batch index")
    ordered = sorted(records, key=lambda r: r["stable_id"])
    if len(ordered) != EXPECTED_COUNT or len({r["stable_id"] for r in ordered}) != EXPECTED_COUNT:
        raise SyncError("missing or duplicated catalog identities")
    return ordered[index * BATCH_SIZE:(index + 1) * BATCH_SIZE]


def snapshot() -> tuple[dict[str, dict], dict[str, dict], dict]:
    """Validate every backed-up original and every current body before selecting a batch."""
    path = OUTPUT / "draft-body-backup-and-plan.json"
    if not path.is_file():
        raise SyncError("full original-body backup must be downloaded first")
    plan = json.loads(path.read_text(encoding="utf-8"))
    if (plan.get("schema"), plan.get("project_id"), plan.get("count"),
        plan.get("pilot_id")) != (1, PROJECT, EXPECTED_COUNT, sync["PILOT_ID"]):
        raise SyncError("invalid pinned original-body backup metadata")
    saved = plan.get("records")
    if not isinstance(saved, list) or len(saved) != EXPECTED_COUNT:
        raise SyncError("incomplete original-body backup")
    unique = {r["stable_id"]: r for r in saved}
    if (len(unique) != EXPECTED_COUNT or
        len({r["item_id"] for r in saved}) != EXPECTED_COUNT or
        len({r["draft_issue_id"] for r in saved}) != EXPECTED_COUNT):
        raise SyncError("duplicate or missing backed-up native identities")
    for record in saved:
        if (digest(record["before"]) != record["before_sha256"] or
            digest(record["after"]) != record["after_sha256"] or
            not record["after"].startswith(
                "<!-- miracle-project-catalog:" + record["stable_id"] + " -->"
            )):
            raise SyncError("backup content integrity failed")
    # Checks source-commit ancestry, complete canonical Git ranges and 172
    # owner-authenticated current DraftIssue bodies before *any* write.
    live, meta = sync["preflight"](pinned_revision=plan["revision"])
    if meta["revision"] != plan["revision"] or len(live) != EXPECTED_COUNT:
        raise SyncError("source snapshot or archive inventory changed")
    current = {r["stable_id"]: r for r in live}
    if set(current) != set(unique):
        raise SyncError("native identities drifted since complete backup")
    fixed = ("item_id", "draft_issue_id", "title", "catalog_title", "source_path",
             "source_sha256", "source_lines", "source_blob", "after_sha256")
    for identity, prior in unique.items():
        now = current[identity]
        if any(now[field] != prior[field] for field in fixed):
            raise SyncError("source or native identity drift: " + identity)
        if now["before_sha256"] not in (prior["before_sha256"], prior["after_sha256"]):
            raise SyncError("unreviewed native body modification: " + identity)
    return unique, current, plan


def read_exact(record: dict) -> str:
    node = run_gh(
        "query($id:ID!){node(id:$id){... on DraftIssue{id title body}}}",
        id=record["draft_issue_id"]
    ).get("node") or {}
    if (node.get("id") != record["draft_issue_id"] or
        node.get("title") != record["title"] or not isinstance(node.get("body"), str)):
        raise SyncError("pre/post-write node identity or body failed")
    return node["body"]


def operate(index: int, action: str) -> None:
    saved, live, plan = snapshot()
    batch = choose(list(saved.values()), index)
    # The initial whole-archive preflight is a guard, not an atomic transaction.
    # Re-read each individual DraftIssue immediately before a mutation.
    changed = 0
    for original in batch:
        identity = original["stable_id"]
        expected_before = original["before_sha256"]
        expected_after = original["after_sha256"]
        present_hash = live[identity]["before_sha256"]
        if action == "plan":
            status = "canonical" if present_hash == expected_after else "original"
            print(identity + ": " + status)
            continue
        wanted = original["before"] if action == "restore" else original["after"]
        expected = expected_before if action == "restore" else expected_after
        permitted = expected_after if action == "restore" else expected_before
        fresh_hash = digest(read_exact(original))
        if fresh_hash == expected:
            print("SKIP: " + identity + " already has requested snapshot")
            continue
        if fresh_hash != permitted:
            raise SyncError("stale body: refusing write for " + identity)
        result = run_gh(
            """mutation($draft:ID!,$text:String!){
                updateProjectV2DraftIssue(input:{draftIssueId:$draft,body:$text}){
                    draftIssue{id title body}
                }
            }""",
            draft=original["draft_issue_id"], text=wanted
        )
        response = (result.get("updateProjectV2DraftIssue") or {}).get("draftIssue") or {}
        if (response.get("id") != original["draft_issue_id"] or
            response.get("title") != original["title"] or
            digest(response.get("body") or "") != expected):
            raise SyncError("mutation result mismatch for " + identity)
        if digest(read_exact(original)) != expected:
            raise SyncError("independent readback mismatch for " + identity)
        changed += 1
        print("PASS: " + identity + " -> " + expected)
    print("Batch " + str(index) + ": " + str(len(batch)) + " candidates, " +
          str(changed) + " successful mutations (" + action + "); " +
          "source commit " + plan["revision"])
    # A failed batch may have partially committed. Retry is guarded and idempotent.
    # GitHub DraftIssue updates expose no server-side atomic compare-and-swap.



def plan_all() -> None:
    """One authenticated complete-archive snapshot for all 18 bounded batches."""
    saved, live, plan = snapshot()
    to_apply = 0
    for index in range(18):
        batch = choose(list(saved.values()), index)
        pending = [r["stable_id"] for r in batch
                   if live[r["stable_id"]]["before_sha256"] != r["after_sha256"]]
        to_apply += len(pending)
        print("Batch %02d: %d pending / %d total" %
              (index, len(pending), len(batch)))
        for identity in pending:
            print("  pending: " + identity)
    print("Read-only complete backup/source/native inventory: " +
          str(EXPECTED_COUNT - to_apply) + " already canonical, " +
          str(to_apply) + " remaining; source revision " + plan["revision"])
    print("NO PROJECT WRITE PERFORMED")


def self_test() -> None:
    synthetic = [{"stable_id": "entry-%03d" % i} for i in range(EXPECTED_COUNT)]
    parts = [choose(synthetic, i) for i in range(18)]
    assert sum(map(len, parts)) == EXPECTED_COUNT
    assert len(parts[0]) == 10 and len(parts[-1]) == 2
    assert {r["stable_id"] for batch in parts for r in batch} == {
        r["stable_id"] for r in synthetic
    }
    for invalid in (-1, 18, 172):
        try:
            choose(synthetic, invalid)
        except SyncError:
            pass
        else:
            raise AssertionError("batch index accepted outside range")
    try:
        choose(synthetic[:-1] + [synthetic[0]], 0)
    except SyncError:
        pass
    else:
        raise AssertionError("duplicate stable ID accepted")
    assert digest("ü") == hashlib.sha256("ü".encode("utf-8")).hexdigest()
    print("PASS: fixed 18-batch/172-identity partition and invalid-target refusals")


def main() -> None:
    args = sys.argv[1:]
    if args == ["--self-test"]:
        self_test()
    elif args == ["--plan-all"]:
        plan_all()
    elif len(args) == 2 and args[0] in ("--plan-batch", "--apply-batch", "--restore-batch"):
        if not args[1].isdigit() or str(int(args[1])) != args[1]:
            raise SyncError("batch index must be an unpadded decimal")
        action = {"--plan-batch": "plan", "--apply-batch": "apply",
                  "--restore-batch": "restore"}[args[0]]
        operate(int(args[1]), action)
    else:
        raise SyncError("Usage: --self-test | --plan-all | (--plan-batch|--apply-batch|--restore-batch) 0..17")


if __name__ == "__main__":
    try:
        main()
    except (SyncError, ValueError, KeyError, TypeError, OSError) as exc:
        print("ERROR: " + str(exc), file=sys.stderr)
        raise SystemExit(1)
