#!/usr/bin/env python3
"""Read-only audit of Miracle v0.5.27's attached release provenance.

A historical G6 evidence check, not a new release or acceptance operation.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
from typing import Any

REPO = "spwn02/Miracle"
TAG = "v0.5.27"
ASSET = "release-metadata.json"
SOURCE_SHA = "e1e5817c651d124d02930b86510e2b05fb7c7d2d"
ASSET_SHA256 = "27a7a78fea1df167a79e6e7386bc62efe4faf257212d5fa09f8453248cba9635"
ASSET_ID = 645665032
EXPECTED = {
    "schemaVersion": 1,
    "project": {"name": "Miracle", "version": "0.5.27", "tag": TAG},
    "source": {"repository": REPO, "branch": "master", "revision": SOURCE_SHA},
    "referenceToolchain": {
        "repository": "spwn02/clang-cxx26",
        "developmentBranch": "cxx26",
        "snapshot": "cxx26-2026.10.09.2",
        "revision": "040df2b05ea338d107fac7387a2e4db835d746e0",
        "asset": "clang-cxx26-2026.10.09.2-linux-x86_64",
    },
    "compilerSupport": {"gcc": {"branch": "gcc", "releaseBearing": False}},
}


class AuditError(RuntimeError):
    pass


def api(resource: str) -> dict[str, Any]:
    proc = subprocess.run(["gh", "api", resource], capture_output=True,
                          text=True, check=False)
    if proc.returncode:
        raise AuditError("GitHub metadata fetch failed (inspect authenticated runner)")
    try:
        value = json.loads(proc.stdout)
    except ValueError as exc:
        raise AuditError("GitHub returned invalid JSON") from exc
    if not isinstance(value, dict):
        raise AuditError("Unexpected GitHub metadata shape")
    return value


def verify(release: dict[str, Any], ref: dict[str, Any], raw: bytes,
           expected_digest: str = ASSET_SHA256) -> None:
    if (release.get("tag_name") != TAG or release.get("draft") is not False
            or release.get("published_at") is None):
        raise AuditError("Release identity/status is not the published historical release")
    if (ref.get("ref") != f"refs/tags/{TAG}"
            or (ref.get("object") or {}).get("sha") != SOURCE_SHA
            or (ref.get("object") or {}).get("type") != "commit"):
        raise AuditError("Actual tag is not the expected lightweight source commit")
    assets = release.get("assets")
    if not isinstance(assets, list):
        raise AuditError("Release does not enumerate assets")
    matches = [a for a in assets if a.get("name") == ASSET]
    if len(matches) != 1:
        raise AuditError("Release must have exactly one provenance attachment")
    attachment = matches[0]
    if (attachment.get("id") != ASSET_ID or attachment.get("state") != "uploaded"
            or attachment.get("size") != len(raw)):
        raise AuditError("Provenance attachment ID/state/size differs")
    digest = hashlib.sha256(raw).hexdigest()
    if digest != expected_digest:
        raise AuditError("Downloaded release metadata SHA-256 differs")
    if attachment.get("digest") != "sha256:" + digest:
        raise AuditError("GitHub asset digest differs from downloaded content")
    try:
        metadata = json.loads(raw)
    except (ValueError, UnicodeDecodeError) as exc:
        raise AuditError("Downloaded asset is not valid JSON") from exc
    if metadata != EXPECTED:
        raise AuditError("Historical release JSON fields differ from tagged-source expectations")


def self_test() -> None:
    authentic_shape = (json.dumps(EXPECTED, indent=2, sort_keys=True) + "\n").encode()
    assert len(authentic_shape) == 608
    assert hashlib.sha256(authentic_shape).hexdigest() == ASSET_SHA256
    raw = json.dumps(EXPECTED, sort_keys=True).encode()
    fixture_digest = hashlib.sha256(raw).hexdigest()
    release = {"tag_name": TAG, "draft": False, "published_at": "2026-10-16T00:47:37Z",
               "assets": [{"name": ASSET, "id": ASSET_ID, "state": "uploaded",
                           "size": len(raw), "digest": "sha256:" + fixture_digest}]}
    ref = {"ref": f"refs/tags/{TAG}", "object": {"sha": SOURCE_SHA, "type": "commit"}}
    verify(release, ref, raw, fixture_digest)
    for bad_release, bad_ref, bad_raw in [
        ({**release, "draft": True}, ref, raw),
        (release, {**ref, "object": {"sha": "a" * 40, "type": "commit"}}, raw),
        (release, ref, raw + b"\n"),
        (release, ref, json.dumps({**EXPECTED, "schemaVersion": 2}).encode()),
        ({**release, "assets": []}, ref, raw),
    ]:
        try:
            verify(bad_release, bad_ref, bad_raw, fixture_digest)
        except AuditError:
            continue
        raise AssertionError("Invalid release provenance fixture accepted")
    wrong_raw = json.dumps({**EXPECTED, "schemaVersion": 2}, sort_keys=True).encode()
    wrong_digest = hashlib.sha256(wrong_raw).hexdigest()
    wrong_release = {**release, "assets": [{**release["assets"][0],
        "size": len(wrong_raw), "digest": "sha256:" + wrong_digest}]}
    try:
        verify(wrong_release, ref, wrong_raw, wrong_digest)
    except AuditError as exc:
        assert "JSON fields" in str(exc)
    else:
        raise AssertionError("Invalid JSON accepted after matching transport digest")
    print("PASS: G6 release hash, JSON, tag, and negative fixtures")


def main() -> None:
    if sys.argv[1:] == ["--self-test"]:
        self_test()
        return
    if sys.argv[1:]:
        raise AuditError("Usage: audit-g6-release-provenance.py [--self-test]")
    release = api(f"repos/{REPO}/releases/tags/{TAG}")
    ref = api(f"repos/{REPO}/git/ref/tags/{TAG}")
    with tempfile.TemporaryDirectory(prefix="g6-release-") as directory:
        proc = subprocess.run(["gh", "release", "download", TAG, "--repo", REPO,
                               "--pattern", ASSET, "--dir", directory],
                              capture_output=True, text=True, check=False)
        if proc.returncode:
            raise AuditError("Release provenance attachment download failed")
        raw = Path(directory, ASSET).read_bytes()
    verify(release, ref, raw)
    print(f"PASS: {REPO} {TAG} published release, tag SHA {SOURCE_SHA}")
    print(f"PASS: downloaded {ASSET} ID {ASSET_ID}, {len(raw)} bytes")
    print(f"PASS: SHA-256 {ASSET_SHA256}, exact historical JSON source/toolchain fields")
    print("READ-ONLY: no release, tag, Project, Issue, or acceptance mutations.")
    print("NOT VERIFIED: any post-G4-protection release or G6 owner sign-off.")


if __name__ == "__main__":
    try:
        main()
    except AuditError as exc:
        print("ERROR: " + str(exc), file=sys.stderr)
        raise SystemExit(1)
