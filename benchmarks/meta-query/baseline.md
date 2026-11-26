# Meta Query completion baseline

Completion measurements were recorded with:

```text
snapshot: cxx26-2026.10.02
revision: c64402ae0a5986b438d03fac632b3744a4c104fe
clang:    22.1.8
platform: Linux x86_64
```

The architecture was originally calibrated on `cxx26-2026.09.30.2`. A compiler defect discovered during implementation was fixed by the later immutable snapshot, so current-snapshot absolute measurements are kept separate from the older historical calibration instead of being mixed into one ratio.

## Whole `Miracle.Meta` module

Compared with the legacy tuple/stage module on the same compiler:

| Metric | Legacy tuple/stage Query | Final value-IR Query | Change |
|---|---:|---:|---:|
| AST type + declaration allocation | 9,913,584 B | 5,659,200 B | -42.91% |
| median module precompile wall | 0.36 s | 0.22 s | -38.9% |
| median peak RSS | 138.3 MiB | 123.6 MiB | -10.6% |

## Architecture isolation

The depth fixture compares the historical tuple/state architecture, pack-indexed and overload alternatives, an optimized handwritten baseline, and the final reflected value IR. At depth 64 the abstraction increment above the handwritten structural baseline is:

```text
legacy QueryState/tuple: +5,910,904 B
pack-indexed:         +  388,904 B
overload pipeline:    +  339,760 B
final value IR:       +   58,440 B
```

For the final value-IR architecture, function declarations, function-template declarations, and class-template specializations remain constant from depth 1 through depth 64. Logical operation count therefore does not become per-operation template topology.

The historical direct-adjacency calibration on `2026.09.30.2` remains useful context (`~1.385 MB` incremental at depth 64), but it is not compared as an exact byte-for-byte ratio against the newer snapshot/fixture.

## Public Query scaling

With structural callable values reused across stages, public Query depth 1 -> 64 adds no function declarations, function-template declarations, class-template specializations, or class-template declarations. Structural AST growth is value-IR data rather than stage-as-type topology.

A 1024-subject × depth-64 completion stress case compiled successfully.

## Generated code

A runtime-visible `filter -> map -> skip -> take -> last` Query and its handwritten equivalent both optimize to:

```asm
movl $18, %eax
retq
```

An explicit `materialize()` lookup also produces the same runtime shape as a handwritten static array: one static 4-int backing array plus an indexed load.

## Important one-shot caveat

One-shot public Query is **not** faster than direct raw `<meta>` on this snapshot. At 1024 fields, the measured medians were `0.46 s` raw versus `2.91 s` Query. The implementation does not claim universal negative compiler cost for each isolated invocation.

Likewise, repeatedly rebuilding and terminating the exact same Query is retained as a diagnostic failure: at 32 fields / 64 repetitions the measured medians were `0.51 s` raw versus `6.99 s` Query.

The workload-level negative-cost requirement is satisfied instead by genuine reusable reflection state. For 128 fields and 16 repeated source-universe requests:

```text
independently repeated raw reflection: 1.20 s
canonical typed cache:                 0.13 s
ergonomic raw-Info cache bridge:       0.15 s
```

The public-value cache bridge is therefore about 8x faster than independently repeated raw reflection on that fixture.

These timings are host-dependent diagnostics; structural shape, same-run ratios, generated assembly, and scaling behavior are the regression signals.
