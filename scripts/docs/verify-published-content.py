#!/usr/bin/env python3
"""Compare every canonical mdBook chapter's full main content to public Pages.

Run only after a successful master Pages deployment, against a fresh local
mdBook build from exactly the deployed commit. Checks full rendered prose and
link targets, not just a heading or sample sentence; external side effects none.
"""
from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor, as_completed
from html.parser import HTMLParser
import os
from pathlib import Path
import re
import sys
from urllib.request import Request, urlopen

BASE = "https://spwn02.github.io/Miracle/"
CHAPTER = re.compile(r"^\s*(?:[-*]\s+)?\[[^\]]+\]\(([^)]+\.md)\)\s*$")


class MainContent(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.depth = 0
        self.content: list[tuple[str, str]] = []
        self.main_seen = False
        self.ignoring = 0

    def handle_starttag(self, tag, attrs) -> None:
        attrs_map = dict(attrs)
        if tag == "main" and not self.main_seen:
            self.main_seen = True
            self.depth = 1
        elif self.depth:
            if tag not in ("img", "input", "br", "hr", "meta", "link", "source",
                           "wbr", "area", "base", "embed", "track", "col"):
                self.depth += 1
        else:
            return
        if tag in ("script", "style", "svg"):
            self.ignoring += 1
        if self.ignoring == 0:
            if tag in ("h1", "h2", "h3", "h4", "h5", "h6", "p", "li", "code", "blockquote", "table", "tr"):
                self.content.append(("boundary", tag))
            if tag == "a" and attrs_map.get("href"):
                self.content.append(("href", attrs_map["href"]))
            if tag in ("input", "img"):
                values = tuple((k, v) for k, v in attrs if k in ("alt", "checked", "disabled", "type"))
                self.content.append((tag, repr(values)))

    def handle_startendtag(self, tag, attrs) -> None:
        # HTMLParser's default invokes handle_endtag even for self-closing
        # void elements. Those never increment depth in handle_starttag.
        self.handle_starttag(tag, attrs)
        if tag not in ("img", "input", "br", "hr", "meta", "link", "source",
                       "wbr", "area", "base", "embed", "track", "col"):
            self.handle_endtag(tag)

    def handle_endtag(self, tag) -> None:
        if self.depth:
            if tag in ("script", "style", "svg"):
                self.ignoring = max(0, self.ignoring - 1)
            self.depth -= 1

    def handle_data(self, text) -> None:
        if self.depth and not self.ignoring:
            value = " ".join(text.split())
            if value:
                self.content.append(("text", value))


def normalized(html: str) -> tuple[tuple[str, str], ...]:
    parser = MainContent()
    parser.feed(html)
    if not parser.main_seen:
        raise ValueError("Missing <main> landmark in generated/published page")
    if len(parser.content) < 3:
        raise ValueError("Suspiciously empty <main> content")
    return tuple(parser.content)


def chapters() -> list[str]:
    matches = []
    for row in Path("docs/SUMMARY.md").read_text(encoding="utf-8").splitlines():
        m = CHAPTER.fullmatch(row)
        if m:
            matches.append(str(Path(m[1]).with_suffix(".html")).replace("\\", "/"))
    if not matches or len(matches) != len(set(matches)):
        raise ValueError("Empty or duplicate canonical mdBook chapter list")
    return matches


def verify_one(rel: str) -> str:
    local = Path("site") / rel
    expected = normalized(local.read_text(encoding="utf-8"))
    uri = BASE + rel + "?revision=" + os.environ.get("GITHUB_SHA", "manual")[:12]
    request = Request(uri, headers={"User-Agent": "Miracle-G3-FullContent/1.0",
                                   "Cache-Control": "no-cache"})
    with urlopen(request, timeout=25) as response:
        if response.status != 200:
            raise ValueError(f"HTTP {response.status} for {uri}")
        body = response.read(4_000_001)
        if len(body) > 4_000_000:
            raise ValueError(f"Unexpected oversized deployed chapter {uri}")
        final = response.geturl()
        if not final.startswith(BASE):
            raise ValueError(f"Unexpected remote redirect {final}")
    actual = normalized(body.decode("utf-8"))
    if actual != expected:
        for i, (a, b) in enumerate(zip(expected, actual)):
            if a != b:
                raise ValueError(f"{rel}: DIFFERENCE at token {i}: local={a!r}, public={b!r}")
        raise ValueError(f"{rel}: mismatched token counts local={len(expected)}, public={len(actual)}")
    return rel


def self_test() -> None:
    doc = '<html><main><h1>Title</h1><p>Some <a href="x.html">linked text</a>.</p></main></html>'
    assert normalized(doc) == normalized(doc)
    assert normalized(doc) != normalized(doc.replace("linked text", "stale text"))
    assert normalized(doc) != normalized(doc.replace("x.html", "wrong.html"))
    try:
        normalized("<html><body>Missing main</body></html>")
    except ValueError:
        pass
    else:
        raise AssertionError("No-main negative fixture falsely passed")
    print("PASS: full chapter parity extraction and negative fixtures")


def main() -> int:
    if len(sys.argv) == 2 and sys.argv[1] == "--self-test":
        self_test()
        return 0
    if len(sys.argv) != 1:
        print("usage: verify-published-content.py [--self-test]", file=sys.stderr)
        return 2
    try:
        paths = chapters()
        errors: list[str] = []
        with ThreadPoolExecutor(max_workers=6) as pool:
            futures = {pool.submit(verify_one, path): path for path in paths}
            for future in as_completed(futures):
                rel = futures[future]
                try:
                    future.result()
                    print(f"PASS: full canonical main content {rel}", flush=True)
                except (OSError, ValueError, UnicodeError) as exc:
                    errors.append(f"{rel}: {exc}")
        if errors:
            for error in errors:
                print(f"ERROR: {error}", file=sys.stderr)
            return 1
        print(f"PASS: {len(paths)}/{len(paths)} full chapter bodies match deployed public Pages")
        return 0
    except (OSError, ValueError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
