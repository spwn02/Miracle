#!/usr/bin/env python3
"""Read-only GitHub repository security-settings audit for Miracle.

Uses the maintainer's authenticated GitHub CLI. Prints only HTTP status
codes and allowlisted security settings, never vulnerability/secret alert payloads.

A 403 or 404 means inaccessible/unknown unless the repository owner
independently verifies the feature in GitHub settings.
"""
from __future__ import annotations

import json
import re
import shutil
import subprocess
import sys

REPO = "spwn02/Miracle"
STATUS = re.compile(r"^HTTP/\S+\s+(\d{3})(?:\s|$)", re.MULTILINE)
CHECKS = {
    "Private vulnerability reporting": f"repos/{REPO}/private-vulnerability-reporting",
    "Dependency vulnerability alerts": f"repos/{REPO}/vulnerability-alerts",
    "Dependabot security updates": f"repos/{REPO}/automated-security-fixes",
    "Code scanning default setup": f"repos/{REPO}/code-scanning/default-setup",
    # Do not call alert-list endpoints; their bodies can contain sensitive data.
}


def request(path: str, *, headers: bool = False) -> tuple[int | None, str]:
    args = ["gh", "api"]
    if headers:
        args.append("--include")
    args.append(path)
    result = subprocess.run(args, capture_output=True, text=True, check=False)
    if headers:
        matches = STATUS.findall(result.stdout)
        return (int(matches[-1]) if matches else None), ""
    if result.returncode != 0:
        return None, ""
    return 200, result.stdout


def main() -> int:
    if not shutil.which("gh"):
        print("ERROR: gh CLI is required.", file=sys.stderr)
        return 1

    status, payload = request(f"repos/{REPO}")
    if status != 200:
        print("ERROR: cannot access repository metadata with gh auth.", file=sys.stderr)
        return 1
    try:
        repository = json.loads(payload)
    except json.JSONDecodeError:
        print("ERROR: repository metadata is invalid JSON.", file=sys.stderr)
        return 1

    print(f"Repository: {REPO}")
    print(f"Public: {repository.get('private') is False}")
    settings = repository.get("security_and_analysis")
    if settings is None:
        print("security_and_analysis: NOT EXPOSED (do not infer disabled)")
    else:
        for name, value in sorted(settings.items()):
            flag = value.get("status") if isinstance(value, dict) else None
            print(f"security_and_analysis.{name}: {flag or 'unknown'}")

    print("\nSecurity endpoint reachability (no alert contents shown):")
    for label, path in CHECKS.items():
        code, _ = request(path, headers=True)
        if code in {200, 204}:
            result = "accessible; inspect feature status in Settings"
        elif code in {403, 404, 451}:
            result = "unknown (disabled, unavailable, or insufficient access)"
        elif code is None:
            result = "unknown (no parseable response)"
        else:
            result = "review manually"
        print(f"- {label}: HTTP {code or 'n/a'} — {result}")

    print("\nAllowlisted configuration values (no alert payloads printed):")
    for label, path, keys in (
        ("Private vulnerability reporting",
         f"repos/{REPO}/private-vulnerability-reporting", ("enabled",)),
        ("Dependabot security updates",
         f"repos/{REPO}/automated-security-fixes", ("enabled", "paused")),
        ("Code scanning default setup",
         f"repos/{REPO}/code-scanning/default-setup", ("state",)),
    ):
        status, payload = request(path)
        if status != 200:
            print(f"- {label}: UNKNOWN (GET failed)")
            continue
        try:
            data = json.loads(payload)
        except json.JSONDecodeError:
            print(f"- {label}: UNKNOWN (invalid JSON)")
            continue
        if not isinstance(data, dict):
            print(f"- {label}: UNKNOWN (unexpected response shape)")
            continue
        allowed = {}
        for key in keys:
            value = data.get(key)
            if isinstance(value, bool):
                allowed[key] = value
            elif key == "state" and value in (
                "configured", "not-configured", "in-progress", "failed"
            ):
                allowed[key] = value
        print(f"- {label}: {allowed if allowed else 'UNKNOWN'}")

    print("\nReview exact values under:")
    print(f"  https://github.com/{REPO}/settings/security_analysis")
    print(f"  https://github.com/{REPO}/security")
    print("Any missing or inaccessible property remains UNVERIFIED.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
