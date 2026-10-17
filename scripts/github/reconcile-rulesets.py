#!/usr/bin/env python3
"""Read/verify Miracle GitHub protection recipes; apply only on explicit request.

Usage:
    python3 scripts/github/reconcile-rulesets.py --check
    python3 scripts/github/reconcile-rulesets.py --verify
    python3 scripts/github/reconcile-rulesets.py --apply

The script does not store access tokens or change any GitHub settings during
--check or --verify. Applying requires maintainer GitHub CLI admin access.
It only creates missing rulesets; it never silently edits/removes a ruleset.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess
import sys


REPOSITORY = "spwn02/Miracle"
ENDPOINT = f"repos/{REPOSITORY}/rulesets"
BASE = Path(__file__).resolve().parent / "rulesets"
FILES = (
    BASE / "protect-development-branches.json",
    BASE / "protect-release-tags.json",
)
EXPECTED = {
    "Protect Miracle development branches": (
        "branch",
        ("refs/heads/master", "refs/heads/gcc"),
        ("deletion", "non_fast_forward"),
    ),
    "Protect Miracle release tags": (
        "tag",
        ("refs/tags/v*",),
        ("deletion", "update"),
    ),
}


def gh(args: list[str], data: dict | None = None):
    command = ["gh", "api", *args]
    completed = subprocess.run(
        command,
        input=json.dumps(data) if data is not None else None,
        text=True,
        capture_output=True,
        check=False,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            f"GitHub CLI API request failed (exit {completed.returncode}): "
            f"{' '.join(command)}\n{completed.stderr.strip()}"
        )
    try:
        return json.loads(completed.stdout)
    except json.JSONDecodeError as exc:
        raise RuntimeError("GitHub API returned invalid JSON") from exc


def fingerprint(ruleset: dict) -> tuple:
    conditions = ruleset.get("conditions") or {}
    ref = conditions.get("ref_name") or {}
    return (
        ruleset.get("target"),
        ruleset.get("enforcement"),
        tuple(sorted(ref.get("include") or [])),
        tuple(sorted(ref.get("exclude") or [])),
        tuple(sorted(rule.get("type") for rule in (ruleset.get("rules") or []))),
        tuple(sorted(
            (actor.get("actor_type"), actor.get("actor_id"), actor.get("bypass_mode"))
            for actor in (ruleset.get("bypass_actors") or [])
        )),
    )


def load_desired() -> dict[str, dict]:
    desired: dict[str, dict] = {}
    for file in FILES:
        data = json.loads(file.read_text(encoding="utf-8"))
        name = data["name"]
        if name in desired or name not in EXPECTED:
            raise RuntimeError(f"Unexpected or duplicated ruleset: {name}")
        target, refs, rules = EXPECTED[name]
        if data["target"] != target:
            raise RuntimeError(f"Incorrect target in {file}")
        if tuple(data["conditions"]["ref_name"]["include"]) != refs:
            raise RuntimeError(f"Incorrect ref scope in {file}")
        if data["conditions"]["ref_name"]["exclude"]:
            raise RuntimeError(f"Unexpected excluded refs in {file}")
        if tuple(x["type"] for x in data["rules"]) != rules:
            raise RuntimeError(f"Incorrect rule types in {file}")
        if data["enforcement"] != "active" or data.get("bypass_actors") != []:
            raise RuntimeError(f"Unexpected enforcement/bypass in {file}")
        desired[name] = data
    if len(desired) != len(EXPECTED):
        raise RuntimeError("Required ruleset recipe absent")
    return desired


def get_rulesets() -> tuple[dict[str, dict], list[dict]]:
    listed = gh([ENDPOINT + "?per_page=100"])
    if not isinstance(listed, list):
        raise RuntimeError("Unexpected GitHub rulesets listing")
    named: dict[str, dict] = {}
    for entry in listed:
        name = entry["name"]
        if name in named:
            raise RuntimeError(f"Multiple rulesets named {name}")
        named[name] = gh([f"{ENDPOINT}/{entry['id']}"])
    return named, listed


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    options = parser.add_mutually_exclusive_group(required=True)
    options.add_argument("--check", action="store_true")
    options.add_argument("--verify", action="store_true")
    options.add_argument("--apply", action="store_true")
    mode = parser.parse_args()

    recipes = load_desired()
    existing, _ = get_rulesets()
    unknown = sorted(set(existing) - set(recipes))
    if unknown:
        raise RuntimeError(
            f"Other rulesets exist: {unknown}. Review overlapping protections "
            "in GitHub before applying these recipes."
        )
    needs_create: list[str] = []
    for name, spec in recipes.items():
        actual = existing.get(name)
        if actual is None:
            print(f"MISSING: {name}")
            needs_create.append(name)
        elif fingerprint(actual) != fingerprint(spec):
            raise RuntimeError(f"RULESET DRIFT: {name}. Review GitHub settings manually.")
        else:
            print(f"VERIFIED: {name} (active, scoped, no bypass)")

    if mode.verify and needs_create:
        print("Verification incomplete: required rulesets are missing.", file=sys.stderr)
        return 1
    if mode.apply:
        for name in needs_create:
            created = gh(["--method", "POST", ENDPOINT, "--input", "-"], recipes[name])
            print(f"CREATED: {name} (id={created['id']})")
        after, _ = get_rulesets()
        for name, spec in recipes.items():
            if name not in after or fingerprint(after[name]) != fingerprint(spec):
                raise RuntimeError(f"Post-apply ruleset mismatch: {name}")
        print("PASS: both repository rulesets now active.")
    elif mode.check:
        print("READ-ONLY: nothing changed. Use --apply only after maintainer review.")
    else:
        print("PASS: both rulesets match the version-controlled recipes.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (RuntimeError, OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
