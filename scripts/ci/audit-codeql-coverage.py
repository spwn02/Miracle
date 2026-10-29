#!/usr/bin/env python3
"""Compare a GitHub CodeQL analyzed-files CSV with Miracle's tracked C++ sources.

This checks file-level extraction evidence, not semantic support for C++26.
It makes no network calls and does not modify CodeQL or GitHub security settings.
"""
from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path
import subprocess
import sys

EXTENSIONS = {".cppm", ".cpp"}
ROOTS = ("src/", "tests/", "examples/", "benchmarks/")
REQUIRED = {
    "Configuration", "Language", "File Path", "Successfully Extracted",
    "Failure Reason",
}


def tracked_sources(repo: Path) -> list[str]:
    output = subprocess.check_output(
        ["git", "-C", str(repo), "ls-files", "-z"], text=False
    )
    paths = [
        path.decode("utf-8") for path in output.split(b"\0") if path
    ]
    return sorted(
        path for path in paths
        if path.startswith(ROOTS) and Path(path).suffix in EXTENSIONS
    )


def match_path(raw: str, inventory: set[str]) -> str | None:
    # GitHub exports may contain repository-relative paths, leading slashes,
    # or absolute runner workspaces. Match full repository paths, not basenames.
    value = raw.replace("\\", "/").rstrip("/")
    matches = [
        candidate for candidate in inventory
        if value == candidate or value.endswith("/" + candidate)
    ]
    if len(matches) > 1:
        raise ValueError("Ambiguous CSV path: " + raw)
    return matches[0] if matches else None


def evaluate(inventory: list[str], rows: list[dict[str, str]]) -> dict:
    candidates = set(inventory)
    extracted: dict[str, bool] = {}
    reasons: dict[str, str] = {}
    unmatched: list[str] = []
    cpp_rows = 0
    for row in rows:
        if "/language:c-cpp" not in row["Configuration"]:
            continue
        cpp_rows += 1
        raw = row["File Path"]
        path = match_path(raw, candidates)
        if path is None:
            unmatched.append(raw)
            continue
        flag = row["Successfully Extracted"].strip().lower()
        if flag not in {"true", "false"}:
            raise ValueError(f"Unrecognized extraction flag for {path}: {flag!r}")
        value = flag == "true"
        if path in extracted:
            raise ValueError(f"Duplicate C/C++ extraction record: {path}")
        extracted[path] = value
        reasons[path] = row["Failure Reason"]

    modules = [path for path in inventory if Path(path).suffix in {".cppm", ".cppm"}]
    conventional = [path for path in inventory if Path(path).suffix == ".cpp"]

    def group(paths: list[str]) -> dict:
        return {
            "tracked": len(paths),
            "extracted": sum(extracted.get(path) is True for path in paths),
            "unextracted": sorted(path for path in paths if extracted.get(path) is False),
            "notReported": sorted(path for path in paths if path not in extracted),
        }

    return {
        "schemaVersion": 1,
        "reportKind": "file-extraction-only",
        "semanticCxx26ModulesVerified": False,
        "configuration": "/language:c-cpp",
        "cppCsvRows": cpp_rows,
        "trackedCppFiles": len(inventory),
        "matchedCppFiles": len(extracted),
        "moduleInterfaces": group(modules),
        "conventionalCxx": group(conventional),
        "unmatchedCppCsvPaths": sorted(unmatched),
        "unextractedFailureReasons": {
            path: reasons[path] for path in sorted(extracted) if not extracted[path]
        },
    }


def read_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8-sig") as stream:
        reader = csv.DictReader(stream)
        missing = REQUIRED - set(reader.fieldnames or [])
        if missing:
            raise ValueError("Missing CSV headers: " + ", ".join(sorted(missing)))
        return list(reader)


def self_test() -> None:
    files = [
        "src/private/Test.cpp",
        "src/public/Miracle.cppm",
        "src/public/meta/Reflect.cppm",
    ]
    rows = [
        {"Configuration": "/language:c-cpp", "Language": "C++",
         "File Path": f"/home/runner/work/Miracle/Miracle/{path}",
         "Successfully Extracted": str(index == 0).lower(),
         "Failure Reason": ""}
        for index, path in enumerate(files)
    ]
    report = evaluate(files, rows)
    assert report["trackedCppFiles"] == 3
    assert report["conventionalCxx"]["extracted"] == 1
    assert report["moduleInterfaces"]["tracked"] == 2
    assert report["moduleInterfaces"]["extracted"] == 0
    assert len(report["moduleInterfaces"]["unextracted"]) == 2
    assert report["semanticCxx26ModulesVerified"] is False
    assert match_path("Different.cppm", set(files)) is None
    assert match_path("src/public/Miracle.cppm", set(files)) == files[1]
    assert evaluate(files, rows[:-1])["moduleInterfaces"]["notReported"] == [files[2]]
    try:
        evaluate(files, rows + [rows[0]])
    except ValueError as error:
        assert "Duplicate" in str(error)
    else:
        raise AssertionError("Duplicate CSV record accepted")
    try:
        evaluate(files, [dict(rows[0], **{"Successfully Extracted": "unknown"})])
    except ValueError as error:
        assert "Unrecognized" in str(error)
    else:
        raise AssertionError("Invalid extracted flag accepted")
    print("PASS: CodeQL extraction audit, .cppm/.cppm paths and negative fixtures")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--csv", type=Path, help="GitHub Tool Status analyzed-files CSV")
    parser.add_argument("--repo", type=Path, default=Path("."))
    parser.add_argument("--output", type=Path, help="Write structured JSON audit")
    parser.add_argument("--require-modules", action="store_true",
                        help="Return nonzero if modules are not all extracted")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    if args.self_test:
        self_test()
        return 0
    if args.csv is None:
        parser.error("--csv is required unless --self-test is used")

    try:
        tracked = tracked_sources(args.repo.resolve())
        if not tracked:
            raise ValueError("No tracked C++ sources; check --repo")
        report = evaluate(tracked, read_rows(args.csv))
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1

    serialized = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.write_text(serialized, encoding="utf-8")
    print(serialized, end="")
    module = report["moduleInterfaces"]
    if args.require_modules and (
        module["extracted"] != module["tracked"] or
        report["unmatchedCppCsvPaths"] or
        report["conventionalCxx"]["notReported"]
    ):
        print("ERROR: requested complete extraction not established",
              file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
