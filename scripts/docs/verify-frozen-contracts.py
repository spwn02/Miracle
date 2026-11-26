#!/usr/bin/env python3
"""Verify byte-level preservation of Miracle's imported frozen contracts.

Only historical source identity is checked. This is NOT a test of current
implementation conformance, approval provenance, or C++26 security coverage.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import sys
import tempfile

DEFAULT_MANIFEST = "docs/contracts/SOURCES.json"


def safe_path(root: Path, value: str) -> Path:
    part = PurePosixPath(value)
    if part.is_absolute() or not part.parts or any(p in {".", ".."} for p in part.parts):
        raise ValueError(f"Unsafe contract path: {value}")
    resolved = (root / value).resolve()
    if not resolved.is_relative_to(root.resolve()):
        raise ValueError(f"Contract path escapes repository: {value}")
    return resolved


def blob_sha1(content: bytes) -> str:
    return hashlib.sha1(b"blob " + str(len(content)).encode("ascii") + b"\0" + content).hexdigest()


def audit(root: Path, manifest: dict, require_originals: bool = False) -> tuple[list[str], list[str]]:
    errors: list[str] = []
    warnings: list[str] = []
    if manifest.get("schemaVersion") != 1:
        return ["Unsupported contract provenance manifest version"], []
    sources = manifest.get("lockedSources")
    pending = manifest.get("pendingOriginals")
    if not isinstance(sources, list) or not isinstance(pending, list) or not sources:
        return ["Manifest must list locked sources and pending originals"], []
    seen: set[str] = set()
    for entry in sources:
        identifier = entry["id"]
        path = safe_path(root, entry["path"])
        if identifier in seen:
            errors.append(f"Duplicate contract ID: {identifier}")
            continue
        seen.add(identifier)
        if not path.is_file():
            errors.append(f"Missing frozen contract: {entry['path']}")
            continue
        actual = path.read_bytes()
        observed_git_blob = blob_sha1(actual)
        if observed_git_blob != entry["gitBlobSha1"]:
            errors.append(f"Frozen contract changed: {entry['path']}")
        expected_sha256 = entry.get("sha256")
        if expected_sha256 and hashlib.sha256(actual).hexdigest() != expected_sha256:
            errors.append(f"Frozen contract SHA-256 mismatch: {entry['path']}")

    for entry in pending:
        if entry["id"] in seen:
            errors.append(f"Duplicate contract ID: {entry['id']}")
        seen.add(entry["id"])
        path = safe_path(root, entry["path"])
        if path.exists():
            if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != entry["sha256"]:
                errors.append(f"Unexpected or altered original contract: {entry['path']}")
            else:
                errors.append(f"Original now present: move {entry['id']} from pendingOriginals to lockedSources with its confirmed Git blob")
        else:
            warnings.append(f"Original source not yet in Git: {entry['path']}")
    if require_originals and pending:
        errors.append(f"{len(pending)} original contract(s) still pending")
    return errors, warnings


def self_test() -> None:
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        path = root / "docs/contracts/locked.md"
        path.parent.mkdir(parents=True)
        path.write_bytes(b"frozen contract\n")
        content = path.read_bytes()
        record = {
            "schemaVersion": 1,
            "lockedSources": [{"id": "x", "path": "docs/contracts/locked.md",
                               "gitBlobSha1": blob_sha1(content),
                               "sha256": hashlib.sha256(content).hexdigest()}],
            "pendingOriginals": [{"id": "y", "path": "docs/contracts/original.docx",
                                  "sha256": hashlib.sha256(b"original").hexdigest()}],
        }
        errors, warnings = audit(root, record)
        assert not errors and len(warnings) == 1
        assert audit(root, record, require_originals=True)[0]
        path.write_bytes(content + b"changed")
        assert any("changed" in e for e in audit(root, record)[0])
        path.write_bytes(content)
        assert not audit(root, record)[0]
        original = root / "docs/contracts/original.docx"
        original.write_bytes(b"original")
        assert any("move" in e for e in audit(root, record)[0])
        try:
            safe_path(root, "../escape")
        except ValueError:
            pass
        else:
            raise AssertionError("Accepted escaping source path")
    print("PASS: frozen-source identity, altered content, pending originals, path guards")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path("."))
    parser.add_argument("--manifest", default=DEFAULT_MANIFEST)
    parser.add_argument("--require-originals", action="store_true")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_test()
        return 0
    try:
        root = args.repo.resolve()
        source = safe_path(root, args.manifest)
        data = json.loads(source.read_text(encoding="utf-8"))
        errors, warnings = audit(root, data, require_originals=args.require_originals)
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1
    for warning in warnings:
        print(f"KNOWN LIMITATION: {warning}")
    for error in errors:
        print(f"ERROR: {error}", file=sys.stderr)
    if errors:
        return 1
    print(f"PASS: {len(data['lockedSources'])} frozen-source/reader blobs match archived identities; "
          f"{len(warnings)} original source artifact(s) pending")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
