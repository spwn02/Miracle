#!/usr/bin/env python3
"""Read-only Stage B GitHub branch disposition audit; never changes refs."""
from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys

OWNER_REPO = "spwn02/Miracle"
OUTPUT = Path(os.getenv("STAGE_B_BRANCH_OUTPUT", "stage-b-branch-evidence"))
PINNED_PREFIXES = ("docs/", "fix/", "infra/", "contracts/")
PROTECTED = frozenset({"master", "gcc"})


class BranchError(RuntimeError):
    pass


def api(path: str):
    proc = subprocess.run(["gh", "api", "repos/" + OWNER_REPO + "/" + path],
                          capture_output=True, text=True, check=False)
    if proc.returncode:
        raise BranchError("GitHub read failed; no operation performed")
    try:
        return json.loads(proc.stdout)
    except json.JSONDecodeError as exc:
        raise BranchError("invalid GitHub JSON") from exc


def classify(branch: dict, pr: dict | None, diff: dict) -> str:
    if branch.get("protected") or branch.get("name") in PROTECTED:
        return "protected-retain"
    if pr is None:
        return "unmapped-investigate"
    if pr.get("merged_at") is None:
        return "unmerged-retain"
    if branch.get("commit", {}).get("sha") != pr.get("head", {}).get("sha"):
        return "head-drift-investigate"
    if diff.get("ahead_by") != 0 or diff.get("status") not in ("behind", "identical"):
        return "not-proven-ancestor-retain"
    return "candidate-for-owner-reviewed-deletion"


def self_test():
    sample = {"name": "docs/example", "commit": {"sha": "a" * 40}, "protected": False}
    pr = {"merged_at": "2026-10-24T10:00:00Z", "head": {"sha": "a" * 40}}
    assert classify(sample, pr, {"ahead_by": 0, "status": "behind"}) == "candidate-for-owner-reviewed-deletion"
    assert classify(sample, None, {"ahead_by": 0}) == "unmapped-investigate"
    assert classify(sample, {**pr, "merged_at": None}, {"ahead_by": 0}) == "unmerged-retain"
    assert classify(sample, pr, {"ahead_by": 1, "status": "diverged"}) == "not-proven-ancestor-retain"
    assert classify({**sample, "name": "gcc"}, pr, {"ahead_by": 0}) == "protected-retain"
    assert classify(sample, {**pr, "head": {"sha": "b" * 40}}, {"ahead_by": 0}) == "head-drift-investigate"
    print("PASS: merged head/ancestry, protected, drift and unmerged branch classification")


def main():
    if sys.argv[1:] == ["--self-test"]:
        self_test()
        return
    if sys.argv[1:]:
        raise BranchError("Usage: stage-b-branch-plan.py [--self-test]")
    user = subprocess.run(["gh", "api", "user", "--jq", ".login"],
                          capture_output=True, text=True, check=False)
    if user.returncode or user.stdout.strip() != "spwn02":
        raise BranchError("not authenticated as expected owner")
    project_branches = api("branches?per_page=100&page=1")
    if not isinstance(project_branches, list) or len(project_branches) >= 100:
        raise BranchError("branches pagination could be incomplete")
    prs = api("pulls?state=all&per_page=100&page=1")
    if not isinstance(prs, list) or len(prs) >= 100:
        raise BranchError("PR pagination could be incomplete")
    by_head = {}
    for p in prs:
        name = (p.get("head") or {}).get("ref")
        if name and name not in by_head:
            by_head[name] = p
    rows = []
    for b in project_branches:
        name = b.get("name")
        sha = (b.get("commit") or {}).get("sha")
        if not isinstance(name, str) or not isinstance(sha, str) or len(sha) != 40:
            raise BranchError("malformed branch")
        pr = by_head.get(name)
        if name in PROTECTED or b.get("protected"):
            diff = {}
        else:
            # Compare exact recorded tip to master; never assume merged PR means ancestor.
            diff = api("compare/master..." + sha)
        status = classify(b, pr, diff)
        rows.append({"name": name, "head_sha": sha,
                     "protected": bool(b.get("protected")),
                     "pr_number": pr.get("number") if pr else None,
                     "pr_merged_at": pr.get("merged_at") if pr else None,
                     "pr_head_sha": (pr.get("head") or {}).get("sha") if pr else None,
                     "ahead_by": diff.get("ahead_by"),
                     "behind_by": diff.get("behind_by"), "status": status})
    rows.sort(key=lambda x: x["name"])
    from collections import Counter
    groups = dict(Counter(x["status"] for x in rows))
    OUTPUT.mkdir(exist_ok=True, parents=True)
    path = OUTPUT / "branch-manifest.json"
    path.write_text(json.dumps({"schema": 1, "repo": OWNER_REPO,
                                "mode": "read-only; no branches changed",
                                "count": len(rows), "summary": groups,
                                "branches": rows}, indent=2) + "\n", encoding="utf-8")
    (OUTPUT / "summary.md").write_text(
        "# Stage B Branch Disposition — Read-Only\n\n"
        f"- Branches checked: **{len(rows)}**\n"
        f"- Candidate classifications: `{groups}`\n"
        "- Source: live GitHub Branches / Pull Requests / exact-tip Compare API.\n"
        "- Every candidate requires exact SHA recheck and owner-reviewed manifest "
        "before deletion. No protected branch, Issue or PR was changed.\n",
        encoding="utf-8")
    print((OUTPUT / "summary.md").read_text(encoding="utf-8"))


if __name__ == "__main__":
    try:
        main()
    except (BranchError, OSError, ValueError, KeyError) as exc:
        print("ERROR: " + str(exc), file=sys.stderr)
        raise SystemExit(1)
