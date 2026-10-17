#!/usr/bin/env python3
"""Fail if a GitHub Actions workflow uses an unpinned external action.

This lexical check intentionally recognizes only action `uses:` entries.
Local relative reusable workflows are allowed. This is a lightweight supply
chain regression guard, not a general YAML schema/security linter.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys


USE = re.compile(r"^\s*(?:-\s*)?uses:\s*([^\s#]+)")
IMMUTABLE = re.compile(r"^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+(?:/[^@\s]+)?@[0-9a-f]{40}$")


def scan(source: str, filename: str) -> list[str]:
    errors: list[str] = []
    for number, line in enumerate(source.splitlines(), start=1):
        if line.lstrip().startswith("#"):
            continue
        found = USE.match(line)
        if found is None:
            continue
        action = found.group(1)
        if action.startswith("./"):
            continue
        if not IMMUTABLE.fullmatch(action):
            errors.append(
                f"{filename}:{number}: external action is not pinned to a 40-digit commit SHA: {action}"
            )
    return errors


def main() -> int:
    valid = "      - uses: actions/checkout@" + "a" * 40 + " # v6\n"
    invalid = "      - uses: actions/checkout@v6\n"
    local = "    uses: ./.github/workflows/ci.yml\n"
    assert not scan(valid, "self-test")
    assert scan(invalid, "self-test")
    assert not scan(local, "self-test")

    folder = Path(".github/workflows")
    files = sorted(folder.glob("*.yml")) + sorted(folder.glob("*.yaml"))
    if not files:
        print("No GitHub Actions workflows found.", file=sys.stderr)
        return 1

    errors = [
        issue
        for file in files
        for issue in scan(file.read_text(encoding="utf-8"), str(file))
    ]
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print(f"PASS: all external actions in {len(files)} workflows are SHA-pinned.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
