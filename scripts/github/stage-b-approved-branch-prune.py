#!/usr/bin/env python3
"""Owner-approved, exact-lease Stage B branch cleanup. Never deletes Project data.

The sole --apply entrypoint is restricted by the owner-only Actions workflow.
37 pinned merged/ancestor branches only. Original commits remain reachable via
37 deliberately named recovery tags until a separate owner decision.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import re
import subprocess
import sys
from urllib.parse import quote

REPO = "spwn02/Miracle"
MANIFEST = Path("scripts/github/stage-b-approved-37.json")
EVIDENCE = Path("stage-b-branch-cleanup-evidence")
EXPECTED_COUNT = 37
EXPECTED_SOURCE_BLOB = "1930abd1b385fec082eeaf686c8a95d5e244c089"
EXPECTED_OWNER = "spwn02"
EXPECTED_COMMAND = "/miracle-stage-b delete-approved-37-20261025"
VALID_BRANCH = re.compile(r"^(?:contracts|docs|fix|infra)/[A-Za-z0-9._/-]+$")


class Refusal(RuntimeError):
    pass


def run(args: list[str], *, data: str | None = None, tolerate_404: bool = False):
    proc = subprocess.run(
        args, input=data, text=True, capture_output=True, check=False
    )
    if proc.returncode:
        if tolerate_404 and "HTTP 404" in proc.stderr:
            return None
        raise Refusal("command refused: " + " ".join(args[:4]) +
                      " (" + proc.stderr.strip()[-700:] + ")")
    return proc.stdout


def api(path: str, *, method: str = "GET", payload: dict | None = None,
        tolerate_404: bool = False):
    args = ["gh", "api", "-X", method, "repos/" + REPO + "/" + path]
    if payload is not None:
        args += ["--input", "-"]
    raw = run(args, data=json.dumps(payload) if payload is not None else None,
              tolerate_404=tolerate_404)
    return None if raw is None else json.loads(raw)


def plan():
    obj = json.loads(MANIFEST.read_text(encoding="utf-8"))
    if (obj.get("schema") != 1 or obj.get("repository") != REPO or
        obj.get("source_issue") != 69 or
        obj.get("source_artifact") != 12613503581 or
        obj.get("source_markdown_git_blob") != EXPECTED_SOURCE_BLOB or
        obj.get("recovery_tag_prefix") != "stage-b-recovery-20261025" or
        obj.get("master_at_approval") !=
            "8dc193d1c6c3defa1f0d43294410ff51bcd58c18"):
        raise Refusal("approved manifest provenance or fields changed")
    targets = obj.get("targets")
    if not isinstance(targets, list) or len(targets) != EXPECTED_COUNT:
        raise Refusal("approved target count mismatch")
    if (len({r["name"] for r in targets}) != EXPECTED_COUNT or
        len({r["sha"] for r in targets}) != EXPECTED_COUNT or
        len({r["pr"] for r in targets}) != EXPECTED_COUNT):
        raise Refusal("duplicate name/SHA/PR in approved target list")
    retained = set(obj.get("protected_retain", [])) | set(
        obj.get("other_retain", []))
    if retained != {"master", "gcc", "infra/living-development-workspace",
                    "infra/stage-b-issue-event-evidence",
                    "infra/stage-b-reviewed-batch-write-route"}:
        raise Refusal("five approved retained branches changed")
    for i, t in enumerate(targets, 1):
        if (t.get("index") != i or
            not isinstance(t.get("name"), str) or
            not VALID_BRANCH.fullmatch(t["name"]) or
            not re.fullmatch(r"[0-9a-f]{40}", str(t.get("sha", ""))) or
            not isinstance(t.get("pr"), int) or t["pr"] <= 0 or
            t["name"] in retained):
            raise Refusal("invalid exact approved target index " + str(i))
    # The separate reviewed, Git-blob-pinned disposition is the semantic
    # approval source. A modified JSON allowlist must never widen deletion.
    markdown = Path("docs/infrastructure/STAGE-B-DISPOSITION-AND-EVENT-REVIEW-20261025.md").read_text(encoding="utf-8")
    pieces = markdown.split("### Exact 37 deletion *candidates*, not permission to delete")
    if len(pieces) != 2 or "**Pre-deletion gate:**" not in pieces[1]:
        raise Refusal("approved disposition table not uniquely found")
    candidate_table = pieces[1].split("**Pre-deletion gate:**", 1)[0]
    rows = re.findall(r"^\\| \\x60([^\\x60]+)\\x60 \\| \\x60([0-9a-f]{40})\\x60 \\| #(\\d+) \\|$",
                      candidate_table, flags=re.MULTILINE)
    approved_from_source = [
        {"index": i, "name": name, "sha": sha, "pr": int(pr)}
        for i, (name, sha, pr) in enumerate(rows, 1)
    ]
    if len(approved_from_source) != EXPECTED_COUNT or approved_from_source != targets:
        raise Refusal("JSON target set differs from canonical exact approved 37")
    return obj, targets


def recovery_tag(t):
    return "stage-b-recovery-20261025/" + format(t["index"], "02d")


def get_branch(name: str):
    return api("branches/" + quote(name, safe="/"), tolerate_404=True)


def get_recovery(tag: str):
    return api("git/ref/tags/" + tag, tolerate_404=True)


def validate_single(t: dict, open_prs: list[dict] | None = None):
    b = get_branch(t["name"])
    if (not isinstance(b, dict) or
        b.get("name") != t["name"] or b.get("protected") or
        (b.get("commit") or {}).get("sha") != t["sha"]):
        raise Refusal("branch missing, protected or drifted: " + t["name"])
    pr = api("pulls/" + str(t["pr"]))
    head = pr.get("head") or {}
    if (pr.get("state") != "closed" or not pr.get("merged_at") or
        head.get("ref") != t["name"] or head.get("sha") != t["sha"] or
        (head.get("repo") or {}).get("full_name") != REPO):
        raise Refusal("PR not merged or head changed for " + t["name"])
    if open_prs is not None and any(
        (p.get("head") or {}).get("ref") == t["name"] and
        ((p.get("head") or {}).get("repo") or {}).get("full_name") == REPO
        for p in open_prs
    ):
        raise Refusal("open PR depends on " + t["name"])
    diff = api("compare/master..." + t["sha"])
    if diff.get("ahead_by") != 0 or diff.get("status") not in ("behind", "identical"):
        raise Refusal("branch not proven master ancestor: " + t["name"])


def validate_all(targets):
    branches = api("branches?per_page=100&page=1")
    if not isinstance(branches, list) or len(branches) >= 100:
        raise Refusal("branch inventory incomplete")
    current = {b["name"]: b for b in branches}
    if len(current) != len(branches):
        raise Refusal("duplicate branch names in live inventory")
    if not current.get("master", {}).get("protected") or not current.get("gcc", {}).get("protected"):
        raise Refusal("protected branches not proven")
    if not {"master", "gcc", "infra/living-development-workspace",
            "infra/stage-b-reviewed-batch-write-route",
            "infra/stage-b-issue-event-evidence"}.issubset(current):
        raise Refusal("previously retained branch is missing")
    approval_master = json.loads(MANIFEST.read_text(encoding="utf-8"))["master_at_approval"]
    ancestry = api("compare/" + approval_master + "...master")
    if ancestry.get("behind_by") != 0 or ancestry.get("status") not in ("ahead", "identical"):
        raise Refusal("master history no longer descends from approved snapshot")
    prs = api("pulls?state=open&per_page=100&page=1")
    if not isinstance(prs, list) or len(prs) >= 100:
        raise Refusal("open PR pagination incomplete")
    for t in targets:
        validate_single(t, prs)
    return len(branches)


def save(state):
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    (EVIDENCE / "result.json").write_text(
        json.dumps(state, indent=2) + "\n", encoding="utf-8"
    )


def self_test():
    obj, t = plan()
    assert len(t) == 37 and t[0]["index"] == 1 and t[-1]["index"] == 37
    assert recovery_tag(t[0]) == "stage-b-recovery-20261025/01"
    assert recovery_tag(t[-1]) == "stage-b-recovery-20261025/37"
    assert not VALID_BRANCH.fullmatch("master")
    assert not VALID_BRANCH.fullmatch("infra/unsafe branch")
    assert not VALID_BRANCH.fullmatch("infra/x;echo")
    assert obj["protected_retain"] == ["master", "gcc"]
    print("PASS: 37 fixed exact PR/SHA/branch identities, recovery names and guardrails")


def apply():
    if (os.environ.get("GITHUB_ACTOR") != EXPECTED_OWNER or
        os.environ.get("GITHUB_EVENT_NAME") != "issue_comment" or
        os.environ.get("STAGE_B_COMMAND") != EXPECTED_COMMAND or
        os.environ.get("GITHUB_REF") != "refs/heads/master" or
        not os.environ.get("GH_TOKEN")):
        raise Refusal("owner-only Issue #69 Actions context not proven")
    obj, targets = plan()
    pinned = run(["git", "rev-parse", "HEAD:docs/infrastructure/"
                  "STAGE-B-DISPOSITION-AND-EVENT-REVIEW-20261025.md"]).strip()
    if pinned != EXPECTED_SOURCE_BLOB:
        raise Refusal("reviewed canonical Stage B source manifest changed")
    state = {"schema": 1, "repo": REPO, "stage": "before-preflight",
             "approved_count": EXPECTED_COUNT,
             "master_at_approval": obj["master_at_approval"],
             "targets": targets, "recovery_tags": [], "deleted": [],
             "retained": obj["protected_retain"] + obj["other_retain"]}
    save(state)
    print("Step 1/4: independently preflight all 37 approved exact refs and PRs", flush=True)
    state["observed_branch_count"] = validate_all(targets)
    state["stage"] = "preflight-passed"
    save(state)
    print("Step 2/4: create and verify one recovery tag per exact approved SHA", flush=True)
    for t in targets:
        tag = recovery_tag(t)
        existing = get_recovery(tag)
        if existing is None:
            created = api("git/refs", method="POST",
                          payload={"ref": "refs/tags/" + tag, "sha": t["sha"]})
            existing = created
        if (existing.get("ref") != "refs/tags/" + tag or
            (existing.get("object") or {}).get("sha") != t["sha"]):
            raise Refusal("recovery ref collision or digest mismatch: " + tag)
        verified = get_recovery(tag)
        if not verified or (verified.get("object") or {}).get("sha") != t["sha"]:
            raise Refusal("recovery ref not independently readable: " + tag)
        state["recovery_tags"].append({"tag": tag, "sha": t["sha"],
                                       "original_branch": t["name"]})
        save(state)
    if len(state["recovery_tags"]) != EXPECTED_COUNT:
        raise Refusal("recovery ref count incomplete")
    print("Step 3/4: revalidate all approved branches after complete recovery creation", flush=True)
    validate_all(targets)
    # gh supplies an authenticated Git credential helper; checkout does not
    # persist credentials, and no token is placed in a Git remote URL or logs.
    run(["gh", "auth", "setup-git"])
    state["stage"] = "exact-lease-delete"
    save(state)
    for t in targets:
        # Fresh per-ref metadata, merged PR and ancestor check immediately
        # before push. The Git receive-pack lease rejects a moved remote tip.
        open_prs = api("pulls?state=open&per_page=100&page=1")
        if not isinstance(open_prs, list) or len(open_prs) >= 100:
            raise Refusal("open PR inventory incomplete while deleting")
        validate_single(t, open_prs)
        ref = "refs/heads/" + t["name"]
        run(["git", "push", "--porcelain",
             "--force-with-lease=" + ref + ":" + t["sha"],
             "https://github.com/" + REPO + ".git", ":" + ref])
        if get_branch(t["name"]) is not None:
            raise Refusal("deleted branch still present: " + t["name"])
        if (get_recovery(recovery_tag(t)) or {}).get("object", {}).get("sha") != t["sha"]:
            raise Refusal("recovery ref changed for " + t["name"])
        state["deleted"].append({"name": t["name"], "sha": t["sha"],
                                 "pr": t["pr"], "recovery_tag": recovery_tag(t)})
        save(state)
        print("PASS: exact lease deletion", t["index"], t["name"], flush=True)
    print("Step 4/4: independent full ref and recovery verification", flush=True)
    final_branches = api("branches?per_page=100&page=1")
    if not isinstance(final_branches, list) or len(final_branches) >= 100:
        raise Refusal("post-delete branch inventory incomplete")
    existing_names = {x["name"] for x in final_branches}
    if len(state["deleted"]) != 37 or any(x["name"] in existing_names for x in targets):
        raise Refusal("unremoved approved branch in final inventory")
    for name in state["retained"]:
        if name not in existing_names:
            raise Refusal("retained branch missing in final inventory: " + name)
    state["stage"] = "verified-complete"
    state["remaining_branch_count"] = len(final_branches)
    state["remaining_branch_names"] = sorted(existing_names)
    save(state)
    print("PASS: 37/37 exact-SHA branches removed, 37/37 recovery tags preserved; "
          "retained branches unchanged in membership", flush=True)


if __name__ == "__main__":
    try:
        if sys.argv[1:] == ["--self-test"]:
            self_test()
        elif sys.argv[1:] == ["--apply-approved-37"]:
            apply()
        else:
            raise Refusal("Usage: --self-test | --apply-approved-37")
    except (Refusal, ValueError, KeyError, OSError) as exc:
        print("REFUSED/STOPPED: " + str(exc), file=sys.stderr)
        raise SystemExit(1)
