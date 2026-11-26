#!/usr/bin/env python3
"""Trace validator for Phase M test IDs.

Definitions are the table rows `| TEST-X-NNN |` of docs/phases/M/IMPLEMENTATION.md; the bold prose headings
(`**TEST-X-NNN..MMM [..]**`) only summarise them and are not counted. References come from `tests/`.

Reports missing (defined, never referenced), duplicate (defined more than once) and orphan (referenced, never
defined) IDs. Exit status is non-zero for duplicates and orphans; missing IDs fail only with --strict-missing.
"""

from __future__ import annotations

import argparse
import re
import sys
from collections import Counter
from pathlib import Path

ID = r"TEST-[A-Z0-9]+(?:-[A-Z0-9]+)*-\d{3}"
ROW = re.compile(rf"^\|\s*({ID})\s*\|", re.M)
REFERENCE = re.compile(rf"({ID})((?:(?:/|\.\.)\d{{3}})*)")
SUFFIX = re.compile(r"(/|\.\.)(\d{3})")
SUFFIXES = {".cpp", ".cppm", ".cmake", ".txt", ".py", ".md"}


def definitions(spec: str) -> Counter[str]:
    found: Counter[str] = Counter()
    for match in ROW.finditer(spec):
        found[match.group(1)] += 1
    return found


def expand_reference(first: str, tail: str) -> set[str]:
    """`TEST-X-001..003` and `TEST-X-001/004/006..008` name every ID they list or span."""
    stem, number = first.rsplit("-", 1)
    found = {first}
    previous = int(number)
    for separator, digits in SUFFIX.findall(tail):
        value = int(digits)
        found.update(f"{stem}-{item:03d}" for item in range(previous + 1 if separator == ".." else value, value + 1))
        previous = value
    return found


def references(root: Path) -> set[str]:
    found: set[str] = set()
    for path in sorted(root.rglob("*")):
        if path.is_file() and path.suffix in SUFFIXES and "build" not in path.parts:
            for first, tail in REFERENCE.findall(path.read_text(encoding="utf-8", errors="replace")):
                found |= expand_reference(first, tail)
    return found


def analyse(spec: str, referenced: set[str]) -> tuple[list[str], list[str], list[str]]:
    defined = definitions(spec)
    missing = sorted(identifier for identifier in defined if identifier not in referenced)
    duplicate = sorted(identifier for identifier, count in defined.items() if count > 1)
    orphan = sorted(identifier for identifier in referenced if identifier not in defined)
    return missing, duplicate, orphan


def self_test() -> None:
    spec = "| TEST-A-001 | x |\n| TEST-A-001 | y |\n| TEST-B-001 | z |\n| TEST-B-002 | w |\n**TEST-D-001..002 [REQUIRED].**\n"
    missing, duplicate, orphan = analyse(spec, {"TEST-B-001", "TEST-C-001"})
    assert missing == ["TEST-A-001", "TEST-B-002"], missing
    assert duplicate == ["TEST-A-001"], duplicate
    assert orphan == ["TEST-C-001"], orphan
    assert expand_reference("TEST-X-001", "/004..006") == {f"TEST-X-00{n}" for n in (1, 4, 5, 6)}
    assert expand_reference("TEST-X-001", "..003") == {f"TEST-X-00{n}" for n in (1, 2, 3)}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--spec", type=Path, default=Path("docs/phases/M/IMPLEMENTATION.md"))
    parser.add_argument("--tests", type=Path, default=Path("tests"))
    parser.add_argument("--strict-missing", action="store_true")
    parser.add_argument("--self-test", action="store_true")
    options = parser.parse_args()

    self_test()
    if options.self_test:
        print("check-test-trace self-test passed")
        return 0

    spec = (options.root / options.spec).read_text(encoding="utf-8")
    missing, duplicate, orphan = analyse(spec, references(options.root / options.tests))
    for label, values in (("missing", missing), ("duplicate", duplicate), ("orphan", orphan)):
        print(f"{label}: {len(values)}")
        for identifier in values:
            print(f"  {identifier}")
    failed = bool(duplicate or orphan or (options.strict_missing and missing))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
