# Meta cache reference compiler-cost baseline

Reference snapshot recorded on 2026-09-26 with:

```text
clang-cxx26 cxx26-2026.09.19.1
LLVM/Clang 22.1.8
revision 71ba1036c652094d4869f34e0ba7fb5c756f4399
CMake 4.4.3
Ninja 1.13.2
Linux x86_64
```

Values below are representative medians from repeated direct compiler invocations after the imported module BMIs/module maps were built. They are not absolute machine-independent thresholds; `run.py` gates same-run **ratios** so host load does not turn a compiler-performance check into a clock-speed benchmark.

| Members | Variant | Front-end wall (s) | Peak RSS (MiB) |
| ---: | --- | ---: | ---: |
| 32 | RawOnce | 1.075 | 199.3 |
| 32 | MiracleOnce | 1.050 | 195.1 |
| 32 | MiracleRawOnce | 1.010 | 196.9 |
| 32 | RawRepeat | 2.085 | 208.8 |
| 32 | MiracleRepeat | 1.035 | 195.8 |
| 32 | MiracleRawRepeat | 1.050 | 195.9 |
| 32 | RawVisibleRepeat | 2.060 | 209.4 |
| 32 | MiracleVisibleRepeat | 1.020 | 198.1 |
| 32 | MiracleRawVisibleRepeat | 1.035 | 198.2 |
| 128 | RawOnce | 1.075 | 201.0 |
| 128 | MiracleOnce | 1.010 | 197.0 |
| 128 | MiracleRawOnce | 1.000 | 197.4 |
| 128 | RawRepeat | 1.865 | 210.1 |
| 128 | MiracleRepeat | 1.025 | 197.8 |
| 128 | MiracleRawRepeat | 1.020 | 197.8 |
| 128 | RawVisibleRepeat | 1.880 | 205.3 |
| 128 | MiracleVisibleRepeat | 1.025 | 198.4 |
| 128 | MiracleRawVisibleRepeat | 1.070 | 195.8 |
| 512 | RawOnce | 1.110 | 200.4 |
| 512 | MiracleOnce | 1.115 | 197.6 |
| 512 | MiracleRawOnce | 1.105 | 196.7 |
| 512 | RawRepeat | 1.875 | 208.3 |
| 512 | MiracleRepeat | 1.080 | 195.8 |
| 512 | MiracleRawRepeat | 1.120 | 198.2 |
| 512 | RawVisibleRepeat | 1.825 | 210.7 |
| 512 | MiracleVisibleRepeat | 1.130 | 196.7 |
| 512 | MiracleRawVisibleRepeat | 1.190 | 198.3 |
| 1024 | RawOnce | 1.150 | 203.0 |
| 1024 | MiracleOnce | 1.120 | 198.6 |
| 1024 | MiracleRawOnce | 1.155 | 195.0 |
| 1024 | RawRepeat | 1.870 | 208.5 |
| 1024 | MiracleRepeat | 1.110 | 197.9 |
| 1024 | MiracleRawRepeat | 1.125 | 198.6 |
| 1024 | RawVisibleRepeat | 1.905 | 208.6 |
| 1024 | MiracleVisibleRepeat | 1.305 | 200.1 |
| 1024 | MiracleRawVisibleRepeat | 1.240 | 199.9 |

The important shape is stable across all four stress sizes:

- one-shot typed/raw Miracle source lookups remain within measurement noise of direct `<meta>` reflection;
- repeated unchecked cache lookups use roughly 50–60% of the raw reflection wall time in this snapshot;
- repeated caller-access projections use roughly 50–69% of the raw wall time;
- cached cases also reduce peak compiler RSS by roughly 4–7% in the repeated fixtures;
- the raw-`Info` reflective bridge stays close to the typed cache path, so value-oriented API ergonomics do not erase the
  cache benefit.

Re-run `run.py` after compiler/cache changes. A changed host may shift the absolute numbers substantially; relative same-run gates are the authoritative regression signal.
