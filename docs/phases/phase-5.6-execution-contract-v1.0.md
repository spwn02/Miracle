# Miracle — Phase 5.6 Execution Contract v1.0

**Title:** Standards Reconciliation, Comprehensive Audit & Final Hardening  
**Status:** Locked for execution — 2026-10-13  
**Baseline:** Reviewed Phase 5.5 patch and previously locked Miracle contracts  
**Target:** Final Phase 5.X stabilization candidate; closure is evidence-driven, not automatic

## 0. Mission and governing principles

Phase 5.6 shall correct Miracle.Meta's conformance target, reconcile its API and semantics with **adopted C++26**, comprehensively audit the implementation created and modified during Phase 5.X, correct defects, reduce unnecessary complexity, optimize demonstrated bottlenecks, and verify the resulting foundation without starting new Miracle feature phases.

The following rules are non-negotiable:

- **Standard first.** Experimental Clang/libc++ facilities never establish standard status, even when they compile and have tests.
- **Preserve the reviewed baseline.** Miracle's locked contracts and reviewed/persisted Phase 5.5 patch establish implementation provenance. A GitHub tree or compiler header cannot silently replace reviewed semantics.
- **Separate three questions:** What does C++26 specify? What does Miracle intentionally expose? What does the current toolchain happen to implement?
- **Audit before optimizing.** Characterize existing behavior; remove obsolete branches before optimizing them; verify every meaningful change.
- **Evidence over assertions.** No completion claim from a passing test suite, equal declaration counts, or a toolchain-derived facility count alone.
- **No compiler-defect workarounds disguised as API design.** Report confirmed ICEs, wrong-code, unavailable accepted facilities, and other toolchain defects with minimized reproducers.
- **Minimal necessary change.** Fix correctness and structural flaws without gratuitously rewriting sound Phase 5.5 work.
- **No unauthorized repository changes.** Produce reviewable patches; no push, force reset, or automatic replacement of the user's source of truth.

## 1. Authorities and pinned references

1. **C++26 authority:** WG21 **N5050** (2026-06-01), identified by its editors' report **N5051** as the final C++26 draft and basis of the Draft International Standard. Incorporate only formally applicable C++26 corrections/defect resolutions, with an explicit evidence trail.
2. **Historical decision evidence:** Accepted WG21 reflection papers and their adoption records, including P2996 and subsequent accepted modifications. Proposals without adoption are not normative.
3. **Readable cross-check:** `https://cppreference.com/cpp/header/meta` and relevant component pages. cppreference is an important reference and index, not the normative authority.
4. **Miracle design authority:** Locked Miracle architectural/API contracts for deliberate nonstandard conveniences, provided they do not misrepresent C++26 or rely on abandoned extensions.
5. **Toolchain:** The selected reproducible cxx26 Clang/libc++ snapshot, used for compatibility and validation only. Version, revision, flags, libc++ source, and environment must be recorded.

**Important:** The live `eel.is/c++draft` document is useful for navigation but may include post-C++26 changes. It is not a substitute for the pinned N5050 C++26 baseline. Later C++29 proposals, including enum synthesis, must be segregated.

Freeze the exact standard references and toolchain configurations in a machine-readable or structured ledger before editing implementation code. If post-freeze authority is disputed, use `Needs-resolution`; never select the compiler's behavior as the tiebreaker.

## 2. Scope and exclusions

### In scope

- Official C++26 `<meta>` inventory, including facilities defined in `std` and `std::meta`, plus related core-language constructs where Miracle relies on them.
- Signature and semantic verification: overloads, requirements, return types, constant-evaluation rules, access controls, substitutions, exceptions, storage, annotations, type traits, and supported generation.
- Removal of abandoned/compiler-derived public API, tests, adapters, examples, and dependencies; targeted corrections to conforming API.
- Complete partition-by-partition source audit, Query/cache and lifetime hardening, defect remediation, code simplification, and measured performance improvements.
- Consumer impact analysis and narrowly required Switch compatibility changes, kept in a separate patch.
- Documentation and validation infrastructure.

### Out of scope

- Adoption of unapproved C++29 or experimental features; recreation of discarded enum-generation facilities under new names.
- New Validation, Shapes, Traits, Derive, Serde, Clap, Enum++, Match, RPC, networking, or unrelated core features.
- Broad Query or Meta API redesign solely for aesthetic preference.
- Unrelated refactors of Miracle or Switch, new compatibility layers merely preserving invalid experimental behavior, or compiler patches as deliverables.

## 3. Standards/parity model

Construct the canonical C++26 inventory directly from N5050 and formally applicable C++26 amendments, **not by scraping the compiler's `<meta>` header**. Record at facility/overload granularity as appropriate. The previous Phase 5.5 **272/272** result and **522/522** partition-extraction count are historical process evidence, not C++26 parity proof or target counts.

Track independently:

1. **Normative inventory completeness:** Every applicable C++26 facility has a citation and definitive normative classification.
2. **Miracle contract coverage:** Every intended Miracle mapping (direct standard use, facade, or extension) has a documented disposition and verified semantics.
3. **Toolchain support:** Available, partial, divergent, missing, or compiler defect; never contributes to the normative facility count.

A facility intentionally consumed directly through `std::meta` need not acquire a duplicate Miracle wrapper. Conversely, a Miracle wrapper is not conforming simply because its name matches. Report *inventory coverage* and *implemented contract coverage* as separate metrics with explicit denominators; do not publish an ambiguous single parity percentage.

### Required standards ledger fields

`facility_id`, qualified name, signature/overload, category, N5050 clause, adopted-paper/correction provenance, normative status (`C++26` / `C++29 proposal` / `historical` / `not-established`), Miracle mapping, public/private visibility, implementation disposition (`retain` / `correct` / `implement` / `remove` / `direct-standard` / `investigate`), implementation status, toolchain status, positive/negative/semantic test IDs, dependency impact, finding IDs, verification evidence.

**Initial known case:** `define_enum`, `enumerator_spec`, and `is_enumerator_spec` are not to be treated as adopted C++26 facilities. `enumerators_of`, `data_member_spec`, `is_data_member_spec`, and `define_aggregate` must not be removed by association; confirm their actual normative signatures and behavior against N5050. The complete list of removals is an audit result, not something to infer from these examples.

## 4. Checkpoint plan and exit gates

Each checkpoint follows **inspect → change → test → benchmark where relevant → review diff → record evidence**. Do not advance with unexplained P0/P1 findings. Intermediate patches may be scoped by checkpoint; final publication remains consolidated.

### 5.6.0 — Baseline preservation and audit infrastructure

**Work:** Verify the supplied latest Phase 5.5 patch, its exact base revision, the locked contracts, the Miracle/optional Switch trees, module exports/declaration inventory, tests, compiler toolchain, environment, build flags, and historical benchmark reports. Record hashes and a replay procedure. Establish controls for tests and performance comparisons without altering production semantics.

**Outputs:** Baseline manifest, module/dependency map, source inventory, test/benchmark baseline, reproducibility ledger.

**Exit:** Reviewed baseline can be reconstructed and verified; provenance conflicts are reported rather than silently resolved.

### 5.6.1 — C++26 standards reconciliation

**Work:** Derive the normative facility/overload inventory and semantics from N5050, plus formally applicable C++26 corrections. Cross-check cppreference and adopted-paper history. Keep proposals, obsolete compiler artifacts, and uncertainty in separate categories. Include standard facilities Miracle uses directly even where no wrapper exists.

**Outputs:** Authoritative `Standards-Matrix.md`, standards-source ledger, unresolved normative questions.

**Exit:** Every candidate facility has a sourced classification; discrepancies against the former toolchain matrix are explainable and reviewable.

### 5.6.2 — API reconciliation and legacy cleanup

**Work:** Compare the canonical inventory with exports, internal calls, tests, documentation, and consumers. Remove abandoned facilities and their residual dependencies; repair signatures and overloads; preserve legitimate extensions only with explicit contract justification. Remove deprecated behavior completely rather than quietly maintaining experimental aliases. Keep accepted generation such as `define_aggregate` intact.

**Outputs:** Declaration-level disposition ledger, scoped cleanup patch, API-change/consumer-impact report, compile-fail checks where feasible.

**Exit:** No known abandoned extension is presented as C++26 or leaked through Miracle's public Meta API; each removed/changed declaration has a documented reason.

### 5.6.3 — Full source and architecture audit

**Work:** Review **all Phase 5.X Meta source**, including exported partitions `Types`, `Storage`, `Operators`, `Names`, `Predicates`, `Access`, `Members`, `Layout`, `Annotations`, `Values`, `Substitution`, `Generation`, `TypeQueries`, `TypeTransforms`, `Reflect`, `Query`, and private `Cache`; review the module interface, build wiring, tests, and documentation as cross-cutting surfaces.

For each partition inspect:

- API constraints, return categories, diagnostics, invalid/edge inputs, standard semantics, constant-evaluation validity, and exception provenance.
- Dangling references/views, copy/move-only callables, boolean proxy/reference identity, ownership of temporaries, overload ambiguity, integer bounds, iterator/range lawfulness, and relevant UB.
- Module ownership, import direction, export hygiene, unnecessary dependencies, duplicated helpers, unnecessary abstraction, and overly broad exposure.
- Reflection enumeration/cache key correctness, value stability, repeated work, storage promotion, compile-time and runtime allocations, materialization barriers, and template-instantiation pressure.
- Consistency and usefulness of the `Reflect<T>` facade and Query vocabulary without gratuitous duplication of the standard library.

**Outputs:** Partition-by-partition `Engineering-Audit.md`, candidate simplifications, risk-rated findings and test obligations.

**Exit:** Every partition has been reviewed with source-location evidence; discovered issues have dispositions. This is a source review, not a test-pass checkbox.

### 5.6.4 — Defect fixes and adversarial regression coverage

**Work:** Correct confirmed source defects in small related batches, adding a regression test for each observable issue. Cover positive, negative/compile-fail, constexpr/consteval, and integration behavior. Exercise composition and ownership paths: borrowed/owning sources, lvalues/rvalues, temporaries, nested and empty ranges, copy/move-only callables, bool references, `sortBy`, `flatMap`, `zip`, `skipWhile`, `takeWhile`, reflection access contexts, substitution, annotations, error propagation, and type transforms where contractually applicable.

**Outputs:** Fix patches, `Regression-Ledger.md`, minimized reproductions, updated expected semantics.

**Exit:** P0/P1 Miracle defects resolved; fixes covered by focused tests; negative tests fail for intended reasons, not incidental compiler errors.

### 5.6.5 — Measured performance optimization

**Work:** Profile compilation wall time, peak compiler RSS, constexpr evaluation where observable, template instantiation/diagnostic cost, Query scaling by chain depth/source cardinality, repeated cache/reflection access, module-build cost, and applicable runtime allocation/copy cost. Use identical source scenarios, compiler revision, flags, build conditions, and hardware; repeat trials, report medians plus variability and baseline details. Change code only when a bottleneck is measured and semantic preservation is demonstrable.

**Outputs:** `Performance-Report.md`, before/after raw results, reproducible commands, justification for each optimization.

**Exit:** No significant unexplained regression beyond measured noise; claimed improvements reproducible; readability/maintenance costs documented. There is no arbitrary percentage-improvement quota.

### 5.6.6 — Second-pass audit and integration

**Work:** Re-review the **final combined diff**, not merely individual checkpoint diffs, for accidental export changes, stale helpers, regressions, overly clever code, missing tests, stale docs, and interference between fixes. Exercise Miracle and Switch together; retain separation of patches. Verify provenance replay and compiler-failure isolation.

**Outputs:** Independent second-pass review ledger, consumer-compatibility report, corrected consolidated patch candidates.

**Exit:** No unresolved P0/P1 Miracle defects; all changed public APIs have documented outcomes; cross-cutting changes have been validated.

### 5.6.7 — Final verification and closure

**Work:** Execute the agreed complete matrix: Debug/Release; module/import consumer builds; warnings-as-errors; `clang-format`; `clang-tidy` where configured; normal test suite; ASan/UBSan as applicable; dedicated compile-fail suite; bounded compile-time stress and benchmark checks; Switch compatibility; `git diff --check`, replay/hash integrity, final standards inventory/export scan, and documentation consistency.

**Outputs:** Consolidated Miracle patch, separate Switch patch if needed, all reports, exact command/results ledger, closure statement.

**Exit:** Every acceptance gate in Section 8 is evidenced or the phase remains explicitly unclosed.

## 5. Defect classification and handling

| Priority | Definition | Closure policy |
|---|---|---|
| P0 | Undefined behavior, fundamental contract violation, or falsely classified normative feature | Must resolve |
| P1 | Observable semantic/lifetime defect, serious build breakage, significant proven resource regression | Must resolve |
| P2 | Maintainability defect, redundant abstraction, measurable inefficiency, inconsistent design | Fix if justified; explicit documented deferral possible |
| P3 | Cosmetic/style/docs issue | Batch and resolve or explicitly defer |

Every finding must include `id`, severity, file/location, reproduction/evidence, affected contract/standard clause, root cause, fix or rationale, tests, measurement if applicable, and final disposition. No silent semantic or public-API changes.

## 6. Toolchain divergence policy

- Accepted but unimplemented C++26 facilities are listed as **compiler-blocked**, not nonstandard.
- Experimental/nonstandard facilities exposed by the compiler remain **nonstandard**, even if compilation and tests succeed.
- Separate a **normative test** from a **compiler compatibility test**; do not weaken standards expectations to make the fork pass.
- Confirmed compiler failures require minimal reproducers, snapshot/revision and flags, expected versus actual behavior, and classification (ICE, rejection, wrong-code, etc.).
- No compiler-patching work in Phase 5.6 unless expressly authorized. Do not retain an abandoned API merely to avoid downstream cleanup.
- Compilation-only verification cannot establish semantics when the compiler and library share the same defect. Corroborate with wording, independent reasoning, and targeted tests.

## 7. Change discipline, benchmarks, and consumers

- Use one bounded checkpoint at a time; use related small change batches inside each checkpoint. Review and record results before advancing.
- Keep source of truth local/reviewed. Use GitHub only as a reconciliation signal, not an overriding authority.
- Keep Miracle and Switch diffs **separate**. Switch work is restricted to integration fallout caused by valid Miracle changes; preserve the separately reviewed Switch state and pins until explicitly addressed.
- Where a correction necessarily breaks former invalid API behavior, document the break instead of adding hidden shims.
- Preserve all accepted Phase 5.5 fixes and measurements unless a documented defect requires changing them.
- Never mark performance wins without raw repeatable measurements or claim bit-for-bit equivalent semantics without evidence.

## 8. Final acceptance gates

Phase 5.X may be declared **closed** only if:

1. The pinned normative C++26 inventory has 100% classification coverage, with cited wording and resolved normative ambiguities.
2. The Miracle mapping/coverage matrix has unambiguous denominators, verified semantics, and no compiler-derived acceptance criteria.
3. No abandoned experimental facility remains misrepresented as official C++26 or persists as an undocumented public dependency.
4. Every Phase 5.X Meta partition has a documented source/architecture/maintainability audit.
5. No unresolved P0/P1 **Miracle implementation** defect remains.
6. Every corrected observable bug and intentional public API change has focused regression coverage and documentation.
7. Query, cache, reference/lifetime, access, exception, and constant-evaluation guarantees from valid Phase 5.5 work remain intact.
8. Standard-supported behavior and explicitly compiler-blocked behavior are separately and honestly reported, with minimized compiler reproducers where needed.
9. Benchmark results show no significant unexplained regressions against the preserved baseline; optimizations have reproducible evidence.
10. The required Miracle build, formatting, lint, tests, sanitizers, and consumer integration checks pass within documented toolchain capabilities; blocked mandatory checks cannot be silently marked as passes.
11. The final combined source/export inventory, documentation, dependency graph, and replay integrity are reconciled without unexplained differences.
12. The final combined diff receives a second engineering review, and all deliverables contain enough evidence for another reviewer to reproduce the conclusion.

**Status vocabulary:** `Closed`, `Implementation-complete / external-compiler-blocked`, or `Open`. Only `Closed` satisfies all gates; an external blocker may justify a qualified implementation status but must not be reported as unconditional closure.

## 9. Required deliverables

- `Miracle-Phase-5.6.patch` — one consolidated patch against the **reviewed Phase 5.5 baseline**.
- `Miracle-Phase-5.6-Standards-Matrix.md` — authoritative C++26 inventory, signature/semantic audit, mappings and dispositions.
- `Miracle-Phase-5.6-Engineering-Audit.md` — partition-level findings, remediation, second-pass review evidence.
- `Miracle-Phase-5.6-Performance-Report.md` — repeatable baseline and resulting measurements.
- `Miracle-Phase-5.6-Regression-Ledger.md` — traceability from defects and API changes to tests.
- `Miracle-Phase-5.6-Compiler-Divergences.md` — compiler support gaps and minimized defects.
- `Miracle-Phase-5.6-Closure.md` — exact commands, outcomes, hashes, scope changes, unresolved limits and final status.
- `Switch-Phase-5.6-Compatibility.patch` **only if needed**, delivered separately from Miracle.

All document and patch names are proposed stable output names for this phase; no claim of implementation is implied.

## 10. First execution action

Start **5.6.0 only**: inspect and hash the user-retained Phase 5.5 patch, establish the reviewed application base, reproduce the current build/test baseline and capture source/export/test inventories. Then freeze the standard reference ledger against N5050 before making implementation edits. The plan is locked; the source has not yet been inspected or modified by this contract.

---

### Source anchors

- WG21 N5050 (C++26 final draft, 2026-06-01): https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/n5050.pdf
- WG21 N5051 editors' report: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/n5051.html
- cppreference C++26 `<meta>` reference: https://cppreference.com/cpp/header/meta
- P2996R13 (reflection wording history): https://open-std.org/JTC1/SC22/WG21/docs/papers/2025/p2996r13.html
- P4033R1 (future C++29 enum synthesis proposal, not adopted C++26): https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p4033r1.html
