# Miracle — Phase M progress ledger

**Status:** In progress — author-owned implementation; batched final review planned.
**Last reconciled:** 2026-10-30
**Repository baseline inspected:** `e2da473e6dd5637f1f70625738f8378e211a7478` (Miracle v0.5.28).
**Approved contract:** [`DESIGN.md`](DESIGN.md) v2.0.
**Technical guide:** [`IMPLEMENTATION.md`](IMPLEMENTATION.md) v2.2.
**Decision record:** [`../../adr/0001-phase-m-unified-format-color.md`](../../adr/0001-phase-m-unified-format-color.md).

## Confirmed repository implementation at inspected baseline

- `.cppm`/`.cpp` source migration is already reflected in the inspected tree.
- `Miracle.Numeric`, `Miracle.Types`, `Miracle.Range`, `Miracle.StaticString`, generated build modules, and `Miracle.Feature` extraction have committed source work.
- `Miracle.Feature` already has `Types`, `Descriptor`, `Set`, `Catalog`, and `Requirement` partitions. Do not reapply an obsolete monolithic-Feature migration plan.
- `Miracle:Iter`, `Miracle:Diagnostic`, and several additional legacy umbrella partitions remain; `Miracle.Color` and `Miracle.Format` were not yet present.
- Windows `src/private/windows/Memory.cpp` at inspected revision declares `module Miracle.Core;` while importing `:Memory`; verify/fix module identity with the target platform workflow.
- Reference C++26 CI passed on the inspected pre-Color/Format revision; **this is not evidence for the new APIs**.

## Milestone ledger (implementation ≠ verification ≠ acceptance)

| Milestone | Source progress observed | Review/acceptance | Author action / evidence |
|---|---|---|---|
| M.1 Baseline/graph | Partial remote inventory; no reproducible perf baseline in this handoff | Not accepted | Commit DAG, module map, cold/incremental measurements and toolchain identity |
| M.2 Extensions/toolchain | Source rename reflected in repository | Not accepted | Produce final path-inventory/build+consumer verification for the M.2 commit |
| M.3 Foundations | Numeric/Types/StaticString work present; Color and Format absent | Not accepted | Complete Color/Format direct import, source APIs, constexpr tests |
| M.4 Diagnostics | Legacy `Miracle:Diagnostic` still present | Not accepted | Move semantic module and adapt renderer to Format sink/events |
| M.5 Other modules | Partial (Feature/Range); Iter and others still partitions | Not accepted | Complete named module ownership, correct implementation-unit declarations |
| M.6 Decomposition | Feature partitioned; Iter and Meta Query pending analysis | Not accepted | Split where dependency/instantiation analysis justifies; preserve Meta behavior |
| M.7 Umbrella/package | Umbrella still reexports legacy partitions | Not accepted | Reexport independently importable modules and test installed package |
| M.8 Final gates | Not executed for finished Phase M | Not accepted | Run full reference compiler, negative/positive fixtures, consumer matrix and perf comparison |

## Scope, verification and handoff

- Color/Format F7–F10 and DEC-COL-001–003 / DEC-FMT-001–004: **design approved 2026-10-30**.
- Exact new API reference-compiler verification: **NOT RUN**. Host C++23 probes attached to earlier draft establish only limited language shapes.
- No milestone is marked Accepted solely from a commit, passing CI subset or this ledger.
- Author commits each milestone independently. Reviewer conducts one batched milestone-by-milestone contract review after the author's M.8 handoff.
- Protocol review outcomes: Accepted / Changes requested / Verification pending / Contract revision required.
- Core Revisit owns broad format grammars, dynamic formatting and advanced color frameworks; not Phase M.

**Next author action:** Implement the approved minimal `Miracle.Color` and `Miracle.Format` interfaces, plus specified API and compiler probes, against current local checkout; reconcile any divergences from this remote snapshot in commits/progress. No production code was authored by the reviewer.

## Color/Format conformance pass — 2026-11-25

**Verified with:** `clang-cxx26` b56ae4e61cf1 (cxx26-2026.10.27), libc++, `-std=c++26`, `tests` preset, clean build in an isolated copy.
**Result:** `MiracleTests` 95 cases / 500 assertions pass; `ctest` 71/71 pass. Evidence covers in-tree standalone compilation only; installed-consumer and `static_assert(false, message)` negative fixtures are **NOT RUN**.

Fixed against spec v2.2:
- `Color::parseHex`: required `#`, rejected bare digits, fixed `#RGB` expansion (was `0xABC` → `0xABCABC`; now `17*n` per nibble), no `from_chars`.
- `Color::checkedRgb`: constrained to non-bool `std::integral` (floats were a hard error, not substitution failure).
- `Color::fromHsv`: rewritten per ALG-COL-002 (sector algorithm, half-up quantization, clamp).
- `Miracle::ColorMode` / `Miracle::Presentation` moved from `Miracle::format` to `Miracle` (API-FMT-002).
- `Fragment<N>`: literals now store N-1 bytes (the NUL was included); `style()` observer; owned `std::array`.
- `Code<P, Width>`: `P` excludes the NUL; empty prefix omits the bracketed code; shares the decimal encoder with the runtime path.
- `Message`: constructor/CTAD construction; `const` extenders returning new types; inferred, overflow-checked capacity; single context; non-bool/char integral append via modular magnitude; canonical grammar with one final LF; C0/DEL escaping (`\n \r \t \0 \x1B \xNN`).
- Added `CodeView`, `EventKind`, `Event`, `WriteResult`, `RenderState` (with irreversible `failed`), `TextSink`, `emit`, `finish` (DEC-FMT-002). `renderTo`/`render` and `Message` share the one layout and one ANSI encoder (ALG-FMT-004; reset on every style change, final reset on finish).

Owner-approved deviations kept (2026-11-25): `Miracle.Color` imports `Miracle.Numeric`; `std::formatter<Color|Hsv>` exported; `format::message()` factory and `StyleSpan` names. `Message` storage stays public so it remains a structural annotation value.

Judgment calls to confirm: the `--> context` line follows the heading with no blank line (matches §5.3 example); all later sections get one blank separator. Fields use two-space indent; notes/help use three (`   = note:`). `Message` has no `Fragment` summary-less constructor and no style-span-array factory (not in spec).

### M.4 step 4 — shared ANSI encoder (2026-11-25)
- `DiagnosticRenderer` (src/private/Diagnostic.cpp) now writes through exported `format::AnsiWriter` (via `StreamSink`); private ANSI strings removed.
- Plain output byte-identical. Terminal output changed deliberately: SGR now separate sequences with indexed colours (`ESC[1m ESC[38;5;9m`), severity colours from `severityStyle`; reset after every tree. `tests/diagnostic.cpp` goldens updated.
- Evidence: reference compiler, 95 cases / 500 assertions, ctest 71/71.
- Not done: module rename `Miracle.Diagnostic` → `Miracle.Diagnostics`, installed-consumer fixture, runtime code descriptor → `CodeView` adapter.

### M.4 steps 1–5 (2026-11-25)
- Module renamed `Miracle.Diagnostic` → `Miracle.Diagnostics`; files moved to `src/public/diagnostics/Diagnostics.cppm`, `src/private/Diagnostics.cpp`. Importers (Error, Panic, Miracle) and CMake updated. `Miracle::Diagnostic` type name unchanged.
- `DiagnosticCodeDescriptor::codeView()` → `format::CodeView`; runtime header uses exported `format::writeCode`, the same encoder as the compile-time Message heading. Prefix is now escaped like Format's.
- Evidence: ctest 71/71 (isolated copy, reference compiler). Still open: installed-consumer fixture.
- Installed consumer `tests/consumer/diagnostics` (imports only `Miracle.Diagnostics`; real Diagnostic with code/span/note/help/cause; plain/terminal render; Severity/ColorMode/Presentation identity asserts). `Miracle.Diagnostics` now `export import Miracle.Format`. Built against a `cmake --install` prefix with the reference compiler: exit 0. Added to `scripts/ci/verify-container.sh`.
- M.4 fixtures: compile-fail `diagnostic-non-enum`, `diagnostic-empty-prefix` (pre-existing: negative, align-without-prefix; duplicate prefix/align/message covered by static_asserts). Runtime: `snippetEdgeCases` (empty, no selection, label-only, zero-length, huge/out-of-range, reversed, multi-line, line-crossing, secondary), `zeroWidthCodes`, `styleResetsAfterTree`. Existing deep/wide cause tests cover iterative traversal. Result: ctest 73/73, 98 cases / 517 assertions.

### M.5 audit (2026-11-25)
- Inventory: 21 primary `export module Miracle.*` interfaces (+ umbrella `Miracle`); only Format/Feature/Meta use proper partitions; no `module Miracle;` implementation units remain (Diagnostics, Error, Fs, Panic, Profiling, Memory×3 platforms all name their own module).
- Added per-module direct-import probes `tests/import/*.cpp` (17 modules; no umbrella) wired as `Miracle.Import.<Name>` ctest cases, plus existing `Miracle.Meta.Import`. Result: ctest 91/91 (isolated copy, reference compiler).
- Not covered: per-module installed-consumer builds, BMI boundary inspection, Windows/unsupported Memory units (not compiled on this host). M.6 (Iter 3700 lines, Meta Query 2190 lines) untouched.

### M.6 Iter decomposition (2026-11-25)
- `Miracle.Iter` is now a re-export-only primary over partitions `:Detail` (forward decls, `enable_view` specs, adaptive invocation, MovableBox/AdaptiveCallable/ComposedProjection, projected iterator/sentinel), `:SourceViews` (MaybeRef/MaybeValue/OnceWith/RepeatWith/FromFn/Successors/Cycle + their borrowed spec), `:Views` (ProjectedFilterView/Scan/Intersperse*/MapWindows/ArrayChunks), `:Core` (Infinity, SizeHint, terminal traits, Peekable, Iter, ParallelIter, member definitions, Iter borrowed spec), `:SourceFactories` (iter/empty/once/repeat*/fromFn/successors/zip). Order: Detail → {SourceViews, Views} → Core → SourceFactories; acyclic. No Consumers partition (class members stay in Core, per spec).
- Mechanical move of original text; no declaration, constraint or overload edits (original kept at the old line ranges). Existing tests/iter.cpp unchanged and green.
- `tests/import/Iter.cpp`: direct-import probe for INV-ITER-003/004 (view/borrowed/category assertions, tuple + reflected-aggregate invocation, move-only callable, non-common sentinel source, factories from another partition). Result: ctest 91/91.
- Meta Query: decision NOT to split (spec §6.9/M.6 decision policy): REQ-META-001 forbids declaration-point/observation changes, no import-boundary benefit, size alone is not a defect. Retained as one cohesive partition.
- Feature: audit only; already decomposed (Types/Descriptor/Set/Catalog/Requirement); `Miracle.Import.Feature` probe sees the exported APIs directly.

### M.7 integration (2026-11-25)
- Umbrella `Miracle` re-exports all 21 primary modules (Color/Format included); no partition imports.
- Consumers, reference compiler: add_subdirectory (build+run exit 0), FetchContent with local source (exit 0), clean Release install → `find_package` consumer (exit 0), `tests/consumer/diagnostics` (exit 0), all 18 `tests/import/*.cpp` direct-import probes built against the installed Release package (all exit 0).
- Not run: disabled-optional-feature matrix beyond the existing `Miracle.Features.*` ctest cases, clangd/Neovim smoke, GCC. `docs/SUMMARY.md` unchanged (no new chapters).
- Import graph: Format → Color/Numeric only; Color → Numeric only; no Format→Diagnostics edge (Diagnostics imports Format); Iter partitions acyclic.

### M.8 partial evidence (2026-11-25)
- Cold build (`tests` preset, 22 cores, `ninja -t clean` between runs, 5 runs): HEAD 38.1 s median (37.4–39.1); working tree 39.7 s (37.6–41.8). Delta within run-to-run overlap; tree also adds 18 import probes, 2 compile-fail fixtures and new tests.
- ACCEPTANCE.md gate statuses updated to reflect evidence. Not run: incremental/BMI/peak-memory benchmarks, Meta Query stress, GCC, CodeQL, CI container script. Author sign-off pending; nothing committed or tagged.

### M.8 further evidence (2026-11-25)
- Format negative fixtures (7, all registered as `Miracle.Features.CompileFail.Format*`): field-after-note, context-after-note, second-context, note-after-help, empty field key, append-to-diagnostic, and `static_assert(false, Message)` showing `error[FMT007]: bad Iter pipeline`. ctest 98/98.
- BMI footprint (`Miracle*.pcm`): HEAD 50 files / 10.6 MB; tree 58 files / 13.6 MB (+5 Iter partitions, Emit, Text, more Format partitions). `libMiracle.a` 9.81 MB → 9.84 MB.
- Incremental `tests`-preset rebuild after appending a comment, 3 runs, median (min–max): private .cpp 2.4 s (1.7–3.3) → 2.5 s (2.2–3.4); `Numeric.cppm` (leaf) 54.4 s (49.1–68.0) → 68.7 s (47.2–83.5); `Iter.cppm` 54.6 s (48.6–65.0) → 63.2 s (51.8–68.3). Ranges overlap heavily; the tree builds ~27 more targets (18 import probes, extra tests), which accounts for part of the public-edit delta. Not isolated per target.
- Cold-build peak RSS of the largest compiler process: 1.50 GB → 1.50 GB (unchanged).
- Disabled-feature matrix: no optional build feature is registered in the main project yet (CMakeLists.txt: "no optional heavy feature"), so the matrix is vacuous; gating logic is covered by the existing `Miracle.Features.*` CMake/compile-fail ctest cases (green).

### M.8 Meta Query stress (2026-11-25)
- `src/public/meta` is byte-identical to the baseline, so Query behaviour is unchanged by construction. Compile-only fixtures (1 run, Debug, reference compiler): RawOnce1024 13.3 s → 13.1 s; MiracleOnce1024 17.3 s → 15.3 s. The `Repeat` fixtures fail with "constexpr evaluation hit maximum step limit" identically on the HEAD baseline and the current tree (pre-existing; they need the step-limit flag used for the recorded baseline) — not a regression.
