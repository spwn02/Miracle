#!/usr/bin/env python3
"""Owner-only native GitHub Project #3 Issue intake, no duplicate Issue creation.

  /miracle-project intake-inspect 49
  /miracle-project intake-add 49

Adds an EXISTING open repository Issue only when absent. Never changes Status,
Disposition, dates, archived drafts, frozen contracts or other Project fields.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
import time
from typing import Any

PROJECT_ID = "PVT_kwHOCp2Gs84Bna-3"
REPO = "spwn02/Miracle"
PREFIX = "/miracle-project intake-"
QUERY = """
query($number:Int!) {
  user(login:"spwn02") { projectV2(number:3) { id } }
  repository(owner:"spwn02", name:"Miracle") {
    issue(number:$number) {
      id number url title body state
      projectItems(first:100, includeArchived:true) {
        totalCount
        nodes {
          id
          isArchived
          project { id }
          content {
            ... on Issue { id number url repository { nameWithOwner } }
          }
        }
      }
    }
  }
}
"""
ADD = """
mutation($projectId:ID!, $contentId:ID!) {
  addProjectV2ItemById(input:{projectId:$projectId,contentId:$contentId}) {
    item { id project { id } content { ... on Issue { id number } } }
  }
}
"""


class Refusal(RuntimeError):
    pass


def parse(raw: str) -> tuple[str, int]:
    matched = re.fullmatch(r"/miracle-project intake-(inspect|add) ([1-9][0-9]{0,6})", raw)
    if not matched:
        raise Refusal("expected one exact intake-inspect/intake-add command with Issue number")
    return matched.group(1), int(matched.group(2))


def gh(*args: str) -> Any:
    command = subprocess.run(["gh", "api", *args], capture_output=True, text=True, check=False)
    if command.returncode:
        raise Refusal("GitHub API request failed; inspect owner token and Project access")
    try:
        value = json.loads(command.stdout)
    except ValueError as exc:
        raise Refusal("GitHub API returned malformed JSON") from exc
    if not isinstance(value, dict) or value.get("errors"):
        raise Refusal("GitHub GraphQL reported an error")
    return value


def membership(data: dict, number: int) -> tuple[dict, str | None]:
    project = (((data.get("data") or {}).get("user") or {}).get("projectV2") or {})
    issue = (((data.get("data") or {}).get("repository") or {}).get("issue") or {})
    url = f"https://github.com/{REPO}/issues/{number}"
    if project.get("id") != PROJECT_ID:
        raise Refusal("Project ID mismatch")
    if (issue.get("number") != number or issue.get("url") != url or
            not isinstance(issue.get("id"), str) or not issue["id"].startswith("I_") or
            not isinstance(issue.get("title"), str) or not issue["title"] or
            not isinstance(issue.get("body"), str)):
        raise Refusal("Missing, incomplete or unexpected repository Issue")
    connection = issue.get("projectItems") or {}
    total, nodes = connection.get("totalCount"), connection.get("nodes")
    if type(total) is not int or total > 100 or not isinstance(nodes, list) or len(nodes) != total:
        raise Refusal("Incomplete Issue-to-Project membership enumeration")
    matches = [x for x in nodes if (x.get("project") or {}).get("id") == PROJECT_ID]
    if len(matches) > 1:
        raise Refusal("Duplicate Project membership for Issue")
    if matches:
        item = matches[0]
        content = item.get("content") or {}
        if (item.get("isArchived") is not False or
                not isinstance(item.get("id"), str) or not item["id"] or
                content.get("id") != issue["id"] or content.get("number") != number or
                content.get("url") != url or
                (content.get("repository") or {}).get("nameWithOwner") != REPO):
            raise Refusal("Archived or ambiguous Project item; manual investigation required")
        return issue, item["id"]
    return issue, None


def inspect(number: int) -> tuple[dict, str | None]:
    data = gh("graphql", "-f", "query=" + QUERY, "-F", "number=" + str(number))
    return membership(data, number)


def run(mode: str, number: int) -> None:
    who = subprocess.run(["gh", "api", "user", "--jq", ".login"], capture_output=True, text=True)
    if who.returncode or who.stdout.strip() != "spwn02":
        raise Refusal("credential is not authenticated as the expected Project owner")
    issue, item = inspect(number)
    if mode == "inspect":
        print(json.dumps({"issue":issue["url"], "state":issue["state"],
                          "project":"Miracle Development #3",
                          "membership":"active" if item else "absent",
                          "item_id":item},sort_keys=True))
        return
    if issue["state"] != "OPEN" or not issue["body"].strip() or issue["title"].startswith("[Archive/"):
        raise Refusal("only described, open, nonarchive repository Issues may be enrolled")
    if item:
        print(f"SKIP: Issue #{number} already has a unique active Project #3 item")
        return
    result = gh("graphql", "-f", "query=" + ADD,
                "-f", "projectId=" + PROJECT_ID, "-f", "contentId=" + issue["id"])
    candidate = (((result.get("data") or {}).get("addProjectV2ItemById") or {}).get("item") or {})
    if ((candidate.get("project") or {}).get("id") != PROJECT_ID or
            (candidate.get("content") or {}).get("id") != issue["id"] or
            (candidate.get("content") or {}).get("number") != number or
            not candidate.get("id")):
        raise Refusal("add mutation returned unexpected item; inspect before retrying")
    for attempt in range(5):
        again, confirmed = inspect(number)
        if confirmed == candidate["id"] and again["id"] == issue["id"]:
            print(f"PASS: Issue #{number} enrolled in Project #3; native item {confirmed} verified")
            return
        if attempt < 4:
            time.sleep(1)
    raise Refusal("post-write Project membership mismatch; inspect before retrying")


def self_test() -> None:
    assert parse("/miracle-project intake-inspect 49") == ("inspect",49)
    assert parse("/miracle-project intake-add 49") == ("add",49)
    for bad in ("/miracle-project intake-add 0",
                "/miracle-project intake-add 49; echo hi",
                "/miracle-project intake-add +49",
                "/miracle-project intake-add 49 extra",
                "/miracle-project intake-add 49\n",
                "/miracle-project intake-add 99999999",
                "/miracle-project intake-delete 49"):
        try:
            parse(bad)
        except Refusal:
            continue
        raise AssertionError("unsafe command accepted: " + bad)
    issue={"id":"I_abc","number":49,"url":"https://github.com/spwn02/Miracle/issues/49",
           "title":"[Workspace] Intake","body":"## Acceptance criteria\n- A","state":"OPEN"}
    project_item={"id":"PVTI_abc","isArchived":False,"project":{"id":PROJECT_ID},
                  "content":{"id":"I_abc","number":49,"url":issue["url"],
                             "repository":{"nameWithOwner":REPO}}}
    def payload(nodes):
        return {"data":{"user":{"projectV2":{"id":PROJECT_ID}},
                        "repository":{"issue":{**issue,"projectItems":{"totalCount":len(nodes),"nodes":nodes}}}}}
    assert membership(payload([]),49)[1] is None
    assert membership(payload([project_item]),49)[1] == "PVTI_abc"
    for entries in ([project_item,project_item],[{**project_item,"isArchived":True}]):
        try:
            membership(payload(entries),49)
        except Refusal:
            continue
        raise AssertionError("ambiguous/archived membership accepted")
    print("PASS: intake parser, owner-identity contract and unique active Project membership fixtures")


if __name__ == "__main__":
    try:
        if sys.argv[1:] == ["--self-test"]:
            self_test()
        elif len(sys.argv) == 1:
            run(*parse(os.environ.get("REQUEST","")))
        else:
            raise Refusal("Usage: project-issue-intake.py [--self-test]")
    except Refusal as exc:
        print("ERROR: "+str(exc),file=sys.stderr)
        raise SystemExit(1)
