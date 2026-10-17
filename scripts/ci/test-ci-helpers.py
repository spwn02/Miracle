#!/usr/bin/env python3
"""Smoke-test canonical CI helpers without invoking compilers or releases."""
from __future__ import annotations
import ast
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
CI = ROOT / "scripts" / "ci"

def cmd(*args, cwd=ROOT, env=None):
    p = subprocess.run(args, cwd=cwd, env=env, text=True, capture_output=True)
    if p.returncode:
        raise RuntimeError(f"{args}: exit {p.returncode}\n{p.stderr[-1200:]}")
    return p.stdout.strip()

def main():
    if (ROOT / ".github" / "scripts").exists():
        raise RuntimeError("Legacy .github/scripts must no longer exist")
    for name in ("create-release-metadata.py", "report-gcc-convergence.py"):
        ast.parse((CI / name).read_text(), filename=name)
    cmd("bash", "-n", str(CI / "validate-gcc.sh"))
    refs = {
        "gcc.yml": ("validate-gcc.sh", "report-gcc-convergence.py"),
        "sync-gcc.yml": ("validate-gcc.sh", "report-gcc-convergence.py"),
        "release.yml": ("create-release-metadata.py",),
    }
    for workflow, needed in refs.items():
        s = (ROOT / ".github/workflows" / workflow).read_text()
        if ".github/scripts/" in s:
            raise RuntimeError(f"Legacy script reference in {workflow}")
        for name in needed:
            if f"scripts/ci/{name}" not in s:
                raise RuntimeError(f"Missing script reference: {workflow} -> {name}")

    with tempfile.TemporaryDirectory(prefix="miracle-ci-") as t:
        temp = Path(t)
        metadata = temp / "release.json"
        cmd(sys.executable, str(CI / "create-release-metadata.py"),
            "--project", "Miracle", "--version", "0.5.0", "--tag", "v0.5.0",
            "--source-repository", "spwn02/Miracle",
            "--source-revision", "a" * 40,
            "--reference-repository", "spwn02/clang-cxx26",
            "--reference-branch", "cxx26",
            "--reference-snapshot", "fixture",
            "--reference-revision", "b" * 40,
            "--reference-asset", "fixture", "--output", str(metadata))
        m = json.loads(metadata.read_text())
        assert m["schemaVersion"] == 1
        assert m["source"]["revision"] == "a" * 40
        assert m["referenceToolchain"]["revision"] == "b" * 40
        assert m["compilerSupport"]["gcc"]["releaseBearing"] is False

        repo = temp / "repo"
        repo.mkdir()
        env = dict(os.environ)
        env.update(GIT_AUTHOR_NAME="Miracle CI", GIT_AUTHOR_EMAIL="ci@example.invalid",
                   GIT_COMMITTER_NAME="Miracle CI", GIT_COMMITTER_EMAIL="ci@example.invalid")
        env.pop("GITHUB_STEP_SUMMARY", None)
        cmd("git", "init", "-q", cwd=repo, env=env)
        source = repo / "source.txt"
        source.write_text("before\n")
        cmd("git", "add", "source.txt", cwd=repo, env=env)
        cmd("git", "-c", "commit.gpgsign=false", "commit", "-qm", "base", cwd=repo, env=env)
        base = cmd("git", "rev-parse", "HEAD", cwd=repo, env=env)
        source.write_text("after\n")
        cmd("git", "add", "source.txt", cwd=repo, env=env)
        cmd("git", "-c", "commit.gpgsign=false", "commit", "-qm", "next", cwd=repo, env=env)
        report = temp / "gcc.json"
        cmd(sys.executable, str(CI / "report-gcc-convergence.py"),
            "--master", base, "--gcc", "HEAD", "--output", str(report), cwd=repo, env=env)
        a = json.loads(report.read_text())
        assert a["schemaVersion"] == 1
        assert a["aheadBy"] == 1 and a["behindBy"] == 0
        assert a["changedFileCount"] == 1 and not a["treeEqual"]
        assert a["files"][0]["path"] == "source.txt"
    print("PASS: unified scripts/ci layout, release metadata and GCC convergence fixtures")

if __name__ == "__main__":
    main()
