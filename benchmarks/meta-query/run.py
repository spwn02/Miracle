#!/usr/bin/env python3
"""Measure raw versus fused Miracle.Meta Query compiler cost."""

from __future__ import annotations

import argparse
import json
import re
import shlex
import statistics
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path

VARIANTS = ("RawOnce", "MiracleOnce", "RawRepeat", "MiracleRepeat")
DEFAULT_SIZES = (32, 128, 512, 1024)
TIME_PATTERN = re.compile(r"__MIRACLE_META_QUERY_TIME__ ([0-9.]+) ([0-9]+)")


@dataclass(frozen=True)
class Measurement:
    wall_seconds: float
    peak_rss_mib: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("build_dir", type=Path)
    parser.add_argument("--samples", type=int, default=3)
    parser.add_argument("--sizes", type=int, nargs="+", default=DEFAULT_SIZES)
    return parser.parse_args()


def load_commands(build_dir: Path) -> dict[str, dict[str, str]]:
    commands = json.loads((build_dir / "compile_commands.json").read_text())
    return {
        Path(entry["file"]).name: entry
        for entry in commands
        if "/benchmarks/meta-query/" in entry["file"]
    }


def measure(entry: dict[str, str], samples: int) -> Measurement:
    walls: list[float] = []
    rss_kib: list[int] = []
    command = shlex.split(entry["command"])
    output_index = command.index("-o") + 1

    with tempfile.TemporaryDirectory(prefix="miracle-meta-query-") as temporary:
        command[output_index] = str(Path(temporary) / "fixture.o")
        for _ in range(samples):
            result = subprocess.run(
                ["/usr/bin/time", "-f", "__MIRACLE_META_QUERY_TIME__ %e %M", *command],
                cwd=entry["directory"],
                check=False,
                capture_output=True,
                text=True,
            )
            if result.returncode != 0:
                raise RuntimeError(
                    f"compiler benchmark failed for {entry['file']}\n{result.stdout}\n{result.stderr}"
                )
            matches = TIME_PATTERN.findall(result.stderr)
            if not matches:
                raise RuntimeError(f"GNU time output missing for {entry['file']}")
            wall, rss = matches[-1]
            walls.append(float(wall))
            rss_kib.append(int(rss))

    return Measurement(statistics.median(walls), statistics.median(rss_kib) / 1024.0)


def main() -> int:
    args = parse_args()
    if args.samples < 1:
        raise SystemExit("--samples must be >= 1")
    build_dir = args.build_dir.resolve()
    cache = build_dir / "CMakeCache.txt"
    cmake = next(
        line.split("=", 1)[1]
        for line in cache.read_text().splitlines()
        if line.startswith("CMAKE_COMMAND:INTERNAL=")
    )
    targets = [
        f"MiracleMetaQuery{variant}{size}"
        for size in args.sizes
        for variant in VARIANTS
    ]
    subprocess.run(
        [cmake, "--build", str(build_dir), "--target", *targets, "-j2"], check=True
    )
    commands = load_commands(build_dir)

    results: dict[int, dict[str, Measurement]] = {}
    print("| Members | Variant | Front-end wall (s) | Peak RSS (MiB) |")
    print("| ---: | --- | ---: | ---: |")
    for size in args.sizes:
        size_results: dict[str, Measurement] = {}
        results[size] = size_results
        for variant in VARIANTS:
            source = f"{variant}-{size}.cxx"
            if source not in commands:
                raise RuntimeError(f"compile command not found for {source}")
            measured = measure(commands[source], args.samples)
            size_results[variant] = measured
            print(
                f"| {size} | {variant} | {measured.wall_seconds:.3f} | {measured.peak_rss_mib:.1f} |",
                flush=True,
            )

    print("\nQuery fusion ratios:")
    for size in args.sizes:
        raw = results[size]
        print(
            f"{size}: once wall={raw['MiracleOnce'].wall_seconds / raw['RawOnce'].wall_seconds:.3f}x, "
            f"RSS={raw['MiracleOnce'].peak_rss_mib / raw['RawOnce'].peak_rss_mib:.3f}x; "
            f"repeat wall={raw['MiracleRepeat'].wall_seconds / raw['RawRepeat'].wall_seconds:.3f}x, "
            f"RSS={raw['MiracleRepeat'].peak_rss_mib / raw['RawRepeat'].peak_rss_mib:.3f}x"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
