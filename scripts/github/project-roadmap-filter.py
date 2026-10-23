#!/usr/bin/env python3
"""Guarded, single-view Roadmap scoping in user-owned Project #3.

The one permitted mutation changes the existing Roadmap filter from empty to:
    is:issue is:open disposition:Planned
No view creation, layout change, item mutation or scheduling/dates.
"""
from __future__ import annotations

import json
import subprocess
import sys
from typing import Any

PROJECT_ID = "PVT_kwHOCp2Gs84Bna-3"
EXPECTED_LOGIN = "spwn02"
EXPECTED_VIEW = "Roadmap"
TARGET_FILTER = "is:issue is:open disposition:Planned"

READ = """
query {
  user(login:"spwn02") {
    projectV2(number:3) {
      id
      views(first:100) {
        totalCount
        nodes { id name layout filter }
      }
    }
  }
}
"""

WRITE = """
mutation($viewId:ID!, $filter:String!) {
  updateProjectV2View(input:{viewId:$viewId,filter:$filter}) {
    projectV2View {
      id name layout filter
      project { id }
    }
  }
}
"""


class Refusal(RuntimeError):
    pass


def gh(*args: str) -> Any:
    result = subprocess.run(["gh", *args], capture_output=True, text=True, check=False)
    if result.returncode != 0:
        raise Refusal("GitHub CLI request failed (exit status " + str(result.returncode) + ")")
    try:
        return json.loads(result.stdout)
    except json.JSONDecodeError as exc:
        raise Refusal("GitHub returned invalid JSON") from exc


def verify_response(raw: Any) -> dict[str, Any]:
    if not isinstance(raw, dict) or raw.get("errors"):
        raise Refusal("GitHub GraphQL schema/access error")
    project = (((raw.get("data") or {}).get("user") or {}).get("projectV2") or {})
    if project.get("id") != PROJECT_ID:
        raise Refusal("Unexpected Project identity")
    views = project.get("views") or {}
    if not isinstance(views.get("totalCount"), int) or views["totalCount"] > 100:
        raise Refusal("Incomplete native view enumeration")
    entries = views.get("nodes")
    if not isinstance(entries, list) or len(entries) != views["totalCount"]:
        raise Refusal("View enumeration incomplete")
    matches = [v for v in entries if v.get("name") == EXPECTED_VIEW]
    if len(matches) != 1 or matches[0].get("layout") != "ROADMAP_LAYOUT":
        raise Refusal("Roadmap missing/ambiguous or wrong layout")
    view = matches[0]
    if not isinstance(view.get("id"), str) or not view["id"]:
        raise Refusal("Roadmap has no stable view identifier")
    if view.get("filter") not in (None, "", TARGET_FILTER):
        raise Refusal("Owner has an unexpected existing Roadmap filter; refusing overwrite")
    return view


def read_view() -> dict[str, Any]:
    return verify_response(gh("api", "graphql", "-f", "query=" + READ))


def execute(mode: str) -> None:
    who = subprocess.run(["gh", "api", "user", "--jq", ".login"],
                         capture_output=True, text=True, check=False)
    if who.returncode or who.stdout.strip() != EXPECTED_LOGIN:
        raise Refusal("Projects credential is not authenticated as Project owner")

    before = read_view()
    existing = before.get("filter") or ""
    if mode == "--plan":
        print("PLAN: " + EXPECTED_VIEW + " existing filter: " + repr(existing))
        print("PLAN: desired filter: " + repr(TARGET_FILTER))
        print("PLAN: only the filter changes. No archived Project items, dates or contracts change.")
        return
    if mode == "--verify":
        if existing != TARGET_FILTER:
            raise Refusal("Roadmap filter not yet configured to show open Planned Issues only")
        print("PASS: live Roadmap filter " + repr(TARGET_FILTER) + "; layout is native ROADMAP_LAYOUT")
        return
    if existing == TARGET_FILTER:
        print("SKIP: Roadmap filter already matches the reviewed contract")
        return
    if existing != "":
        raise Refusal("Unexpected Roadmap filter; refusing to overwrite it")
    response = gh("api", "graphql", "-f", "query=" + WRITE,
                  "-f", "viewId=" + before["id"], "-f", "filter=" + TARGET_FILTER)
    if response.get("errors"):
        raise Refusal("GitHub refused ProjectV2View mutation")
    updated = (((response.get("data") or {}).get("updateProjectV2View") or {})
               .get("projectV2View") or {})
    if (updated.get("id") != before["id"]
            or updated.get("name") != EXPECTED_VIEW
            or updated.get("layout") != "ROADMAP_LAYOUT"
            or updated.get("filter") != TARGET_FILTER
            or (updated.get("project") or {}).get("id") != PROJECT_ID):
        raise Refusal("Mutation response did not confirm exact Roadmap view/filter")
    after = read_view()
    if after["id"] != before["id"] or after.get("filter") != TARGET_FILTER:
        raise Refusal("Post-write Roadmap read-back mismatch; investigate before retry")
    print("PASS: existing Roadmap filter changed from empty to " + repr(TARGET_FILTER))


def self_test() -> None:
    view = {"id": "PVTV_123", "name": "Roadmap", "layout": "ROADMAP_LAYOUT", "filter": ""}
    project = {"data": {"user": {"projectV2": {
        "id": PROJECT_ID, "views": {"totalCount": 1, "nodes": [view]}
    }}}}
    assert verify_response(project)["filter"] == ""
    for filter_value in (TARGET_FILTER, None):
        project["data"]["user"]["projectV2"]["views"]["nodes"][0]["filter"] = filter_value
        assert verify_response(project).get("filter") == filter_value
    for invalid in ("foo", "is:issue", "is:draft"):
        project["data"]["user"]["projectV2"]["views"]["nodes"][0]["filter"] = invalid
        try:
            verify_response(project)
        except Refusal:
            pass
        else:
            raise AssertionError("unexpected view filter was accepted")
    print("PASS: immutable Project/view identity and scoped Roadmap filter fixtures")


if __name__ == "__main__":
    if sys.argv[1:] == ["--self-test"]:
        self_test()
    elif sys.argv[1:] in (["--plan"], ["--verify"], ["--apply"]):
        try:
            execute(sys.argv[1])
        except Refusal as exc:
            print("ERROR: " + str(exc), file=sys.stderr)
            raise SystemExit(1)
    else:
        raise SystemExit("Usage: project-roadmap-filter.py [--self-test|--plan|--verify|--apply]")
