# Meta Query compiler-cost benchmark

These fixtures compare a direct C++26 `<meta>` multi-stage member transformation with the equivalent fused `Miracle::meta::Query` pipeline. Both variants import `Miracle.Meta`, and the benchmark times the exact compiler command from `compile_commands.json` after CMake has generated module maps and imported BMIs.

The workload is:

```text
fields -> retain instance data -> obtain names -> discard missing names -> retain names longer than three characters
```

The Miracle representation lowers this into normalized structural value IR and typed execution segments. The raw baseline performs the equivalent work directly in a consteval loop. `RawRepeat` and `MiracleRepeat` execute a size-scaled repeated workload of roughly 2048 logical source visits to expose compile-time reuse and fusion behavior.

Run with:

```sh
python3 benchmarks/meta-query/run.py build/tests --samples 3
```

The benchmark reports front-end wall time and peak RSS. It is diagnostic rather than a fixed absolute-time gate because compiler load is host-dependent; the important signal is whether Query topology stays bounded as pipeline depth grows, generated code remains handwritten-equivalent, and reusable reflection state produces genuine repeated-workload savings where the semantics permit it.

The control-flow fixtures also compare the correct Boolean predicate path (`hasIdentifier`) with an intentionally exception-driven spelling (`try`/`catch` around `requireName`) on the same mixed reflection sequence. This comparison is diagnostic evidence for the policy, not an endorsement of both spellings: ordinary Boolean questions should use predicates, while exceptions remain reserved for strict semantic failure and genuine speculative interpretation/fallback.

The pinned completion measurements, including the one-shot caveat and depth-64 structural results, are recorded in [`baseline.md`](baseline.md).
