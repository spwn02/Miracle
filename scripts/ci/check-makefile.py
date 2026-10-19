#!/usr/bin/env python3
"""Exercise Miracle's Make CLI without requiring CMake, Docker or a compiler."""
from __future__ import annotations

from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]


def invoke(*arguments: str, success: bool = True) -> str:
    run = subprocess.run(
        ["make", "--no-print-directory", *arguments],
        cwd=ROOT, capture_output=True, text=True, check=False,
    )
    if (run.returncode == 0) != success:
        raise AssertionError(
            f"make {' '.join(arguments)}: expected success={success}, "
            f"got status={run.returncode}\n{run.stdout}\n{run.stderr}"
        )
    return run.stdout + run.stderr


def requires(actual: str, *fragments: str) -> None:
    for fragment in fragments:
        if fragment not in actual:
            raise AssertionError(f"Expected {fragment!r} in:\n{actual}")


def main() -> int:
    help_output = invoke("help")
    requires(help_output, "make configure", "make verify", "make docker-build", "make docker-dev", "make docker-verify", "make docs")

    configure = invoke("-n", "configure", "PRESET=release", "TOOLCHAIN=/tmp/cxx26/toolchain.cmake")
    requires(configure, 'cmake --preset "release" --fresh', 'CMAKE_TOOLCHAIN_FILE="/tmp/cxx26/toolchain.cmake"')
    requires(invoke("-n", "build", "PRESET=debug", "TOOLCHAIN=/tmp/cxx26/toolchain.cmake"),
             'cmake --build --preset "debug"', '--parallel "2"')
    requires(invoke("-n", "test", "TOOLCHAIN=/tmp/cxx26/toolchain.cmake"),
             'cmake --build --preset "tests"', "ctest --preset tests --output-on-failure")
    requires(invoke("-n", "verify", "TOOLCHAIN=/tmp/cxx26/toolchain.cmake"),
             'cmake --preset "tests" --fresh', 'ctest --preset tests')
    requires(invoke("-n", "docker-verify", "NO_CACHE=1", "DOCKER=mock-docker", "IMAGE=my-cxx26:test"),
             "mock-docker build --platform linux/amd64 --no-cache", "-f Dockerfile",
             "mock-docker run --rm --platform linux/amd64", '"my-cxx26:test"',
             "bash scripts/ci/verify-container.sh'")
    requires(invoke("-n", "docker-smoke"), "bash scripts/ci/verify-container.sh --smoke")
    requires(invoke("-n", "docker-dev"), "--user", "id -u", "id -g",
             "type=bind", "target=/workspace/Miracle", "--env HOME=/tmp", "--workdir /workspace/Miracle")
    requires(invoke("-n", "docker-shell"), "readonly", "exec bash")
    requires(invoke("-n", "docs"), "mdbook build", "python3 scripts/docs/verify-site.py site")
    clean = invoke("-n", "clean", "PRESET=debug")
    requires(clean, 'cmake --build --preset "debug" --target clean')
    if "rm -rf" in clean:
        raise AssertionError("Make clean must not delete directories")

    requires(invoke("-n", "configure", "PRESET=unknown", success=False), "Unknown PRESET")
    requires(invoke("configure", "TOOLCHAIN=", success=False), "reference toolchain not selected")
    requires(invoke("configure", "TOOLCHAIN=/unlikely/does-not-exist/toolchain.cmake", success=False),
             "missing reference toolchain file")
    print("PASS: Make CLI help, presets, reference-toolchain guard, Docker and docs smoke fixtures")
    return 0


if __name__ == "__main__":
    sys.exit(main())
