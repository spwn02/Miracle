# Miracle — Phase M: Module Migration
## Technical Implementation Blueprint v1.1

| Field | Value |
|---|---|
| Document | **Author-owned technical implementation guide** (supersedes the superficial guide v1.0) |
| Phase | **M — Module Migration**, standalone between Phase 5 and Phase 6 |
| Normative design | `docs/phases/M/DESIGN.md`, *Architecture & Execution Contract v1.0* |
| Process | `docs/DEVELOPMENT_PROTOCOL.md`, *Miracle Development Protocol v1.0* |
| Implementation | **Project author**; partner supplies design guidance and performs a batched final review |
| Source baseline inspected | `spwn02/Miracle@469083ed1b30e3770667e1fe35473e9f96ed1e7a` (remote `master`) |
| Reference compiler | `cxx26-2026.10.27`, source `b56ae4e61cf1f46495f0fb08f2b9fc270aad162c`, with matching libc++ |
| Destination | `docs/phases/M/IMPLEMENTATION.md` (recommended; author decides when to commit) |
| Status | **Technical guidance; not implementation, testing, ratification, or milestone acceptance** |

> **Read this first.** This document deliberately provides concrete declarations, dependency decisions, algorithms, partition maps, CMake changes, compiler experiments, and test scaffolds. Its samples are *implementation sketches*, not a hidden production patch or silently frozen APIs. Use **[R] Required**, **[P] Preferred implementation**, **[E] Example**, **[D] Deferred** throughout. Where this guide differs from the approved Design Contract, the contract prevails; record any material deviation and seek an explicit amendment under Protocol §6.1. The authoritative **local** source tree takes precedence over this inspected remote baseline.

## 0. The deliverable and the working agreement

The goal is **not** “convert every partition to a new module” by search-and-replace. It is to produce a DAG of independently importable components, preserve existing C++26 behavior, make `Miracle` a pure umbrella, and standardize **Miracle-authored compiler message bodies** through a minimal, self-contained `Miracle.Format`. The phase must remain distinct from future Validation and Core Revisit.

**Review model.** The author implements M.1–M.8 in one uninterrupted sequence with a commit boundary for each milestone. The partner conducts **one batched final review that still issues eight separate milestone outcomes**, per Protocol §8. Interim compiler/test checks are the author's local development gates; *formal* acceptance is deferred, not waived. A design conflict requiring a contract amendment is the only reason to pause the relevant implementation decision before continuing.

**Source of authority:** Phase M Contract §§2–11; Protocol §§2, 5–9, 11, 14. A commit is evidence of an implementation step, not proof of a passing build. No published release or tag is implied.

### 0.1 Exact non-goals

[D] Do **not** implement `Miracle.Meta:Validation`, `ValidationCode`, Shapes, Traits, Derive, new `Vec`/`String`/`Span` storage, a general format-string language, a logging framework, `std::format` replacement, or separate binary/CMake targets per module. Do not move Query's IR semantics or the existing Meta snapshot observation points. Any *narrow* additional Format capability required by an approved earlier phase needs a scoped amendment.

### 0.2 File layout to implement

```text
src/
├── public/
│   ├── Miracle.cppm                        # umbrella, only export-imports
│   ├── numeric/Numeric.cppm
│   ├── types/Types.cppm
│   ├── format/Format.cppm                   # initially one file; split only if needed
│   ├── static_string/StaticString.cppm
│   ├── range/Range.cppm
│   ├── iter/
│   │   ├── Iter.cppm                       # façade, re-export selected partitions
│   │   ├── Detail.cppm                     # recommended shared template machinery
│   │   ├── Sources.cppm                    # source/view implementations
│   │   ├── Adaptors.cppm                   # view adaptors, see M.6 placement map
│   │   ├── Peekable.cppm                   # optional; depend on tested partition graph
│   │   ├── Pipeline.cppm                   # main Iter template + terminals
│   │   └── Parallel.cppm                   # optional, only if no import cycle
│   ├── diagnostics/
│   │   ├── Diagnostics.cppm
│   │   ├── Codes.cppm                      # optional coherent partition
│   │   └── Model.cppm                      # optional coherent partition
│   ├── feature/
│   │   ├── Feature.cppm
│   │   ├── Descriptor.cppm                 # optional
│   │   └── Resolution.cppm                 # optional
│   ├── error/Error.cppm
│   ├── panic/Panic.cppm
│   ├── fs/Fs.cppm
│   ├── concepts/Concepts.cppm
│   ├── memory/Memory.cppm
│   ├── profiling/Profiling.cppm
│   ├── bitflags/Bitflags.cppm
│   ├── debug/Debug.cppm
│   ├── build/
│   │   ├── Build.cppm.in
│   │   ├── Capabilities.cppm.in
│   │   └── FeatureConfiguration.cppm.in
│   └── meta/
│       ├── Meta.cppm
│       ├── Types.cppm                      # Miracle.Meta:Types, NOT Miracle.Types
│       ├── Cache.cppm
│       ├── Reflect.cppm
│       ├── Query.cppm
│       └── ...                             # migrate all other Meta partitions
└── private/
    ├── diagnostics/Diagnostics.cpp
    ├── error/Error.cpp
    ├── panic/Panic.cpp
    ├── fs/Fs.cpp
    ├── profiling/Profiling.cpp
    └── memory/{linux,windows,unsupported}/Memory.cpp
```

**Important:** This is a *placement plan*, not an instruction to create empty `Detail`/`Codes`/`Parallel` files. `Iter` and `Meta.Query` have nontrivial mutual helper dependencies. Extract a partition only when its exported entities, incoming/outgoing imports, and deferred template instantiation are proven correct. Retaining one large coherent interface is preferable to making it subtly ill-formed.

---

## 1. Verified current ownership and intended new owners

Remote baseline has **33 `.ixx` interfaces**, **three `.ixx.in` templates**, **eight `.cxx` platform/implementation files**, **two named modules** (`Miracle`, `Miracle.Meta`), and one static-library target `Miracle` / `Miracle::Miracle`. Source-file lengths below were observed on the referenced revision, not measured on the author's local tree.

| Old source/declaration | Proposed owner after M.3–M.5 | Important dependencies/notes |
|---|---|---|
| `Miracle:Types` (`Types.ixx`, 63 lines) | `Miracle.Numeric` + `Miracle.Types` | Move *only* numeric aliases; preserve all remaining alias signatures |
| `Miracle:StaticString` (945 lines) | `Miracle.StaticString` | `Numeric`, optionally `Format`; owns `BasicStaticString`, `StaticString`, `staticFormat`, `std::formatter` specialization |
| `Miracle:Range` (99 lines) | `Miracle.Range` | Numeric/`std`; no `Iter` reverse import |
| `Miracle:Iter` (3,700 lines) | `Miracle.Iter` | Range, Types, Format; internal modules only after topology proof |
| `Miracle:Diagnostic` (569 lines) | `Miracle.Diagnostics` | Numeric/Types, StaticString, Format; preserve `Miracle::Diagnostic` singular type name |
| `Miracle:Feature` (1,365 lines) | `Miracle.Feature` | StaticString, Capabilities, Format; preserve resolver and `StaticString` report semantics |
| `Miracle:Error` (96 lines) | `Miracle.Error` | Types, Diagnostics |
| `Miracle:Panic` (48 lines) | `Miracle.Panic` | Error, Diagnostics, Types |
| `Miracle:Fs` (61 lines) | `Miracle.Fs` | Error, Types |
| `Miracle:Concepts` (23 lines) | `Miracle.Concepts` | Types as presently written |
| `Miracle:Memory` (17 lines) | `Miracle.Memory` | Types + platform `.cpp` owner migration |
| `Miracle:Profiling` (97 lines) | `Miracle.Profiling` | Debug, Types |
| `meta/Bitflags.ixx` (`Miracle:Bitflags`, 159 lines) | `Miracle.Bitflags` in `bitflags/` | Existing direct `import Miracle.Meta;` remains as needed |
| `meta/Debug.ixx` (`Miracle:Debug`, 216 lines) | `Miracle.Debug` in `debug/` | Existing dependencies include Meta, StaticString, Concepts |
| Generated `Miracle:Build` | `Miracle.Build` | Generated build facts, separate named module |
| Generated `Miracle:Capabilities` | `Miracle.Capabilities` | CMake probe facts, separate named module |
| Generated `Miracle:FeatureConfiguration` | `Miracle.FeatureConfiguration` | Feature, StaticString; build identity unchanged |
| Existing `Miracle.Meta:*` partitions | **Still `Miracle.Meta:*`** | Do **not** convert all Meta partitions to independent named modules |
| `Miracle.ixx` umbrella | `Miracle.cppm` umbrella | `export module Miracle;` plus `export import Miracle.*;` only |

**Runtime units:** `src/private/{Diagnostic,Error,Fs,Panic,Profiling}.cxx` currently declare `module Miracle;`; `src/private/{linux,unsupported}/Memory.cxx` also declare `module Miracle;`. Update each to its new owning module. **Source-level issue:** `src/private/windows/Memory.cxx` currently declares `module Miracle.Core;` and `import :Memory;`, a mismatch even before migration. Correct and test it as part of the authored changes; do not claim Windows CI ran unless it did.

**Meta-specific detail.** `Miracle.Meta` already has interface partitions `Types, Storage, Operators, Names, Predicates, Access, Cache, Members, Layout, Annotations, Values, Substitution, Generation, TypeQueries, TypeTransforms, Reflect, Query`; `Cache` is imported internally rather than directly re-exported. The three changed ownership boundaries above must not convert Meta's internal semantic helpers into public standalone modules by accident.

---

## 2. M.1 — Establish a reproducible baseline *before touching sources*

### 2.1 Inventory and source graph

[R] Record the exact local SHA, branch, workspace cleanliness, **reference compiler AND matching libc++**, CMake, Ninja, operating system, CPU count, RAM, and relevant flags. The historical commit mentioned above is a cross-check only.

```bash
# From the authoritative local checkout.
git rev-parse HEAD
git status --short --branch
git ls-files 'src/**' | grep -E '\.(ixx|cxx)(\.in)?$'
rg -n '^(export )?module |^(export )?import ' src/public src/private
rg -n '\.ixx|\.cxx' CMakeLists.txt cmake .github scripts tests examples benchmarks docs
```

For every module unit, record: file, `export module`/`module` declaration, each `import` and `export import`, its actual *named-module owner*, CMake target/file-set visibility, whether generated, and direct users. A relative import such as `import :Types;` belongs to the **current enclosing named module** (`Miracle` or `Miracle.Meta`); it is **not** the same declaration across those modules.

A simple graph tool is worth writing in `scripts/dev/` or under M evidence. Its core algorithm is:

```python
# Illustrative logic, not a complete parser or a claim of tool execution.
# Collect (owner, unit, imports) from every .cppm/.ixx/.cpp/.cxx source.
for unit in units:
    for imported in unit.imports:
        if imported == 'std':
            continue
        target = (unit.owner + imported) if imported.startswith(':') else imported
        edges[unit.module_unit].add(target)

# Expand the graph over module units, not only named-module labels.
# Check that no producer transitively depends on the umbrella.
# Use DFS visiting/visited colors or a Kahn topological sort for cycles.
# Emit: import-graph.json + import-graph.dot + a failure exit code on a cycle.
```

[P] Parse preprocessor branches/configured feature sources carefully; plain regex is a cross-check, not an authority over compiler scanning. Compare graph output to CMake/Ninja's actual module collation. Preserve a machine-readable **old declaration owner → new declaration owner** manifest as migration evidence. For exported declarations, include public concepts, annotations, template specializations, and customization points—not just primary classes.

### 2.2 Performance baseline (same host for M.8)

[R] Do not claim “modules made builds faster” without paired measurements. Use the repo presets under the pinned toolchain:

```bash
# Match the documented reference compiler snapshot and libc++ first.
source "$CXX26_TOOLCHAIN_ROOT/share/clang-cxx26/activate.sh"

cmake --preset tests --fresh \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
/usr/bin/time -v cmake --build --preset tests --parallel 2 \
  2> build/tests/baseline-build.time.txt
ctest --preset tests --output-on-failure
```

Record at least three *cold* builds with identical conditions and separate *warm* no-change runs. For implementation edits, record the changed file, which BMI/TU objects rebuilt, the wall time, and whether the file has an importer. For interface edits, record the invalidation fan-out. Never use `--fresh` as an incremental-rebuild measurement; it deliberately discards the old configuration. Keep raw samples and report median/range, peak RSS, BMI total bytes/count, and the consumer compile time for `import Miracle;`.

**Known pre-existing CI state at inspected SHA:** reference C++26 CI succeeded; documentation CI failed because the new `docs/phases/M/DESIGN.md` wasn't linked in `docs/SUMMARY.md`. Repair the navigation in M.2 and record it as an integration fix, not as a compiler issue.

**M.1 commit:** baseline inventory, mapping proposal, graph, reproducible benchmark harness and measured values (or explicit missing-measurement reason). Nothing has been source-migrated yet.

---

## 3. M.2 — `.ixx`/`.cxx` → `.cppm`/`.cpp` with unchanged module ownership

### 3.1 Separate filename migration from named-module migration

[R] First move files **without rewriting `export module Miracle:...` or `module Miracle;`**. That isolates extension/build issues from module ownership and ABI issues. Example:

```bash
git mv src/public/Iter.ixx src/public/Iter.cppm
git mv src/private/Error.cxx src/private/Error.cpp
# Move all 33 existing .ixx interfaces, three generated .ixx.in templates,
# all eight platform/private .cxx files, and applicable test/example/bench files.
```

Use `.cppm` for primary interface units and importable partitions; `.cpp` for ordinary translation units and module implementations. The language still recognizes `export module Miracle:Iter;` in `Iter.cppm`. Avoid a broad `sed` rewrite that changes import ownership during M.2.

### 3.2 Explicit CMake adjustments

The current CMake 4.4 configuration defines one target, one `PUBLIC FILE_SET TYPE CXX_MODULES`, generated interface sources, C++26 `import std`, and Ninja scanning. Retain these properties; change path registrations and file generators.

```cmake
# Required shape after suffix migration; keep the full original source list.
configure_file(
  src/public/Build.cppm.in
  "${miracle_generated_dir}/Build.cppm"
  @ONLY NEWLINE_STYLE LF)

# Repeat for Capabilities.cppm.in and FeatureConfiguration.cppm.in.

target_sources(Miracle
  PUBLIC FILE_SET miracle_modules TYPE CXX_MODULES
  BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/public"
            "${miracle_generated_dir}"
  FILES
    src/public/Miracle.cppm
    src/public/Iter.cppm
    # ... all existing interface units ...
    "${miracle_generated_dir}/Build.cppm"
    "${miracle_generated_dir}/Capabilities.cppm"
    "${miracle_generated_dir}/FeatureConfiguration.cppm"
  PRIVATE
    src/private/Error.cpp
    # ... all other implementation and platform files ...
)
```

The final extraction will relocate sources into directories in M.3–M.5; don't make M.2 unexpectedly change module names. **No `file(GLOB_RECURSE)` replacement is necessary**; maintain an explicit source inventory so omissions are reviewable. Preserve optional `MIRACLE_BUILD_FEATURE_SOURCES` behavior.

### 3.3 Exact tooling/search targets

Update *all* references, including strings in tests and generated CMake projects:

```text
CMakeLists.txt                            root source file list + generated outputs
src/public/{Build,Capabilities,FeatureConfiguration}.ixx.in
cmake/Capabilities.cmake                 every .cxx compile probe it emits
cmake/Features.cmake                     source registration values, if paths stored
scripts/ci/audit-codeql-coverage.py     both extensions AND classifying conventional files
scripts/ci/verify-nvim-lsp.lua           src/private/Error.cpp expected path
tests/CMakeLists.txt; tests/cmake/*       fixture paths, generated source checks
tests/consumer/{add-subdirectory,fetch-content,find-package}/
examples/; benchmarks/                  CMake sources, templates, generated filenames, parsers
Docker/editor/CI scripts                 clangd, compile_commands.json, coverage globs
docs/SUMMARY.md                          add Phase M contract navigation link
```

Current coverage script contains `EXTENSIONS = {".cxx", ".ixx", ".cppm"}` and classifies conventional sources by `.cxx`. It must include `.cpp` and update the latter classification, e.g.:

```python
MODULE_SUFFIXES = {'.ixx', '.cppm'}         # retain legacy recognition for audit history
IMPLEMENTATION_SUFFIXES = {'.cxx', '.cpp'}
EXTENSIONS = MODULE_SUFFIXES | IMPLEMENTATION_SUFFIXES
module_interfaces = [p for p in paths if Path(p).suffix in MODULE_SUFFIXES]
conventional = [p for p in paths if Path(p).suffix in IMPLEMENTATION_SUFFIXES]
```

Distinguish **file recognition** from actual CodeQL C++26 module semantic support. `.cppm` alone does not fix CodeQL's historically missing module extraction. Do not corrupt archived history merely to eliminate every textual mention of `.ixx`.

### 3.4 Compiler/packaging experiment

Create a small disposable consumer to prove Clang/CMake actually discover `.cppm` and that a clean installed package exports it. Test `add_subdirectory`, FetchContent, and installed `find_package`, not only in-tree `Miracle` compilation. Record the ref compiler's actual acceptance; CMake chooses the module compile commands from `FILE_SET TYPE CXX_MODULES`.

**M.2 commit:** every tracked extension/path updated, source module declarations unchanged, Debug/Release/tests and installed consumer pass; fix `SUMMARY.md` in the same scoped milestone.

---
## 4. M.3 — Foundations: exact `Numeric`, `Types`, and `StaticString` migration

### 4.1 `Miracle.Numeric` — exact Phase M contents

[R] Extract only the **existing primitive numeric type aliases**. This is the entire intended interface; do not introduce arithmetic algorithms, checked casts, strong integer wrappers, numeric traits, `Byte` or containers. Fixed-width names remain aliases of the standard fixed-width types; `f32` and `f64` remain the current built-in types.

```cpp
// src/public/numeric/Numeric.cppm
export module Miracle.Numeric;

import std;

export namespace Miracle {
    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using f32 = float;
    using f64 = double;

    using usize = std::size_t;
    using isize = std::ptrdiff_t;
}
```

**Why:** the current `Miracle:Types` aliases combine scalar primitives with containers, ownership aliases, filesystem paths, and flat containers. A low-level formatter or string type should not need that entire conceptual vocabulary. `Miracle.Numeric` is one of two zero-Miracle-dependency roots; `Miracle.Format` is the other.

### 4.2 `Miracle.Types` — retain precisely the residual aliases

[R] Do not delete or “improve” the remaining aliases during Phase M. Move the original declarations to `Miracle.Types` unchanged. Re-export Numeric so established source usage that imports the broader vocabulary continues to see numeric names:

```cpp
// src/public/types/Types.cppm
export module Miracle.Types;

export import Miracle.Numeric; // public vocabulary includes the scalar aliases
import std;

export namespace Miracle {
    using String     = std::string;
    using StringView = std::string_view;
    using Byte       = std::byte;
    using Path       = std::filesystem::path;

    template<class T, usize N>
    using Array = std::array<T, N>;

    template<class T>
    using Vec = std::vector<T>;

    template<class T>
    using UPtr = std::unique_ptr<T>;

    template<class T>
    using Ref = std::reference_wrapper<T>;

    template<class A, class B>
    using Pair = std::pair<A, B>;

    template<class... Ts>
    using Tuple = std::tuple<Ts...>;

    template<class T>
    using Option = std::optional<T>;

    inline constexpr std::nullopt_t None = std::nullopt;

    template<class T, usize Extent = std::dynamic_extent>
    using Span = std::span<T, Extent>;

    template<class T, class Compare = std::less<T>>
    using FlatSet = std::flat_set<T, Compare, Vec<T>>;

    template<class K, class V, class Compare = std::less<K>>
    using FlatMap = std::flat_map<K, V, Compare, Vec<K>, Vec<V>>;

    template<class T, usize Capacity>
    using InplaceVec = std::inplace_vector<T, Capacity>;

    template<class T>
    using Hive = std::hive<T>;
}
```

This sketch is intentionally faithful to the **63-line existing `Types.ixx`**. The alias list, default arguments, namespaces, and exact underlying `std::` types are semantic migration invariants. The `.cppm` compile must use the reference C++26/libc++ that already supports these vocabulary types; no fallback reimplementation is authorized.

**Source compatibility test:** compare `std::same_as<Miracle::Vec<int>, std::vector<int>>`, `std::same_as<Miracle::Option<int&>, std::optional<int&>>` if this is supported by the pinned toolchain, `Miracle::None`, `usize`, `FlatMap`, and client code from the existing tests. The `std::optional<T&>` test is intentionally reference-toolchain-specific. Don't introduce a new type merely to satisfy an older compiler.

### 4.3 `Miracle.StaticString` — preserve all three responsibilities

The current `StaticString.ixx` is not just a string class. It contains these actual clusters:

| Current region (approx.) | Existing responsibility | Migration recommendation |
|---|---|---|
| lines 13–500 | `BasicStaticString<Char, Capacity>` core type, storage and operations | Keep class definition together in `StaticString.cppm` initially |
| lines 503–521 | module-local `StaticStringAccess` mutation bridge | Keep next to core; don't accidentally export it |
| lines 522–722 | deduction guide, `StaticString` alias, split result, public algorithms | May later be a `:Algorithms` partition after visibility proof |
| lines 723–884 | `StaticStringFormatting` helpers and brace parser | Possible `:Formatting` partition, only after seeing import graph |
| lines 885–931 | exported `staticFormat` | Keep its complete templated definition reachable to consumers |
| lines 933 onward | `std::formatter<BasicStaticString<...>>` specialization | Must remain reachable and correctly attached/visible after extraction |

Minimal first-pass interface:

```cpp
// src/public/static_string/StaticString.cppm
export module Miracle.StaticString;

import std;
import Miracle.Numeric;
// import Miracle.Format;  // add only when migrating authored messages

// Move the existing declarations and definitions intact.
// Preserve Miracle::BasicStaticString, StaticString, staticFormat,
// the module-local StaticStringAccess bridge and std::formatter specialization.
```

[R] Don't change structural NTTP representation (`storage`, `length`), capacity semantics, split behavior, `staticFormat` mini-grammar, or formatter specializations while moving ownership. The current class asserts that `Char` is trivially copyable, trivially default-constructible and standard-layout. This predicate is unchanged even if its *message* changes. The existing `staticFormat` checks character type, brace syntax and placeholder count; those must retain identical acceptance/rejection conditions.

**Bootstrap**: `StaticString` is allowed to depend on `Miracle.Format`; Format must never depend back on StaticString. For a simple invariant of Format itself, literal `static_assert` is legal. For StaticString's rich errors, see §5.6 for how to build messages on failing branches without introducing unconditional reflection work.

### 4.4 `Range` and generated facts

```cpp
// src/public/range/Range.cppm
export module Miracle.Range;
import std;
import Miracle.Numeric;
// Retain existing RangeElement, CompatibleRangeEndpoints,
// Range<T> and deduction guides exactly as implemented.
```

The current `Range` code is ~99 lines; no decomposition is justified. Preserve half-open, reversed/empty, signedness and stride constraints. `Iter` imports Range; Range must not import Iter.

Generated modules are distinct named-module roots/consumers, not three additional partitions of `Miracle`:

```cpp
// src/public/build/Build.cppm.in
export module Miracle.Build;
export namespace Miracle::build {
    // Move existing generated platform/architecture/optimized facts unchanged.
}

// src/public/build/Capabilities.cppm.in
export module Miracle.Capabilities;
export namespace Miracle::capabilities {
    // Existing detected constants unchanged.
}

// src/public/build/FeatureConfiguration.cppm.in
export module Miracle.FeatureConfiguration;
import Miracle.Feature;
import Miracle.StaticString;
export namespace Miracle::feature {
    // buildFeatures, buildCapabilities, buildIdentity,
    // buildEnabled/current/defaults: existing semantics unchanged.
}
```

These templates must continue to generate output into the build directory and participate in the `CXX_MODULES` file set. Keep **feature build identity hashing and generated facts** byte-for-byte equivalent for equivalent configurations. Since `FeatureConfiguration` depends on `Miracle.Feature`, its final independent-module conversion may be performed near M.5; do not force it early merely because the generated source file already exists.

---

## 5. M.3 centerpiece — a technically implementable `Miracle.Format`

### 5.1 Design decision: one low-level renderer, not a mini-Validation engine

[R] `Miracle.Format` imports **no `Miracle.*` module**. It is a formatting facility, not a semantic rule evaluator, not a diagnostic registry, and not a general-purpose `std::format` replacement. It must be usable in the earliest foundations, including diagnostics that explain *why StaticString itself is invalid*.

[P] Start with **one module interface**, `format/Format.cppm`. Introduce `:Message` or `:Layout` partitions only if they substantially improve clarity/compilation without creating a public→private BMI dependency. Small source units are a preference, not a mandate.

The most conservative API is a **fixed-size value message** and a **linear bounded builder**:

```cpp
import Miracle.Format;

constexpr auto message = Miracle::format::Builder<512>{}
    .error({"ITR", 4, 3}, "callable cannot accept sequence element")
    .context("Iter::map")
    .detail("element", "std::tuple<int, float>")
    .note("neither direct invocation nor tuple decomposition succeeded")
    .help("provide a callable accepting the input or a supported decomposition")
    .build();

// C++26: static_assert(SomeDependentCondition, message);
```

The fluent spelling is **[P], not [R]** (Contract §6.2 explicitly marks factories illustrative). The **required capabilities** are `size()`/`data()` for C++26 static assertions, bounded deterministic storage, severity/code/summary, context, details, notes, help, explicit overflow policy, and low successful-path compile cost.

### 5.2 Suggested exact data model and invariants

The following is sufficiently close to direct implementation to serve as a starting point. It intentionally uses only the C++ standard library and does not use `StaticString`, `Numeric`, dynamic memory, `std::format`, `std::vector`, or `std::meta`:

```cpp
// src/public/format/Format.cppm — reference DESIGN SKETCH
export module Miracle.Format;
import std;

export namespace Miracle::format {

// Keep formatting categories distinct from Miracle::Severity, which is owned
// by Miracle.Diagnostics. Diagnostics can map its public enum to this enum.
enum class Level : unsigned char { Debug, Note, Warning, Error };

struct Code final {
    std::string_view prefix{};    // e.g. "ITR" / "STR" / "FTR"
    std::uint64_t number{};      // a domain-owned stable code value
    std::size_t minWidth{3};     // minimum zero-pad; never truncate digits
};

template<std::size_t Capacity>
struct Message final {
    static_assert(Capacity < std::numeric_limits<std::size_t>::max(),
                  "error[FMT001]: message capacity is not representable");
    std::array<char, Capacity + 1> storage{}; // includes trailing NUL
    std::size_t used{};                     // excludes trailing NUL

    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
        return used;
    }
    [[nodiscard]] constexpr auto data() const noexcept -> const char* {
        return storage.data();
    }
};

} // namespace Miracle::format
```

**Core invariants:** `0 <= used <= Capacity`, `storage[used] == '\0'`, `size()` counts *written characters* (not capacity and not the terminal zero), and every counted character is initialized and remains owned by the message object. Values must be usable as constant expressions; `Message` shouldn't contain dangling `string_view`, pointers to a builder-local buffer, or a runtime allocator. C++26 user-generated `static_assert` messages read the returned object's `data()[0..size())` during constant evaluation. The standard does not guarantee the compiler will reproduce its entire wrapper/output formatting verbatim.

Using a `Message<Capacity>` with explicit active length avoids instantiating a new return type for every appended character. A specialized exact-extent result may be considered later if benchmarks justify it. The builder's stack/local storage can be fixed size; the final message remains a value with no ownership ambiguity.

### 5.3 Builder mechanics — concrete minimal implementation sketch

```cpp
// Continuing inside export namespace Miracle::format

template<std::size_t Capacity>
class Builder final {
public:
    [[nodiscard]] constexpr auto error(Code code, std::string_view summary)
        -> Builder& {
        return heading(Level::Error, code, summary);
    }

    // Use when no stable domain code has yet been assigned. Never invent an ID.
    [[nodiscard]] constexpr auto error(std::string_view summary) -> Builder& {
        append("error: "); append(summary); append("\n");
        return *this;
    }

    [[nodiscard]] constexpr auto heading(
        Level level, Code code, std::string_view summary) -> Builder& {
        append(levelName(level));
        append("[");
        append(code.prefix);
        appendDecimal(code.number, code.minWidth);
        append("]: ");
        append(summary);
        append("\n");
        return *this;
    }

    [[nodiscard]] constexpr auto context(std::string_view value) -> Builder& {
        append("  --> "); append(value); append("\n");
        return *this;
    }

    [[nodiscard]] constexpr auto detail(
        std::string_view key, std::string_view value) -> Builder& {
        append("  "); append(key); append(": ");
        append(value); append("\n");
        return *this;
    }

    [[nodiscard]] constexpr auto note(std::string_view value) -> Builder& {
        append("  = note: "); append(value); append("\n");
        return *this;
    }

    [[nodiscard]] constexpr auto help(std::string_view value) -> Builder& {
        append("  = help: "); append(value); append("\n");
        return *this;
    }

    [[nodiscard]] constexpr auto build() const -> Message<Capacity> {
        return message_;
    }

private:
    Message<Capacity> message_{};

    static constexpr auto levelName(Level level) -> std::string_view {
        switch (level) {
            case Level::Debug:   return "debug";
            case Level::Note:    return "note";
            case Level::Warning: return "warning";
            case Level::Error:   return "error";
        }
        return "error"; // defensive only; enum is intentionally closed
    }

    constexpr auto appendCharacter(char c) -> void {
        if (message_.used == Capacity) {
            // Evaluated only if a concrete requested message overflows.
            // In a required constant evaluation, this is an explicit failure.
            throw "Miracle.Format: diagnostic capacity exceeded";
        }
        message_.storage[message_.used++] = c;
        message_.storage[message_.used] = '\0';
    }

    constexpr auto append(std::string_view text) -> void {
        for (char c : text) appendCharacter(c);
    }

    constexpr auto appendDecimal(
        std::uint64_t value, std::size_t minWidth) -> void {
        std::array<char, 20> digits{};   // 20 decimal digits suffice for u64
        std::size_t count{};
        do {
            digits[count++] = char('0' + (value % 10));
            value /= 10;
        } while (value != 0);
        for (std::size_t n = count; n < minWidth; ++n) {
            appendCharacter('0');
        }
        while (count != 0) appendCharacter(digits[--count]);
    }
};

// Example use in a constant-evaluation context:
// constexpr auto msg = Builder<256>{}
//     .error({"STR", 1, 3}, "invalid character type")
//     .context("BasicStaticString")
//     .help("use a compatible trivial standard-layout character")
//     .build();
```

**[P] Implementation refinements before production:** `Builder` could expose rvalue-only fluent operations returning `Builder&&`, mirroring other Miracle builders, rather than references to a temporary. Prefer whichever shape is simpler and compiles with the pinned C++26 toolchain. For *temporary chaining*, the sketch's lvalue-reference return works during the full expression and `build()` copies the finished value; explicitly test this lifetime property. You may add a `consteval makeError(...)` wrapper to make well-formed constant-evaluation intent obvious. Do **not** introduce a giant variadic formatting DSL during Phase M.

### 5.4 Overflow and sizing rules

[R] A message must never silently lose its code/summary or write outside capacity. Prefer **exact/derived capacity** for known literal components; for reflected names and variable context, choose an explicit documented budget and a deliberate overflow behavior. The sketch throws at constant evaluation: this is a recognizable *formatting-capacity failure* but does **not** produce the intended domain diagnostic, so routine messages should be sized high enough or computed in advance.

[P] Implement a lightweight `formattedSize` calculation (sum of lengths + prefixes + decimal width + newlines) to allocate `Message<requiredSize>` when all input texts are known as constant expressions or structural NTTPs. For dynamic-length reflection display results, either compute an upper bound before instantiation or use a bounded fallback that explicitly says *which details were omitted* while preserving the code/summary. Do not truncate a UTF-8 code point midway if the feature supports non-ASCII inputs; if not supported, document and test the Phase M ASCII-only assumption.

**No eager global code catalog.** The `Code` above is a normalized lightweight *input*. Its numeric identity remains domain-owned, not inferred by Format. The existing annotated diagnostic enums remain owned by Diagnostics; internal Iter/StaticString codes may be module-local constants/enums until an agreed public code vocabulary is necessary.

### 5.5 Canonical message grammar and layout

[R] Standardize the Miracle-authored **message body**, not the compiler's own source excerpts:

```text
error[ITR004]: callable cannot accept sequence element
  --> Iter::map
  callable: MyCallable
  element:  std::tuple<int, float>
  = note: direct invocation and supported decompositions failed
  = help: provide a callable that accepts the element or supported fields
```

Grammar:

```text
header     := <severity> '[' <prefix> <zero-padded decimal code> ']: ' <summary> '\n'
uncoded    := <severity> ': ' <summary> '\n'                # only before a domain code is assigned
context    := '  --> ' <operation or subject> '\n'              # optional
attribute  := '  ' <key> ': ' <value> '\n'                    # zero or more
note       := '  = note: ' <explanation> '\n'                 # zero or more
help       := '  = help: ' <action> '\n'                       # zero or more
message    := header context? attribute* note* help*
```

Domain `prefix`/numeric code come from the module assigning the error. For codes not yet assigned, the design permits a suitable stable identifier-less fallback; don't create arbitrary numeric IDs without reserving them in the owning domain. Preserve `diagnostics::align(n)` semantics: *minimum* padding, not maximum digits. Output order is stable; don't generate notes from nondeterministic hash-map traversal.

[P] Make layout primitives accept a trivial sink concept (`push(char)` or `append(string_view)`), then test both a fixed compile-time buffer and the *existing* runtime Diagnostics renderer. If moving runtime Diagnostics onto the shared sink would visibly change its output or require a broad redesign, preserve runtime behavior and share only the layout rules that can be reused safely, as Contract §§6–7 require.

### 5.6 Fail-path-only generation in foundation templates

Problem: writing a fully reflective diagnostic expression directly into every unconditional `static_assert` can cause expensive evaluation or extra hard errors for valid instantiations. Use a dependent branch that is instantiated only when its condition is false.

```cpp
// Suggested internal helper in StaticString, after import Miracle.Format.
template<class Char>
consteval auto enforceCharacterRequirements() -> bool {
    constexpr bool valid =
        std::is_trivially_copyable_v<Char> &&
        std::is_trivially_default_constructible_v<Char> &&
        std::is_standard_layout_v<Char>;

    if constexpr (!valid) {
        constexpr auto message = Miracle::format::Builder<512>{}
            .error({"STR", 1, 3}, "invalid BasicStaticString character type")
            .context("BasicStaticString<Char, Capacity>")
            .note("Char must be trivially copyable, default-constructible, and standard-layout")
            .help("select a compatible character type")
            .build();
        static_assert(valid, message); // dependent on Char; hard error only here
    }
    return valid;
}

// Inside BasicStaticString<Char, Capacity>:
// static_assert(enforceCharacterRequirements<Char>());
```

**Technical note:** The above sketch hardcodes `Capacity` into the context *text* without rendering its numeric value. If reporting the actual capacity is important, take it as a template argument to the helper and use `appendDecimal` on the **failing** branch. Do not put a hard assertion in a `requires` predicate or change existing SFINAE/concept substitution semantics simply to improve the message. Internal implementation invariants that cannot legally instantiate Format still receive concise literal assertions.

For reflective diagnostics, the **caller**, not Format, may obtain a type name under the pinned reference toolchain:

```cpp
// Inside an Iter or Meta compile-time failure branch, not inside Format.
// constexpr auto typeName = std::meta::display_string_of(^^Item);
// auto message = Builder<...>{}.detail("item", typeName)...build();
```

If the display function returns implementation-defined wording, test the **presence and layout** of the field rather than exact compiler-dependent spelling outside the pinned compiler. If the reflection result's lifetime is temporary, the builder must copy its characters into `Message` while the name is valid; don't store a `string_view` in the finalized object.

### 5.7 Format acceptance experiments (write these early)

```cpp
// Positive C++26 compile fixture (illustrative):
import Miracle.Format;

constexpr auto validMessage = Miracle::format::Builder<128>{}
    .error({"FMT", 1, 3}, "test message")
    .help("a bounded compile-time string")
    .build();

static_assert(validMessage.size() != 0);
static_assert(validMessage.data()[0] == 'e');
static_assert(true, validMessage);   // exercises C++26 message-expression rules

// Separate NEGATIVE fixture:
// static_assert(false, validMessage);
// expected error body contains "error[FMT001]: test message".
```

Tests must include: zero-character buffer/successful empty message; exact-capacity writes; one-over-capacity failure; zero code and multi-digit code wider than padding; `Debug/Note/Warning/Error` headers; ordered details/notes/help; multiline context; large reflected name; positive no-diagnostic branch; and a negative `static_assert(false, message)` fixture. One compiler-negative fixture per scenario should assert a failing compiler exit code **and a stable Miracle message substring**. Never compare an entire compiler traceback across compiler versions.

**M.3 exit:** `import Miracle.Numeric`, `Miracle.Types`, `Miracle.Format`, `Miracle.StaticString` work individually; Format has no Miracle imports; formatted assertion proof passes; existing StaticString/Range/Feature baseline behavior remains intact. Record a coherent M.3 commit.

---
## 6. M.4 — `Miracle.Diagnostics`: code domains, model, runtime rendering

### 6.1 What stays semantically unchanged

Current `Diagnostic.ixx` (~569 lines) contains these actual layers:

| Existing region (approx.) | What it implements | Recommended destination |
|---|---|---|
| lines 7–94 | annotation marker/factories, `Severity`, `DiagnosticCodeDescriptor` | `Diagnostics.cppm`, or `:Vocabulary` if genuinely useful |
| lines 96–272 | `std::meta` annotation inspection, domain validity, descriptor construction | `Diagnostics.cppm`, or `:Codes`/`:Domain` |
| lines 273–538 | `SourceRange`, `SpanRole`, move-only `DiagnosticSpan`, `Diagnostic` | `Diagnostics.cppm`, or `:Model` |
| lines 539 onward | `RenderOptions` / public renderer surface | public interface; implementation in Diagnostics.cpp |
| `src/private/Diagnostic.cxx` | runtime rendering | `src/private/diagnostics/Diagnostics.cpp`, owned by `Miracle.Diagnostics` |

[R] Preserve existing public spelling (`Miracle::Diagnostic`, `DiagnosticSpan`, `DiagnosticCode`, `diagnostics::prefix`, `diagnostics::align`, `diagnostics::message`, `Severity`, descriptors). Only the **named module** becomes plural `Miracle.Diagnostics`. Do not introduce a duplicate `Miracle::Diagnostic` definition as a bridge from the old module.

```cpp
// src/public/diagnostics/Diagnostics.cppm
export module Miracle.Diagnostics;

import std;
import Miracle.Numeric;
import Miracle.Types;         // only if needed for exact existing public aliases
import Miracle.StaticString;
import Miracle.Format;

// Transfer existing annotation, domain, model, and renderer declarations.
// Keep std::meta-based code-domain analysis here, not in Miracle.Format.
```

```cpp
// src/private/diagnostics/Diagnostics.cpp
module Miracle.Diagnostics;

import std;
import Miracle.Types;
import Miracle.Format;
// import the owning primary module/partitions as actually needed;
// copy the existing Diagnostic.cxx implementation without behavior changes.
```

**Module ownership invariant:** an implementation unit with `module Miracle.Diagnostics;` may reference definitions attached to that named module; it must not remain `module Miracle;` with imports rewritten solely for superficial compilation. The same rule applies to every `.cpp` implementation unit in M.5.

### 6.2 Connection to Format: normalize identity, not semantic validity

Current `DiagnosticCodeDescriptor` is runtime-owned and contains `String domain/prefix/enumerator/canonicalMessage`, `u64 numeric`, and `usize alignment`. `Miracle.Format` must not import it; that would create a foundation → higher-level dependency. Instead, **adapt downward at the caller**:

```cpp
// Illustration only; this is an adapter owned by Miracle.Diagnostics.
auto toFormatCode(const DiagnosticCodeDescriptor& d)
    -> Miracle::format::Code {
    return {
        .prefix = d.prefix,    // std::string -> std::string_view for duration of call
        .number = d.numeric,
        .minWidth = d.alignment
    };
}
```

The existing reflected enum is **the one source of code identity**; an adapter must not create a second enum/code mapping. The runtime formatter may use shared Format layout operations or conform to the same grammar using its current `std::string` storage. **Preserve existing renderer output** and change only explicitly approved authored-message presentation. The runtime renderer has source snippets, colors, labels and locations that Format need not understand.

**Existing predicate:** `diagnosticCodeValid<Code>()` checks exactly one marker/prefix, optional alignment constraints, enumerator annotation legality and nonnegative values for signed underlying enums. This stays unchanged. `DiagnosticCode` remains a concept, not a `static_assert` that hard-errors during substitution. Introduce rich format messages at *enforcement boundaries* where hard errors were already intended, not inside concept validity itself.

### 6.3 When a diagnostic needs module-local code

A small private error-code vocabulary can live in the component that generates the error:

```cpp
// In a private Iter implementation section/partition.
enum class IterDiagnosticCode : unsigned {
    IncompatibleCallable = 4,
    InvalidLookaheadReturn = 5
};

consteval auto iterCode(IterDiagnosticCode code) -> Miracle::format::Code {
    return {"ITR", static_cast<unsigned>(code), 3};
}
```

[R] These IDs must be stable once published as user-observable diagnostic codes. Define a source-of-truth table or enum per domain and include it in testing/documentation. **Do not** copy the same ID into a separate Validation-engine registry. The future Validation engine may *adapt* or reference code identities, but remains out of scope for Phase M.

### 6.4 Focused tests and exit criteria

Add direct `import Miracle.Diagnostics;` positive consumers exercising annotated enums, `diagnostics::prefix("...")`, `diagnostics::align(n)`, error/warning/note constructors, move-only spans, source selections, and existing runtime output. Negative checks should target invalid marker/domain configurations **without turning previously SFINAE-friendly concepts into eager failures**. Compare exact formatted *authored message text*, but normalize compiler wrappers (paths, line numbers, template backtraces) for compile-fail snapshots.

**M.4 exit:** standalone Diagnostics works in source/build/install consumption; shared formatting grammar works; runtime semantics and domain reflection remain unchanged; no imports from Diagnostics to Meta Validation. Commit with a dedicated M.4 boundary.

---

## 7. M.5 — Remaining named-module migration, concrete import transformations

### 7.1 Mechanical rule: `import :X` does not follow a moved entity automatically

Before migration:

```cpp
// Error.cppm (M.2 already completed; name still old at this stage)
export module Miracle:Error;
import :Types;
import :Diagnostic;
```

After **moving ownership**:

```cpp
// src/public/error/Error.cppm
export module Miracle.Error;
import std;
import Miracle.Types;
import Miracle.Diagnostics;
```

And the corresponding implementation must also change owner:

```cpp
// src/private/error/Error.cpp
module Miracle.Error;
import std;
import Miracle.Types;
import Miracle.Diagnostics;
// import Miracle.Error or its appropriate partitions if needed;
// move existing definitions without semantic changes.
```

**Why:** a relative import `:Diagnostic` is resolved relative to the importing **named module**, not by filesystem directory. Leaving it after extraction attempts to import `Miracle.Error:Diagnostic`, not the new `Miracle.Diagnostics`. Conversely, replacing every `import` with `export import` is incorrect: only re-export dependencies that the component's public surface actually promises its consumers.

### 7.2 Explicit ownership and dependency migration table

| New file / module declaration | Convert old imports into | Implementation owner |
|---|---|---|
| `range/Range.cppm`: `export module Miracle.Range;` | `import std; import Miracle.Numeric;` | entirely interface as presently needed |
| `iter/Iter.cppm`: `export module Miracle.Iter;` | `import std; import Miracle.Types; import Miracle.Range; import Miracle.Format;` | templates remain in interface partitions; see M.6 |
| `feature/Feature.cppm`: `export module Miracle.Feature;` | `import std; import Miracle.Types; import Miracle.StaticString; import Miracle.Capabilities; import Miracle.Format;` | mostly consteval templates/interface |
| `error/Error.cppm`: `export module Miracle.Error;` | `import std; import Miracle.Types; import Miracle.Diagnostics;` | `module Miracle.Error;` |
| `panic/Panic.cppm`: `export module Miracle.Panic;` | `import std; import Miracle.Types; import Miracle.Diagnostics; import Miracle.Error;` | `module Miracle.Panic;` |
| `fs/Fs.cppm`: `export module Miracle.Fs;` | `import std; import Miracle.Types; import Miracle.Error;` | `module Miracle.Fs;` |
| `concepts/Concepts.cppm`: `export module Miracle.Concepts;` | currently `import :Types;` → `import Miracle.Types;` | no extra implementation needed |
| `memory/Memory.cppm`: `export module Miracle.Memory;` | currently `import :Types;` → `import Miracle.Types;` | `module Miracle.Memory;` for all platforms |
| `debug/Debug.cppm`: `export module Miracle.Debug;` | Types, `Miracle.Meta`, StaticString, Concepts | no runtime owner in baseline |
| `bitflags/Bitflags.cppm`: `export module Miracle.Bitflags;` | Types, `Miracle.Meta` | no runtime owner in baseline |
| `profiling/Profiling.cppm`: `export module Miracle.Profiling;` | Debug, Types, `std` | `module Miracle.Profiling;` |
| `build/FeatureConfiguration.cppm.in`: `export module Miracle.FeatureConfiguration;` | `Miracle.Feature`, `Miracle.StaticString` | generated interface |

The table is **guidance based on actual old imports**, not permission to add unnecessary imports. In particular, `Miracle.Format` should be imported only where the component now uses its formatter; `Miracle.Iter` should not import all of Meta just to obtain the existing `std::meta` reflection it already accesses directly.

### 7.3 Implementation-unit ownership table

| Existing `.cxx` | Final `.cpp` path | Required module declaration |
|---|---|---|
| `src/private/Diagnostic.cxx` | `src/private/diagnostics/Diagnostics.cpp` | `module Miracle.Diagnostics;` |
| `src/private/Error.cxx` | `src/private/error/Error.cpp` | `module Miracle.Error;` |
| `src/private/Panic.cxx` | `src/private/panic/Panic.cpp` | `module Miracle.Panic;` |
| `src/private/Fs.cxx` | `src/private/fs/Fs.cpp` | `module Miracle.Fs;` |
| `src/private/Profiling.cxx` | `src/private/profiling/Profiling.cpp` | `module Miracle.Profiling;` |
| `src/private/linux/Memory.cxx` | `src/private/memory/linux/Memory.cpp` | `module Miracle.Memory;` |
| `src/private/unsupported/Memory.cxx` | `src/private/memory/unsupported/Memory.cpp` | `module Miracle.Memory;` |
| `src/private/windows/Memory.cxx` | `src/private/memory/windows/Memory.cpp` | **correct old `module Miracle.Core;`** → `module Miracle.Memory;` |

Only the platform-selected Memory implementation belongs in a given CMake configuration. Do not accidentally compile all three into the same target. Preserve platform routing and CMake `miracle_memory_source` selection.

### 7.4 Generated features and optional source registration

The existing build feature engine obtains `MIRACLE_BUILD_FEATURE_SOURCES` from `cmake/Features.cmake` and uses generated `FeatureConfiguration`. Any optional module interface introduced through `miracle_register_build_feature(SOURCES ...)` must (a) use the new path, (b) retain the same enable/default/dependency/conflict semantics, and (c) be added to an appropriate CMake module file set. A disabled feature must not be accidentally imported transitively by `Miracle` or another always-enabled module. Capture both **enabled** and **disabled** test configurations; don't just test default settings.

### 7.5 Move semantics unchanged

Most of M.5 should be mechanically reviewable. Preserve current `Error`/`Result<T>` behavior, `Panic` diagnostic path, filesystem options, `Debug` formatting, `Bitflags` reflected semantics and `Profiling` event capture. The migration is not the place to fix hypothetical API ergonomics or introduce a new error propagation model.

**M.5 exit:** all accepted standalone modules import individually; no extracted implementation remains attached to `module Miracle;`; Windows ownership typo corrected and marked as source-level corrected unless Windows compilation was actually run; all smoke tests pass. Commit(s) grouped under M.5, retaining attribution for final batched review.

---
## 8. M.6 — How to split the large interfaces without destroying their semantics

This is the **highest-risk** implementation milestone. C++ named modules do **not** have nested partitions. A name like `Miracle.Iter:Sources.Optional` is still one **flat** partition name under `Miracle.Iter`; it is not an independently owned submodule. Moving text into files without a dependency plan can change visibility, overload resolution, specialization visibility and when constant evaluation happens.

### 8.1 Five rules for partition extraction

**[R1] Shared forward declarations and standard customizations first.** In `Iter`, the first ~80 lines contain forward declarations and `std::ranges::enable_view` variable-template specializations. These specializations must remain reachable **before** consumers ask whether relevant types model `view`. Moving them late can change concept satisfaction.

**[R2] Each interface partition has a strictly earlier dependency set.** A partition may import `:Detail` or `:Sources` only if those partitions do not import it in return. Use the graph auditor from M.1 to reject cycles *before* recompiling.

**[R3] Template bodies stay reachable.** Most Iter/Feature/Query behavior is implemented in templates and `consteval` functions. Moving those bodies into plain private `.cpp` is unsafe unless they are non-template functions with a supported externally available declaration, or explicit instantiation is genuinely exhaustive. Test with user-defined types in an out-of-tree installed consumer.

**[R4] Keep private semantics private, but accessible where required.** A helper imported by another partition may need an interface partition even when it is not a user-facing API. Non-exported implementation details in an importable partition can be reachable to the owning module. The exact visibility/installation behavior must be verified under pinned Clang and CMake; do **not** `export import` every internal helper just to silence an import error.

**[R5] Preserve evaluation order and identity.** Meta Query state normalization, `std::meta` access contexts, reflection snapshots, and lazy evaluation must not change merely because their definitions moved to another module unit. An identical source expression at a different declaration point can observe a different reflected universe. This is a semantic concern, not a formatting issue.

### 8.2 `Miracle.Iter`: map actual source regions to coherent units

At the inspected baseline, `Iter.ixx` has approximately **3,700 lines** with these concrete sections:

| Current lines (approx.) | Actual symbols/mechanisms | Preferred destination |
|---|---|---|
| 1–80 | forward declarations for `MaybeRefView`, `OnceWith`, `RepeatWith`, `FromFn`, `Successors`, `Cycle`, `Scan`, `Intersperse`, `MapWindows`, `ArrayChunks`, `Peekable`, `Iter`; `std::ranges::enable_view` specializations | `iter/Forward.cppm` (recommended addition) |
| 82–582 | `HasTupleProtocol`, reflected aggregate member cache, `adaptiveInvoke`, `adaptiveInvokeWithPrefix`, `MovableBox`, `AdaptiveCallable`, `AdaptiveBinaryCallable`, `ComposedProjection`, `ProjectedIterator`, `ProjectedSentinel` | `iter/Detail.cppm` (or two detail units only if needed) |
| 583–1174 | `MaybeRefView`, `MaybeValueView`, `OnceWith`, `RepeatWith`, `FromFn`, `Successors`, `Cycle`, `ProjectedFilterView` | `iter/Sources.cppm` |
| 1175–1952 | `Scan`, `Intersperse`, `IntersperseWith`, `MapWindows`, `ArrayChunks` | `iter/Adaptors.cppm` / `Views.cppm` |
| 1953–2035 | `Infinity`, `infinity`, `SizeHint`, `ParallelIter` declaration, terminal reference/value helper types | `iter/Prelude.cppm` or keep adjacent to Pipeline |
| 2036–2380 | `Peekable` cursor and one-item cache | `iter/Peekable.cppm` if graph permits |
| 2381–3446 | `Iter<View, Projection>` façade, map/filter/chain/terminals, `Projected` lifetime and borrow handling | `iter/Pipeline.cppm` |
| 3447–3611 | `ParallelIter`, standard execution-policy terminals, out-of-class `Iter::parallel` definitions | `iter/Parallel.cppm` if standalone partitions compile correctly |
| 3612–3693 | `iter(...)` overloads, `empty`, `once`, `repeat`, `repeatWith`, `fromFn`, `successors`, etc. | `iter/Factories.cppm` |
| 3694–end | `std::ranges::enable_borrowed_range` specializations | with `Pipeline` or a tested customization partition, *not* a late invisible helper |

This is a **suggested extraction map**, not a command to cut at exact numeric lines. Names and spans were inspected in the baseline; final line positions change as code moves. For source type `OnceWith`, any internal initialization helpers must travel with it or be imported from the correct detail partition.

### 8.3 A workable Iter partition import DAG

A candidate graph (edge `A → B` means A imports B):

```text
                         :Forward
                            │
                          :Detail
                         /       \
                    :Sources   :Prelude
                         \       /
                          :Adaptors
                               │
                           :Peekable
                               │
                           :Pipeline
                               │
                           :Parallel
                               │
                           :Factories
                               │
                     Miracle.Iter primary
```

This diagram is **conceptual**: the precise edges should be determined by actual qualified symbol use. Avoid unnecessary serialization (`:Peekable` may depend on `:Detail` rather than `:Adaptors`; `:Prelude` can be independent). The **non-negotiable** property is no back-edge from `:Pipeline` to `:Parallel`: the Pipeline class can *declare* its parallel methods using a forward-declared `ParallelIter`, while `:Parallel` defines the class and then the out-of-class methods, as the existing source already does.

Suggested interface files:

```cpp
// iter/Forward.cppm
export module Miracle.Iter:Forward;
import std;
import Miracle.Types;

export namespace Miracle {
    template<class Function> class OnceWith;
    template<class Function> class RepeatWith;
    // ... copy the exact current forward declarations and constraints ...
    template<std::ranges::view View, class Projection = std::identity>
    class Iter;
}

// Bring forward the existing std::ranges::enable_view partial specializations
// for the forward-declared view types, in their current visibility order.
```

```cpp
// iter/Detail.cppm
export module Miracle.Iter:Detail;
import std;
import Miracle.Types;
import :Forward;

// Existing non-user-facing adaptive invocation and projected-iterator machinery.
// Make declarations available to other partitions only as required by Clang's
// module reachability rules. No new public API names are intended.
```

```cpp
// iter/Sources.cppm
export module Miracle.Iter:Sources;
import std;
import :Forward;
import :Detail;

// Existing source-view definitions and their exact iterator/sentinel behavior.
```

```cpp
// iter/Adaptors.cppm
export module Miracle.Iter:Adaptors;
import std;
import :Forward;
import :Detail;
import :Sources;

// Existing Scan/Intersperse/MapWindows/ArrayChunks definitions.
```

```cpp
// iter/Pipeline.cppm
export module Miracle.Iter:Pipeline;
import std;
import Miracle.Range;
import :Forward;
import :Detail;
import :Sources;
import :Adaptors;
import :Prelude;
import :Peekable;

// Full Iter<View, Projection> definition and terminal helpers.
// Preserve return categories, pending projections, refs and constraints.
```

```cpp
// iter/Parallel.cppm
export module Miracle.Iter:Parallel;
import std;
import :Pipeline;

// Full ParallelIter implementation, then the out-of-class Iter::parallel()
// definitions. No import from :Pipeline back into :Parallel.
```

```cpp
// iter/Iter.cppm
export module Miracle.Iter;

export import :Forward;
export import :Sources;
export import :Adaptors;
export import :Prelude;
export import :Peekable;
export import :Pipeline;
export import :Parallel;
export import :Factories;
// Do not automatically export-import :Detail if it is internal only.
```

**Note on partition declarations:** some helper declarations originally have module-private linkage, while some type forward declarations are explicitly exported. You must preserve actual export visibility, not just file boundaries. A helper declaration that is required by `:Pipeline` may need a carefully chosen reachable interface declaration in `:Detail`. Importability into the entire module and exposure to external consumers are separate questions; test both. Because every public interface partition participates in the installed `CXX_MODULES` dependency graph, include it in the package file set even when its declarations are not re-exported as a public facility.

**Prefer staged extraction:** first separate `Forward`, `Detail`, and source/view definitions; compile. Next extract `Peekable`. Next extract `Pipeline` and parallel/factories, compile again. If a split forces leakage of many private helpers or reflection lookup differences, keep those layers together in a single interface partition and record why.

### 8.4 Iter semantics that splitting must not accidentally change

The original source specifically defines **adaptive invocation priority**:

1. Invoke callable with the logical item directly when `std::invocable<Function, Item>`.
2. Otherwise try tuple-like decomposition via standard tuple protocol.
3. Otherwise try aggregate field decomposition using `std::meta::nonstatic_data_members_of` and the current access context.
4. Only then report a deliberate incompatibility.

Do not reorder these branches, replace them with eager `std::invoke_result_t`, or move reflection queries into a different declaration context without checking their access behavior. The `reflectedMembers<Object>` cache captures an access context and participates in aggregate fallback; a careless move across partitions can change what is observable.

The main Iter class also maintains a **pending projection** and exact logical reference category. `map`, `filter`, terminals, `Peekable` (borrowed vs owned optional payload), `ArrayChunks`, and parallel-policy terminals can change lifetime or copy/move semantics if split helpers become different overload sets. Relevant tests should cover move-only item types, borrowed lvalue views, rvalue-owned optional sources, `Option<T&>`/terminal references, lazy callable invocation count, `SizeHint` and `infinity`, and parallel policy constraints.

`std::ranges::enable_view` / `enable_borrowed_range` specializations deserve explicit direct-import/out-of-tree tests:

```cpp
import std;
import Miracle.Iter;

// Use concrete types instantiated from Miracle's public iter factories;
// test std::ranges::view, borrowed_range where specified,
// and that mutable/const range APIs remain unchanged.
```

A `std::ranges` specialization that becomes visible **after** the corresponding concept was first checked may produce an inconsistent result or a compiler diagnostic. Keep its declaration order materially equivalent to the original interface.

### 8.5 `Miracle.Feature`: dependency-aligned decomposition

`Feature.ixx` is about 1,365 lines and has clear layers:

| Existing region (approx.) | Symbols | Candidate partition |
|---|---|---|
| 1–137 | `OptionKind`, selectors, hash/capability helpers | `:Vocabulary` / `:Detail` |
| 138–655 | `Version`, `CapabilitySet`, option tokens, `Descriptor`, `dependsOn`/`conflictsWith`/`implies`, `Group` | `:Descriptor` |
| 656–830 | `BuildFeatureSet`, `buildSet`, `ResolutionReport`, `FeatureSet` | `:Set` |
| 831–1277 | `Catalog` graph closure, validation, dependency/conflict/capability resolution, `catalog` | `:Catalog` |
| 1279–1365 | `Requirement` and related helper templates | `:Requirement` |

Recommended import order:

```text
:Vocabulary → :Descriptor → :Set → :Catalog → :Requirement
                                          │
                                    Miracle.Feature primary
                                          │
                                 Miracle.FeatureConfiguration
```

This is one **named module**, `Miracle.Feature`, with flat partitions. The `Catalog` owns the immutable graph and resolution algorithm. Avoid reconstructing it separately in `Requirement` or FeatureConfiguration. `ResolutionReport<Set, MessageCapacity>` currently stores `StaticString<MessageCapacity>`; preserve that exact result type and its capacity derivation when adopting Format layout. Existing resolver methods:

```cpp
// Existing core pattern; keep behavior, not necessarily exact spelling.
constexpr auto report = analyze<Requested...>(BuildFeatures, detectedCapabilities());
static_assert(report.succeeded(), report.message);
```

If you normalize message text through Format, convert the finished textual representation back to the existing report storage in Feature (Format must not import StaticString). A **Feature-local** adapter can use the existing public structural storage, without changing the public report type:

```cpp
// Feature-local sketch. Public BasicStaticString storage/length are intentionally
// structural for NTTP use; avoid making this adapter part of the public API.
template<std::size_t Capacity>
constexpr auto asStaticString(const Miracle::format::Message<Capacity>& source)
    -> Miracle::StaticString<Capacity> {
    Miracle::StaticString<Capacity> result{};
    for (std::size_t i = 0; i < source.size(); ++i) {
        result.storage[i] = source.data()[i];
    }
    result.length = source.size();
    result.storage[result.length] = '\0';
    return result;
}
```

This works only if the **original structural storage invariants** are preserved; before adopting it, prefer an existing public construction API if one can represent the active length correctly. The existing constructor accepting `std::array<Char,N>` sets length to `N`, so blindly copying a partially filled `Message<Capacity>` into an array constructor would be wrong. The `ResolutionReport` capacity also must be checked against any new header/context characters. Keep failure categories and error selection/cycle diagnostic content unchanged. Test duplicate names, missing dependencies, required capability rejection, conflicts, default-enabled closure, build/semantic kind distinctions, and deterministic ordering.

`Catalog` is a large coherent template. If splitting it creates heavy cross-unit dependencies, retain `Catalog` as a single partition and split only vocabulary/descriptors/requirements. This is valid and arguably more maintainable than a forced half-Catalog split.

### 8.6 `Miracle.Meta:Query`: split only at real semantic boundaries

`Meta.Query` is already a **partition of independent named module `Miracle.Meta`**, and about **2,191 lines**. It does **not** need to become a standalone `Miracle.Query` module in Phase M. The risk here is high because declaration-point-sensitive reflection, structural program identity and caching interact.

Actual source regions from `meta/Query.ixx`:

| Approx. lines | Existing mechanisms | Optional new same-module partition |
|---|---|---|
| 1–207 | `Instruction`, homogeneous opcode, persistent `ProgramNode`, append and local rewrite rules | `Miracle.Meta:QueryProgram` |
| 208–459 | `SegmentNode`, `SegmentPlan`, segment compilation/indices, `EvaluationStatePlan` | `Miracle.Meta:QueryPlan` |
| 460–1000 | capture environment, callable binding, source adapters, optional/reference result types, materialization checks | `Miracle.Meta:QueryState` |
| 1009–1476 | typed segment execution, compiled evaluator dispatch, finalization | `Miracle.Meta:QueryEval` |
| 1477–2190 | exported `Query<State>` façade, operators, terminal and creation functions | preserve `Miracle.Meta:Query` |

Suggested dependency direction if—**and only if**—tests establish that separation is safe:

```text
Meta:Types / Meta:Cache / Meta:Reflect
                   │
           :QueryProgram
                   │
             :QueryPlan
                   │
            :QueryState
                   │
             :QueryEval
                   │
          :Query (public façade)
                   │
          Miracle.Meta primary
```

**Hazards worth testing before any split:**

- `ProgramNode` is reflected and its *structural identity* is deliberately used as a persistent program representation. Do not substitute heap vectors or global interning.
- Value-level rewrite rules are conservative; e.g., `Take(0)` must **not** erase a previously observable predicate/callable prefix. Do not normalize across typed segment boundaries incorrectly.
- `State` contains source, capture environment, program, and segment plan; ordinary lvalue Queries copy state while rvalue Queries move. Preserve move-only closure support.
- `evaluateState` finalizes the open segment then dispatches the same compiled evaluation. Do not change execution order, result cardinality, or retry/splicing timing.
- `std::meta::define_static_array` and cache category helpers preserve their existing observation-point semantics. Reflection identity/caching tests must be rerun after changes to declaration placement.
- Helper templates used in public `Query` bodies must remain reachable to installed consumers, including custom `T` and move-only callables.

**My recommended choice for Phase M:** First modernize the file extension and import paths for Query while leaving its core in one partition. Then optionally extract **one** semantically independent layer (e.g. the immutable value IR) only if the dependency proof, semantic snapshot tests, and compile benchmarks justify it. The contract doesn't require splitting Query merely because it is long. A large, stable template-heavy unit with a good internal section structure is better than a fragile multi-partition design.

### 8.7 `StaticString` and Diagnostics: optional, smaller splits

**StaticString**: keep `BasicStaticString` and `StaticStringAccess` in the primary interface to avoid fragmenting structural NTTP mutation semantics. If useful, separate `:Algorithms` (split and string algorithms) and `:Formatting` (argument handling and public `staticFormat`) *after* defining which helpers must be reachable. Keep `std::formatter` specialization visible to direct importers. The private `StaticStringAccess` bridge must not become a new exported public mutable API.

**Diagnostics**: favor `:Codes` (annotation types and code validity), `:Model` (spans/diagnostic values), and a small primary interface. Keep runtime rendering in `module Miracle.Diagnostics;` `.cpp`. Verify `DiagnosticCode` and `DiagnosticSpan` are visible from a direct `import Miracle.Diagnostics;`, and don't accidentally expose internal `makeDiagnosticCodeDescriptor` helpers as new public API solely for cross-partition compilation.

### 8.8 Measuring whether the split was worthwhile

Collect (a) **module import depth**, (b) BMI count/bytes, (c) clean build critical path and compiler memory, and (d) rebuild invalidation after editing `Iter` implementation vs interface. If extra partitions create a long serial chain or huge imported BMI transitive graph, merge them back without changing semantic ownership. No fixed line-count threshold or performance-improvement percentage is contractually required.

**M.6 exit:** named-module cohesion preserved, concrete partition DAG recorded, no template-instantiation failures from installed clients, no Meta Query behavioral changes, and performance measured. Record every deliberate non-split with a short technical reason.

---
## 9. M.7 — Pure umbrella, CMake installation, and direct-import consumers

### 9.1 The target umbrella is intentionally boring

[R] `Miracle.cppm` owns no feature declarations, static caches, helper classes, or definitions. Its only substantive declarations are `export import` of accepted public named modules. Suggested final inventory (reconcile with the local source tree and enabled feature set):

```cpp
// src/public/Miracle.cppm
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
export import Miracle.Concepts;
export import Miracle.Memory;
export import Miracle.Profiling;
export import Miracle.Bitflags;
export import Miracle.Debug;
export import Miracle.Meta;
export import Miracle.Build;
export import Miracle.Capabilities;
export import Miracle.FeatureConfiguration;
```

`Miracle.Types` may already re-export Numeric, but the explicit umbrella entry advertises the standalone public component; this is okay so long as the same declarations have only one owning module. Generated and optional facilities should match their actual feature/availability contracts. No component is permitted to `import Miracle;` internally—doing so destroys the DAG and independent-import economics.

### 9.2 One CMake target containing multiple named modules

`Miracle::Miracle` remains the consumer link identity. A named C++ module is **not** a CMake target, Rust crate, or separate binary by default:

```cmake
add_library(Miracle STATIC)
add_library(Miracle::Miracle ALIAS Miracle)

target_compile_features(Miracle PUBLIC cxx_std_26)
set_target_properties(Miracle PROPERTIES
  CXX_SCAN_FOR_MODULES ON
  CXX_MODULE_STD ON
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)

target_sources(Miracle
  PUBLIC FILE_SET miracle_modules TYPE CXX_MODULES
  BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/public"
            "${miracle_generated_dir}"
  FILES
    src/public/Miracle.cppm
    src/public/numeric/Numeric.cppm
    src/public/types/Types.cppm
    src/public/format/Format.cppm
    src/public/static_string/StaticString.cppm
    src/public/range/Range.cppm
    src/public/iter/Iter.cppm
    # All actual public interface partitions required by the graph ...
    src/public/diagnostics/Diagnostics.cppm
    src/public/feature/Feature.cppm
    src/public/meta/Meta.cppm
    # All existing Meta interface partitions ...
    "${miracle_generated_dir}/Build.cppm"
    "${miracle_generated_dir}/Capabilities.cppm"
    "${miracle_generated_dir}/FeatureConfiguration.cppm"
    ${MIRACLE_BUILD_FEATURE_SOURCES}
  PRIVATE
    src/private/diagnostics/Diagnostics.cpp
    src/private/error/Error.cpp
    src/private/panic/Panic.cpp
    src/private/fs/Fs.cpp
    src/private/profiling/Profiling.cpp
    "${miracle_memory_source}"
)
```

This intentionally omits several entries for readability; **do not paste it in place of the actual full `CMakeLists.txt`**. Retain existing install/export commands, feature-gated public/private link libraries and generated outputs. In particular, install the `miracle_modules` file set and its CMake module metadata for downstream `find_package` usage; normal in-tree compilation is insufficient evidence of package correctness.

A **public** module interface must not depend on a partition omitted from the installed module source/metadata graph. If you introduce a `:Detail` interface partition needed by a public template body, verify it is exported/installable in CMake's appropriate interface file set even if it provides no exported public API. An ordinary non-importable `.cpp` implementation unit can remain private. CMake file-set visibility and C++ declaration export are different mechanisms.

### 9.3 Consumer matrix with concrete smoke programs

Use minimal executables to test both visibility and linkage:

```cpp
// tests/consumer/granular/numeric.cpp (example)
import Miracle.Numeric;
import std;

static_assert(std::same_as<Miracle::u64, std::uint64_t>);
static_assert(std::same_as<Miracle::usize, std::size_t>);
int main() { Miracle::u32 value{42}; return value == 42 ? 0 : 1; }
```

```cpp
// tests/consumer/granular/format.cpp (example)
import Miracle.Format;

constexpr auto message = Miracle::format::Builder<128>{}
    .error({"FMT", 1, 3}, "format import smoke")
    .build();
static_assert(message.size() != 0);
int main() { return 0; }
```

```cpp
// tests/consumer/granular/iter.cpp (example)
import Miracle.Iter;
import std;

int main() {
    // Use the real public iter(...) factory and its current terminal API.
    // Create at least one custom user-defined element/callable template
    // instantiation to catch missing partition reachability.
    const auto count = Miracle::iter(0, 4).count();
    return count == 4 ? 0 : 1;
}
```

The `count()` fixture is **illustrative**; verify the actual current method name/signature before committing a test. Do not use the absence of an API as evidence of a module problem. Add analogous consumers for `Miracle.Diagnostics`, `Miracle.Feature`, `Miracle.Meta`, generated `Build/Capabilities/FeatureConfiguration`, and every other accepted public module. One consumer must import **only** its selected Miracle named module (plus `std`) to prove no hidden dependence on first importing the umbrella.

Use all three supported CMake ingestion paths:

```bash
# With pinned toolchain active and after building the library:
cmake -S tests/consumer/add-subdirectory -B build/m-add -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE" \
  -DMIRACLE_SOURCE_DIR="$PWD"
cmake --build build/m-add --parallel 2

cmake -S tests/consumer/fetch-content -B build/m-fetch -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE" \
  -DFETCHCONTENT_SOURCE_DIR_MIRACLE="$PWD"
cmake --build build/m-fetch --parallel 2

cmake --preset release --fresh \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
cmake --build --preset release --parallel 2
cmake --install build/release --prefix "$PWD/build/m-install"

cmake -S tests/consumer/find-package -B build/m-package -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE" \
  -DCMAKE_PREFIX_PATH="$PWD/build/m-install"
cmake --build build/m-package --parallel 2
```

Run the produced consumer executables. Repeat with the complete `import Miracle;` umbrella. A link failure or missing template instantiation on *installed* clients is a Blocker even if an in-tree import succeeds.

### 9.4 Install-cleanliness and ABI checks

Do a clean build after each **ownership** migration. Do not reuse BMIs built under old names. Confirm that no public declaration is defined both in `Miracle:OldPartition` and `Miracle.NewName`, and that no old implementation unit remains in `module Miracle;`. Existing namespace `Miracle::` spellings remain source-compatible, but named-module linkage/ABI identity can legitimately change; do not promise binary compatibility with old compiled clients.

**M.7 exit:** every public named module imported individually through all supported package paths, `import Miracle;` works, and `Miracle::Miracle` remains the link target. Record the actual per-module consumer results and commit.

---

## 10. M.8 — Exact verification suite and final author handoff

### 10.1 Positive and negative compiler tests (do not replace with runtime tests)

Compile-fail tests require **compiler rejection**, correct intended failure location, and stable **Miracle-authored body**. Treat compiler-generated headers, colors and template backtraces as outside Miracle's byte-for-byte guarantee.

| Area | Positive / negative fixture | What to verify |
|---|---|---|
| `Format` | literal, exact capacity, one-past-capacity, code width | no truncation; bounded compile-time output; `size()/data()` valid |
| `StaticString` | trivial char, invalid nontrivial char, malformed braces, mismatched placeholders | same conditions as baseline, richer authored messages |
| `Iter` | direct callable, tuple-like fallback, aggregate fallback, no viable path | direct > tuple > aggregate order; no eager failed branch; type/context details |
| `Iter` ownership | lvalue borrowed range, rvalue optional, move-only element, `Peekable` mutation | no dangling references, copies, altered terminal return categories |
| `Feature` | catalog closure, conflict, missing capability, cycle, duplicate option | same error categories, ordering, `ResolutionReport` semantics |
| `Diagnostics` | valid annotated enum; invalid domain marker/alignment; runtime span | unchanged `DiagnosticCode` and rendering behavior |
| `Meta.Query` | capture, typed segments, `Take(0)`, aggregation, materialization, access-sensitive reflection | same lazy execution, constant-evaluation identities, cache behavior |
| Modules | direct import of each component, one custom templated client | no hidden umbrella dependency, no private missing partition |
| Generated facts | enabled/disabled features, platform capability identity | same value and configuration gating |
| Platform units | platform-selected Memory implementation owners | no wrong-owner source; test Windows when available |

A useful compiler-negative test harness (recommended; keep expected texts as dedicated `.txt` or test metadata):

```python
# scripts/tests/compile_fail.py — concise test harness sketch
import subprocess
import sys

# Obtain the exact compiler+flags from a configure-time generated command or
# the pinned CMake compilation database. Don't guess stdlib/linker settings.
command = sys.argv[1:]
completed = subprocess.run(command, text=True, capture_output=True)
output = completed.stdout + completed.stderr
assert completed.returncode != 0, "expected compiler rejection"
assert "error[ITR004]:" in output, "Miracle diagnostic header not found"
# Add local assertions for context/help and which branch was selected.
```

Use per-test expected bodies and avoid one hardcoded `ITR004` requirement for unrelated failures. Run *positive* fixtures too: a failure-only diagnostic builder must not instantiate on a successful condition. Compile-time and runtime test outcomes must be logged separately.

### 10.2 Full reference build and tooling checks

```bash
# Only after toolchain activation with matching libc++:
cmake --preset debug --fresh \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
cmake --build --preset debug --parallel 2

cmake --preset release --fresh \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
cmake --build --preset release --parallel 2

cmake --preset tests --fresh \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
cmake --build --preset tests --parallel 2
ctest --preset tests --output-on-failure

python3 scripts/ci/audit-codeql-coverage.py --self-test
python3 scripts/docs/verify-frozen-contracts.py --self-test
python3 scripts/docs/verify-frozen-contracts.py

# Only in the configured pinned developer container as originally required:
bash scripts/ci/verify-container.sh
nvim --headless --clean -c 'luafile scripts/ci/verify-nvim-lsp.lua'

# With the repository's pinned mdBook executable installed:
mdbook build
python3 scripts/docs/verify-site.py site
python3 scripts/docs/verify-links.py site
```

The documentation navigation for Phase M must appear in `docs/SUMMARY.md`, because the baseline documentation CI failed precisely for missing `phases/M/DESIGN.md` navigation. `verify-frozen-contracts.py` must continue to protect historic frozen artifacts; **do not modify them to make the new architecture appear historically approved**.

CodeQL coverage is a **known limitation**: the historical report extracted 0/33 `.ixx` module interface sources. Switching to `.cppm` doesn't prove the extractor understands modules or reflection. Update the file inventory, and if an authenticated new CodeQL extraction export is available, compare it carefully; do not turn unsupported semantic analysis into a fake gate.

### 10.3 Compiler and platform evidence

Primary `master` validation uses the pinned `clang-cxx26` + matching libc++. Do not claim that C++26 reflection behavior is portable to GCC merely because a GCC branch builds a subset. Record Windows build checks only if actually run; the known `module Miracle.Core;` mismatch can be **source-reviewed** without pretending Windows compilation occurred. Compiler limitations and library defects are different categories in the review dossier.

### 10.4 Matched performance comparison

Re-run the M.1 benchmark harness after M.8 on the same machine/toolchain and equivalent build flags:

```text
Metric                              Baseline            After M            Delta / explanation
----------------------------------  ------------------  -----------------  -------------------------
Clean full Debug build              <raw samples>       <raw samples>      <median/range>
Clean full Release build            <raw samples>       <raw samples>      <median/range>
Iter implementation change rebuild  <raw samples>       <raw samples>      <invalidated units>
Iter public interface edit rebuild  <raw samples>       <raw samples>      <invalidated units>
Diagnostics implementation rebuild  <raw samples>       <raw samples>      <invalidated units>
Format interface edit rebuild       N/A new module      <raw samples>      <downstream fan-out>
Minimal Numeric import consumer     N/A new module      <raw samples>      <absolute comparison>
Umbrella import consumer            <raw samples>       <raw samples>      <median/range>
Meta.Query compile/constexpr stress <raw samples>       <raw samples>      <same fixtures>
Peak RSS / BMI count and bytes      <raw samples>       <raw samples>      <measured>
```

For changes that have **no exact pre-migration counterpart** (e.g. `Miracle.Numeric` direct import), report the new absolute cost and a defensible comparison, not a fictional speedup. If a material regression occurs, investigate partition chain depth, unnecessary re-exports, and duplicated compiler work. Record an architectural explanation or propose a fix/author exception before acceptance.

### 10.5 Evidence package and batched review procedure

Write `docs/phases/M/PROGRESS.md` as a **compact operational ledger**, not a diary. For each milestone M.1–M.8, record start/end commit, implementation status, local test commands/results, known failures, and formal review status. Store useful raw evidence under `docs/phases/M/evidence/` where appropriate; avoid committing huge binary artifacts, generated BMIs, or secrets.

Recommended M.8 handoff dossier:

```text
docs/phases/M/
├── DESIGN.md                  # authoritative architecture contract
├── IMPLEMENTATION.md          # this guide, once author-approved/published
├── PROGRESS.md                # M.1–M.8 commit map, statuses, next action
└── evidence/
    ├── module-inventory-before.json
    ├── module-inventory-after.json
    ├── import-graph-before.dot
    ├── import-graph-after.dot
    ├── compiler-identity.txt
    ├── import-consumers.md
    ├── diagnostic-snapshots/
    ├── regressions.md
    ├── build-baseline.csv
    ├── build-after.csv
    └── M8-verification.md
```

The exact filenames are **[P]**, not required by the Design Contract; the *evidence content* is required. If one program can prove an invariant without multiple duplicate files, use the simpler presentation.

At final handoff, provide:

```text
Branch/PR: <branch / PR URL>
Baseline revision: <SHA>
Final revision: <SHA>
Milestone commits: M.1=<sha>, M.2=<sha>, ... M.8=<sha>
Author implementation status: implementation-complete / review pending
Required evidence: docs/phases/M/evidence/
Known deviations/unsupported tests: <explicit list>
Request: batched Protocol review of M.1–M.8, separate milestone outcomes,
         severity-ranked findings, and overall Phase M acceptance recommendation.
```

Per Protocol §8, the reviewer must return **Accepted**, **Changes requested**, **Verification pending**, or **Contract revision required** for **each milestone**, with findings classified **Blocker / Major / Minor / Observation**. Your subsequent fixes remain author-owned. A combined review does not bypass contract amendment/author sign-off when a real semantic change is proposed.

---

## 11. Design decisions, risk classification, and explicit deferrals

### 11.1 Decisions already established by the Phase M contract **[R]**

- C++26-first, pinned reference compiler; one initial `Miracle::Miracle` library target.
- `Miracle` is a pure re-exporting umbrella; independent `Miracle.<Component>` named modules.
- `.cppm` for interfaces/partitions, `.cpp` for implementation and ordinary C++ source, `.cppm.in` for generated interfaces.
- `Miracle.Numeric` contains scalar numeric aliases only; `Miracle.Types` retains existing residual aliases.
- `Miracle.Format` imports **zero Miracle modules**, builds deterministic bounded C++26 assertion messages, and provides canonical Miracle-authored diagnostic layout.
- Existing Diagnostics, Iter, Feature, Meta Query behavior and reflection-sensitive execution are preserved.
- Format's generic extension/formatting language is deferred to Core Revisit unless a scoped earlier approved need is demonstrated.
- No arbitrary minimum number of partitions, file-size target, or promised percentage build speedup.

### 11.2 Recommendations that the author can refine **[P]**

- Exact `Format` public factory names, builder API, and whether its internal message buffer is `Message<Capacity>` or an equivalent structural representation.
- The number of Iter interface partitions and whether `Peekable`/`Parallel`/`Factories` merit separate units.
- Feature `:Vocabulary/:Descriptor/:Set/:Catalog/:Requirement` partition boundaries.
- Any Meta Query decomposition beyond preserving the existing `Miracle.Meta:Query` unit.
- Whether implementation-private diagnostic IDs are enums, constants, or equivalent domain-owned values.
- Performance harness implementation and evidence filenames.

**A recommended implementation is not an API freeze.** Changing it requires demonstrating equivalence against *required* behavior; changing a required architectural invariant requires a contract amendment.

### 11.3 Blockers to avoid

| Failure | Why it is serious | Required correction/proof |
|---|---|---|
| `Miracle.Format` imports `Miracle.StaticString` | bootstrap cycle / contract violation | remove the dependency; keep its own minimal char buffer |
| Former `Miracle:Diagnostic` and new `Miracle.Diagnostics` both define public symbols | conflicting ownership/ODR or ABI confusion | one actual owner per exported entity |
| `:Forward` `enable_view` specialization appears after concept use | changes range-model satisfaction | restore visibility order; direct-import tests |
| `Miracle.Iter:Pipeline` imports `:Parallel`, and `:Parallel` imports `:Pipeline` | partition cycle | forward-declare `ParallelIter`, out-of-class definitions in later partition |
| `std::meta` reflection helpers change access point/observation timing | compile-time semantic regression | preserve evaluation context; run Meta/Iter snapshot tests |
| `Feature` report type or error selection changes | violates approved semantics | maintain existing `ResolutionReport` shape and resolver behavior |
| `DiagnosticCode` concept changed to eager assertion | breaks substitution/capability queries | preserve concept checking; report richly only at hard enforcement |
| Template definitions moved to `.cpp` and inaccessible to clients | out-of-tree instantiation/link failure | keep reachable in interface partitions; test custom templates |
| Windows Memory still declares `module Miracle.Core;` | incorrect owner relative to `Miracle.Memory` | correct owner and verify platform status |
| `.cpp` omitted by security/source inventory | silent gap in reported coverage | update parser's module/implementation suffix sets |
| Installed package lacks an imported public partition | in-tree pass hides public failure | install + `find_package` direct import smoke |
| Generated source or feature gating changed accidentally | wrong build identity / optional feature inclusion | diff configured outputs and enabled/disabled tests |
| Documentation misses Phase M navigation | known baseline docs failure persists | update SUMMARY and run `verify-site.py` |
| No baseline measurements | cannot evaluate M performance goal | record before M.2 or document explicit inability/exception |

---

## 12. Commit structure, milestone acceptance, and final definition of done

One *primary* commit per milestone is a reasonable author workflow; additional fixups should be traceable to their parent milestone. Suggested commit subjects (examples, not prescribed):

```text
docs(architecture): record Phase M baseline and module graph       [M.1]
refactor(build): migrate interfaces to cppm and sources to cpp      [M.2]
refactor(core): extract numeric, types, format and static string    [M.3]
refactor(diagnostics): extract module and normalize errors          [M.4]
refactor(modules): extract remaining Miracle components             [M.5]
refactor(iter): decompose template-heavy module interfaces         [M.6]
refactor(modules): finalize umbrella and installed imports         [M.7]
test(migration): record Phase M evidence and review handoff         [M.8]
```

**Milestone ready for final review** means its implementation commit exists, tested local behavior and deviations are recorded, and evidence can be retrieved at that revision. **Milestone Accepted** requires its formal review outcome and author approval, even when all eight are reviewed in one session. **Phase Accepted** requires all blocking criteria in Design Contract §10 to pass or have explicit approved exceptions, an identified source revision, and the author's final sign-off. A release tag is a separate decision.

**Suggested final checklist (author-facing):**

```text
[ ] M.1 source/graph/benchmark baseline recorded at an exact commit
[ ] M.2 every interface and C++ file renamed, all references/tooling updated
[ ] M.3 Numeric/Types/StaticString/Format and generated foundations work separately
[ ] M.4 Diagnostics migrated; authored compiler messages use common grammar
[ ] M.5 remaining named modules + implementation owners migrated
[ ] M.6 Iter/Feature/Meta splitting decisions documented and tests pass
[ ] M.7 umbrella re-exports complete; granular/installed clients pass
[ ] M.8 clean builds, compile-fail tests, Meta regressions, docs, CI, perf evidence
[ ] No unexpected public API/semantic changes or unresolved module cycles
[ ] Final SHA and per-milestone commits recorded; formal batched review requested
```

### Source references used to prepare this guide

- `docs/phases/M/DESIGN.md` — Phase M Blueprint Contract v1.0 (baseline SHA above).
- `docs/DEVELOPMENT_PROTOCOL.md` — author ownership, guidance standard (§7), verification (§8), progress (§11), approval (§9).
- `docs/architecture.md`, `docs/compiler-support.md` — package, semantics and C++26 policy.
- `CMakeLists.txt`, `cmake/Capabilities.cmake`, `cmake/Features.cmake` — source/file-set generation and probing.
- `src/public/{Types,StaticString,Range,Iter,Feature,Diagnostic,Error,Panic,Fs,Memory,Profiling,Concepts}.ixx` — current public implementations.
- `src/public/meta/{Meta,Types,Cache,Reflect,Query,Bitflags,Debug,...}.ixx` — reflection/Query and misplaced independent components.
- `src/private/*.cxx`, `src/private/{linux,windows,unsupported}/Memory.cxx` — runtime owners.
- `docs/infrastructure/G4-PROGRESS.md`, `scripts/ci/audit-codeql-coverage.py`, `scripts/ci/verify-nvim-lsp.lua`, `docs/SUMMARY.md` — documented extension and tooling implications.
- C++26 static assertions, C++ module partition rules and CMake `CXX_MODULES` behavior are **language/build-system background**, while the file inventory and current API mappings above derive from the inspected Miracle source.

*Technical Implementation Blueprint v1.1. Authored for the developer's implementation, not committed to GitHub, and not evidence of any completed compile, migration, benchmark or milestone review.*
