#!/usr/bin/env python3
"""G6 repository-source intake/navigation regression guard.

No GitHub UI, Discussions category, repository admin setting or private
reporting channel is actually exercised by this file.
"""
from __future__ import annotations

from pathlib import Path
import sys

REPO = "https://github.com/spwn02/Miracle"
FORMS = (
    "bug_report.yml",
    "compiler_issue.yml",
    "feature_proposal.yml",
    "architecture_proposal.yml",
    "infrastructure_issue.yml",
)
POLICIES = ("SECURITY.md", "CODE_OF_CONDUCT.md", "SUPPORT.md",
            "CONTRIBUTING.md", "ACCESSIBILITY.md")
PAGES = ("G6-OPERATING-PLAN.md", "G6-PROGRESS.md")


def scan_text(source: dict[str, str]) -> list[str]:
    errors: list[str] = []

    def require(file: str, fragment: str) -> None:
        if fragment not in source.get(file, ""):
            errors.append(f"{file}: missing {fragment!r}")

    chooser = ".github/ISSUE_TEMPLATE/config.yml"
    require(chooser, "blank_issues_enabled: false")
    require(chooser, REPO + "/blob/master/SECURITY.md")
    require(chooser, REPO + "/blob/master/CONTRIBUTING.md")
    require(chooser, REPO + "/discussions/categories/q-a")
    require(chooser, REPO + "/discussions/categories/ideas")
    for form in FORMS:
        path = ".github/ISSUE_TEMPLATE/" + form
        require(path, "name:")
        require(path, "description:")
        require(path, "body:")
        require("SUPPORT.md", REPO + "/issues/new?template=" + form)

    require("SUPPORT.md", "[SECURITY.md](SECURITY.md)")
    require("SUPPORT.md", "[CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)")
    for category in ("q-a", "ideas", "announcements", "show-and-tell"):
        require("SUPPORT.md", REPO + "/discussions/categories/" + category)
    require("SUPPORT.md", "Answers enabled")
    require("README.md", REPO + "/discussions")
    require("CONTRIBUTING.md", REPO + "/discussions/categories/q-a")
    require("SECURITY.md", "private vulnerability reporting")
    require("CODE_OF_CONDUCT.md", "confidential")
    require("CONTRIBUTING.md", "docs/DEVELOPMENT_PROTOCOL.md")
    require("CONTRIBUTING.md", "SUPPORT.md")
    require("README.md", "CONTRIBUTING.md")
    require("README.md", "SECURITY.md")
    require(".github/PULL_REQUEST_TEMPLATE.md", "Related issue(s)")
    require(".github/PULL_REQUEST_TEMPLATE.md", "Governing contract(s)")
    require(".github/PULL_REQUEST_TEMPLATE.md", "Not run")
    require(".github/PULL_REQUEST_TEMPLATE.md", "Verification evidence")

    for page in PAGES:
        path = "docs/infrastructure/" + page
        if not source.get(path):
            errors.append(path + ": empty or missing")
        require("docs/SUMMARY.md", "infrastructure/" + page)

    require("docs/infrastructure/G6-PROGRESS.md", "has_discussions: true")
    require("docs/infrastructure/G6-OPERATING-PLAN.md", "Project #3")
    return errors


def sources_from_disk(root: Path) -> dict[str, str]:
    paths = [
        "README.md", "SUPPORT.md", "CONTRIBUTING.md", "SECURITY.md",
        "CODE_OF_CONDUCT.md", "ACCESSIBILITY.md", "docs/SUMMARY.md",
        ".github/PULL_REQUEST_TEMPLATE.md",
        ".github/ISSUE_TEMPLATE/config.yml",
        *(".github/ISSUE_TEMPLATE/" + name for name in FORMS),
        *("docs/infrastructure/" + name for name in PAGES),
    ]
    found: dict[str, str] = {}
    for path in paths:
        try:
            found[path] = (root / path).read_text(encoding="utf-8")
        except OSError:
            found[path] = ""
    return found


def negative_fixtures(base: dict[str, str]) -> list[str]:
    faults: list[str] = []
    cases = (
        ("removed bug entry", "SUPPORT.md",
         REPO + "/issues/new?template=bug_report.yml"),
        ("removed security reporting route", "SUPPORT.md",
         "[SECURITY.md](SECURITY.md)"),
        ("removed private reporting link", ".github/ISSUE_TEMPLATE/config.yml",
         REPO + "/blob/master/SECURITY.md"),
        ("removed PR verification evidence", ".github/PULL_REQUEST_TEMPLATE.md",
         "Verification evidence"),
        ("removed Q&A issue chooser route", ".github/ISSUE_TEMPLATE/config.yml",
         REPO + "/discussions/categories/q-a"),
        ("removed Ideas issue chooser route", ".github/ISSUE_TEMPLATE/config.yml",
         REPO + "/discussions/categories/ideas"),
        ("removed support Show and tell route", "SUPPORT.md",
         REPO + "/discussions/categories/show-and-tell"),
        ("removed contributing Q&A route", "CONTRIBUTING.md",
         REPO + "/discussions/categories/q-a"),
        ("missing G6 documentation navigation", "docs/SUMMARY.md",
         "infrastructure/G6-PROGRESS.md"),
    )
    for label, filename, needle in cases:
        source = base[filename]
        if needle not in source:
            faults.append(f"unavailable mutation fixture: {label}")
            continue
        mutated = dict(base)
        mutated[filename] = source.replace(needle, "", 1)
        if not scan_text(mutated):
            faults.append(f"mutation escaped community guard: {label}")
    return faults


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    sources = sources_from_disk(root)
    errors = scan_text(sources)
    if not errors:
        errors = negative_fixtures(sources)
    if errors:
        for error in errors:
            print("ERROR:", error, file=sys.stderr)
        return 1
    print("PASS: G6 source routes, policies, issue/PR intake, mdBook navigation and 9 negative fixtures")
    print("NOTE: GitHub category click-through, real Q&A answers, live forms, private contact and Project #3 require independent checks.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
