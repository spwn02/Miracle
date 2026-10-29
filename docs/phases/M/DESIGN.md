# Miracle — Module Migration Phase
## Architecture & Execution Contract v1.0

| Field | Value |
|---|---|
| Phase | **M — Module Migration** (standalone phase, between Phase 5 and Phase 6) |
| Contract class | **Blueprint Contract** — foundational / high-risk architecture |
| Status | **Final design draft — prepared for author ratification and repository adoption** |
| Date | 2026-10-29 |
| Project | Miracle — C++26 Core Library |
| Reference baseline | Miracle v0.5.27 and current `master` repository cross-check; the author's designated local checkout prevails |
| Development process | Miracle Development Protocol v1.0 |
| Successor | Phase 6 — Validation and subsequent structural-programming work |

> **Authority.** The author has agreed to the architectural direction in design discussions. This document becomes a binding repository contract only after author acceptance and addition to the designated Miracle Git checkout. It neither changes the historical frozen contract archive nor claims that any source migration, test, or benchmark has already occurred.

---

## 1. Purpose and position in the roadmap

Phase M modernizes Miracle's module architecture **after Phase 5 and before Phase 6**. It is **not** Phase 5.7, a Meta Query extension, a Core Revisit, or a Validation implementation phase.

The existing `Miracle` named module currently owns numerous partitions for otherwise independent subsystems. Phase M converts those cohesive facilities into **independently importable named modules**, while retaining `Miracle` as the convenient umbrella. It reorganizes oversized compilation units into well-chosen interface partitions and implementation units, migrates source filename conventions from MSVC-influenced `.ixx`/`.cxx` to Clang-oriented `.cppm`/`.cpp`, and establishes **`Miracle.Format` as the dependency-independent foundation for consistent Miracle-authored compile-time diagnostics**.

The phase is a structural migration, not a library-semantic rewrite. Its acceptance is independent of Phase 6's implementation.

### 1.1 Goals

- Preserve a simple `import Miracle;` experience while enabling meaningful granular imports such as `import Miracle.Iter;`, `import Miracle.StaticString;`, and `import Miracle.Diagnostics;`.
- Represent architectural independence with named modules and explicit acyclic imports.
- Give large cohesive components their own directories, interface partitions, and implementation units.
- Establish `Miracle.Numeric` as the primitive numeric vocabulary, without making it a general arithmetic framework.
- Retain existing higher-level type aliases during transition, pending their planned Core Revisit.
- Create `Miracle.Format` for common compile-time-safe message storage, composition, and stable Miracle diagnostic-layout conventions.
- Standardize **Miracle-authored `static_assert` message bodies wherever technically feasible** without requiring lower-level components to import Diagnostics or Validation.
- Preserve the existing C++26-first semantics, including Meta Query, reflection access, caching, declaration-point-sensitive behavior, Iter, Feature, and runtime Diagnostics.
- Keep CMake, generated modules, consumers, developer tools, benchmarks, tests, packaging, and analysis inventories consistent with the new source conventions.
- Measure compilation effects rather than claiming that additional named modules automatically improve performance.

### 1.2 Explicit non-goals

- No implementation of Validation rules/policies, Shapes, Traits, Derive, or Phase 6 functionality.
- No Core Revisit of `Vec`, `String`, `StringView`, `Span`, `Option`, or other standard-backed vocabulary aliases.
- No general-purpose replacement for `std::format`, no general-purpose formatting language, no logging framework, and no redesigned general string/container library.
- No replacement of existing diagnostics with a new semantics engine; **formatting and rule evaluation are separate responsibilities**.
- No compulsory splitting into one library target per named module; no promise of faster builds without measurements.
- No compatibility shims that define the same exported class in two different named modules.
- No modification of frozen historical contract texts or retroactive approval claims.
- No CodeQL coverage claim based solely on filename changes.

A post-migration requirement from an **approved preceding phase** may justify a small, independently reviewed extension to `Miracle.Format`; otherwise broad Format work belongs to **Core Revisit**.

---

## 2. Architectural invariants

**M-ARCH-01 — Independent modules.** Cohesive public facilities own declarations in independently importable C++ named modules, not in unrelated partitions of `Miracle`.

**M-ARCH-02 — Umbrella only.** The primary `Miracle` interface is a lightweight aggregate of `export import` declarations for supported public components. It is not the owner of their definitions or implementation logic.

**M-ARCH-03 — Acyclic graph.** All production-module import dependencies form a DAG. A module must not import the umbrella; no foundational component may depend on Meta Validation or Traits to explain its own errors.

**M-ARCH-04 — Ownership preservation.** Each exported entity has one authoritative declaration/definition owner. `Miracle::` namespace spelling and existing user-facing semantics are preserved where applicable. A move between named modules may change module attachment, mangling, and ABI; binary compatibility is **not** guaranteed.

**M-ARCH-05 — Cohesion over count.** Named modules represent semantic/dependency boundaries; partitions and implementation units manage internal decomposition. Do not create tiny named modules or artificial file splits solely to reduce lines or increase module counts.

**M-ARCH-06 — Internal isolation.** Private helpers are not exported. Module implementation units and internal partitions are used where necessary; a public interface may not rely on a required import that CMake withholds from installed consumers.

**M-ARCH-07 — Compiler-work budget.** Module splitting must not introduce eager import/instantiation of previously lazy subsystems; verify initial and incremental compilation time, memory, and BMI costs.

**M-ARCH-08 — Single semantics.** Moving a facility changes its module ownership, not its behavior. Any intentional API or semantic change requires separately approved scope.

**M-ARCH-09 — Source conventions.** Source-controlled C++ module interfaces and importable partitions use `.cppm`; conventional and module implementation units use `.cpp`. Generated module interface templates use `.cppm.in` and produce `.cppm` outputs. Exceptions require explicit documentation and verified toolchain need.

**M-ARCH-10 — Diagnostic consistency.** Miracle-authored compile-time hard-diagnostic **message bodies** follow one presentation contract wherever language and instantiation constraints allow. Compiler-provided frames, location displays, overload notes, and external constraints remain compiler-controlled.

---

## 3. Public modules and ownership mapping

### 3.1 Required module inventory

| Target named module | Existing origin / role | Required migration rule |
|---|---|---|
| `Miracle` | `Miracle.ixx` umbrella | Pure primary re-export interface; no feature ownership |
| `Miracle.Numeric` | Numeric aliases currently in `Miracle:Types` | Own `u8`/`i8` families, `f32`/`f64`, `usize`/`isize`; standard library only |
| `Miracle.Types` | Remaining `Miracle:Types` aliases | Transitional owner of existing standard-backed aliases; may re-export Numeric; Core Revisit decides later redesign |
| `Miracle.Format` | New foundational capability | Own compile-time formatting primitives and canonical diagnostic-layout conventions; no Miracle-module imports |
| `Miracle.StaticString` | `Miracle:StaticString` | Own existing StaticString public API and formatting operations; may consume Format |
| `Miracle.Range` | `Miracle:Range` | Own Range semantics and public API |
| `Miracle.Iter` | `Miracle:Iter` | Own Iter source, adaptors, views, terminals; decompose internally |
| `Miracle.Diagnostics` | `Miracle:Diagnostic` | Own diagnostic domains, annotation vocabulary, reports and runtime presentation; consume Format |
| `Miracle.Feature` | `Miracle:Feature` | Own Feature descriptors/resolution; preserve rich C++26 diagnostic behavior |
| `Miracle.Error` | `Miracle:Error` | Own Error/Result-facing facilities; depend on Diagnostics as needed |
| `Miracle.Panic` | `Miracle:Panic` | Own panic facilities; import Error/Diagnostics as needed |
| `Miracle.Fs` | `Miracle:Fs` | Own filesystem utilities; retain Error dependencies |
| `Miracle.Concepts` | `Miracle:Concepts` | Own exported concepts and their actual dependencies |
| `Miracle.Memory` | `Miracle:Memory` | Own memory vocabulary and platform implementation units |
| `Miracle.Profiling` | `Miracle:Profiling` | Own profiling; retain independently justified dependencies |
| `Miracle.Bitflags` | `src/public/meta/Bitflags.ixx`, currently `Miracle:Bitflags` | Move out of `meta/` source folder; remain a separate cohesive facility importing Meta if needed |
| `Miracle.Debug` | `src/public/meta/Debug.ixx`, currently `Miracle:Debug` | Move to own directory; retain Meta/StaticString/Concepts dependencies as justified |
| `Miracle.Build` | Generated `Miracle:Build` | Preserve immutable build facts, move to `.cppm` template/output |
| `Miracle.Capabilities` | Generated `Miracle:Capabilities` | Preserve toolchain capability facts; move template/output |
| `Miracle.FeatureConfiguration` | Generated `Miracle:FeatureConfiguration` | Preserve build-selectable feature configuration; move template/output |
| `Miracle.Meta` | Existing separate named module and reflection partitions | Preserve the public façade, cache semantics, and access/reflection behavior; reorganize only where worthwhile |

**Inventory control:** The table reflects the inspected remote baseline, not a license to drop newly added local components. At migration start, compare the table with the **authoritative local source tree, CMake feature registrations, and generated source inventory**, and add any missing facility without altering its semantics. The exact decomposition of `Iter`, `Feature`, `Meta.Query`, and similar large units is finalized from actual import/visibility constraints, not line count alone.

### 3.2 Naming and symbol rules

- Public named modules use `Miracle.<Component>` with conventional PascalCase component names; primary umbrella stays exactly `Miracle`.
- Existing user-visible declarations generally remain in namespace `Miracle` and their existing nested namespaces; **module names do not force namespace renaming**.
- Importable interface partitions use `export module Miracle.Iter:Sources;` or similarly descriptive partition names; ordinary implementation units use `module Miracle.Iter;`.
- Partition names with periods are flat names, **not partitions of partitions**.
- Do not infer a public module path from an implementation directory; directories are organization, not semantic namespaces.
- Preserve established API names, e.g. `Miracle::StaticString`, `Miracle::Iter`, `Miracle::Diagnostic`, `Miracle::feature`, unless a separate explicit API decision says otherwise.

### 3.3 Umbrella contract

Illustrative shape (the final list must match all accepted public components):

```cpp
export module Miracle;

export import Miracle.Numeric;
export import Miracle.Types;
export import Miracle.Format;
export import Miracle.StaticString;
export import Miracle.Range;
export import Miracle.Iter;
export import Miracle.Diagnostics;
export import Miracle.Feature;
export import Miracle.Error;
export import Miracle.Panic;
export import Miracle.Fs;
export import Miracle.Meta;
// ... other accepted public modules and generated components
```

A consumer may use `import Miracle;` or import each cohesive public module directly. One does **not** imply separate binaries or CMake targets.

---

## 4. Dependency rules and layering

### 4.1 Required directions

```text
C++26 standard library
    ├── Miracle.Format                 [no Miracle imports]
    ├── Miracle.Numeric                [no Miracle imports]
    │      └── Miracle.Types           [transitional aliases]
    └── other independent generated facts as appropriate

Miracle.StaticString  ──► Format / Numeric (as needed)
Miracle.Range         ──► Numeric / standard library (as needed)
Miracle.Diagnostics   ──► Format / StaticString / Numeric or Types (as needed)
Miracle.Iter          ──► Range / Format / low-level vocabulary (as needed)
Miracle.Feature       ──► StaticString / Format / Capabilities / low-level vocabulary
Miracle.Error         ──► Diagnostics / low-level vocabulary
Miracle.Panic         ──► Error / Diagnostics
Miracle.Fs            ──► Error / low-level vocabulary
Miracle.Meta          ──► permitted foundations; must remain independent of Traits
Miracle.Bitflags      ──► Meta if required
Miracle.Debug         ──► Meta / StaticString / Concepts as required
Miracle.Profiling     ──► Debug / low-level vocabulary if justified

Miracle               ──► export-imports accepted public modules
```

Arrows signify **imports by the source on the left**, not a requirement to introduce every optional edge. This is a target direction guide, not an unverified claim that every current source already satisfies it.

### 4.2 Strict prohibitions

- `Miracle.Format` imports **no** `Miracle.*` module, including Numeric and StaticString.
- `Miracle.StaticString` does not import Diagnostics, Meta, Validation, Shapes, or Traits.
- `Miracle.Diagnostics` does not import Validation or the `Miracle` umbrella. If reflecting diagnostic-code enums remains necessary, use C++26 `std::meta` directly or an approved acyclic lower-level adapter; do not create a cycle for the sake of an abstraction.
- `Miracle.Iter` does not import Validation/Traits merely for its generic algorithms or diagnostic messages.
- `Miracle.Meta`'s reflection substrate remains independent of Validation consumers; partition imports must not create Query ↔ Validation or Meta ↔ Diagnostics cycles.
- New containers in Core Revisit may not be made dependencies of their own diagnostic/validation bootstrap path.

A prospective extra dependency must be analyzed by **transitive graph reachability**, not merely by checking that two directly importing modules differ.

### 4.3 Internal partitions and CMake visibility

- A named module may own multiple interface partitions and multiple implementation units; it cannot own true nested partitions.
- Interface partitions re-exported by a public primary interface must remain available to downstream compilation and installation.
- CMake `PRIVATE` visibility is **not** a way to hide a module interface that a `PUBLIC` interface imports; CMake rejects such dependencies or creates unusable package boundaries.
- Keep implementation details out of exported declarations; prefer ordinary module implementation units for private runtime code and appropriate internal partitions for legitimately shared module-local compile-time machinery.
- Verify Clang and CMake treatment of non-exported partitions through a **build-and-install consumer test** before relying on a particular `PRIVATE` file-set arrangement.
- No global import registry, implicit side-effect module initialization, or generated all-module source includes.

---

## 5. Target source organization

**Representative target tree**; ellipses indicate existing or independently justified units, not automatic authorization to add empty partitions:

```text
src/
├── public/
│   ├── Miracle.cppm
│   ├── numeric/
│   │   └── Numeric.cppm
│   ├── types/
│   │   └── Types.cppm
│   ├── format/
│   │   ├── Format.cppm
│   │   ├── Message.cppm          # only if independently useful
│   │   └── Layout.cppm           # only if independently useful
│   ├── static_string/
│   │   ├── StaticString.cppm
│   │   └── ...
│   ├── diagnostics/
│   │   ├── Diagnostics.cppm
│   │   ├── Vocabulary.cppm
│   │   └── Codes.cppm
│   ├── range/
│   │   └── Range.cppm
│   ├── iter/
│   │   ├── Iter.cppm
│   │   ├── Sources.cppm
│   │   ├── Adaptors.cppm
│   │   ├── Views.cppm
│   │   ├── Consumers.cppm
│   │   └── ...
│   ├── feature/
│   │   ├── Feature.cppm
│   │   └── ...
│   ├── error/
│   │   └── Error.cppm
│   ├── panic/
│   │   └── Panic.cppm
│   ├── fs/
│   │   └── Fs.cppm
│   ├── concepts/
│   │   └── Concepts.cppm
│   ├── debug/
│   │   └── Debug.cppm
│   ├── bitflags/
│   │   └── Bitflags.cppm
│   ├── memory/
│   │   └── Memory.cppm
│   ├── profiling/
│   │   └── Profiling.cppm
│   ├── build/
│   │   ├── Build.cppm.in
│   │   ├── Capabilities.cppm.in
│   │   └── FeatureConfiguration.cppm.in
│   └── meta/
│       ├── Meta.cppm
│       ├── Types.cppm
│       ├── Cache.cppm
│       ├── Reflect.cppm
│       ├── Query.cppm
│       └── ...                  # preserve other current Meta units
└── private/
    ├── diagnostics/
    │   └── Diagnostics.cpp
    ├── error/
    │   └── Error.cpp
    ├── panic/
    │   └── Panic.cpp
    ├── fs/
    │   └── Fs.cpp
    ├── iter/
    │   └── ...
    ├── feature/
    │   └── ...
    ├── profiling/
    │   └── Profiling.cpp
    └── memory/
        ├── linux/
        ├── windows/
        └── unsupported/
```

Generated `.cppm` outputs belong in the **build tree**, not alongside source-controlled `.cppm.in` templates. `Validation.cppm`, Shapes, Traits, and Derive are **not deliverables of Phase M**; their future placement can be reserved but they must not be implemented in this migration.

**Decomposition constraints:** Keep source organization consistent with dependency direction. Splitting a 3,700-line `Iter` unit or a 2,191-line Meta Query unit is warranted only where separable interfaces and implementation pieces exist; preserve lazy behavior and avoid deep partition chains. Large template bodies may legitimately remain in module interface units where users need their definitions at instantiation. Do not relocate templates into `.cpp` and silently break general instantiation.

---

## 6. `Miracle.Format` — foundational formatting contract

### 6.1 Purpose and hard dependency boundary

`Miracle.Format` is a **low-level, dependency-independent** formatting foundation. Its initial purpose is to provide reliable compile-time message construction and Miracle's canonical diagnostic presentation conventions. The name deliberately permits later, broader formatting capabilities, but **their implementation is deferred to Core Revisit unless an approved earlier phase genuinely requires them**.

It imports the standard library as needed and **no other Miracle named module**. It is not an alias for `Miracle.Diagnostics`, a replacement for `StaticString::staticFormat`, a rules engine, or a general text templating framework.

### 6.2 Phase M minimum responsibilities

1. Provide a constexpr/consteval-safe, fixed-capacity message representation supporting C++26 `static_assert` requirements (`size()` and `data()` with valid constant-expression character storage).
2. Provide bounded, deterministic message assembly (append, indentation, labels, and necessary simple value rendering) without depending on Miracle containers.
3. Define one **canonical, human-readable Miracle diagnostic layout** shared by compile-time and, where reasonably integrable, runtime diagnostics.
4. Permit lower-level modules to attach domain-owned code identities, summaries, context, details, notes, and help without importing Diagnostics.
5. Ensure capacity failures are explicit and do not silently corrupt or truncate the essential code/summary; choose exact compile-time capacity when feasible and predictable bounded fallback otherwise.
6. Avoid expensive diagnostic construction in successful template-instantiation paths where compile-time branch elimination can prevent it.
7. Maintain deterministic output order and stable layout for golden-message tests, accounting for compiler-specific presentation outside the message body.

The exact factory spellings such as `format::error(...).context(...).note(...).help(...).build()` are **illustrative**, not an additional unapproved public API freeze. The implementation may choose a small builder, direct formatting functions, or other simple mechanisms satisfying these semantics. Do not turn an illustrative fluent API into mandatory complexity.

### 6.3 Canonical authored-message grammar

Target illustrative message body:

```text
error[ITR004]: callable cannot accept sequence element
  --> Iter::map

  callable: MyCallable
  element:  std::tuple<int, float>

  = note: neither direct invocation nor supported decomposition succeeded
  = help: provide a compatible callable
```

Required common elements:

- Lead with severity, a stable domain-scoped code **when assigned**, and a concise summary: `error[ABC001]: ...`; preserve numeric padding policy.
- Provide contextual location/operation using consistent indentation when available. This is user-authored context, **not** a portable replacement for the compiler's source location.
- Present relevant expected/actual or named details deterministically; avoid arbitrary template backtrace dumps.
- Render explanatory notes and actionable help using consistent prefixes, spacing, and line breaks.
- Omit optional sections cleanly. Do not invent reflection-derived detail when unavailable.
- Use consistent format for hard assertions in StaticString, Iter, Feature, Diagnostics, and other modules wherever technically possible.

The formatter owns **presentation**, not the semantic determination of failure. Error codes belong to the reporting domain/module; Format must not import those modules, infer diagnostic severity from template constraints, or keep a global code registry. An optional direct `Format`-level light code value or normalized code descriptor is appropriate; Diagnostics can adapt its existing enum/annotation domains to that representation.

### 6.4 Source-specific diagnostic providers

Modules may define non-exported providers or small message-building helpers that gather domain context and delegate presentation to Format. Examples include failed Iter invocation paths, invalid StaticString formatting grammar, or Feature resolution cycles.

- Domain providers may inspect source-specific conditions and provide information; **Format never calls them directly**.
- Simple invariants need not allocate a dedicated provider type.
- Expensive reflection-generated detail is formed on genuinely failing compile-time branches where possible.
- Existing Feature compile-time analysis and `report.message` semantics remain intact; migration may normalize layout without changing failure conditions or resolution order.
- Meta operations that currently throw recoverable `std::meta::exception` must **not** be indiscriminately converted to `static_assert` failures.

### 6.5 Bootstrap exceptions

Even Format must enforce its own invariants before richer message infrastructure is usable. A small set of straightforward literal `static_assert` messages or ordinary constraints is allowed for such bootstrap cases. The messages should follow the same style wherever practical, but **the contract must not create a Format → Format dependency or require all compile-time errors to instantiate the formatter**.

Concept subsumption diagnostics, language-generated errors, third-party template failures, and compiler-instantiation traces cannot be fully controlled by Miracle. Acceptance targets the **Miracle-authored message body**, not byte-for-byte identical compiler output.

### 6.6 Deferred Format features

Defer to Core Revisit: general formatting mini-language, comprehensive argument formatting, allocators and runtime sink frameworks, container/string replacement integration, localization, streaming/text serialization, and replacing `std::format`. A narrowly necessary capability for a preceding approved phase may be added only with an explicit scope amendment and acceptance tests.

---

## 7. `Miracle.Diagnostics` and compile-time interoperability

- Preserve existing domain annotations (`diagnostics`, `diagnostics::prefix(...)`, `diagnostics::message(...)`, `diagnostics::align(...)`), `DiagnosticCode` semantics, severity, code descriptors, diagnostic rendering, and Error/Feature integrations.
- Extract the existing `Miracle:Diagnostic` partition into the standalone named module **`Miracle.Diagnostics`**. Retain public `Miracle::Diagnostic` and other existing declaration spellings; the plural module name does not require renaming the class.
- `Miracle.Diagnostics` imports `Miracle.Format` and other genuine foundations as required. Avoid importing `Miracle.Meta` or higher-level Validation solely to interpret its own diagnostic-code annotations; standard C++26 reflection can remain part of Diagnostics' bootstrap if appropriate.
- The runtime diagnostic renderer may use the same layout primitives or a tested equivalent sink, but its runtime storage/rendering behavior must not regress. The objective is one canonical layout, not forcing every runtime operation through a compile-time container.
- In the future, Phase 6 Validation can own `ValidationCode` as an appropriate dedicated diagnostic code domain and use Diagnostics/Format **downward**. Phase M does **not** implement `ValidationCode`, validation policies, or rules.
- Diagnostic-domain codes and message content must have a single authority. Do not maintain mirrored enum values or duplicated formatting grammars merely to accommodate module boundaries.

---

## 8. Source suffixes, build system, and developer tooling

### 8.1 File migrations

- All source-controlled `.ixx` **module interface units** migrate to `.cppm`; ordinary `.cxx` **C++ implementation sources, tests, examples, benchmarks, and probes** migrate to `.cpp` where applicable.
- All `.ixx.in` generated-module templates migrate to `.cppm.in`; configured outputs end in `.cppm`.
- Keep actual syntax consistent with unit kind: `export module ...;` for an interface, `module ...;` for an implementation; internal partitions may use their appropriate importable `.cppm` convention when required by the reference toolchain.
- Treat **file extension change and module ownership change as separate reviewable edits**, even if eventually integrated into one milestone. A source suffix alone does not confer new semantics or faster compilation.
- Use Git-aware moves to preserve provenance where possible; verify rename/split attribution.

### 8.2 CMake and installation

- Retain the `Miracle` static-library target and exported alias **`Miracle::Miracle`** initially.
- Retain `target_sources(... FILE_SET TYPE CXX_MODULES ...)` for public module interfaces; update paths and add extracted named modules and the new Format foundation.
- Keep generated Build, Capabilities, and FeatureConfiguration modules functioning under the new names; preserve feature gating, public/private link dependencies, and build identity semantics.
- Export/install every interface needed by installed consumers. Verify CMake's module collator/visibility, BMI mappings, and all transitive imports from the installed package.
- Do not create independent CMake targets or binary artifacts per module by default. Such a change needs separate motivation, build/performance evidence, and approval.
- Preserve the supported consumption modes: source checkout/subdirectory, FetchContent, and installed `find_package`.
- Maintain documented `import std` and CMake compiler requirements. The pinned C++26 reference compiler and matching libc++ remain the authority for `master`; GCC compatibility is evaluated independently under its existing policy.

### 8.3 Comprehensive path migration

Audit and update all hardcoded `.ixx`/`.cxx` assumptions in:

- Root and nested CMake files, presets, install/export rules, generated templates, and feature-registration `SOURCES` values.
- `cmake/Capabilities.cmake` compiler probes and generated build facts.
- Self-tests, examples, benchmark generators and scripts, including generated source suffixes and parsed compilation-database paths.
- Docker/editor/Neovim/clangd smoke tests and filetype detection.
- CI/build scripts, code-format/lint tooling, coverage inventory scripts, workflow globs, and documentation examples.
- Source-inventory scripts that classify `.cxx` as implementation units: **include `.cpp`** and ensure `.cppm` remains in the correct module category.
- Any branch-specific compatibility scripts; only deliberately retained historical document references may continue mentioning old paths, with clear provenance.

**Known repository constraints:** The observed CMake baseline uses CMake 4.4 and Ninja module scanning; its generated module files include `Build.ixx.in`, `Capabilities.ixx.in`, and `FeatureConfiguration.ixx.in`. `scripts/ci/audit-codeql-coverage.py` currently recognizes `{.cxx,.ixx,.cppm}` but must also recognize `.cpp` and update source classifications. `scripts/ci/verify-nvim-lsp.lua` currently expects `src/private/Error.cxx` and must follow the migrated path. Existing `.ixx` references occur in feature documentation and CMake tests. The actual local tree may contain further references.

### 8.4 Static-analysis evidence

The documented 2026-10-21 CodeQL export reported **0/33 `.ixx` module interfaces extracted**, despite extraction of ordinary `.cxx` files. This is a **historical measurement of the old baseline**, not a forecast for `.cppm`.

- Preserve default CodeQL and the accepted known-limitations record unless explicitly changed through its own review.
- Do not claim that `.cppm` renaming fixes C++26 module/reflection analysis.
- Reconcile post-migration file inventory counts and provide a new extraction report *if available*, distinguishing recognized, extracted, unextracted, and unreported files.
- A coverage checker must not silently omit `.cpp` or new module partitions. Do not turn unsupported module extraction into a required CI gate without valid tool support.

---

## 9. Migration sequencing and checkpoints

Every milestone follows the Development Protocol's author-owned implementation, reviewed changes, and evidence-backed acceptance. Intermediate patches may temporarily use both naming conventions **only if the complete build remains correct**; no dual definition of the same exported entity is allowed.

| Milestone | Scope | Exit evidence |
|---|---|---|
| **M.1 — Baseline & graph** | Record authoritative local source/target inventory, current import graph, generated modules, compiler/CI baseline and performance metrics; reconcile frozen contracts | Approved module-ownership map, acyclic target graph, reproducible baseline |
| **M.2 — Extension/toolchain migration** | `.ixx` → `.cppm`, `.cxx` → `.cpp`, templates/probes/docs/scripts/tests/editor integration; no module ownership changes yet | All reference builds and consumer paths pass with new suffixes; no missing source inventory |
| **M.3 — Foundation extraction** | Extract Numeric, transitional Types, Format, StaticString and foundational generated modules; establish no-cycle boundaries | Each component independently importable and testable; Format has zero Miracle imports |
| **M.4 — Diagnostic infrastructure** | Extract Diagnostics and integrate canonical message layout with current domain/renderer semantics | Existing diagnostics behavior preserved; compile-time message snapshots pass |
| **M.5 — Remaining component extraction** | Range, Iter, Feature, Error, Panic, Fs, Concepts, Debug, Bitflags, Memory, Profiling; move runtime implementation ownership | Granular-import builds pass; no duplicate entities or old `module Miracle;` owners for migrated implementations |
| **M.6 — Source decomposition** | Split unusually large cohesive units only where interfaces and internal ownership warrant it, including Iter and potentially Feature / Meta Query | Template instantiation from consumers works; no new cycles or material compile regressions |
| **M.7 — Umbrella & packaging** | Rebuild `Miracle.cppm` as public re-export interface; update installed package and build-feature configurations | `import Miracle;` source compatibility and all direct imports work under all supported consumption methods |
| **M.8 — Acceptance & closeout** | Full regression, diagnostic snapshots, benchmarks, coverage-inventory audit, documentation and explicit author acceptance | Acceptance record with results, caveats, and source revision; no unsupported completion claims |

Milestone numbering is a proposed execution order and may be adjusted by the author in the accepted phase roadmap. Diagnostic normalization may be implemented incrementally from M.3 through M.6; its full acceptance is M.8. Do not defer all diagnostic cleanup until after modules are reorganized if doing so would obstruct compilation or review.

---

## 10. Verification and acceptance matrix

### 10.1 Functional and semantic regression

- All existing public APIs compile through `import Miracle;` and the appropriate granular named imports; compare public symbols and representative sample code at source level.
- All existing Iter, Range, StaticString, Feature, Diagnostics, Error/Panic/Fs, Bitflags/Debug/Profiling/Memory, and Meta tests pass against the baseline with no behavior changes.
- Meta Query correctness, value-IR, lazy semantics, failure behavior, caching categories, access filtering, reflection identities and declaration-point-sensitive snapshot tests remain unchanged.
- Generated build facts and feature-resolution results remain identical for equivalent build configurations.
- No incidental new hard errors arise from eager import of unrelated modules or from moving template bodies into inaccessible `.cpp` units.

### 10.2 Compile-time diagnostic snapshots

Test at least these intentionally ill-formed cases, checking Miracle-authored message bodies and preserving useful compiler failure locations:

1. `StaticString` incompatible character type.
2. `staticFormat` malformed braces, unsupported format argument, and placeholder-count mismatch where the API reaches a hard diagnostic.
3. `Iter` direct/tuple/aggregate adaptive invocation failures, including relevant type and operation context.
4. `Feature` duplicate configuration and dependency/capability failures, including existing rich report behavior.
5. Invalid diagnostic-code annotation/domain usages where a deliberate static assertion is feasible.
6. A complex, multiline diagnostic with context/details/notes/help and a deliberately long type name.
7. Positive template cases proving diagnostic builders are not needlessly instantiated.

Snapshots normalize compiler-owned wrappers; only Miracle-authored message grammar is required to be stable. Tests must not assert a specific implementation-defined reflection display spelling across different compilers unless the test is toolchain-pinned.

### 10.3 Build, installation and tooling

- Clean and incremental pinned-reference Clang/C++26 builds; Debug and Release; warnings-as-errors as configured.
- Source checkout/`add_subdirectory`, FetchContent, and installed `find_package` consumer configurations.
- `import Miracle;` and direct `import Miracle.Numeric;`, `Miracle.Format;`, `Miracle.StaticString;`, `Miracle.Iter;`, `Miracle.Diagnostics;`, `Miracle.Meta;` consumer smoke tests; expand to all exported modules.
- Generated build-fact and build-feature option matrices, including disabled optional features.
- Module scanning and import dependency collation with Ninja; installed/exported CMake targets and module metadata.
- `clang-format`, configured static checks, Neovim/clangd compilation database smoke tests, benchmarks and script self-tests.
- Source inventory/CodeQL audit detects all `.cppm` and `.cpp` files without claiming unsupported C++26 analysis.
- Separate GCC compatibility assessment per current branch/compiler policy; explicitly report unsupported compiler features rather than silently weaken the `master` contract.

### 10.4 Compilation-performance budget

Record reproducible old/new measurements of:

- Full cold build wall time and critical path.
- Rebuild after editing an implementation unit of Iter, Diagnostics, and one foundational component.
- Rebuild after a public interface edit in the same three classes.
- Compiler peak memory and generated BMI count/size.
- Compile time of a minimal umbrella-import consumer versus granular `Numeric`, `Iter`, `Diagnostics`, and `Meta` consumers.
- Template/constant-evaluation stress benchmarks, including existing Meta Query stress cases.

Use the same toolchain, options, generator, hardware, warm/cold cache characterization, and repeated measurements where possible. **No arbitrary percentage improvement is assumed.** A material regression requires investigation, explanation, and author sign-off or remediation before Phase M acceptance.

### 10.5 Required migration invariants at closeout

1. The import graph is acyclic and documented.
2. `Miracle` is a re-exporting umbrella with no independently owned feature APIs.
3. `Miracle.Format` directly imports no Miracle named module and does not implement generalized formatting beyond its approved minimal scope.
4. Each accepted public facility is independently importable and available to installed consumers.
5. `.cppm`/`.cpp` migration covers tracked sources, generated outputs, tests, tools, package metadata, and analysis inventories.
6. Existing public semantics and Meta/Feature/Iter diagnostic behavior are not regressed.
7. Miracle-authored hard compile-time errors follow the common layout where feasible, and limitations/exceptions are documented.
8. No duplicate module ownership, hidden dependency cycles, or broken template instantiation boundaries remain.
9. Build/test/benchmark evidence is attached to the accepted source revision.
10. The author explicitly accepts the phase; Phase 6 starts only when its necessary foundations are verified.

---

## 11. Compatibility, risks, and change control

**Source compatibility:** Preserve established namespaces, public type/function names and observable behavior for `import Miracle;` clients. Granular imports are new. Previously used partition import syntax is not promised as public downstream API.

**Binary/build compatibility:** Moving declarations across named-module ownership changes is potentially ABI- and BMI-incompatible. Consumers may need a full clean rebuild; old `.pcm`/BMI and install outputs must not coexist silently with new ownership. No binary-compatibility guarantee is made for this refactor.

**Primary risks:** transitive import cycles, declaration ownership/ODR errors, template definitions unavailable at point of instantiation, generated-source omissions, public/private CMake module-file-set mistakes, compiler-specific module scanning, incorrect diagnosis of CodeQL coverage, and build graph performance regressions.

**Mitigations:** small independently reviewable milestones, diagrammed import graph, direct granular-import and installed-package tests, automated tracked-source inventories, compiler diagnostics snapshots, clean-build acceptance, and measured baseline comparison.

**Change control:** Any semantic API adjustment, Format scope expansion, new library-target split, public module identity rename beyond the accepted inventory, compiler requirement change, or deviation from frozen Meta/Iter/Feature contracts requires an explicit reviewed amendment. Historical frozen contracts remain preserved; repository adoption of this phase records how architecture migrates without silently rewriting them.

---

## 12. Completion statement and handoff

Phase M may be marked **Accepted / Complete** only when the author has reviewed an evidence-backed acceptance report referring to the precise code revision, and every blocking acceptance criterion in §10 is met or an expressly documented exception is approved.

**Phase 6 handoff condition:** `Miracle.Meta` can add the Validation implementation against an independently importable, acyclic foundation, including Format and Diagnostics, without moving fundamental modules again or introducing bootstrap cycles.

**Core Revisit handoff condition:** `Miracle.Format` remains a deliberately minimal formatting foundation. Broad text formatting, public generic formatting extensions, runtime sink ecosystems, and integration with redesigned string/container types remain deferred unless an approved preceding phase establishes an actual dependency.

> **Contract status:** Final design draft, prepared for author ratification. No repository write, migration implementation, successful test run, performance claim, or release tag is implied by this document.

---

## References and provenance

- `docs/DEVELOPMENT_PROTOCOL.md` — Development Protocol v1.0 and contract authority.
- `docs/architecture.md` — existing public names, C++26 policy, package identity and source/ABI compatibility rules.
- `docs/contracts/index.md` and `docs/contracts/miracle-foundation-v1.0.md` — frozen historical architecture/API obligations; not rewritten here.
- `CMakeLists.txt`, `cmake/Capabilities.cmake`, `cmake/Features.cmake` — current generated modules, source registration, file sets and installation.
- `src/public/Miracle.ixx`, `src/public/Types.ixx`, `src/public/Iter.ixx`, `src/public/StaticString.ixx`, `src/public/Diagnostic.ixx`, `src/public/Feature.ixx`, `src/public/meta/*.ixx`, `src/private/*.cxx` — remote implementation cross-check.
- `docs/infrastructure/G4-PROGRESS.md`, `docs/infrastructure/G4-CODEQL-COVERAGE-INVENTORY.md`, `scripts/ci/audit-codeql-coverage.py`, `scripts/ci/verify-nvim-lsp.lua` — known suffix/tooling migration implications and historical CodeQL limitation.
- Clang Standard C++ Modules: <https://clang.llvm.org/docs/StandardCPlusPlusModules.html>.
- CMake C++ Modules guide: <https://cmake.org/cmake/help/latest/manual/cmake-cxxmodules.7.html>.
- C++26 user-generated static assertions: <https://en.cppreference.com/w/cpp/language/static_assert.html>.

The repository references were checked against the connected remote `master` in preparation for this draft. The author's designated local tree remains authoritative where it differs.
