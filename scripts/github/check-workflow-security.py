#!/usr/bin/env python3
"""Check Miracle-specific GitHub Actions trust boundaries.

This is a focused static regression guard for our version-controlled
workflows, not a general-purpose YAML parser or full security analysis.
It checks known source patterns without executing the workflow.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys


DIRECTORY = Path(".github/workflows")
FILES = ("ci.yml", "gcc.yml", "docs.yml", "release.yml", "sync-gcc.yml")
READ_ONLY = ("ci.yml", "gcc.yml", "docs.yml", "release.yml")


def has_job_scoped_contents_write(source: str, job: str) -> bool:
    """Check a YAML job's own permissions block in linear time.

    This deliberately inspects only known GitHub workflow indentation, rather
    than parsing arbitrary YAML. It must not match permissions on sibling jobs,
    or lines nested inside steps and multiline shell scripts.
    """
    _, separator, jobs = source.partition("\njobs:\n")
    if not separator:
        return False

    in_job = False
    in_permissions = False
    for line in jobs.splitlines():
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        indentation = len(line) - len(line.lstrip(" "))
        if indentation < 2:
            break
        if indentation == 2:
            in_job = line == f"  {job}:"
            in_permissions = False
        elif in_job and indentation == 4:
            in_permissions = line == "    permissions:"
        elif in_job and in_permissions and indentation == 6:
            if line.strip() == "contents: write":
                return True
    return False


def check(workflows: dict[str, str]) -> list[str]:
    errors: list[str] = []
    for name in FILES:
        if name not in workflows:
            errors.append(f"Missing workflow: {name}")
    if errors:
        return errors

    for name, source in workflows.items():
        if ".github/scripts/" in source:
            errors.append(f"{name}: obsolete .github/scripts/ reference")
        if re.search(r"(?m)^\s*pull_request_target\s*:", source):
            errors.append(f"{name}: pull_request_target is forbidden")
        if re.search(r"(?m)^\s*workflow_run\s*:", source):
            errors.append(f"{name}: workflow_run chaining requires review")
        if re.search(r"(?m)^\s*permissions\s*:\s*write-all\s*$", source):
            errors.append(f"{name}: write-all token permission is forbidden")

    for name in FILES:
        source = workflows[name]
        pre_jobs, marker, jobs = source.partition("\njobs:\n")
        if not marker:
            errors.append(f"{name}: jobs mapping missing")
            continue
        root_permissions = re.search(
            r"(?ms)^permissions:\n((?:^[ \t]+[^\n]*\n?)*)", pre_jobs
        )
        if not root_permissions or not re.search(
            r"(?m)^  contents: read\s*$", root_permissions.group(1)
        ):
            errors.append(f"{name}: root GITHUB_TOKEN must be contents: read")

        if name in READ_ONLY:
            if not re.search(r"(?m)^\s*persist-credentials: false\s*$", jobs):
                errors.append(f"{name}: disable persisted checkout credentials")
            if re.search(r"(?m)^  (?:contents|issues|pull-requests|actions|packages|pages|id-token): write\s*$", pre_jobs):
                errors.append(f"{name}: unauthorized root write permission")

    docs = workflows["docs.yml"]
    required_pages_condition = (
        "github.event_name != 'pull_request'"
        " && github.ref == 'refs/heads/master'"
    )
    if docs.count(required_pages_condition) < 3:
        errors.append("docs.yml: Pages upload/configure/deploy must exclude PRs and non-master")
    for fragment in ("pages: write", "id-token: write", "environment:", "name: github-pages"):
        if fragment not in docs:
            errors.append(f"docs.yml: missing Pages deployment invariant: {fragment}")

    sync = workflows["sync-gcc.yml"]
    if not has_job_scoped_contents_write(sync, "sync"):
        errors.append("sync-gcc.yml: expected job-scoped contents: write")
    if "git push origin HEAD:refs/heads/gcc" not in sync:
        errors.append("sync-gcc.yml: expected non-forced GCC synchronization")
    if re.search(r"(?m)^.*git push\b.*(?:--force|-f\b)", sync):
        errors.append("sync-gcc.yml: forced ref updates forbidden")

    release = workflows["release.yml"]
    if not has_job_scoped_contents_write(release, "release"):
        errors.append("release.yml: release publish must have job-scoped write token")
    if "pull_request:" in release:
        errors.append("release.yml: release must not run on pull requests")

    for name in ("ci.yml", "gcc.yml"):
        if "pull_request:" not in workflows[name]:
            errors.append(f"{name}: ordinary PR verification trigger missing")
    return errors


def main() -> int:
    workflows = {
        filename: (DIRECTORY / filename).read_text(encoding="utf-8")
        for filename in FILES
        if (DIRECTORY / filename).is_file()
    }

    # Mutation-based negative checks must fail; they do not touch tracked files.
    if not any(workflows.get(name) for name in FILES):
        print("ERROR: required workflows absent", file=sys.stderr)
        return 1
    errors = check(workflows)
    if errors:
        for item in errors:
            print(f"ERROR: {item}", file=sys.stderr)
        return 1

    mutants = (
        ("untrusted privileged trigger", "ci.yml",
         "  pull_request_target:\n"),
        ("privileged checkout", "ci.yml",
         "          persist-credentials: true"),
        ("unguarded docs publish", "docs.yml",
         "if: github.event_name != 'pull_request' && github.ref == 'refs/heads/master'"),
        ("forced gcc push", "sync-gcc.yml",
         "git push --force origin HEAD:refs/heads/gcc"),
    )
    for label, filename, mutation in mutants:
        mutated = dict(workflows)
        original = mutated[filename]
        if label == "untrusted privileged trigger":
            mutated[filename] = original.replace("\npermissions:", "\n" + mutation + "permissions:", 1)
        elif label == "privileged checkout":
            mutated[filename] = original.replace(
                "persist-credentials: false", mutation.strip(), 1
            )
        elif label == "unguarded docs publish":
            mutated[filename] = original.replace(mutation, "if: always()")
        else:
            mutated[filename] = original.replace(
                "git push origin HEAD:refs/heads/gcc", mutation, 1
            )
        if mutated[filename] == original:
            print(f"ERROR: fixture mutation did not apply: {label}", file=sys.stderr)
            return 1
        if not check(mutated):
            print(f"ERROR: mutation escaped guard: {label}", file=sys.stderr)
            return 1

    alternate = dict(workflows)
    alternate["gcc.yml"] = alternate["gcc.yml"].replace(
        "scripts/ci/validate-gcc.sh", ".github/scripts/validate-gcc.sh", 1
    )
    if alternate["gcc.yml"] == workflows["gcc.yml"] or not check(alternate):
        print("ERROR: obsolete helper path fixture escaped check", file=sys.stderr)
        return 1
    # Prevent privilege checks from accidentally reading a sibling job's
    # permissions or text in a multiline shell-script block.
    sibling = (
        "name: Fixture\njobs:\n"
        "  sync:\n    permissions:\n      contents: read\n"
        "  release:\n    permissions:\n      contents: write\n"
    )
    if has_job_scoped_contents_write(sibling, "sync"):
        print("ERROR: sibling-job permissions escaped scoped guard", file=sys.stderr)
        return 1
    if not has_job_scoped_contents_write(sibling, "release"):
        print("ERROR: real release job permissions were missed", file=sys.stderr)
        return 1
    embedded = (
        "name: Fixture\njobs:\n"
        "  sync:\n    steps:\n      - run: |\n"
        "          permissions:\n            contents: write\n"
    )
    if has_job_scoped_contents_write(embedded, "sync"):
        print("ERROR: shell-script text was misread as job permissions", file=sys.stderr)
        return 1

    # Previously, the two nested DOTALL regexes required exponentially more
    # backtracking for long near-matches. This line-oriented check is linear.
    near_match = (
        "name: Fixture\njobs:\n  sync:\n"
        + ("    # nonmatching workflow line\n" * 16000)
        + "    permissions:\n      contents: read\n"
    )
    if has_job_scoped_contents_write(near_match, "sync"):
        print("ERROR: non-writing stress fixture passed guard", file=sys.stderr)
        return 1

    print(f"PASS: {len(FILES)} workflow trust boundaries, 5 mutations, and 4 parser fixtures.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
