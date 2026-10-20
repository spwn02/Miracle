#!/usr/bin/env python3
"""Crawl every generated mdBook HTML link and anchor.

Internal links/fragment targets are mandatory. External links are deduplicated
and optionally checked over HTTP with bounded concurrency. Inaccessible/403/429
external endpoints are explicit INDETERMINATE, not silently counted as passing.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
from html.parser import HTMLParser
from pathlib import Path
import posixpath
import re
import sys
from urllib.parse import unquote, urlsplit
from urllib.request import Request, urlopen
from urllib.error import HTTPError, URLError


class Anchors(HTMLParser):
    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.links: list[str] = []
        self.targets: set[str] = set()

    def handle_starttag(self, tag, attrs) -> None:
        fields = dict(attrs)
        if fields.get("id"):
            self.targets.add(fields["id"])
        if tag == "a":
            if fields.get("name"):
                self.targets.add(fields["name"])
            if fields.get("href") is not None:
                self.links.append(fields["href"])


def parse_html(filename: Path) -> Anchors:
    doc = Anchors()
    doc.feed(filename.read_text(encoding="utf-8"))
    return doc


def internal_path(site: Path, page: Path, link: str) -> tuple[Path, str]:
    parsed = urlsplit(link)
    raw = unquote(parsed.path)
    if raw.startswith("/Miracle/"):
        raw = raw.removeprefix("/Miracle/")
        target = site / raw
    elif raw.startswith("/"):
        raise ValueError(f"site-root path is outside /Miracle/: {link}")
    else:
        target = page.parent / raw if raw else page
    if raw.endswith("/") or target.is_dir():
        target = target / "index.html"
    target = target.resolve()
    if not target.is_relative_to(site.resolve()):
        raise ValueError(f"path escapes site directory: {link}")
    return target, unquote(parsed.fragment)


def crawl(site: Path) -> tuple[list[str], set[str], int, int]:
    pages = sorted(site.rglob("*.html"))
    if not pages:
        raise ValueError(f"No generated pages in {site}")
    inventory = {p.resolve(): parse_html(p) for p in pages}
    errors: list[str] = []
    external: set[str] = set()
    checked = 0
    fragments = 0
    for page, html in inventory.items():
        for link in html.links:
            if link.startswith(("mailto:", "tel:")):
                continue
            parsed = urlsplit(link)
            if parsed.scheme in ("https", "http") and parsed.netloc:
                # The public site itself must still resolve to a real generated target.
                if parsed.netloc == "spwn02.github.io" and parsed.path.startswith("/Miracle/"):
                    try:
                        local, frag = internal_path(site, page, parsed.path + ("#" + parsed.fragment if parsed.fragment else ""))
                        if not local.is_file():
                            errors.append(f"{page.relative_to(site)}: absent same-site URL {link}")
                        elif frag and frag not in inventory.get(local, parse_html(local)).targets:
                            errors.append(f"{page.relative_to(site)}: missing same-site anchor {link}")
                    except ValueError as exc:
                        errors.append(f"{page.relative_to(site)}: {exc}")
                else:
                    external.add(link.split("#", 1)[0])
                continue
            if parsed.scheme or parsed.netloc:
                errors.append(f"{page.relative_to(site)}: invalid URI scheme {link}")
                continue
            try:
                target, frag = internal_path(site, page, link)
            except ValueError as exc:
                errors.append(f"{page.relative_to(site)}: {exc}")
                continue
            checked += 1
            if not target.is_file():
                errors.append(f"{page.relative_to(site)}: missing target {link}")
            if frag and target.is_file():
                fragments += 1
                if target.suffix == ".html" and frag not in inventory.get(target, parse_html(target)).targets:
                    errors.append(f"{page.relative_to(site)}: missing fragment {link}")
    print(f"LINK INVENTORY: {len(pages)} rendered HTML files, {checked} internal references, "
          f"{fragments} fragments, {len(external)} unique external URL(s)")
    return errors, external, checked, fragments


def http_check(url: str) -> tuple[str, str]:
    # HEAD is blocked by legitimate hosts; use GET, capped bytes and no credentials.
    try:
        request = Request(url, headers={
            "User-Agent": "Miracle-Docs-LinkCheck/1.0",
            "Accept": "text/html,application/xhtml+xml,*/*",
        })
        with urlopen(request, timeout=16) as response:
            response.read(256)
            status = response.status
            if 200 <= status < 400:
                return url, "PASS"
            return url, f"INDETERMINATE HTTP {status}"
    except HTTPError as exc:
        if exc.code in (404, 410):
            return url, f"BROKEN HTTP {exc.code}"
        return url, f"INDETERMINATE HTTP {exc.code}"
    except (OSError, URLError, TimeoutError, ValueError) as exc:
        return url, f"INDETERMINATE {type(exc).__name__}: {exc}"


def self_test() -> None:
    from tempfile import TemporaryDirectory
    with TemporaryDirectory() as temp:
        site = Path(temp)
        (site / "index.html").write_text(
            '<h1 id="intro">Intro</h1><a href="ok.html#there">yes</a><a href="#intro">here</a>')
        (site / "ok.html").write_text('<h2 id="there">There</h2>')
        errors, urls, _, _ = crawl(site)
        assert not errors and not urls, errors
        (site / "index.html").write_text('<a href="ok.html#missing">bad anchor</a>')
        assert any("missing fragment" in x for x in crawl(site)[0])
        (site / "index.html").write_text('<a href="missing.html">bad file</a>')
        assert any("missing target" in x for x in crawl(site)[0])
        print("PASS: link crawler fixtures and negative cases")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("site", nargs="?", type=Path, default=Path("site"))
    ap.add_argument("--external", action="store_true", help="Probe each distinct external URL")
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    if args.self_test:
        self_test()
        return 0
    errors, external, _, _ = crawl(args.site)
    if args.external and external:
        with ThreadPoolExecutor(max_workers=8) as pool:
            for result in as_completed([pool.submit(http_check, url) for url in sorted(external)]):
                url, state = result.result()
                print(f"{state}: {url}", flush=True)
                if state.startswith("BROKEN"):
                    errors.append(f"external: {url}: {state}")
        print("NOTE: INDETERMINATE external endpoints require manual follow-up; not counted as verified.")
    if errors:
        for error in errors[:60]:
            print(f"ERROR: {error}", file=sys.stderr)
        if len(errors) > 60:
            print(f"ERROR: and {len(errors) - 60} additional broken references", file=sys.stderr)
        return 1
    print("PASS: all generated internal links/fragment targets resolved" +
          ("; definite 404/410 external targets rejected" if args.external else ""))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
