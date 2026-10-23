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
FILES = ("ci.yml", "gcc.yml", "docs.yml", "release.yml", "sync-gcc.yml", "miracle-project-operations.yml")
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

    # Project credentials must never be exposed through untrusted PR triggers.
    project = workflows["miracle-project-operations.yml"]
    checks = {
        "owner-only trigger": "github.actor == 'spwn02'",
        "fixed tracking issue": "github.event.issue.number == 29",
        "no PR issue-comment commands": "github.event.issue.pull_request == null",
        "reviewed checkout": "ref: master",
        "no checkout credentials": "persist-credentials: false",
        "scoped Project token": "GH_TOKEN: ${{ secrets.MIRACLE_PROJECT_TOKEN }}",
        "reviewed Roadmap update gate": "github.event.comment.body == '/miracle-project roadmap-apply'",
        "reviewed Roadmap controller": "python3 scripts/github/project-roadmap-filter.py --apply",
        "catalog read-only command": "github.event.comment.body == '/miracle-project archive-audit'",
        "catalog read-only runner": "python3 scripts/github/audit-project-archive.py",
        "no cancellation of ongoing writes": "cancel-in-progress: false",
        "command allowlist": "github.event.comment.body == '/miracle-project reconcile-apply'",
        "item inspect gate": "startsWith(github.event.comment.body, '/miracle-project inspect ')",
        "item update gate": "startsWith(github.event.comment.body, '/miracle-project update ')",
        "progress status gate": "startsWith(github.event.comment.body, '/miracle-project progress ')",
        "Python command validation": "python3 scripts/github/project-item-control.py --self-test",
        "Python command executor": "python3 scripts/github/project-item-control.py",
        "relation inspect gate": "startsWith(github.event.comment.body, '/miracle-project relations ')",
        "relation child gate": "startsWith(github.event.comment.body, '/miracle-project add-child ')",
        "relation blocker gate": "startsWith(github.event.comment.body, '/miracle-project add-blocker ')",
        "scoped repo token": "GH_TOKEN: ${{ github.token }}",
        "issue permission": "issues: write",
        "relation validator": "python3 scripts/github/issue-relations.py --self-test",
        "relation executor": "python3 scripts/github/issue-relations.py",
    }
    for label, fragment in checks.items():
        if fragment not in project:
            errors.append(f"miracle-project-operations.yml: missing {label}")
    validation, sep, execution = project.partition("      - name: Execute only the selected native Project operation")
    if not sep:
        errors.append("miracle-project-operations.yml: missing privileged dispatch step")
    else:
        for operation, mode in (("roadmap-plan", "--plan"), ("roadmap-verify", "--verify"), ("roadmap-apply", "--apply")):
            command = f"python3 scripts/github/project-roadmap-filter.py {mode}"
            # Fixed allowlist logic must be in the GH_TOKEN-bearing execution step,
            # never merely in the preceding no-token parser/validation step.
            if f"'/miracle-project {operation}')" not in execution or command not in execution:
                errors.append(f"miracle-project-operations.yml: missing executable Roadmap {operation} branch")
            if command in validation:
                errors.append(f"miracle-project-operations.yml: Roadmap {operation} executed in validation step")

    if "pull_request:" in project or "pull_request_target:" in project:
        errors.append("miracle-project-operations.yml: no PR trigger may access Project token")
    if project.count("secrets.MIRACLE_PROJECT_TOKEN") != 1:
        errors.append("miracle-project-operations.yml: Project token exposure must be limited to one step")

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

    # Negative fixture: widening command authorization must fail.
    widened = dict(workflows)
    widened["miracle-project-operations.yml"] = widened["miracle-project-operations.yml"].replace(
        "github.actor == 'spwn02'", "github.actor != ''", 1
    )
    if not check(widened):
        print("ERROR: widened Project workflow trigger escaped guard", file=sys.stderr)
        return 1

    widened_fields = dict(workflows)
    widened_fields["miracle-project-operations.yml"] = widened_fields["miracle-project-operations.yml"].replace(
        "startsWith(github.event.comment.body, '/miracle-project update ')", "true", 1
    )
    if not check(widened_fields):
        print("ERROR: widened Project field-command trigger escaped guard", file=sys.stderr)
        return 1

    widened_relations = dict(workflows)
    widened_relations["miracle-project-operations.yml"] = widened_relations["miracle-project-operations.yml"].replace(
        "startsWith(github.event.comment.body, '/miracle-project add-child ')", "true"
    )
    if not check(widened_relations):
        print("ERROR: widened native hierarchy trigger escaped guard", file=sys.stderr)
        return 1

    widened_progress = dict(workflows)
    widened_progress["miracle-project-operations.yml"] = widened_progress["miracle-project-operations.yml"].replace(
        "startsWith(github.event.comment.body, '/miracle-project progress ')", "true"
    )
    if not check(widened_progress):
        print("ERROR: widened Project progress trigger escaped guard", file=sys.stderr)
        return 1

    widened_roadmap = dict(workflows)
    widened_roadmap["miracle-project-operations.yml"] = widened_roadmap["miracle-project-operations.yml"].replace(
        "github.event.comment.body == '/miracle-project roadmap-apply'", "true"
    )
    if not check(widened_roadmap):
        print("ERROR: widened Roadmap write trigger escaped security guard", file=sys.stderr)
        return 1

    misplaced_roadmap = dict(workflows)
    misplaced_roadmap["miracle-project-operations.yml"] = misplaced_roadmap["miracle-project-operations.yml"].replace(
        "python3 scripts/github/project-roadmap-filter.py --apply", "echo 'roadmap apply skipped'", 1
    )
    if not check(misplaced_roadmap):
        print("ERROR: inert Roadmap execute branch escaped security guard", file=sys.stderr)
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

    print(f"PASS: {len(FILES)} workflow trust boundaries, 11 mutations, and 4 parser fixtures.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
