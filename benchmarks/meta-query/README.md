# Meta Query compiler-cost benchmark

These fixtures compare a direct C++26 `<meta>` multi-stage member transformation with the equivalent fused `Miracle::meta::Query` pipeline. Both variants import `Miracle.Meta`, and the benchmark times the exact compiler command from `compile_commands.json` after CMake has generated module maps and imported BMIs.

The workload is:

```text
fields -> retain instance data -> obtain names -> discard missing names -> retain names longer than three characters
```

The Miracle representation keeps this as one normalized Query state machine. The raw baseline performs the equivalent work directly in a consteval loop. `RawRepeat` and `MiracleRepeat` execute a size-scaled repeated workload of roughly 2048 logical source visits to expose compile-time reuse and fusion behavior.

Run with:

```sh
python3 benchmarks/meta-query/run.py build/tests --samples 3
```

The benchmark reports front-end wall time and peak RSS. It is diagnostic rather than a fixed absolute-time gate because compiler load is host-dependent; the important signal is whether the fused Query representation stays near the direct raw expression for one-shot work and avoids multiplying intermediate compile-time sequence machinery for repeated use.

The control-flow fixtures also compare a Boolean predicate path with a `try`/`catch (std::meta::exception)` fallback path on the same mixed reflection sequence. They are intentionally diagnostic rather than a policy gate: both forms are valid compile-time control flow, and the measurements document which spelling is cheaper on the reference compiler.
