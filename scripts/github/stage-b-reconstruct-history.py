#!/usr/bin/env python3
"""Evidence-only Miracle implementation-history projection from reachable Git.

Never edits GitHub Project fields, Issue states, or inferred phase acceptance.
Each timestamp denotes a recorded Git event, not phase start or completion.
"""
from __future__ import annotations

from collections import defaultdict
from datetime import datetime
import json
import os
from pathlib import Path
import re
import subprocess
import sys

AREAS = ("Foundation", "Diagnostics", "Iter", "Range", "Meta")
CATALOG = Path("docs/contracts/PROJECT-CATALOG.json")
DEFAULT_OUTPUT = Path("stage-b-roadmap-evidence/reconstructed-activity.json")
EXCLUDE_ROOTS = ("docs/", "scripts/", ".github/", "cmake/", "benchmarks/")
SAFE_SHA = re.compile(r"^[0-9a-f]{40}$")


class EvidenceError(RuntimeError):
    pass


def git(*args: str) -> str:
    done = subprocess.run(("git", *args), capture_output=True, text=True, check=False)
    if done.returncode:
        raise EvidenceError("Git inspection failed (" + " ".join(args[:2]) + ")")
    return done.stdout.strip()


def area_for(path: str) -> str | None:
    # Match real source, tests and examples, not prose that only names a phase.
    if path.startswith(("src/public/meta/", "tests/meta/")) or re.match(
        r"^(?:tests|examples)/(?:meta|bitflags)\b", path
    ):
        return "Meta"
    p = path.lower()
    for label, stem in (
        ("Diagnostics", "diagnostic"), ("Iter", "iter"), ("Range", "range")
    ):
        if (path.startswith(("src/", "tests/", "examples/")) and
            (re.search(r"(?:/|-)" + stem + r"(?=\.|/|-)", p) is not None)):
            return label
    if path.startswith(("src/", "tests/", "examples/")) and (
        path.startswith(("src/private/", "src/public/", "tests/", "examples/"))
    ):
        return "Foundation"
    return None


def events() -> list[dict]:
    head = git("rev-parse", "HEAD")
    if not SAFE_SHA.fullmatch(head):
        raise EvidenceError("invalid master head")
    # Use reachable history of protected master only, not the now-pruned branches.
    rows = git("log", "HEAD", "--format=%H%x09%aI%x09%cI%x09%s").splitlines()
    if not rows:
        raise EvidenceError("no recorded Git history")
    result = []
    for line in rows:
        fields = line.split("\t", 3)
        if len(fields) != 4 or not SAFE_SHA.fullmatch(fields[0]):
            raise EvidenceError("malformed commit history")
        sha, author_date, committer_date, subject = fields
        for stamp in (author_date, committer_date):
            datetime.fromisoformat(stamp)
        names = sorted(set(
            git("diff-tree", "--no-commit-id", "--name-only", "--root", "-r", sha).splitlines()
        ))
        areas = sorted({a for p in names if (a := area_for(p)) is not None})
        result.append({
            "sha": sha, "author_date": author_date,
            "committer_date": committer_date, "subject": subject,
            "implementation_paths": [
                p for p in names if p.startswith(("src/", "tests/", "examples/"))
            ],
            "areas": areas,
        })
    return result


def build() -> dict:
    catalog = json.loads(CATALOG.read_text(encoding="utf-8"))
    entries = catalog.get("items")
    if not isinstance(entries, list) or len(entries) != 172:
        raise EvidenceError("canonical historical catalog count changed")
    ids = [e.get("id") for e in entries]
    if len(set(ids)) != len(ids):
        raise EvidenceError("duplicate catalog identity")
    all_events = events()
    by_area = defaultdict(list)
    for e in all_events:
        for area in e["areas"]:
            by_area[area].append(e)
    spans = []
    for area in AREAS:
        commits = by_area[area]
        dates = sorted(e["committer_date"] for e in commits)
        spans.append({
            "area": area, "evidence_count": len(commits),
            "first_observed_commit_date": dates[0] if dates else None,
            "last_observed_commit_date": dates[-1] if dates else None,
            "commit_shas": [e["sha"] for e in commits],
            "date_semantics": "committerDate of matched source/test/example change; not project start/end or acceptance",
        })
    phase_links = []
    for phase in ("5.4", "5.6"):
        regex = re.compile(r"\bphase[\s._-]*" + re.escape(phase) + r"\b", re.IGNORECASE)
        explicit = [e["sha"] for e in all_events if regex.search(e["subject"])]
        phase_links.append({
            "phase": phase, "explicit_subject_shas": explicit,
            "mapping_state": "insufficient evidence for phase activity bounds or acceptance",
            "observed_dates": None,
            "note": "Git source publication of a frozen phase contract is not evidence of the original implementation interval",
        })
    return {
        "schema": 1,
        "kind": "READ ONLY ACTIVITY EVIDENCE - NOT NATIVE PROJECT ROADMAP",
        "repo": "spwn02/Miracle",
        "master_sha": git("rev-parse", "HEAD"),
        "git_commit_count": len(all_events),
        "canonical_archive_count": len(entries),
        "area_activity_spans": spans,
        "phase_links": phase_links,
        "events": all_events,
        "constraints": [
            "Commits are evidence of recorded Git activity, not certified work start/finish",
            "The phase 5.4 and 5.6 contract imports do not retroactively certify their implementation dates",
            "Do not write evidence dates into Start date/Target date planning fields",
            "Do not synthesize Project Issues or mark historical archive items Accepted",
            "Use independently named historical date fields after reviewed semantic mapping and owner approval",
        ],
    }


def self_test() -> None:
    assert area_for("src/public/meta/Query.ixx") == "Meta"
    assert area_for("src/public/Iter.ixx") == "Iter"
    assert area_for("src/public/Range.ixx") == "Range"
    assert area_for("src/private/Diagnostic.cxx") == "Diagnostics"
    assert area_for("src/public/Types.ixx") == "Foundation"
    assert area_for("docs/phases/phase-5.6-execution-contract-v1.0.md") is None
    assert area_for(".github/workflows/ci.yml") is None
    assert area_for("scripts/github/stage-b-branch-plan.py") is None
    assert area_for("tests/meta/hardening/pointer-sort.cxx") == "Meta"
    print("PASS: implementation-path classification; contracts cannot fabricate phase activity")


def main() -> None:
    if sys.argv[1:] == ["--self-test"]:
        self_test()
        return
    if len(sys.argv) not in (1, 3) or (len(sys.argv) == 3 and sys.argv[1] != "--out"):
        raise EvidenceError("Usage: stage-b-reconstruct-history.py [--self-test | --out PATH]")
    dest = Path(sys.argv[2]) if len(sys.argv) == 3 else DEFAULT_OUTPUT
    if not dest.parent.exists():
        dest.parent.mkdir(parents=True)
    result = build()
    dest.write_text(json.dumps(result, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print("PASS: recorded", result["git_commit_count"], "reachable commits;",
          "172 catalog references; 5 separately evidenced areas; no Project writes")
    for row in result["area_activity_spans"]:
        print(row["area"], row["evidence_count"],
              row["first_observed_commit_date"], row["last_observed_commit_date"])


if __name__ == "__main__":
    try:
        main()
    except (EvidenceError, OSError, ValueError, KeyError, json.JSONDecodeError) as exc:
        print("REFUSED: " + str(exc), file=sys.stderr)
        raise SystemExit(1)
