#!/usr/bin/env python3
"""Read-only live GitHub G6 public collaboration surface audit.

Checks the REAL repository's Discussions categories and Q&A capability against
the source-controlled navigation. It does not create Discussions, select
answers, deliver confidential reports, or modify repository settings.
"""
from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys
from typing import Any

REPO = "spwn02/Miracle"
PROJECT = "https://github.com/spwn02/Miracle"
CATEGORIES = ("q-a", "ideas", "announcements", "show-and-tell")
QUERY = """
query {
  repository(owner:"spwn02",name:"Miracle") {
    nameWithOwner
    hasDiscussionsEnabled
    discussionCategories(first:100) {
      pageInfo { hasNextPage }
      nodes { slug name isAnswerable }
    }
  }
}
"""


class Refusal(RuntimeError):
    pass


def gh(*args: str) -> dict[str, Any]:
    proc = subprocess.run(["gh", "api", *args],
                          capture_output=True, text=True, check=False)
    if proc.returncode:
        raise Refusal("GitHub API request failed; inspect authenticated Actions run")
    try:
        value = json.loads(proc.stdout)
    except ValueError as exc:
        raise Refusal("GitHub API returned non-JSON content") from exc
    if not isinstance(value, dict) or value.get("errors"):
        raise Refusal("GitHub API denied the query or GraphQL schema changed")
    return value


def assess(meta: dict[str, Any], response: dict[str, Any],
           pages: dict[str, str]) -> list[dict[str, Any]]:
    if (meta.get("full_name") != REPO or meta.get("default_branch") != "master"
            or meta.get("archived") is not False
            or meta.get("has_discussions") is not True):
        raise Refusal("Repository identity or Discussions setting differs from contract")
    repository = (((response.get("data") or {}).get("repository")) or {})
    if repository.get("nameWithOwner") != REPO or repository.get("hasDiscussionsEnabled") is not True:
        raise Refusal("GraphQL repository identity or Discussions setting mismatch")
    connection = repository.get("discussionCategories") or {}
    if (connection.get("pageInfo") or {}).get("hasNextPage") is not False:
        raise Refusal("Discussion categories are incompletely enumerated")
    nodes = connection.get("nodes")
    if not isinstance(nodes, list) or len(nodes) > 100:
        raise Refusal("Unexpected discussion category list")
    seen: dict[str, dict] = {}
    for node in nodes:
        if not isinstance(node, dict) or not isinstance(node.get("slug"), str):
            raise Refusal("Invalid Discussion category")
        slug = node["slug"]
        if slug in seen:
            raise Refusal("Duplicate Discussion category slug")
        seen[slug] = node
    missing = set(CATEGORIES) - set(seen)
    if missing:
        raise Refusal("Required live Discussion categories missing: "+", ".join(sorted(missing)))
    if seen["q-a"].get("isAnswerable") is not True:
        raise Refusal("Q&A category is not configured to accept marked answers")
    chooser = pages.get(".github/ISSUE_TEMPLATE/config.yml", "")
    support = pages.get("SUPPORT.md", "")
    contributing = pages.get("CONTRIBUTING.md", "")
    for slug in CATEGORIES:
        link = f"{PROJECT}/discussions/categories/{slug}"
        if link not in support:
            raise Refusal("Public SUPPORT.md misses category "+slug)
        if slug in ("q-a", "ideas") and link not in chooser:
            raise Refusal("GitHub issue chooser misses category "+slug)
    if f"{PROJECT}/discussions/categories/q-a" not in contributing:
        raise Refusal("Contributor guide misses the live Q&A route")
    security = pages.get("SECURITY.md", "")
    conduct = pages.get("CODE_OF_CONDUCT.md", "")
    if ("private vulnerability reporting" not in security.lower()
            or "confidential" not in conduct.lower()):
        raise Refusal("Published security/conduct route is missing from repository sources")
    return [{k:node.get(k) for k in ("slug","name","isAnswerable")}
            for slug,node in sorted(seen.items()) if slug in CATEGORIES]


def self_test() -> None:
    source={
        ".github/ISSUE_TEMPLATE/config.yml":
            "\n".join(f"{PROJECT}/discussions/categories/{slug}" for slug in ("q-a", "ideas")),
        "SUPPORT.md":
            "\n".join(f"{PROJECT}/discussions/categories/{slug}" for slug in CATEGORIES),
        "CONTRIBUTING.md":PROJECT+"/discussions/categories/q-a",
        "SECURITY.md":"Private vulnerability reporting",
        "CODE_OF_CONDUCT.md":"confidential contact",
    }
    meta={"full_name":REPO,"default_branch":"master","archived":False,"has_discussions":True}
    def payload(answerable: bool = True, slugs: tuple[str,...] = CATEGORIES):
        return {"data":{"repository":{"nameWithOwner":REPO,"hasDiscussionsEnabled":True,
            "discussionCategories":{"pageInfo":{"hasNextPage":False},
            "nodes":[{"slug":slug,"name":slug,"isAnswerable":answerable if slug=="q-a" else False}
                     for slug in slugs]}}}}
    assert len(assess(meta,payload(),source)) == 4
    for bad_meta,resp,pages in [
        ({**meta,"has_discussions":False},payload(),source),
        (meta,payload(False),source),
        (meta,payload(True,("ideas","announcements","show-and-tell")),source),
        (meta,{"data":{"repository":{"nameWithOwner":REPO,"hasDiscussionsEnabled":True,
          "discussionCategories":{"pageInfo":{"hasNextPage":True},"nodes":[]}}}},source),
        (meta,payload(),{**source,"SECURITY.md":""}),
        (meta,payload(),{**source,"SUPPORT.md":""}),
    ]:
        try:
            assess(bad_meta,resp,pages)
        except Refusal:
            continue
        raise AssertionError("Invalid live community or source fixture accepted")
    print("PASS: G6 live Discussions categories/answers, source route and fail-closed fixtures")


def main() -> None:
    if sys.argv[1:] == ["--self-test"]:
        self_test()
        return
    if sys.argv[1:]:
        raise Refusal("Usage: audit-g6-community.py [--self-test]")
    meta = gh(f"repos/{REPO}")
    response = gh("graphql","-f","query="+QUERY)
    sources={}
    for path in ("SUPPORT.md","CONTRIBUTING.md","SECURITY.md",
                 "CODE_OF_CONDUCT.md",".github/ISSUE_TEMPLATE/config.yml"):
        try:
            sources[path]=Path(path).read_text(encoding="utf-8")
        except OSError as exc:
            raise Refusal("Required checked-out source is unavailable: "+path) from exc
    categories=assess(meta,response,sources)
    print(f"PASS: {REPO} live Discussions enabled on master")
    for category in categories:
        print("CATEGORY: "+json.dumps(category,sort_keys=True))
    print("PASS: Required category slugs, Q&A answerability, and published route source agree.")
    print("NOT TESTED: an actual accepted-answer interaction or confidential report delivery.")
    print("READ-ONLY: No Project, Issue, Discussion, reporting channel or release was modified.")


if __name__ == "__main__":
    try:
        main()
    except Refusal as exc:
        print("ERROR: "+str(exc),file=sys.stderr)
        raise SystemExit(1)
