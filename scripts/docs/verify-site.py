#!/usr/bin/env python3
"""Verify GitHub Pages is generated from the canonical Miracle docs tree."""

from __future__ import annotations

from pathlib import Path
import sys


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: python scripts/docs/verify-site.py <generated-site-dir>", file=sys.stderr)
        return 2

    root = Path(sys.argv[1])
    docs = Path("docs")
    if not docs.is_dir():
        print("The canonical docs/ directory is missing.", file=sys.stderr)
        return 1
    if not root.is_dir():
        print(f"Site output missing: {root}", file=sys.stderr)
        return 1

    # MkDocs is configured with use_directory_urls: true. Every tracked
    # canonical Markdown file must produce a corresponding HTML page; this
    # deliberately includes unlisted infrastructure progress/contract files.
    failures: list[str] = []
    sources = sorted(docs.rglob("*.md"))
    if not sources:
        failures.append("No Markdown sources under docs/")
    for source in sources:
        relative = source.relative_to(docs)
        expected = (
            root / "index.html"
            if relative == Path("index.md")
            else root / relative.with_suffix("") / "index.html"
        )
        if not expected.is_file():
            failures.append(f"{source}: missing generated page {expected}")
        elif expected.stat().st_size == 0:
            failures.append(f"{source}: empty generated page {expected}")

    index = root / "index.html"
    if index.is_file():
        page = index.read_text(encoding="utf-8")
        for key in ("Miracle documentation", "Compiler support", "Meta", "Iter"):
            if key not in page:
                failures.append(f"Landing page does not contain {key!r}")

    search_index = root / "search" / "search_index.json"
    if not search_index.is_file() or search_index.stat().st_size == 0:
        failures.append("MkDocs generated search index missing or empty")

    if failures:
        for problem in failures:
            print(f"ERROR: {problem}", file=sys.stderr)
        return 1

    print(f"PASS: {len(sources)} canonical Markdown pages rendered, "
          "navigation verified, search index present.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
