#!/usr/bin/env python3
"""Static G5 input consistency guard (not a replacement for docker build)."""
from __future__ import annotations
import json
from pathlib import Path
import re
import sys


def check(root: Path) -> list[str]:
    faults: list[str] = []
    docker = (root / ".devcontainer/Dockerfile").read_text(encoding="utf-8")
    ci = (root / ".github/workflows/ci.yml").read_text(encoding="utf-8")
    workflow = (root / ".github/workflows/container.yml").read_text(encoding="utf-8")
    descriptor = json.loads((root / ".devcontainer/devcontainer.json").read_text(encoding="utf-8"))
    lifecycle = (root / "scripts/ci/verify-devcontainer-lifecycle.sh").read_text(encoding="utf-8")

    expected = {
        "CXX26_SNAPSHOT": "cxx26-2026.10.09.2",
        "CXX26_REVISION": "040df2b05ea338d107fac7387a2e4db835d746e0",
        "CXX26_ASSET": "clang-cxx26-2026.10.09.2-linux-x86_64",
    }
    for key, value in expected.items():
        if f"{key}: {value}" not in ci or f"ARG {key}={value}" not in docker:
            faults.append(f"{key} must be identical to reference CI")
    for name in (
        "CXX26_ARCHIVE_SHA256", "CXX26_MANIFEST_SHA256",
        "CXX26_SUMS_SHA256", "CMAKE_SHA256", "NINJA_SHA256", "NVIM_SHA256",
    ):
        if not re.search(rf"(?m)^ARG {name}=[a-f0-9]{{64}}$", docker):
            faults.append(f"missing valid full SHA-256 pin: {name}")
    if "ARG NVIM_VERSION=0.12.5" not in docker:
        faults.append("Modern Neovim version must be immutable")
    if "ARG NINJA_VERSION=1.13.2" not in docker:
        faults.append("Ninja 1.13.2 must be pinned for C++ modules")
    if not re.search(r"(?m)^FROM --platform=linux/amd64 ubuntu:22\.04@sha256:[a-f0-9]{64}$", docker):
        faults.append("base image must be a pinned ubuntu:22.04 amd64 image")
    for fragment in (
        "sha256sum --check", "verify-reference-toolchain.py",
        "sha256sum --check", "libc++", "CMAKE_TOOLCHAIN_FILE",
        "USER vscode",
    ):
        if fragment not in docker:
            faults.append(f"missing Dockerfile invariant: {fragment}")
    if descriptor.get("remoteUser") != "vscode":
        faults.append("devcontainer must run as non-root vscode user")
    if descriptor.get("build") != {"dockerfile": "Dockerfile", "context": ".."}:
        faults.append("devcontainer must use the canonical pinned Dockerfile")
    if descriptor.get("postCreateCommand") != "bash scripts/ci/verify-container.sh --smoke":
        faults.append("postCreate must validate the toolchain without expensive builds")
    if "prebuild" in json.dumps(descriptor).lower():
        faults.append("unapproved Codespaces prebuild setting")
    if "secrets" in json.dumps(descriptor).lower():
        faults.append("no secrets belong in checked-in devcontainer configuration")
    if "make docker-build NO_CACHE=1" not in workflow:
        faults.append("container CI must actually build clean amd64 image")
    if "make docker-test" not in workflow:
        faults.append("container CI must run full reference/consumer verification")
    if "permissions:\n  contents: read" not in workflow:
        faults.append("container CI must have read-only token permissions")
    if "bash scripts/ci/verify-devcontainer-lifecycle.sh" not in workflow:
        faults.append("container CI must exercise the real devcontainer lifecycle")
    if "@devcontainers/cli@0.89.0" not in workflow:
        faults.append("headless devcontainer CLI version must be explicit")
    for fragment in (
        " --no-lockfile", "run-user-commands", "docker restart",
        'test "$(id -un)" = vscode',
        "compile_commands.json", "verify-container.sh --smoke",
    ):
        if fragment not in lifecycle:
            faults.append(f"missing headless devcontainer lifecycle invariant: {fragment}")
    return faults


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    problems = check(root)
    if problems:
        print("\n".join(f"ERROR: {p}" for p in problems), file=sys.stderr)
        return 1
    print("PASS: G5 devcontainer identity, checksums, CI gates and no prebuild/secrets")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
