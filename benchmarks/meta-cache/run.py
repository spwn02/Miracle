#!/usr/bin/env python3
"""Measure and gate Meta cache compiler wall time and peak RSS.

The benchmark targets first build normally so CMake has generated module maps and
all imported BMIs are warm. Measurements then invoke each translation unit's
compiler command directly: this excludes link time and dependency scanning while
retaining the exact flags/module map CMake generated for the reference toolchain.
"""

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

VARIANTS = (
    "RawOnce",
    "MiracleOnce",
    "MiracleRawOnce",
    "RawRepeat",
    "MiracleRepeat",
    "MiracleRawRepeat",
    "RawVisibleRepeat",
    "MiracleVisibleRepeat",
    "MiracleRawVisibleRepeat",
)
DEFAULT_SIZES = (32, 128, 512, 1024)
TIME_PATTERN = re.compile(r"__MIRACLE_META_TIME__ ([0-9.]+) ([0-9]+)")


@dataclass(frozen=True)
class Measurement:
    wall_seconds: float
    peak_rss_mib: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "build_dir", type=Path, help="configured Miracle build directory"
    )
    parser.add_argument(
        "--samples", type=int, default=3, help="compiler runs per fixture (default: 3)"
    )
    parser.add_argument("--sizes", type=int, nargs="+", default=DEFAULT_SIZES)
    parser.add_argument(
        "--no-gate",
        action="store_true",
        help="report measurements without enforcing Meta cache relative-cost gates",
    )
    return parser.parse_args()


def cache_value(cache: Path, name: str) -> str:
    prefix = f"{name}:INTERNAL="
    for line in cache.read_text().splitlines():
        if line.startswith(prefix):
            return line[len(prefix) :]
    raise RuntimeError(f"{name} is missing from {cache}")


def load_commands(build_dir: Path) -> dict[str, dict[str, str]]:
    commands = json.loads((build_dir / "compile_commands.json").read_text())
    return {
        Path(entry["file"]).name: entry
        for entry in commands
        if "/benchmarks/meta-cache/" in entry["file"]
    }


def target_name(variant: str, size: int) -> str:
    return f"MiracleMetaCache{variant}{size}"


def source_name(variant: str, size: int) -> str:
    return f"{variant}-{size}.cxx"


def measure(entry: dict[str, str], samples: int) -> Measurement:
    walls: list[float] = []
    rss_kib: list[int] = []
    base_command = shlex.split(entry["command"])

    # Never overwrite Ninja's tracked object while timing the compiler directly.
    # Doing so can invalidate depfile state and make a second benchmark run spend
    # most of its time repairing benchmark targets before measurements begin.
    with tempfile.TemporaryDirectory(prefix="miracle-meta-cache-") as temporary:
        temporary_object = str(Path(temporary) / "fixture.o")
        command = list(base_command)
        try:
            output_index = command.index("-o") + 1
        except ValueError as error:
            raise RuntimeError(
                f"compiler command has no -o output for {entry['file']}"
            ) from error
        command[output_index] = temporary_object

        for _ in range(samples):
            measured = subprocess.run(
                ["/usr/bin/time", "-f", "__MIRACLE_META_TIME__ %e %M", *command],
                cwd=entry["directory"],
                check=False,
                capture_output=True,
                text=True,
            )
            if measured.returncode != 0:
                raise RuntimeError(
                    f"compiler benchmark failed for {entry['file']}\n{measured.stdout}\n{measured.stderr}"
                )

            matches = TIME_PATTERN.findall(measured.stderr)
            if not matches:
                raise RuntimeError(f"GNU time output missing for {entry['file']}")
            wall, rss = matches[-1]
            walls.append(float(wall))
            rss_kib.append(int(rss))

    return Measurement(
        wall_seconds=statistics.median(walls),
        peak_rss_mib=statistics.median(rss_kib) / 1024.0,
    )


def ratio(actual: float, baseline: float) -> float:
    return actual / baseline if baseline else float("inf")


def enforce_gates(size: int, measurements: dict[str, Measurement]) -> list[str]:
    """Return relative-cost gate failures for one reflected subject size.

    Absolute compiler times vary with host load, so Meta cache gates ratios from
    fixtures compiled in the same run. One-shot wrappers may cost at most 25%
    more wall time / 10% more RSS than raw `<meta>`. Repeated cached lookups must
    beat repeated raw reflection by at least 5% wall time and may not increase RSS.
    """

    failures: list[str] = []

    def check_pair(
        candidate: str, baseline: str, max_wall: float, max_rss: float
    ) -> None:
        current = measurements[candidate]
        raw = measurements[baseline]
        wall_ratio = ratio(current.wall_seconds, raw.wall_seconds)
        rss_ratio = ratio(current.peak_rss_mib, raw.peak_rss_mib)
        if wall_ratio > max_wall:
            failures.append(
                f"{size} {candidate}: wall ratio {wall_ratio:.3f} exceeds {max_wall:.3f} vs {baseline}"
            )
        if rss_ratio > max_rss:
            failures.append(
                f"{size} {candidate}: RSS ratio {rss_ratio:.3f} exceeds {max_rss:.3f} vs {baseline}"
            )

    check_pair("MiracleOnce", "RawOnce", 1.25, 1.10)
    check_pair("MiracleRawOnce", "RawOnce", 1.25, 1.10)
    check_pair("MiracleRepeat", "RawRepeat", 0.95, 1.00)
    check_pair("MiracleRawRepeat", "RawRepeat", 0.95, 1.00)
    check_pair("MiracleVisibleRepeat", "RawVisibleRepeat", 0.95, 1.00)
    check_pair("MiracleRawVisibleRepeat", "RawVisibleRepeat", 0.95, 1.00)
    return failures


def main() -> int:
    args = parse_args()
    if args.samples < 1:
        raise SystemExit("--samples must be >= 1")
    if not Path("/usr/bin/time").is_file():
        raise SystemExit(
            "Meta cache RSS measurements require GNU /usr/bin/time on the reference Linux host"
        )

    build_dir = args.build_dir.resolve()
    cache = build_dir / "CMakeCache.txt"
    cmake = cache_value(cache, "CMAKE_COMMAND")
    commands = load_commands(build_dir)

    targets = [
        target_name(variant, size) for size in args.sizes for variant in VARIANTS
    ]
    subprocess.run(
        [cmake, "--build", str(build_dir), "--target", *targets, "-j2"], check=True
    )

    results: dict[int, dict[str, Measurement]] = {}
    print("| Members | Variant | Front-end wall (s) | Peak RSS (MiB) |")
    print("| ---: | --- | ---: | ---: |")
    for size in args.sizes:
        size_results: dict[str, Measurement] = {}
        results[size] = size_results
        for variant in VARIANTS:
            source = source_name(variant, size)
            if source not in commands:
                raise RuntimeError(
                    f"compile command not found for {source}; reconfigure with MIRACLE_BUILD_BENCHMARKS=ON"
                )
            measured = measure(commands[source], args.samples)
            size_results[variant] = measured
            print(
                f"| {size} | {variant} | {measured.wall_seconds:.3f} | "
                f"{measured.peak_rss_mib:.1f} |",
                flush=True,
            )

    if args.no_gate:
        return 0

    failures = [
        failure for size in args.sizes for failure in enforce_gates(size, results[size])
    ]
    if failures:
        print("\nMeta cache compiler-cost gate failures:")
        for failure in failures:
            print(f"- {failure}")
        return 1

    print("\nMeta cache compiler-cost gates: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
