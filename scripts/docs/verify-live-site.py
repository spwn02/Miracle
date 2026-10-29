#!/usr/bin/env python3
"""Representative live mdBook verification after GitHub Pages deployment.

This is an HTTP content/link probe, not a screen-reader test or full crawl.
It never creates GitHub resources and can self-test without network access.
"""
from __future__ import annotations

import argparse
from html.parser import HTMLParser
import os
from pathlib import Path
import sys
import time
from urllib.error import URLError
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[2]
BASE = "https://spwn02.github.io/Miracle"
LINKS = (
    "https://github.com/spwn02/Miracle/tree/master/docs",
    "https://github.com/spwn02/Miracle/blob/master/CONTRIBUTING.md",
    "https://github.com/spwn02/Miracle/issues/new/choose",
)


class TextAndLinks(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.text_parts: list[str] = []
        self.links: set[str] = set()
        self.hidden = 0

    def handle_starttag(self, tag, attrs) -> None:
        if tag in ("script", "style", "svg", "noscript"):
            self.hidden += 1
        if tag in ("p", "li", "h1", "h2", "h3", "div"):
            self.text_parts.append(" ")
        if tag == "a":
            self.links.update(v for k, v in attrs if k == "href" and v)

    def handle_endtag(self, tag) -> None:
        if tag in ("script", "style", "svg", "noscript"):
            self.hidden = max(0, self.hidden - 1)
        if tag in ("p", "li", "h1", "h2", "h3", "div"):
            self.text_parts.append(" ")

    def handle_data(self, data) -> None:
        if not self.hidden:
            self.text_parts.append(data)

    @property
    def text(self) -> str:
        return normalize("".join(self.text_parts))


def normalize(text: str) -> str:
    return " ".join(text.split())


def expected() -> str:
    home = (ROOT / "docs/INDEX.md").read_text(encoding="utf-8")
    paragraph = next(
        (line for line in home.splitlines()
         if line.startswith("The public umbrella module is ")),
        None,
    )
    if paragraph is None:
        raise ValueError("Canonical index paragraph missing")
    for link in LINKS:
        if link not in home:
            raise ValueError(f"Canonical index missing selected external URL: {link}")
    return normalize(paragraph.replace(chr(96), ""))


def verify_page(html: str, phrase: str, links: tuple[str, ...] = ()) -> None:
    parser = TextAndLinks()
    parser.feed(html)
    if normalize(phrase) not in parser.text:
        raise ValueError(f"Published HTML missing canonical paragraph: {phrase!r}")
    for link in links:
        if link not in parser.links:
            raise ValueError(f"Published HTML missing canonical href: {link}")


def fetch(url: str) -> tuple[str, str]:
    request = Request(url, headers={
        "User-Agent": "Miracle-Pages-Smoke/1.0",
        "Cache-Control": "no-cache",
    })
    with urlopen(request, timeout=20) as response:
        final = response.geturl()
        data = response.read(2_000_001)
        if response.status != 200:
            raise ValueError(f"Unexpected HTTP {response.status}: {url}")
        if len(data) > 2_000_000:
            raise ValueError(f"Page exceeds probe size limit: {url}")
    return data.decode("utf-8", errors="replace"), final


def retry(name: str, operation) -> None:
    latest: Exception | None = None
    for attempt in range(1, 9):
        try:
            operation()
            print(f"PASS: {name}", flush=True)
            return
        except (OSError, URLError, ValueError, TimeoutError, UnicodeError) as error:
            latest = error
            if attempt < 8:
                print(f"RETRY {attempt}/8: {name}: {error}", flush=True)
                time.sleep(min(attempt * 2, 10))
    raise RuntimeError(f"{name}: {latest}") from latest


def self_test() -> None:
    sample = '<html><main><p>The public umbrella module is <code>import Miracle;</code>, and the CMake target is <code>Miracle::Miracle</code>.</p><a href="https://github.com/spwn02/Miracle/tree/master/docs">Docs</a></main></html>'
    text = "The public umbrella module is import Miracle;, and the CMake target is Miracle::Miracle."
    verify_page(sample, text, (LINKS[0],))
    for bad_text, bad_links in (
        ("A missing source paragraph", ()),
        (text, (LINKS[1],)),
    ):
        try:
            verify_page(sample, bad_text, bad_links)
            raise AssertionError("Invalid HTML/source fixture unexpectedly passed")
        except ValueError:
            pass
    home = expected()
    assert home.startswith("The public umbrella module is import Miracle;"), home
    print("PASS: offline homepage source/HTML comparison and negative fixtures")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    try:
        if args.self_test:
            self_test()
            return 0
        home = expected()
        suffix = "?verify=" + os.environ.get("GITHUB_SHA", "manual")[:12]

        def live_page(path: str, phrase: str, links: tuple[str, ...] = ()) -> None:
            url = BASE + path + suffix
            def check() -> None:
                html, final = fetch(url)
                if not final.startswith(BASE + "/"):
                    raise ValueError(f"Unexpected Pages redirect: {final}")
                verify_page(html, phrase, links)
            retry(f"live canonical source content {path}", check)

        live_page("/", home, LINKS)
        for link in LINKS:
            def check_link(target=link) -> None:
                _, final = fetch(target)
                if not final.startswith("https://github.com/"):
                    raise ValueError(f"Unexpected outbound redirect: {final}")
            retry("live outbound link " + link, check_link)

        print("PASS: public Pages source-to-render comparison and 3 external links")
        print("LIMIT: no exhaustive link crawling or screen-reader verification")
        return 0
    except (RuntimeError, ValueError, OSError, AssertionError) as error:
        print(f"ERROR: public Pages smoke verification: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
