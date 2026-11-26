# Meta cache compiler-cost benchmark

Meta treats compiler wall time and peak RSS as first-class performance metrics. These fixtures compare raw C++26 reflection with Miracle's canonical cache paths at 32, 128, 512, and 1024 reflected fields.

Configure a build with `MIRACLE_BUILD_BENCHMARKS=ON`, then run:

```sh
python3 benchmarks/meta-cache/run.py build/tests --samples 3
```

Every fixture imports `Miracle.Meta`, including the raw `<meta>` baseline, so the comparison does not charge only Miracle for loading the module BMI. Before measurement the driver builds the excluded benchmark targets once, ensuring CMake's module maps and imported BMIs exist. It then runs the exact compiler command from `compile_commands.json` directly under GNU `time`; dependency scanning and linking are therefore outside the reported wall time and peak RSS.

The variants distinguish:

- `RawOnce`: one direct `nonstatic_data_members_of` plus `define_static_array`;
- `MiracleOnce`: one typed `meta::fields<T>` lookup;
- `MiracleRawOnce`: one value-oriented `meta::fields(^^T)` lookup through the private reflection bridge;
- `RawRepeat`: repeated raw unchecked queries;
- `MiracleRepeat` / `MiracleRawRepeat`: repeated canonical-cache lookups;
- `RawVisibleRepeat`: repeated raw access-filtered queries;
- `MiracleVisibleRepeat` / `MiracleRawVisibleRepeat`: repeated cached caller-access projections.

The repeated workload is normalized to roughly 2048 reflected-member visits per TU, with a minimum of two repetitions for the 1024-field subject. One-shot Miracle results should remain within measurement noise of raw `<meta>`; repeated queries should be cheaper when the compiler does not already canonicalize the equivalent raw work itself.
