#!/usr/bin/env python3
"""Safe, native GitHub Projects field operations from owner-controlled Issue #29.

Read-only: /miracle-project inspect ISSUE
Optimistic guarded write:
  /miracle-project update {"issue":31,"field":"Disposition","expected":"Deferred","value":"Planned"}

Only three Project single-select fields can be changed. No accepted-status,
contract, archive, release, deadline, dependency or Issue-state mutation.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
from typing import Any

PROJECT_ID = "PVT_kwHOCp2Gs84Bna-3"
REPOSITORY = "spwn02/Miracle"
ALLOWED_FIELDS = frozenset({"Kind", "Disposition", "Area"})
INSPECT_PREFIX = "/miracle-project inspect "
UPDATE_PREFIX = "/miracle-project update "

QUERY = """
query($number:Int!, $fieldName:String!) {
  user(login:"spwn02") {
    projectV2(number:3) {
      id
      field(name:$fieldName) {
        ... on ProjectV2SingleSelectField {
          id
          name
          options { id name }
        }
      }
    }
  }
  repository(owner:"spwn02", name:"Miracle") {
    issue(number:$number) {
      number
      url
      state
      projectItems(first:100, includeArchived:true) {
        totalCount
        nodes {
          id
          isArchived
          project { id }
          content { ... on Issue { number url repository { nameWithOwner } } }
          fieldValues(first:100) {
            totalCount
            nodes {
              ... on ProjectV2ItemFieldSingleSelectValue {
                name
                field { ... on ProjectV2SingleSelectField { name } }
              }
            }
          }
        }
      }
    }
  }
}
"""

MUTATION = """
mutation($projectId:ID!, $itemId:ID!, $fieldId:ID!, $optionId:String!) {
  updateProjectV2ItemFieldValue(input:{
    projectId:$projectId
    itemId:$itemId
    fieldId:$fieldId
    value:{singleSelectOptionId:$optionId}
  }) { projectV2Item { id } }
}
"""


class GuardError(RuntimeError):
    pass


def unique_object(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("duplicate JSON property: " + key)
        result[key] = value
    return result


def parse(raw: str) -> tuple[str, int, str | None, str | None, str | None]:
    if raw.startswith(INSPECT_PREFIX):
        number = raw[len(INSPECT_PREFIX):]
        if not re.fullmatch(r"[1-9][0-9]{0,6}", number):
            raise GuardError("inspect requires one positive issue number")
        return ("inspect", int(number), None, None, None)

    if not raw.startswith(UPDATE_PREFIX):
        raise GuardError("unsupported command")
    document = raw[len(UPDATE_PREFIX):]
    if len(document) > 1024:
        raise GuardError("command too large")
    try:
        data = json.loads(document, object_pairs_hook=unique_object)
    except (ValueError, TypeError) as exc:
        raise GuardError("invalid JSON update command") from exc
    if not isinstance(data, dict) or set(data) != {"issue", "field", "expected", "value"}:
        raise GuardError("update requires exactly issue, field, expected and value")
    number = data["issue"]
    field = data["field"]
    expected = data["expected"]
    value = data["value"]
    if type(number) is not int or not 1 <= number <= 9_999_999:
        raise GuardError("invalid issue number")
    if not isinstance(field, str) or field not in ALLOWED_FIELDS:
        raise GuardError("field not in the owner-reviewed allowlist")
    if expected is not None and (not isinstance(expected, str) or not 1 <= len(expected) <= 100):
        raise GuardError("expected must be null or a field-option name")
    if not isinstance(value, str) or not 1 <= len(value) <= 100:
        raise GuardError("value must be one existing single-select option")
    return ("update", number, field, expected, value)


def gh(*args: str) -> Any:
    result = subprocess.run(["gh", *args], capture_output=True, text=True, check=False)
    if result.returncode:
        # Avoid echoing command arguments, API headers, environment or credentials.
        raise GuardError("GitHub request failed (exit status " + str(result.returncode) + ")")
    try:
        return json.loads(result.stdout)
    except json.JSONDecodeError as exc:
        raise GuardError("GitHub returned non-JSON data") from exc


def project_item(number: int, field: str):
    response = gh("api", "graphql", "-f", "query=" + QUERY,
                  "-F", "number=" + str(number), "-f", "fieldName=" + field)
    if response.get("errors"):
        raise GuardError("GraphQL denied the request or changed its schema")
    data = response.get("data") or {}
    project = ((data.get("user") or {}).get("projectV2") or {})
    issue = ((data.get("repository") or {}).get("issue") or {})
    if project.get("id") != PROJECT_ID:
        raise GuardError("wrong Project identity")
    expected_url = "https://github.com/" + REPOSITORY + "/issues/" + str(number)
    if issue.get("number") != number or issue.get("url") != expected_url:
        raise GuardError("wrong or missing Issue identity")
    connection = issue.get("projectItems") or {}
    if connection.get("totalCount", 101) > 100:
        raise GuardError("Project membership pagination incomplete")
    matches = [x for x in connection.get("nodes", [])
               if (x.get("project") or {}).get("id") == PROJECT_ID]
    if len(matches) != 1:
        raise GuardError("Project item missing or ambiguous")
    item = matches[0]
    content = item.get("content") or {}
    if (item.get("isArchived") is not False or
            content.get("number") != number or
            content.get("url") != expected_url or
            (content.get("repository") or {}).get("nameWithOwner") != REPOSITORY or
            (item.get("fieldValues") or {}).get("totalCount", 101) > 100):
        raise GuardError("archived, incomplete or unexpected Project item")
    values: dict[str, str] = {}
    for entry in (item.get("fieldValues") or {}).get("nodes", []):
        if not isinstance(entry, dict) or entry.get("name") is None:
            continue
        field_name = (entry.get("field") or {}).get("name")
        if field_name:
            if field_name in values:
                raise GuardError("duplicate Project field value: " + field_name)
            values[field_name] = entry["name"]
    return project, issue, item["id"], values


def run(raw: str) -> None:
    mode, number, field, expected, desired = parse(raw)
    user = subprocess.run(["gh", "api", "user", "--jq", ".login"],
                          capture_output=True, text=True, check=False)
    if user.returncode or user.stdout.strip() != "spwn02":
        raise GuardError("Project token cannot authenticate as expected owner")

    if mode == "inspect":
        _, issue, item_id, values = project_item(number, "Kind")
        print(json.dumps({"issue": issue["url"], "item_id": item_id,
                          "state": issue["state"], "fields": values},
                         sort_keys=True))
        return

    assert field is not None and desired is not None
    project, issue, item_id, values = project_item(number, field)
    if issue["state"] != "OPEN":
        raise GuardError("this guarded field editor only updates open Issues")

    schema = project.get("field") or {}
    if schema.get("name") != field or not schema.get("id"):
        raise GuardError("target is not a configured single-select Project field")
    options = [o for o in schema.get("options", []) if o.get("name") == desired]
    if len(options) != 1 or not options[0].get("id"):
        raise GuardError("desired option missing or ambiguous")
    current = values.get(field)
    if current == desired:
        print(f"SKIP: Issue #{number} already has {field}={desired}")
        return
    if current != expected:
        raise GuardError(f"concurrent/stale value: {field} currently {current!r}; expected {expected!r}")

    response = gh("api", "graphql", "-f", "query=" + MUTATION,
                  "-f", "projectId=" + PROJECT_ID,
                  "-f", "itemId=" + item_id,
                  "-f", "fieldId=" + schema["id"],
                  "-f", "optionId=" + options[0]["id"])
    if (response.get("errors") or
            (((response.get("data") or {}).get("updateProjectV2ItemFieldValue") or {})
             .get("projectV2Item") or {}).get("id") != item_id):
        raise GuardError("Project field mutation not confirmed")
    _, _, confirmed_id, confirmed_values = project_item(number, field)
    if confirmed_id != item_id or confirmed_values.get(field) != desired:
        raise GuardError("post-write read-back mismatch; inspect Project before retrying")
    print(f"PASS: Issue #{number} {field}: {expected!r} -> {desired!r} (verified)")


def self_test() -> None:
    assert parse("/miracle-project inspect 31") == ("inspect", 31, None, None, None)
    assert parse('/miracle-project update {"issue":29,"field":"Kind","expected":null,"value":"Task"}') == (
        "update", 29, "Kind", None, "Task")
    bad = (
        "/miracle-project inspect 31; rm -rf /",
        "/miracle-project inspect 0",
        '/miracle-project update {"issue":31,"field":"Status","expected":null,"value":"Accepted"}',
        '/miracle-project update {"issue":31,"field":"Kind","expected":null,"value":"Task","value":"Bug"}',
        '/miracle-project update {"issue":31,"field":"Kind","expected":null,"value":"Task","extra":1}',
        '/miracle-project update {"issue":true,"field":"Kind","expected":null,"value":"Task"}',
        '/miracle-project update {"issue":31,"field":"Kind","expected":null,"value":""}',
        '/miracle-project update {"issue":31,"field":[],"expected":null,"value":"Task"}',
        '/miracle-project update {"issue":31,"field":"Kind","expected":null,"value":"Task"} extra',
    )
    for command in bad:
        try:
            parse(command)
        except GuardError:
            pass
        else:
            raise AssertionError("invalid command accepted")
    print("PASS: owner command parsing, field allowlist, and nine negative fixtures.")


if __name__ == "__main__":
    if sys.argv[1:] == ["--self-test"]:
        self_test()
    elif len(sys.argv) == 1:
        try:
            run(os.environ.get("REQUEST", ""))
        except GuardError as exc:
            print("ERROR: " + str(exc), file=sys.stderr)
            raise SystemExit(1)
    else:
        raise SystemExit("Usage: project-item-control.py [--self-test]")
