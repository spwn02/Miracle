# Miracle — Phase M acceptance and review matrix

**Status:** VERIFICATION PENDING — not a signed acceptance report.
**Contract:** [`DESIGN.md`](DESIGN.md) v2.0; [`IMPLEMENTATION.md`](IMPLEMENTATION.md) v2.2.

A milestone commit is an implementation checkpoint, not automatic reviewer acceptance. For the agreed batched review, the author supplies the M.1–M.8 commit range, locked source revision, pinned reference compiler identity, build flags, logs, benchmark hardware/environment and failures/exceptions. Do not substitute host C++23 probes for reference C++26 validation.

| Gate | Required evidence | Status at reconciliation |
|---|---|---|
| A-M-001 | Target module inventory, acyclic imports, sole owning module for each public entity | PARTIAL — module inventory, acyclic imports and no legacy `module Miracle;` units verified 2026-11-25 (PROGRESS.md M.5/M.7) |
| A-M-002 | Tracked source suffix inventory including generated templates/tests/examples/benchmarks/probes/editor scripts | NOT RUN for final tree |
| A-COL-001 | Color RGB checked ranges, HEX parse and conversion; HSV sectors/round trips/boundaries | EVIDENCE — `tests/color.cpp` static + runtime, ctest green (author review pending) |
| A-FMT-001 | Message/Fragment CTAD, sized growth, style runs, `static_assert` constant-expression operand | EVIDENCE — CTAD, capacity, overflow guard, style runs, `static_assert(false, msg)` and dangling-storage fixtures |
| A-FMT-002 | Source-invariant constructors, monotonic context/field/note/help order with negative builds | EVIDENCE — constructors/order constraints and compile-fail negatives (order, runtime views, unterminated arrays, capacity overflow) |
| A-FMT-003 | Plain and ANSI fragments, indexed + RGB colors, no ESC in `data()`/`view()` | EVIDENCE — plain/indexed/RGB SGR and no-ESC `view()` tests |
| A-FMT-004 | Event/TextSink parity with owned Message, invalid order, partial stream failure and reset | EVIDENCE — Event/TextSink parity, invalid order, sink failure, reset tests |
| A-FMT-005 | Control escaping, exactly one trailing LF, deterministic section layout | EVIDENCE — escaping, single trailing LF, section layout tests (blank-line ambiguity §5.3 vs ALG-FMT-002 awaiting author decision) |
| A-DIAG-001 | Existing Diagnostic APIs/enum annotation reflection/source spans/causes/stacktraces preserved | EVIDENCE — Diagnostic tests incl. snippet edge cases, deep/wide causes, compile-fail domains; terminal SGR goldens changed deliberately (PROGRESS.md) |
| A-DIAG-002 | Miracle-authored hard compile-time errors use the common layout where feasible; limitations documented | EVIDENCE — StaticString (`SSTR001–004`), Feature (`FEA001–012`) and Iter (`ITR001–007`) hard errors are Format-authored `error[CODE]: summary` messages with context/fields and the original wording kept; fixtures assert code and text. LIMITATION (documented): Meta Query hard errors, Diagnostics code-annotation concept failures and range/type-level `static_assert`s without messages keep their original text (frozen Phase 5 Meta contract, `std::meta::exception` semantics per REQ-ERR-004, concept-subsumption diagnostics are compiler-owned) |
| A-META-001 | Meta Query caching, declaration-point observation, laziness and negative tests unchanged | EVIDENCE — Meta parity/hardening/query tests and compile-fail fixtures green; Query split into IR/Plan/State/Eval/facade partitions with unchanged results (PROGRESS.md) |
| A-CONS-001 | Direct import each named module; `import Miracle;`; installed `find_package`, FetchContent, add_subdirectory | EVIDENCE — 19 direct-import probes (incl. Meta) on installed Release package, `import Miracle;`, find_package, FetchContent, add_subdirectory, diagnostics consumer |
| A-CI-001 | Pinned Clang+CMake Debug/Release and test/compile-fail suite; GCC compatibility separately | PARTIAL — reference Clang Debug (tests preset, -Werror) + Release install; GCC and CI container script NOT RUN |
| A-PERF-001 | Repeated cold/incremental build and BMI/peak memory results before/after | PARTIAL — cold `tests` build, 5 runs each, HEAD baseline median 38.1 s (37.4–39.1) vs current 39.7 s (37.6–41.8); includes 22 added tests/probes; incremental/BMI/peak RSS recorded in PROGRESS.md (no thresholds claimed); Meta Query stress, granular-import (PERF-04), chained-Message (PERF-06) and render (PERF-07) figures recorded in PROGRESS.md; PERF-08 installed-consumer cold/warm timing recorded |
| A-SEC-001 | CodeQL inventory counts `.cppm` and `.cpp` accurately; no extraction claims without evidence | NOT RUN |
| A-OWN-001 | Explicit author sign-off after reviewer finding classification and exceptions | PENDING |

## Review template

For each milestone: `commit SHA`, `contract clause`, `test fixture`, `observed outcome`, `finding severity (Blocker/Major/Minor/Observation)`, `corrective property`, and `review outcome (Accepted/Changes requested/Verification pending/Contract revision required)`; include at least one representative negative test for high-risk constraints and report any limitations explicitly.

**Release state:** No Phase M completion, acceptance or version tag is claimed by this document.
