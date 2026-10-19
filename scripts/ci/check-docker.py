#!/usr/bin/env python3
"""G5 Docker-only reference image consistency guard (not a substitute for CI build)."""
from __future__ import annotations

from pathlib import Path
import re
import sys


def check(root: Path) -> list[str]:
    faults: list[str] = []
    docker = (root / "Dockerfile").read_text(encoding="utf-8")
    ci = (root / ".github/workflows/ci.yml").read_text(encoding="utf-8")
    workflow = (root / ".github/workflows/container.yml").read_text(encoding="utf-8")
    makefile = (root / "Makefile").read_text(encoding="utf-8")
    workspace = (root / "scripts/ci/verify-docker-workspace.sh").read_text(encoding="utf-8")

    for key, value in {
        "CXX26_SNAPSHOT": "cxx26-2026.10.09.2",
        "CXX26_REVISION": "040df2b05ea338d107fac7387a2e4db835d746e0",
        "CXX26_ASSET": "clang-cxx26-2026.10.09.2-linux-x86_64",
    }.items():
        if f"{key}: {value}" not in ci or f"ARG {key}={value}" not in docker:
            faults.append(f"{key} must match pinned reference CI")
    for name in (
        "CXX26_ARCHIVE_SHA256", "CXX26_MANIFEST_SHA256",
        "CXX26_SUMS_SHA256", "CMAKE_SHA256", "NINJA_SHA256", "NVIM_SHA256",
    ):
        if not re.search(rf"(?m)^ARG {name}=[a-f0-9]{{64}}$", docker):
            faults.append(f"missing full SHA-256 pin: {name}")
    if "ARG NVIM_VERSION=0.12.5" not in docker or "ARG NINJA_VERSION=1.13.2" not in docker:
        faults.append("Ninja and Neovim versions must remain pinned")
    if not re.search(r"(?m)^FROM --platform=linux/amd64 ubuntu:22\.04@sha256:[a-f0-9]{64}$", docker):
        faults.append("base image must be official digest-pinned Ubuntu amd64")
    for fragment in (
        "sha256sum --check", "verify-reference-toolchain.py",
        "libc++", "CMAKE_TOOLCHAIN_FILE", "USER miracle",
        "WORKDIR /workspace/Miracle",
    ):
        if fragment not in docker:
            faults.append(f"missing Dockerfile invariant: {fragment}")
    if (root / ".devcontainer/devcontainer.json").exists() or (root / ".devcontainer/Dockerfile").exists():
        faults.append("Docker-only G5 must not depend on a devcontainer")
    if "-f Dockerfile" not in makefile or "docker-dev:" not in makefile:
        faults.append("Makefile must expose root Dockerfile and persistent docker-dev target")
    for fragment in (
        "make docker-build NO_CACHE=1", "make docker-test",
        "bash scripts/ci/verify-docker-workspace.sh",
        "permissions:\n  contents: read",
    ):
        if fragment not in workflow:
            faults.append(f"missing Docker CI gate: {fragment}")
    for fragment in (
        "type=bind", "id -u", "id -g", "HOME=/tmp", "--workdir /workspace/Miracle",
        "verify-container.sh --smoke", "compile_commands.json",
        "verify-nvim-lsp.lua", ".g5-docker-workspace-marker",
    ):
        if fragment not in workspace:
            faults.append(f"missing Docker workspace acceptance probe: {fragment}")
    for forbidden in ("@devcontainers/cli", "verify-devcontainer-lifecycle.sh"):
        if forbidden in workflow:
            faults.append(f"retired devcontainer dependency remains in Docker CI: {forbidden}")
    return faults


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    problems = check(root)
    if problems:
        print("\n".join(f"ERROR: {item}" for item in problems), file=sys.stderr)
        return 1
    print("PASS: root Dockerfile/toolchain pins, non-root, writable workspace and CI gates")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
