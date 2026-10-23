#!/usr/bin/env python3
"""Native GitHub Issue relationships for Miracle Project #3.

Owner-only commands, routed through the reviewed Issue #29 Actions bridge:
  /miracle-project relations 29
  /miracle-project add-child {"parent":29,"child":36}
  /miracle-project add-blocker {"issue":X,"blocked_by":Y}

Uses the job-scoped repository GITHUB_TOKEN (issues:write), never the
user-owned Projects token. Does not change fields, contracts or issue states.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
import time
from typing import Any

BASE = "repos/spwn02/Miracle"
REPOSITORY_URL = "https://api.github.com/" + BASE
INSPECT = "/miracle-project relations "
CHILD = "/miracle-project add-child "
BLOCKER = "/miracle-project add-blocker "


class Refusal(RuntimeError):
    pass


def unique_object(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("duplicate JSON key")
        result[key] = value
    return result


def positive_number(value: Any) -> bool:
    return type(value) is int and 1 <= value <= 9_999_999


def positive_id(value: Any) -> bool:
    # GitHub's REST issue `id` is a global numeric database identifier,
    # not a small repository issue number. Real Miracle IDs exceed 2**32.
    return type(value) is int and value >= 1


def parse(request: str) -> tuple[str, int, int | None]:
    if request.startswith(INSPECT):
        number = request[len(INSPECT):]
        if not re.fullmatch(r"[1-9][0-9]{0,6}", number):
            raise Refusal("relations requires one positive issue number")
        return "inspect", int(number), None
    if request.startswith(CHILD):
        prefix, keys, operation = CHILD, {"parent", "child"}, "child"
    elif request.startswith(BLOCKER):
        prefix, keys, operation = BLOCKER, {"issue", "blocked_by"}, "blocker"
    else:
        raise Refusal("unsupported relationship command")
    argument = request[len(prefix):]
    if len(argument) > 512:
        raise Refusal("relationship command is oversized")
    try:
        obj = json.loads(argument, object_pairs_hook=unique_object)
    except (ValueError, TypeError) as exc:
        raise Refusal("invalid JSON relationship command") from exc
    if not isinstance(obj, dict) or set(obj) != keys:
        raise Refusal("relationship command has unexpected fields")
    first, second = (obj["parent"], obj["child"]) if operation == "child" else (
        obj["issue"], obj["blocked_by"])
    if not positive_number(first) or not positive_number(second) or first == second:
        raise Refusal("two distinct positive issue numbers are required")
    return operation, first, second


def call(*args: str, allow_no_parent: bool = False) -> Any | None:
    proc = subprocess.run(["gh", "api", *args], text=True, capture_output=True, check=False)
    if proc.returncode:
        # GitHub documents a 404 when an issue has no parent. Do not treat any
        # other 404 (missing resource, forbidden repo) as an empty parent.
        if (allow_no_parent and "HTTP 404" in proc.stderr
                and "No parent issue found" in proc.stderr):
            return None
        raise Refusal("GitHub API request failed (exit status " + str(proc.returncode) + ")")
    try:
        return json.loads(proc.stdout)
    except json.JSONDecodeError as exc:
        raise Refusal("GitHub API returned invalid JSON") from exc


def issue(number: int, *, mutation: bool = False) -> dict[str, Any]:
    result = call(f"{BASE}/issues/{number}")
    if (not isinstance(result, dict) or result.get("number") != number
            or result.get("repository_url") != REPOSITORY_URL
            or result.get("html_url") != f"https://github.com/spwn02/Miracle/issues/{number}"
            or not positive_id(result.get("id")) or "pull_request" in result):
        raise Refusal(f"Issue #{number} is missing, ambiguous or not a repository Issue")
    if mutation and result.get("state") != "open":
        raise Refusal(f"Issue #{number} is not open")
    return result


def collection(endpoint: str) -> list[dict[str, Any]]:
    value = call(f"{BASE}/{endpoint}?per_page=100&page=1")
    if not isinstance(value, list) or len(value) >= 100:
        raise Refusal("Issue relation list incomplete or unexpectedly formatted")
    if not all(isinstance(entry, dict) and positive_id(entry.get("id"))
               and positive_number(entry.get("number")) for entry in value):
        raise Refusal("Unexpected relationship list item")
    return value


def parent(number: int) -> dict[str, Any] | None:
    result = call(f"{BASE}/issues/{number}/parent", allow_no_parent=True)
    if result is None:
        return None
    if (not isinstance(result, dict) or not positive_number(result.get("number"))
            or result.get("repository_url") != REPOSITORY_URL):
        raise Refusal("Unexpected parent Issue response")
    return result


def child_list(number: int) -> list[dict[str, Any]]:
    return collection(f"issues/{number}/sub_issues")


def blocked_by(number: int) -> list[dict[str, Any]]:
    return collection(f"issues/{number}/dependencies/blocked_by")


def blocking(number: int) -> list[dict[str, Any]]:
    return collection(f"issues/{number}/dependencies/blocking")


def numbers(items: list[dict[str, Any]]) -> list[int]:
    return [i["number"] for i in items]


def inspect(number: int) -> None:
    target = issue(number)
    ancestor = parent(number)
    print(json.dumps({
        "issue": target["html_url"],
        "state": target["state"],
        "parent": ancestor["number"] if ancestor else None,
        "children": numbers(child_list(number)),
        "blocked_by": numbers(blocked_by(number)),
        "blocking": numbers(blocking(number)),
    }, sort_keys=True))


def verify(check, description: str) -> None:
    for attempt in range(5):
        if check():
            print("PASS: " + description)
            return
        if attempt < 4:
            time.sleep(1)
    raise Refusal("post-write verification failed: " + description)


def add_child(parent_number: int, child_number: int) -> None:
    leader = issue(parent_number, mutation=True)
    junior = issue(child_number, mutation=True)
    existing = parent(child_number)
    children = child_list(parent_number)
    if existing:
        if existing["number"] == parent_number and child_number in numbers(children):
            print(f"SKIP: #{child_number} already belongs under #{parent_number}")
            return
        raise Refusal(f"#{child_number} already has an incompatible parent")
    if child_number in numbers(children):
        raise Refusal("parent/child responses disagree; refusing mutation")
    # A child Issue must explicitly identify this parent in its proposal, not
    # get arbitrarily moved by an agent with only a number.
    if (f"#{parent_number}" not in (junior.get("body") or "")
            and f"/issues/{parent_number}" not in (junior.get("body") or "")):
        raise Refusal("child Issue does not document its intended parent")
    # No replace_parent flag: GitHub itself checks cycles and hierarchy limits.
    result = call("-X", "POST", f"{BASE}/issues/{parent_number}/sub_issues",
                  "-F", f"sub_issue_id={junior['id']}")
    if not isinstance(result, dict) or result.get("id") != junior["id"]:
        raise Refusal("add-child response did not confirm the intended Issue")
    verify(lambda: (p := parent(child_number)) is not None
           and p["number"] == parent_number
           and child_number in numbers(child_list(parent_number)),
           f"native #{parent_number} -> #{child_number} hierarchy")


def add_blocker(number: int, prerequisite: int) -> None:
    affected = issue(number, mutation=True)
    blocking_issue = issue(prerequisite, mutation=True)
    before = blocked_by(number)
    if prerequisite in numbers(before):
        print(f"SKIP: #{number} is already blocked by #{prerequisite}")
        return
    # Only a documented, explicit engineering prerequisite may become an edge.
    if not re.search(rf"(?im)^\s*(?:[-*]\s*)?Blocked by\s+#?{prerequisite}\b",
                     affected.get("body") or ""):
        raise Refusal(f"#{number} lacks an explicit 'Blocked by #{prerequisite}' rationale")
    if number in numbers(blocked_by(prerequisite)):
        raise Refusal("reverse blocked-by relationship would create a cycle")
    result = call("-X", "POST", f"{BASE}/issues/{number}/dependencies/blocked_by",
                  "-F", f"issue_id={blocking_issue['id']}")
    if not isinstance(result, dict) or result.get("id") != blocking_issue["id"]:
        raise Refusal("add-blocker response did not confirm the intended Issue")
    verify(lambda: prerequisite in numbers(blocked_by(number))
           and number in numbers(blocking(prerequisite)),
           f"native #{number} blocked by #{prerequisite}")


def self_test() -> None:
    assert parse("/miracle-project relations 29") == ("inspect", 29, None)
    assert positive_id(5_970_924_253)
    assert not positive_number(5_970_924_253)
    assert not positive_id(True)
    assert parse('/miracle-project add-child {"parent":29,"child":36}') == ("child", 29, 36)
    assert parse('/miracle-project add-blocker {"issue":44,"blocked_by":36}') == (
        "blocker", 44, 36)
    for bad in (
        "/miracle-project relations 29; sleep 1",
        "/miracle-project relations 0",
        '/miracle-project add-child {"parent":29,"child":29}',
        '/miracle-project add-child {"parent":29,"child":36,"replace_parent":true}',
        '/miracle-project add-child {"parent":29,"child":36,"child":37}',
        '/miracle-project add-child {"parent":true,"child":36}',
        '/miracle-project add-child {"parent":29,"child":"36"}',
        '/miracle-project add-blocker {"issue":36,"blocked_by":36}',
        '/miracle-project add-child {"parent":29,"child":36} trailing',
        '/miracle-project add-child {"parent":[],"child":36}',
    ):
        try:
            parse(bad)
        except Refusal:
            continue
        raise AssertionError("unsafe relationship command accepted: " + bad)
    print("PASS: relationship command parser, field validation and 10 negative fixtures")


def main() -> None:
    if sys.argv[1:] == ["--self-test"]:
        self_test()
        return
    if len(sys.argv) != 1:
        raise Refusal("Usage: issue-relations.py [--self-test]")
    operation, first, second = parse(os.environ.get("REQUEST", ""))
    if operation == "inspect":
        inspect(first)
    elif operation == "child":
        assert second is not None
        add_child(first, second)
    else:
        assert second is not None
        add_blocker(first, second)


if __name__ == "__main__":
    try:
        main()
    except Refusal as exc:
        print("ERROR: " + str(exc), file=sys.stderr)
        raise SystemExit(1)
