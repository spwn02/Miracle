#!/usr/bin/env python3
"""Negative fixtures for the non-destructive G5 workspace preflight."""
from __future__ import annotations

import importlib.util
from pathlib import Path
import tempfile
from unittest.mock import patch


SCRIPT = Path(__file__).with_name("check-build-cache.py")
SPEC = importlib.util.spec_from_file_location("g5_cache_preflight", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
GUARD = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GUARD)


def cache(root: Path, relative: str, recorded_dir: str) -> Path:
    path = root / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        "# CMake-generated cache (fixture)\n"
        "CMAKE_CACHEFILE_DIR:INTERNAL=" + recorded_dir + "\n",
        encoding="utf-8",
    )
    return path


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="miracle-g5-cache-") as tmp:
        root = Path(tmp)
        old = "/workspaces/Miracle"
        current = "/workspace/Miracle"
        nested = "build/tests/_deps/switch-build/CMakeFiles/SwitchCapabilities/abc/build"
        old_cache = cache(root, nested + "/CMakeCache.txt", old + "/" + nested)
        cache(root, "build/tests/CMakeCache.txt", current + "/build/tests")
        cache(root, "build/release/CMakeCache.txt", current + "/build/release")

        before = old_cache.read_bytes()
        errors = GUARD.check(root, current, GUARD.PRESETS)
        assert len(errors) == 1, errors
        assert old in errors[0] and current in errors[0], errors
        assert nested in errors[0], errors
        assert old_cache.read_bytes() == before, "preflight changed a cache"
        assert not GUARD.check(root, current, ("release",)), "unrelated preset blocked"
        assert GUARD.check(root, "relative/path", ("tests",)), "relative root accepted"

        old_cache.write_text(
            "CMAKE_CACHEFILE_DIR:INTERNAL=" + current + "/" + nested + "\n",
            encoding="utf-8",
        )
        assert not GUARD.check(root, current, GUARD.PRESETS), "valid cache rejected"

        with patch.object(GUARD.os, "geteuid", return_value=0):
            problems = GUARD.check(root, current, GUARD.PRESETS, require_non_root=True)
            assert len(problems) == 1 and "sudo/root" in problems[0], problems
        with patch.object(GUARD.os, "geteuid", return_value=1000):
            assert not GUARD.check(root, current, GUARD.PRESETS, require_non_root=True)

    print("PASS: stale nested Switch cache, clean caches, preset scoping, sudo "
          "rejection and non-destructive G5 preflight fixtures")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
