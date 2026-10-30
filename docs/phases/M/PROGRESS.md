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
