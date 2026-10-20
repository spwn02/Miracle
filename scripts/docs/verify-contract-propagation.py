#!/usr/bin/env python3
"""Hermetic G3 contract-content propagation mutation, never committed/published.

A temporary *copy* of one LOCKED contract receives a unique marker and is
rendered by the exact pinned mdBook binary. The authoritative repository file
is read-only and checked unchanged. Live Pages propagation is tested separately
using genuine canonical documentation edits.
"""
from __future__ import annotations

from hashlib import sha256
from pathlib import Path
from tempfile import TemporaryDirectory
import shutil
import subprocess
import sys


CONTRACT = Path("docs/infrastructure/GITHUB-INFRASTRUCTURE-CONTRACT.md")
PROBE = "G3-ISOLATED-MDBOOK-CONTRACT-RENDER-NEGATIVE-CONTROL"
TEXT = "Miracle GitHub Infrastructure Contract v1.0"


def test() -> None:
    original = CONTRACT.read_bytes()
    before = sha256(original).hexdigest()
    if original.count(TEXT.encode()) != 1:
        raise ValueError("Pinned contract heading missing or ambiguous")

    with TemporaryDirectory(prefix="miracle-g3-mutation-") as directory:
        root = Path(directory)
        shutil.copytree("docs", root / "docs")
        shutil.copyfile("book.toml", root / "book.toml")
        if Path("theme").is_dir():
            shutil.copytree("theme", root / "theme")
        temp_contract = root / CONTRACT
        modified = original.replace(TEXT.encode(), (TEXT + " " + PROBE).encode(), 1)
        temp_contract.write_bytes(modified)

        build = subprocess.run(["mdbook", "build"], cwd=root, text=True,
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               timeout=120, check=False)
        if build.returncode:
            raise RuntimeError(f"Isolated mdBook build failed:\n{build.stdout[-3000:]}")
        target = root / "site/infrastructure/GITHUB-INFRASTRUCTURE-CONTRACT.html"
        rendered = target.read_text(encoding="utf-8")
        if PROBE not in rendered:
            raise AssertionError("Contract mutation NOT propagated to isolated output")
        if PROBE in (root / "docs/index.md").read_text(encoding="utf-8"):
            raise AssertionError("Mutation unexpectedly propagated to unrelated source")
        print("PASS: temporary frozen-contract source mutation appears in generated HTML")

    after = sha256(CONTRACT.read_bytes()).hexdigest()
    if before != after:
        raise AssertionError("Authoritative LOCKED contract was modified")
    print("PASS: authoritative contract byte hash unchanged; no synthetic commit")


if __name__ == "__main__":
    try:
        test()
    except (OSError, ValueError, RuntimeError, AssertionError) as error:
        print(f"ERROR: G3 hermetic contract mutation test: {error}", file=sys.stderr)
        raise SystemExit(1)
