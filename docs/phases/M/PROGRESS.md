# Miracle — Phase M progress ledger

**Status:** In progress — author-owned implementation; batched final review planned.
**Last reconciled:** 2026-11-25
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

Updated 2026-11-25. "Implemented" means committed source plus the evidence listed below; nothing is Accepted.

| Milestone | Source progress | Review/acceptance | Evidence / remaining |
|---|---|---|---|
| M.1 Baseline/graph | Source inventory and import graph now produced by `scripts/ci/module-graph.py` (`Miracle.Modules.Graph`); HEAD-vs-tree timings recorded below | Not accepted | No pre-rename historical perf baseline exists (UNRECOVERABLE; HEAD `c26771b` used as the baseline). Raw CSV/JSON artefacts not exported |
| M.2 Extensions/toolchain | `.cppm`/`.cpp` layout in place; module graph script checks every unit is in CMake | Not accepted | CodeQL not available locally |
| M.3 Foundations | Color and Format rewritten to spec; Numeric/Types/StaticString unchanged (StaticString has uncommitted author WIP) | Not accepted | Types does not re-export Numeric (REQ-MOD-004 decision pending) |
| M.4 Diagnostics | `Miracle.Diagnostics`; renderer on shared `AnsiWriter` and code encoder | Not accepted | Control-character escaping policy (§8.2) pending decision |
| M.5 Other modules | All primaries named; per-module import probes | Not accepted | Windows/unsupported Memory units not compiled here |
| M.6 Decomposition | Iter split into five partitions; Meta Query split into IR/Plan/State/Eval/Query | Not accepted | Query split follows the user's explicit instruction; see section below |
| M.7 Umbrella/package | Umbrella re-exports every primary; add_subdirectory, FetchContent, installed consumers pass | Not accepted | disabled-feature matrix vacuous (no optional features registered) |
| M.8 Final gates | Partial: tests, compile-fail fixtures, consumers, benchmarks recorded | Not accepted | Author sign-off, GCC/CodeQL/clangd, CI container run |

## Scope, verification and handoff

- Color/Format F7–F10 and DEC-COL-001–003 / DEC-FMT-001–004: **design approved 2026-10-30**.
- Exact new API reference-compiler verification: run on `cxx26-2026.10.27` and `cxx29-2026.11.10` (see sections below); GCC not run.
- No milestone is marked Accepted solely from a commit, passing CI subset or this ledger.
- Author commits each milestone independently. Reviewer conducts one batched milestone-by-milestone contract review after the author's M.8 handoff.
- Protocol review outcomes: Accepted / Changes requested / Verification pending / Contract revision required.
- Core Revisit owns broad format grammars, dynamic formatting and advanced color frameworks; not Phase M.

**Next author action:** review the open decisions listed at the end of this ledger and sign off (or reject) each acceptance gate.

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

### M.8 remaining gates (2026-11-25)
- Real working tree (including uncommitted `examples/meta.cpp` and `StaticString.cppm` edits), fresh out-of-tree build dir, reference compiler, Debug + tests/examples: builds; ctest 98/98.
- GCC: not applicable on this branch (`cmake/toolchains/GCC.cmake` does not exist; GCC support lives on `origin/gcc`).
- CodeQL: not installed here; extraction coverage unverified (known limitation).
- clangd smoke: system clangd rejects the reference compiler's PCMs (`ast_file_version_too_old`), so semantic results are unusable; the compile database itself is generated. Needs a clangd from the reference toolchain.
- `scripts/ci/verify-container.sh`: not run (needs the container toolchain); its added diagnostics-consumer steps were run by hand.

### Meta Query split and spec audit (2026-11-25, second pass)
- Meta Query split on explicit owner instruction (supersedes the earlier "not split" note): `Miracle.Meta:QueryIR` (opcode/Instruction/ProgramNode), `:QueryPlan` (SegmentNode/SegmentPlan/evaluation-state slots), `:QueryState` (capture environment, sources, adaptive invocation, stored values), `:QueryEval` (emit/evaluate), `:Query` (public `Query<State>` facade, `query()`). Text moved mechanically from the original 2,190-line partition at its existing section boundaries; declaration order, constraints and reflection observation points are unchanged. Import order is linear (IR → Plan → State → Eval → Query), no cycles. Evidence: ctest 99/99 incl. all 47 Meta parity/hardening/compile-fail cases, 18 installed direct-import probes on the Release package. Compile cost (`MiracleOnce1024`, 5 alternating runs, same machine state): pre-split median 13.2 s (12.1–14.1) vs split 12.5 s (11.1–14.6) — neutral.
- New `scripts/ci/module-graph.py` (ctest `Miracle.Modules.Graph`, ALG-MOD-001/TEST-MOD-DAG/OWNER/AUDIT-COVER-001): one primary per module, resolved imports and partitions, no `module Miracle;` units, no partitions outside Format/Feature/Iter/Meta, no `.ixx/.cxx`, acyclic, forbidden-edge matrix (Format↛Diagnostics/StaticString/Meta/Iter, Color↛Format/…, Diagnostics↛Meta), umbrella is re-exports only, every unit in CMakeLists.txt. Negative cases (cycle, forbidden edge, legacy unit) verified by mutation.
- Added tests: TEST-FMT-STATIC-004 (long reflected type name, no truncation), TEST-FMT-SINK-005 (unwritable iterator rejected by constraint), TEST-DIAG-PARITY-001 (compile-time vs runtime heading), TEST-DIAG-TREE-002 (sibling order/indent).
- Docs: `DIAGNOSTICS.md` module list, `CHANGELOG.md` entry, milestone ledger refreshed.
- Open findings / owner decisions (spec vs. repository):
  1. RESOLVED (owner): Debug and Bitflags moved from `src/public/derive/` to `src/public/debug/` and `src/public/bitflags/` per REQ-REST-001.
  2. RESOLVED (owner: keep + document): `Miracle.Types` does not re-export `Miracle.Numeric`; the contract is pinned by compile-fail fixture `types-without-numeric.cpp`.
  3. RESOLVED (owner): Diagnostics now escapes C0/DEL bytes in messages, notes, help, labels, paths and snippet source via `format::writeEscaped` (`\n \r \t \0 \xNN`); snippet carets are computed in escaped columns. Test `untrustedTextIsEscaped`.
  4. RESOLVED (owner): DESIGN.md amended — Format and Color may import `Miracle.Numeric` (alias-only leaf) in addition to Color for Format.
  5. §6.1 layout: private units are flat (`src/private/Diagnostics.cpp`), spec shows `src/private/diagnostics/Diagnostic.cpp`; Iter partition names follow the spec's recommended SourceViews/Views/Core/SourceFactories variant.
  6. RESOLVED (owner: finish it): REQ-STR-002 committed; `Miracle.StaticString` → `Miracle.Format` edge is allowed by the module graph.
  7. TEST-COL property/boundary tests and TEST-REC-COLOR/EVENT rows now covered (see below); TEST-FMT-STATIC-003 is now a compile-fail fixture (`FormatMessageDanglingStorage`, expects "pointer to subobject of 'message' is not a constant expression"); TEST-STR-NEG-001..003 are compile-fail fixtures (`StaticString{UnmatchedOpen,UnmatchedClose,BadPlaceholder,PlaceholderCount,WideFormat,NontrivialChar}`) matching the HEAD messages, which the owner's REQ-STR-002 WIP must keep as substrings; REQ-STR-002 implemented (owner's `StaticStringError` annotations + `staticStringError()`; stub/leftover removed): fixtures now expect `error[SSTR00x]: <summary>`; verified with 107/107 ctest and 104 test cases.
  8. RESOLVED (owner: yes): `Error::display` now renders through `format::AnsiWriter` (no local SGR bytes). Visible change: chain labels use indexed colour 3 (`ESC[38;5;3m`) instead of `ESC[33m`; spec §5.2 prescribes `38;5;i` for indexed colours, so a short-form special case was not added. `tests/error.cpp` snapshot updated.

### Color/Format test-matrix audit (2026-11-26)
- TEST-COL-*: added RGB-001..003, HEX-001, TYPE-002 (structural NTTP), PARSE-002/004..008, HSV-008/009/010 (every sector boundary ±1e-9, `nextafter(360, 0)`, 359.999999), REV-003/004, PROP-001 (10,000 SplitMix64 samples, seed `0x4D6972616375'6C65`, plus tied-channel adversaries; measured max channel error ≤ 1, asserted), PROP-002 (all `#RRGGBB` with zero low byte, 10,000 random, 216 boundary triples; upper and lower case).
- TEST-REC-COLOR-002: all 4,096 `#RGB` combinations, constexpr.
- TEST-REC-EVENT-003: sink failure inside an SGR prefix and while writing the final reset → `SinkFailed`, `state.failed`, partial bytes exact.
- Verified in the isolated tree: `MiracleTests` 104 cases PASS; ctest 100/100.
- Remaining: none in the test matrix (StaticString WIP is the owner's), PERF rows, §10.5–10.6.

### §10.6 API verification matrix — current status (2026-11-26)
`IMPLEMENTATION.md` keeps the spec's original "NOT RUN" column (it describes the pre-implementation document). Status against the pinned reference compiler (clang 22.1.8, cxx26-2026.10.27), isolated tree:

| ID | Status | Evidence |
|---|---|---|
| API-V-COL-01..03 | PASS | `tests/color.cpp` (constexpr RGB/hex/parse/HSV, finite checks, boundary round-trips), `Miracle.Import.Color` |
| API-V-FMT-01 | PASS | `Miracle.Import.Format`, installed `find-package` consumer |
| API-V-FMT-02 | PASS | single owner enforced by `Miracle.Modules.Graph`; `Miracle.Import.Diagnostics` |
| API-V-FMT-03..06 | PASS | `tests/format.cpp` static_asserts (aggregate, CTAD, width, append, section order) and `format-*` compile-fail fixtures |
| API-V-FMT-07 | PASS | `FormatStaticAssertMessage` compile-fail snapshot |
| API-V-FMT-08 | PASS | `runsAreOrdered`, `renderAnsi*` tests |
| API-V-FMT-09 | PASS | `renderTo`/`render`/`emit`/`finish` tests, `CanRenderTo`, `UnwritableIterator` |
| API-V-DIAG-01 | PASS | `tests/diagnostic.cpp`, `tests/consumer/diagnostics/` |
| API-V-ITER-01 | PASS | `tests/iter.cpp`, `Miracle.Import.Iter`, TEST-ITER-MOD-001 |
| API-V-BLD-01 | PASS | Release install + 18 per-module import probes |

PERF coverage: PERF-01 (cold build, 5 runs, peak RSS), PERF-02/03 (incremental private/public edits), BMI footprint and PERF-05 (Meta Query stress; Iter/Feature instantiation counts not stress-timed) are recorded in the M.8 sections above; noise on this machine is large, so only "neutral/overlapping" is claimed. PERF-04/06/07 were measured on 2026-11-26 (below). PERF-08 is measured below.

### PERF-04/06/07 measurements (2026-11-26)
Reference compiler, Debug `tests` BMIs (PERF-04/06) and Release install (PERF-07), 22 cores, other jobs idle; no baseline exists for these (new modules/API), so they are absolute figures, not regressions.
- PERF-04, TU-only compile of the 19 empty import probes (BMIs prebuilt), median of 5 (wall, peak RSS): Build 0.01 s/63 MB, Color 0.01/69, Numeric 0.01/67, Format 0.02/79, Diagnostics 0.02/94, StaticString 0.02/96, Types 0.02/86, Fs 0.02/94, Range 0.03/100, Bitflags 0.09/126, umbrella `import Miracle` 0.13/130, Iter 1.03/220 (its probe instantiates pipelines). Meta has no per-module probe, so it is not covered.
- PERF-06, `Message` of 10-byte fragments, median of 3 (wall, peak RSS): chained appends ×10 0.14 s/126 MB, ×100 0.59 s/138 MB, ×1000 7.4 s/665 MB; one literal of the same total size 0.06–0.07 s/118–120 MB at every size. Chained cost grows super-linearly (the spec's known O(KN) tradeoff). Representative diagnostics chain fewer than ~10 fragments (<0.2 s); 1,000 chained appends is a pathological case, recorded here as the review disposition input (no threshold invented).
- PERF-07, one `Diagnostic` with a 1 MiB source line and 1,000 notes (≈1.6 MB output), Release, 7 runs: plain 2.3 ms median (710 MB/s), ANSI 1.8 ms (924 MB/s), 22 allocations per render in both modes.
- PERF-08, installed Release package (1.6 MB, `relinst`), consumer built from a fresh build dir (configure + build, `import std` BMI included), 3 runs: find-package consumer cold 8.1–8.4 s, warm (touch `main.cpp`, rebuild) 0.13–0.14 s; diagnostics consumer cold 7.9–8.2 s, warm 0.34–0.39 s. All installed consumers and 18 probes build and pass against the Release install, re-run after the `Error::display` change. No pre-split baseline exists for installed consumers.

### Parts XI–XII pass (2026-11-26)
Checked against current source and the isolated tree (ctest 107/107, `MiracleTests` 107 cases):
- §12.1 decisions: DEC-M-004 one linkable target (`Miracle::Miracle`, single `add_library`), DEC-FMT-ARCH-002/003/004/005/007/009 (single `format` namespace, constructor-built `Message`, inferred capacity, plain bytes + style spans, one `Severity`, ANSI only in the renderer), DEC-FMT-003 (one LF after the last line, covered by the emit/render tests), DEC-FMT-004 (escaping, finding 3), DEC-FMT-005/006 (`StyleSpan` coalescing, `CodeView`), DEC-COL-ARCH-001 (Color imports only `Miracle.Numeric` in addition to std, owner-approved), DEC-META-001 (Query split, no behaviour change).
- §12.2 findings: C-001/C-002 resolved by `DESIGN.md` (amended), C-003/C-004 (no `Builder`/`Level`/fixed `Message<512>`; verified by grep and API), C-007 resolved (`src/private/windows/Memory.cpp` declares `module Miracle.Memory;`; Windows compile NOT RUN), C-008 (source/snippet/cause logic stays in `Diagnostics.cpp`).
- §11.10 reject-list: no raw ANSI outside Format after the `Error::display` change, no duplicate severity/colour enums, no fixed buffers, no `module Miracle;` units (graph validator).
- Gaps found and closed in this pass: TEST-FEA-001 (empty catalog) added; TEST-REST-005/006 added as `tests/memory-profiling.cpp` (process-memory snapshot, nested scope depths, aggregates, early-exit scopes, stable snapshot copy); user-facing typo "missing feautre" in `Feature:Catalog` fixed.
- TEST-BLD-INSTALL-001/002/003/005: add_subdirectory, FetchContent (local checkout) and installed find-package consumers plus 18 direct-import probes all build and run (this tree, reference compiler). TEST-BLD-INSTALL-004: installed consumers use exported partitions with no unavailable PRIVATE file set.
- §12.4 invariant matrix, current status: INV-ARCH-001/002/003, INV-MOD-001 PASS (`Miracle.Modules.Graph`, installed probes); INV-FMT-001/002, INV-MSG-001..003, INV-SEC-001, INV-ANSI-001, INV-NUMTXT-001, INV-DIAG-001 PASS (tests/format.cpp, tests/diagnostic.cpp, compile-fail fixtures); INV-ITER-001..004 and INV-META-001/002 PASS by the preserved baseline suites (tests/iter.cpp, tests/meta/*) plus the partition split; INV-RES-001 PASS (`format-*` and `StaticString*` fixtures).
- Not done by this pass: the spec's "generated trace validator" (missing/duplicate/orphan test IDs) does not exist; test IDs appear in comments and this ledger only. Windows Memory compile, GCC, CodeQL, clangd and the CI container script remain NOT RUN.

### Part V and Appendix A–D line-by-line pass (2026-11-26)
Checked every declaration, API-* clause, algorithm and invariant of Part V (§5.1–5.5) and Appendices A–D against `Color.cppm`, `Fragment.cppm`, `Code.cppm`, `Message.cppm`, `Emit.cppm`, `Render.cppm` and the tests. Findings:
- Matches (no change): ALG-COL-001..003, API-COL-001..007 (Color keeps the existing `<=>`, which also provides `==`; the spec lists only `==`. The baseline already exposed relational operators on `Color`/`Hsv`, so removing them would break source compatibility; recorded, not changed), API-FMT-001..006/008..016, Appendix A event/state machine (extra `RenderState::failed` is the spec's own D.3.1 recommendation), REQ-REC-EVENT-001..005, INV-REC-DIAG-001/002, INV-REC-CAP-001/002, B.3 four static_assert cases, C.1–C.4 review checklists, D.6 byte/control contract.
- Deliberate deviations kept: `Message` storage is public and structural because it is used as a reflection annotation value (B.3 says not to promise structural unless a consumer needs it; `format::message(...)` annotations are that consumer); `StyleSpan` instead of `StyleRun` (owner decision); the literal constructor is not `explicit`.
- Fixed: INV-REC-CAP-003 — a `char[N]` that is not NUL-terminated used to lose its last byte silently. All literal-style overloads now go through `detail::literalView`, which rejects it (compile error in constant evaluation, trap at run time); exact bytes keep using `fragment(std::array<char, N>)`. This is option (b) of the spec's three choices; the owner may pick another, but the spec requires one with negative tests, which `format-unterminated-array.cpp` is.
- Added: static_asserts for trivially-copyable Style/TerminalColor/StyleSpan, canonical TerminalColor equality, "no `std::formatter` for Message/Fragment" (API-FMT-016), render not `noexcept` (API-FMT-015), interior-NUL literals; compile-fail fixtures for `styled`/`fragment` with a runtime `std::string_view` and `field` after `help` (§5.4 negative space).
- Meta: moved the stand-alone Meta import probe into `tests/import/Meta.cpp` (`Miracle.Import.Meta`), so the installed-package probe set is now 19 modules including Meta (B.2 module-consumer-06); the former `Miracle.Meta.Import` test is replaced by it.
- Result: ctest 111/111, `MiracleTests` 107 cases (isolated tree, reference compiler); installed Release package: both consumers + 19 probes pass.

### Part VII and VIII pass (2026-11-26)
Checked §7.1 (representation, INV-MSG-001..004, ALG-FMT-001 append/overflow, ALG-FMT-002 transitions, ALG-FMT-003 decimal, ALG-FMT-004 ANSI, ALG-FMT-005 adaptation), §7.2/7.3 (graph validator, installed consumers), §7.4 (error/resource policy), the 30-row risk table (§8) and §8.1–8.3.
- Fixed: `append(integer)` accepted integers wider than 64 bits (only when `std::integral` includes `__int128`, i.e. gnu extensions) and would truncate them in the 64-bit encoder; now constrained to `sizeof <= sizeof(u64)`. `Message<SIZE_MAX, 0>` named a type whose `Capacity + 1` wrapped to an empty array; now a `static_assert` (TEST-FMT-OVERFLOW-001, fixture `FormatCapacityOverflow`).
- Added tests: TEST-FMT-NUM-004 (bool/char/wide-char/128-bit never reach the numeric overload), TEST-FMT-STYLE-006 (20 alternating runs, exact spans), TEST-FMT-OWN-001/002 (owned after local array, copies do not alias), TEST-FMT-COLOR-004 (all 256 indices, foreground and background, as `38;5;i`/`48;5;i`), INV-ANSI-001 (stripping SGR from ANSI output equals plain output, ends in reset), TEST-COL-RGB-004/RISK-005 (`Color{300,0,0}` is rejected by brace narrowing; `Color::rgb` stays unchecked per DEC-COL-001).
- TEST-PERF-FMT-003: a 64 KiB literal `Message` compiles in 0.09 s (113 MB); a 64 KiB diagnostic heading plus one note in 0.63 s (150 MB); 1,000 chained 10-byte appends cost 7.4 s (see PERF-06). No resource failure at either size.
- Risk table status: RISK-001..005, 007..015, 017..025, 028..030 covered by tests or structure listed above; RISK-016 (Format → StaticString) by the module graph validator; RISK-026 (CodeQL exclusion) and RISK-025's Windows compile remain NOT RUN (environment); RISK-027 measured, not gated.
- Result: ctest 113/113, `MiracleTests` 109 cases (isolated tree, reference compiler).
