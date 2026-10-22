#!/usr/bin/env python3
"""Populate Miracle Development Project #3 with catalog references.

Default is read-only. Apply creates GitHub Projects draft items, not repository
issues. Existing tasks and the workflow Status field are never touched.
"""
import argparse
import hashlib
from collections import Counter
import json
from pathlib import Path
import re
import subprocess
import sys

CATALOG = Path("docs/contracts/PROJECT-CATALOG.json")
TYPES = ("Contract", "Phase", "Chapter", "Checkpoint", "Ledger")
VIEWS = {"Contract": "Archive - Contracts",
         "Phase": "Archive - Phases",
         "Chapter": "Archive - Chapters",
         "Checkpoint": "Archive - Checkpoints",
         "Ledger": "Archive - Decision Ledger"}
FIELD = "Artifact Type"
GROUP_FIELD = "Archive Group"
GROUPS = ("Foundation", "Phase 5.4", "Phase 5.6", "Development Protocol", "Architecture")


def gh(*args):
    result = subprocess.run(["gh", *map(str, args)], capture_output=True,
                            text=True, check=True)
    return json.loads(result.stdout)


def verify(repo, data):
    if data["schemaVersion"] != 1:
        raise ValueError("Invalid catalog schema")
    records = data["items"]
    ids, titles = set(), set()
    for record in records:
        ident, title = record["id"], record["title"]
        if record["kind"] not in TYPES or ident in ids or title in titles:
            raise ValueError("Duplicate or invalid catalog entry: " + ident)
        ids.add(ident)
        titles.add(title)
        path = Path(record["path"])
        if path.is_absolute() or ".." in path.parts:
            raise ValueError("Unsafe catalog source path")
        target = (repo / path).resolve()
        if not target.is_relative_to(repo) or not target.is_file():
            raise ValueError("Missing source: " + str(path))
        lines = target.read_text(encoding="utf-8").splitlines()
        lineno = record["line"]
        if not isinstance(lineno, int) or not 1 <= lineno <= len(lines):
            raise ValueError("Invalid line anchor")
        if record["kind"] in ("Chapter", "Checkpoint", "Ledger"):
            actual = re.sub(r"[\x60*_]", "", lines[lineno - 1].lstrip("# ").strip())
            if actual != record["heading"]:
                raise ValueError("Heading drift at " + str(path) + ":" + str(lineno))
        if record.get("parent") and record["parent"] not in (
            entry["id"] for entry in records
        ):
            raise ValueError("Missing parent: " + ident)
    return records


def project(owner, number):
    query = ("query($login:String!,$number:Int!){user(login:$login)"
             "{projectV2(number:$number){id title views(first:50)"
             "{nodes{id name filter}}}}}")
    result = gh("api", "graphql", "-f", "query=" + query,
                "-f", "login=" + owner, "-F", "number=" + str(number))
    value = ((result.get("data") or {}).get("user") or {}).get("projectV2")
    if result.get("errors") or not value or value["title"] != "Miracle Development":
        raise ValueError("Wrong Project or insufficient GitHub Projects access")
    return value


def items(owner, number):
    result = gh("project", "item-list", number, "--owner", owner,
                "--limit", "5000", "--format", "json")["items"]
    if len(result) >= 5000:
        raise ValueError("Project item query may be truncated")
    output = {}
    for item in result:
        title = item.get("title") or (item.get("content") or {}).get("title")
        if title and title.startswith("[Archive/"):
            if title in output:
                raise ValueError("Duplicate Project archive item: " + title)
            output[title] = item
    return output


def select_field(owner, number, label, options_expected):
    def lookup():
        fs = gh("project", "field-list", number, "--owner", owner,
                "--limit", "100", "--format", "json")["fields"]
        return next((f for f in fs if f["name"] == label), None)
    field = lookup()
    if field is None:
        gh("project", "field-create", number, "--owner", owner,
           "--name", label, "--data-type", "SINGLE_SELECT",
           "--single-select-options", ",".join(options_expected), "--format", "json")
        field = lookup()
    options = {o["name"]: o["id"] for o in field.get("options", [])}
    if any(t not in options for t in options_expected):
        raise ValueError("Existing Project field options are incompatible: " + label)
    return field["id"], options


def source_body(record, ref):
    url = ("https://github.com/spwn02/Miracle/blob/" + ref + "/"
           + record["path"] + "#L" + str(record["line"]))
    text = ("<!-- miracle-project-catalog:" + record["id"] + " -->\n"
            "**Canonical Git source:** " + url + "\n\n"
            "**Group:** " + record["group"] + "\n\n"
            "**Record type:** " + record["kind"] + "\n\n")
    if record.get("parent"):
        text += "**Parent record:** " + record["parent"] + "\n\n"
    return (text + "**Historical reference, not a task or approval.** "
            "The source remains authoritative; workflow Status and actual "
            "implementation progress must be verified independently.\n")


def graph(query, **args):
    argv = ["api", "graphql", "-f", "query=" + query]
    for k, v in args.items():
        argv += ["-f", k + "=" + v]
    result = gh(*argv)
    if result.get("errors"):
        raise ValueError(str(result["errors"]))
    return result["data"]


def views_create(proj):
    existing = {v["name"]: v for v in proj["views"]["nodes"]}
    for kind, name in VIEWS.items():
        view = existing.get(name)
        if view is None:
            mutation = ("mutation($id:ID!,$name:String!){createProjectV2View"
                        "(input:{projectId:$id,layout:TABLE_LAYOUT,name:$name})"
                        "{projectV2View{id name filter}}}")
            view = graph(mutation, id=proj["id"], name=name)["createProjectV2View"]["projectV2View"]
        wanted = "artifact-type:" + kind
        if view.get("filter") != wanted:
            mutation = ("mutation($id:ID!,$filter:String!){updateProjectV2View"
                        "(input:{viewId:$id,filter:$filter})"
                        "{projectV2View{id filter}}}")
            graph(mutation, id=view["id"], filter=wanted)
        print("Project view:", name, "(" + wanted + ")")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path("."))
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--apply", action="store_true")
    parser.add_argument("--all", action="store_true",
                        help="Import all entries, rather than a batch of 25")
    args = parser.parse_args()
    root = args.repo.resolve()
    data = json.loads((root / CATALOG).read_text(encoding="utf-8"))
    records = verify(root, data)
    print("Verified source entries:", len(records), dict(Counter(x["kind"] for x in records)))
    if args.self_test:
        return
    owner, number = data["projectOwner"], data["projectNumber"]
    p = project(owner, number)
    found = items(owner, number)
    missing = [x for x in records if x["title"] not in found]
    print("Project missing:", len(missing))
    if not args.apply:
        print("Preview only; after PR #28 merges, run with --apply --all")
        return
    # Refuse to publish broken master-source links from an unmerged PR.
    remote = gh("api", "repos/spwn02/Miracle/contents/" + str(CATALOG) + "?ref=master")
    raw = (root / CATALOG).read_bytes()
    local_sha = hashlib.sha1(b"blob " + str(len(raw)).encode() + b"\0" + raw).hexdigest()
    if remote.get("sha") != local_sha:
        raise ValueError("Local catalog is not byte-identical to current master")
    field_id, options = select_field(owner, number, FIELD, TYPES)
    group_id, groups = select_field(owner, number, GROUP_FIELD, GROUPS)
    for x in missing if args.all else missing[:25]:
        item = gh("project", "item-create", number, "--owner", owner,
                  "--title", x["title"], "--body", source_body(x, data["baseRef"]),
                  "--format", "json")
        if not item.get("id"):
            raise ValueError("Draft item created without ID; inspect before retry")
        gh("project", "item-edit", "--id", item["id"],
           "--project-id", p["id"], "--field-id", field_id,
           "--single-select-option-id", options[x["kind"]], "--format", "json")
        gh("project", "item-edit", "--id", item["id"],
           "--project-id", p["id"], "--field-id", group_id,
           "--single-select-option-id", groups[x["group"]], "--format", "json")
        print("Created:", x["title"])
    views_create(p)
    fresh = items(owner, number)
    left = sum(x["title"] not in fresh for x in records)
    print("Archive records:", len(records) - left, "/", len(records))
    if left:
        print("Rerun with --apply or --apply --all for remaining records.")
        raise SystemExit(3)


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as exc:
        print("ERROR:", exc, file=sys.stderr)
        raise SystemExit(1)
