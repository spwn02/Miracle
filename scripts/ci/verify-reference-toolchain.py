#!/usr/bin/env python3
"""Verify the immutable clang-cxx26 snapshot manifest and installed layout.

This verifies identity/packaging, not language correctness. C++26 modules
and reflection are validated by the real source/consumer builds.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys


def validate(manifest: dict, snapshot: str, revision: str) -> None:
    if not isinstance(manifest, dict):
        raise ValueError("manifest must be a JSON object")
    expected = {
        ("snapshot",): snapshot,
        ("source", "revision"): revision,
        ("source", "repository"): "https://github.com/spwn02/clang-cxx26",
        ("source", "branch"): "cxx26",
        ("compiler", "name"): "clang-cxx26",
        ("platform", "os"): "linux",
        ("platform", "architecture"): "x86_64",
        ("cmake", "toolchainFile"): "share/clang-cxx26/toolchain.cmake",
    }
    for path, wanted in expected.items():
        value = manifest
        for key in path:
            if not isinstance(value, dict) or key not in value:
                raise ValueError(f"missing required manifest field: {'.'.join(path)}")
            value = value[key]
        if value != wanted:
            raise ValueError(f"unexpected {'.'.join(path)}: {value!r} != {wanted!r}")
    version = manifest.get("cmake", {}).get("minimumConsumerVersion")
    if not isinstance(version, str) or tuple(int(s) for s in version.split(".")[:2]) < (4, 4):
        raise ValueError("CMake toolchain minimum consumer version must be >=4.4")


def validate_installation(root: Path) -> None:
    required = (
        "bin/clang", "bin/clang++", "bin/clangd",
        "share/clang-cxx26/toolchain.cmake",
        "share/clang-cxx26/activate.sh",
        "share/clang-cxx26/manifest.json",
    )
    for path in required:
        if not (root / path).is_file():
            raise ValueError(f"missing installed toolchain component: {path}")
    if not any(root.rglob("libc++.modules.json")):
        raise ValueError("missing matching libc++ standard module metadata")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--snapshot", required=True)
    parser.add_argument("--revision", required=True)
    parser.add_argument("--root", type=Path)
    args = parser.parse_args()
    try:
        manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
        validate(manifest, args.snapshot, args.revision)
        if args.root is not None:
            validate_installation(args.root)
    except (OSError, ValueError, json.JSONDecodeError) as err:
        print(f"ERROR: toolchain provenance validation failed: {err}", file=sys.stderr)
        return 1
    print(f"PASS: {args.snapshot} identity, revision and coherent installation metadata")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
