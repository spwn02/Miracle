#!/usr/bin/env python3
"""Read-only reproducible G2 ProjectV2 event provenance check.

Query genuine repository Issues #8, #24 and #49, without modifying their
Project memberships, statuses or histories. GitHub's `wasAutomated` field
must be reported literally; bot attribution alone is not a true value.
"""
from __future__ import annotations

import json
import subprocess
import sys
from typing import Any

ISSUES = (8, 24, 49)
PROJECT_NUMBER = 3
REPO = "spwn02/Miracle"
QUERY = """
query($number:Int!) {
  repository(owner:"spwn02", name:"Miracle") {
    issue(number:$number) {
      number url
      timelineItems(
        first:100,
        itemTypes:[ADDED_TO_PROJECT_V2_EVENT,PROJECT_V2_ITEM_STATUS_CHANGED_EVENT]
      ) {
        totalCount
        pageInfo { hasNextPage endCursor }
        nodes {
          __typename
          ... on AddedToProjectV2Event {
            createdAt
            actor { login }
            wasAutomated
            project { number title url }
          }
          ... on ProjectV2ItemStatusChangedEvent {
            createdAt
            actor { login }
            wasAutomated
            project { number title url }
            previousStatus
            status
          }
        }
      }
    }
  }
}
"""


class AuditError(RuntimeError):
    pass


def decode_response(number: int, data: Any) -> list[dict[str, Any]]:
    if not isinstance(data, dict) or data.get("errors"):
        raise AuditError(f"Issue #{number}: GraphQL schema/permission error")
    issue = (((data.get("data") or {}).get("repository") or {}).get("issue") or {})
    if (issue.get("number") != number
            or issue.get("url") != f"https://github.com/{REPO}/issues/{number}"):
        raise AuditError(f"Issue #{number}: issue identity mismatch")
    timeline = issue.get("timelineItems") or {}
    if (not isinstance(timeline.get("nodes"), list)
            or not isinstance(timeline.get("totalCount"), int)
            or (timeline.get("pageInfo") or {}).get("hasNextPage") is not False
            or timeline["totalCount"] != len(timeline["nodes"])):
        raise AuditError(f"Issue #{number}: timeline pagination incomplete")
    values = []
    for entry in timeline["nodes"]:
        if not isinstance(entry, dict):
            raise AuditError(f"Issue #{number}: malformed event")
        project = entry.get("project") or {}
        if project.get("number") != PROJECT_NUMBER:
            continue
        event_type = entry.get("__typename")
        if event_type not in ("AddedToProjectV2Event", "ProjectV2ItemStatusChangedEvent"):
            raise AuditError(f"Issue #{number}: unexpected Project event type")
        if (type(entry.get("wasAutomated")) is not bool
                or not isinstance(entry.get("createdAt"), str)
                or not isinstance((entry.get("actor") or {}).get("login"), str)
                or project.get("url") != "https://github.com/users/spwn02/projects/3"):
            raise AuditError(f"Issue #{number}: event identity/automation data incomplete")
        values.append({
            "type": event_type,
            "createdAt": entry["createdAt"],
            "actor": entry["actor"]["login"],
            "wasAutomated": entry["wasAutomated"],
            "projectNumber": 3,
            **({"previousStatus": entry.get("previousStatus"), "status": entry.get("status")}
               if event_type == "ProjectV2ItemStatusChangedEvent" else {}),
        })
    return values


def fetch_issue(number: int) -> list[dict[str, Any]]:
    process = subprocess.run(
        ["gh", "api", "graphql", "-f", "query=" + QUERY, "-F", f"number={number}"],
        text=True, capture_output=True, check=False,
    )
    if process.returncode:
        raise AuditError(f"Issue #{number}: GraphQL API invocation failed")
    try:
        return decode_response(number, json.loads(process.stdout))
    except ValueError as exc:
        raise AuditError(f"Issue #{number}: invalid JSON response") from exc


def self_test() -> None:
    def event(automated: bool):
        return {
            "__typename": "AddedToProjectV2Event",
            "createdAt": "2026-10-20T12:34:48Z",
            "actor": {"login": "github-project-automation"},
            "wasAutomated": automated,
            "project": {"number": 3, "title": "Miracle Development",
                        "url": "https://github.com/users/spwn02/projects/3"},
        }
    def payload(events: list[dict[str, Any]]):
        return {"data":{"repository":{"issue":{
            "number":24, "url":"https://github.com/spwn02/Miracle/issues/24",
            "timelineItems":{"totalCount":len(events),
                             "pageInfo":{"hasNextPage":False,"endCursor":None},
                             "nodes":events},
        }}}}
    assert decode_response(24, payload([event(False)]))[0]["wasAutomated"] is False
    assert decode_response(24, payload([event(True)]))[0]["wasAutomated"] is True
    other = event(False)
    other["project"] = {"number":2}
    assert decode_response(24, payload([other])) == []
    truncated = payload([event(False)])
    truncated["data"]["repository"]["issue"]["timelineItems"]["pageInfo"]["hasNextPage"] = True
    try:
        decode_response(24, truncated)
    except AuditError:
        pass
    else:
        raise AssertionError("truncated timeline was accepted")
    missing = payload([event(False)])
    missing["data"]["repository"]["issue"]["timelineItems"]["nodes"][0]["wasAutomated"] = None
    try:
        decode_response(24, missing)
    except AuditError:
        pass
    else:
        raise AssertionError("missing automation flag was accepted")
    print("PASS: exact Project target, true/false preservation, pagination and missing-field fixtures")


def main() -> None:
    if sys.argv[1:] == ["--self-test"]:
        self_test()
        return
    if sys.argv[1:]:
        raise AuditError("Usage: audit-g2-provenance.py [--self-test]")
    who = subprocess.run(["gh","api","user","--jq",".login"],
                         capture_output=True,text=True,check=False)
    if who.returncode or who.stdout.strip() != "spwn02":
        raise AuditError("Project credential cannot authenticate as spwn02")
    all_results = {}
    for number in ISSUES:
        events = fetch_issue(number)
        all_results[number] = events
        adds = [x for x in events if x["type"] == "AddedToProjectV2Event"]
        print(f"ISSUE #{number}: Project #3 additions={len(adds)}, status-events="
              f"{len(events) - len(adds)}")
        for x in events:
            print("  " + json.dumps(x, sort_keys=True))
        bots = [x for x in adds if x["actor"] == "github-project-automation"]
        if not bots:
            print("  INCONCLUSIVE: no bot-authored Project #3 add event exposed")
        elif all(x["wasAutomated"] is False for x in bots):
            print("  DISCREPANCY: bot-authored Project #3 additions have wasAutomated=false")
        else:
            print("  OBSERVED: at least one bot-authored add event returned wasAutomated=true")
    print("READ-ONLY: No Project items, Issues, source files or automation settings were changed.")
    print("No inference of causality or acceptance from actor identity or field labels.")
    if not all(any(x["type"] == "AddedToProjectV2Event" for x in result)
               for result in all_results.values()):
        raise AuditError("At least one expected genuine Issue has no visible Project #3 add event")


if __name__ == "__main__":
    try:
        main()
    except AuditError as exc:
        print("ERROR: " + str(exc), file=sys.stderr)
        raise SystemExit(1)
