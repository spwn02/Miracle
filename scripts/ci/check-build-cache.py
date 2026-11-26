#!/usr/bin/env python3
"""Non-destructive preflight for Miracle's persistent Docker/CMake workspace.

CMake --fresh only removes the selected top-level cache. Nested configure
probes (notably SwitchCapabilities) may retain absolute paths from a previous
workspace, such as the retired /workspaces/Miracle devcontainer mount.
"""
from __future__ import annotations

import argparse
import os
from pathlib import Path, PurePosixPath
import sys

PRESETS = ("tests", "debug", "release")
CACHE_KEY = "CMAKE_CACHEFILE_DIR:INTERNAL="


def cache_directory(cache: Path) -> str | None:
    """Get the build directory CMake recorded, without altering the cache."""
    with cache.open(encoding="utf-8", errors="replace") as source:
        for line in source:
            if line.startswith(CACHE_KEY):
                return line[len(CACHE_KEY):].strip()
    return None


def check(workspace: Path, expected_root: str, presets: tuple[str, ...],
          *, require_non_root: bool = False) -> list[str]:
    errors: list[str] = []
    if require_non_root and os.geteuid() == 0:
        errors.append(
            "docker-dev must be launched as your normal host user, not with sudo/root. "
            "Using sudo maps your checkout to UID 0 inside Docker and creates "
            "root-owned build artifacts. Fix Docker daemon access for your "
            "regular user rather than running 'sudo make docker-dev'."
        )

    root = workspace.resolve()
    container_root = PurePosixPath(expected_root)
    if not container_root.is_absolute():
        return errors + [f"expected workspace root must be absolute: {expected_root}"]

    for preset in presets:
        build_dir = root / "build" / preset
        if not build_dir.is_dir():
            continue
        if not os.access(build_dir, os.W_OK | os.X_OK):
            errors.append(
                f"build/{preset} is not writable by this user; it may be "
                "left over from a prior sudo/root container session."
            )

        # Covers the main preset and nested CMake configure/cache probes.
        # Symlink directories are not traversed by Path.rglob in Python 3.10.
        for cache in sorted(build_dir.rglob("CMakeCache.txt")):
            if cache.is_symlink():
                errors.append(
                    f"{cache.relative_to(root)} is a symlink; refuse to "
                    "inspect a cache outside the checkout."
                )
                continue
            try:
                recorded = cache_directory(cache)
            except OSError as error:
                errors.append(f"cannot read {cache.relative_to(root)}: {error}")
                continue
            if recorded is None:
                continue  # An incomplete/foreign CMake cache is not an old-path proof.
            relative = cache.parent.relative_to(root)
            expected = str(container_root.joinpath(*relative.parts))
            if recorded != expected:
                errors.append(
                    f"{cache.relative_to(root)} was generated at {recorded!r}; "
                    f"current workspace requires {expected!r}."
                )

    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--workspace", type=Path, default=Path(__file__).resolve().parents[2],
        help="Repository checkout containing build/{tests,debug,release}",
    )
    parser.add_argument(
        "--expected-root", required=True,
        help="Absolute path that CMake uses inside the upcoming environment",
    )
    parser.add_argument("--preset", choices=PRESETS, action="append",
                        help="Check only a named CMake preset (default: all)")
    parser.add_argument("--require-non-root", action="store_true",
                        help="Reject sudo/root invocation of the persistent Docker shell")
    args = parser.parse_args()
    errors = check(
        args.workspace, args.expected_root, tuple(args.preset or PRESETS),
        require_non_root=args.require_non_root,
    )
    if errors:
        print("ERROR: Miracle workspace preflight failed:", file=sys.stderr)
        for problem in errors[:12]:
            print(f"  - {problem}", file=sys.stderr)
        if len(errors) > 12:
            print(f"  - ... and {len(errors) - 12} more problem(s)", file=sys.stderr)
        if any("was generated at" in problem for problem in errors):
            print(
                "CMake --fresh does not clear nested caches. Preserve the old "
                "build tree by moving the affected build/<preset> directory to "
                "an unused backup name (for example, build/tests.pre-docker-migration) "
                "before rerunning make verify. No files were changed.",
                file=sys.stderr,
            )
        print(
            "If files are root-owned from an earlier 'sudo make docker-dev', "
            "repair their ownership on the host before retrying. "
            "This check never removes, renames or changes any files.",
            file=sys.stderr,
        )
        return 2

    print("PASS: Docker/CMake workspace paths and host-user permissions are compatible")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
