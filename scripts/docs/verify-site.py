#!/usr/bin/env python3
"""Validate mdBook output against the canonical Miracle documentation tree."""

from __future__ import annotations

from pathlib import Path
import re
import sys


CHAPTER = re.compile(r"^\s*(?:[-*]\s+)?\[.+?\]\(([^)]+\.md)\)\s*$")


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: python scripts/docs/verify-site.py <site-directory>", file=sys.stderr)
        return 2

    site = Path(sys.argv[1])
    docs = Path("docs")
    summary = docs / "SUMMARY.md"
    errors: list[str] = []

    if not site.is_dir() or not docs.is_dir() or not summary.is_file():
        print("Missing generated site, docs/, or docs/SUMMARY.md.", file=sys.stderr)
        return 1

    chapters: list[Path] = []
    for line in summary.read_text(encoding="utf-8").splitlines():
        match = CHAPTER.fullmatch(line)
        if match:
            chapter = Path(match.group(1))
            if chapter.is_absolute() or ".." in chapter.parts:
                errors.append(f"Invalid SUMMARY.md chapter path: {chapter}")
            else:
                chapters.append(chapter)

    expected_sources = {p.relative_to(docs) for p in docs.rglob("*.md")}
    expected_sources.remove(Path("SUMMARY.md"))
    referenced = set(chapters)
    for missing in sorted(expected_sources - referenced):
        errors.append(f"Not linked in SUMMARY.md: {missing}")
    for missing in sorted(referenced - expected_sources):
        errors.append(f"SUMMARY.md references missing document: {missing}")
    if len(chapters) != len(referenced):
        errors.append("SUMMARY.md references one or more chapters more than once.")

    # mdBook uses .html URLs, unlike MkDocs' directory/index.html layout.
    for chapter in chapters:
        html = site / chapter.with_suffix(".html")
        if not html.is_file() or html.stat().st_size == 0:
            errors.append(f"Missing or empty mdBook chapter: {html}")

    homepage = site / "index.html"
    if not homepage.is_file():
        errors.append("The mdBook landing page site/index.html is missing.")
    else:
        rendered = homepage.read_text(encoding="utf-8")
        for phrase in ("Miracle documentation", "Compiler support", "Meta", "Iter"):
            if phrase not in rendered:
                errors.append(f"Landing page missing navigation/text: {phrase!r}")

    # mdBook's built-in search emits searchindex.js or searchindex.json,
    # depending on mdBook version. This is not MkDocs' search/ directory.
    if not any(p.is_file() and p.stat().st_size > 0
               for p in (*site.glob("searchindex.js"), *site.glob("searchindex.json"))):
        errors.append("mdBook search index missing or empty.")

    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    print(f"PASS: {len(chapters)} canonical mdBook chapters rendered, "
          "SUMMARY.md complete, navigation and search present.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
