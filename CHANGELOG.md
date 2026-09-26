# Changelog

All notable user-visible changes to Miracle are documented here.

The project is currently pre-1.0 and follows semantic versioning for release numbering.

## Unreleased

### Added

- Added lazy `Iter` pipelines with universal range/integer/Option/Result construction, adaptive tuple/reflected-aggregate callable invocation, positional slicing, Rust-inspired adaptors and terminals, and standard-range capability preservation.
- Added standard-backed `empty`, `once`, `repeat`, and `repeatN` Iter sources plus allocation-free `onceWith`, `repeatWith`, `fromFn`, and `successors` generated sources.
- Added `copied`, `cloned`, and forward-range `cycle` adaptors; reverse `nthBack`, `rFind`, and `rFold` terminals; and C++-native lexicographical `compare`.
- Added the iterator-parity surface: stateful `Peekable`, public semantic adaptor values, free `chain`/`zip`, optional-seed `successors`, `std::generator` interoperability, selected Rust-nightly `intersperseWith`/`mapWindows`/`arrayChunks`/`eqBy`, and explicit Rust-to-Miracle parity/defer/divergence documentation.
- Added Meta vocabulary with the standalone `Miracle.Meta` module, `Reflect<T>`/`reflect<T>()`, strict C++26 reflection sources and transformations, composable reflection predicates.
- Added explicit `ParallelIter` execution as a thin façade over standard C++ execution policies for indexable pipelines.
- Added nonnumeric `Infinity`/`infinity` positional-bound vocabulary and conservative `SizeHint` iterator size knowledge.
- Added structural finite half-open `Range<T>` views with safe signed sizing, standard range interoperability, and conservative endpoint deduction.
- Added reflected enum diagnostic domains, structured move-only diagnostics/spans, source-aware rendering, Error and C++26 contract adapters, and terminal `panic()` integration with stacktraces and debugger breakpoints.
- Added executable capability probes for contracts, debugging, stacktraces, UTF-8 literal encoding, and typed reflection-annotation extraction.
- Added the structural compile-time Feature engine with build/semantic descriptors, dependency/implication/conflict resolution, feature groups, capability requirements, canonical local sets, third-party metadata, and reflection-friendly requirements.
- Added a build-system feature that omits disabled heavy module sources and dependencies and generates the typed configured `BuildFeatureSet`, capability universe, and build identity.
- Added structural `BasicStaticString<Char, Capacity>` / `StaticString<Capacity>` with NTTP support, searching, trimming, case conversion, replacement/removal, split/join, hashing, compile-time diagnostic formatting, and `std::formatter` integration.
- Exposed executable toolchain probe results as generated `Miracle::capability` compile-time facts consumed by the Feature engine.

### Changed

- Promoted reference validation and release provenance to the LLVM 22.1.8-synchronized `cxx26-2026.09.19.1` toolchain snapshot (`71ba1036c652094d4869f34e0ba7fb5c756f4399`).
- Reworked Miracle-owned Iter range types around composition and structural `std::ranges::view` conformance rather than `view_interface`/CRTP inheritance, and modernized borrowed optional terminal results to C++26 `Option<T&>` semantics.

### Fixed

- Preserved reserve-range emptiness for positively-strided integer `iter(...)` construction and made fused map/filter traversal valid for non-common ranges.
- Preserved move-only ownership through rvalue Option/Expected sources, `filterMap`, `mapWhile`, `scan`, and `unzip` materialization.
- Made `count()` traverse lazy pipelines so projections and `inspect()` side effects are observed, and cached extrema keys so key projections execute once per visited item.
- Made `rposition()` search and short-circuit from the back, aligned `max`, `maxBy`, and `maxByKey` with last-equivalent-maximum selection.
- Replaced the forward-only identity `peekable()` façade with a real one-item lookahead state machine that supports single-pass sources, borrowing, mutable peeking, conditional consumption/mapping, and truthful double-ended access where available.

### Performance

- Simplified Iter to delegate ordinary traversal to standard views/algorithms, removing the custom exhaustion lattice, worker pool, bounded-concurrency protocol, and custom executor runtime.
- Composed consecutive Iter maps into one pending projection and fused projected-filter terminals so mapped values are evaluated once per source item without `cache_latest` capability loss.
- Preserved random-access and sized traversal through chunks, windows, and intersperse when the source supports those capabilities.
- Kept callable-generated sources allocation-free with one invocation per logical iter, and implemented `cycle()` by restarting its forward base without buffering.
- Rendered diagnostic cause trees with iterative parent/index DFS in O(n) time, O(depth) auxiliary storage, and O(1) native recursion.
- Resolved local feature sets through cached catalog-order graph indices and a flat constexpr traversal stack, avoiding repeated relationship lookup and dependency-depth call recursion.
- Reused configured capability probe results when generating C++ capability facts instead of introducing duplicate compile-time/compiler probes.

### Diagnostics

- Added configurable diagnostic-code prefixes/messages/alignment, plain/terminal presentation, notes/help, explicit source snippets/selections, causal diagnostics, and normalized unknown runtime codes.
- Diagnosed duplicate features, missing build/dependency/capability requirements, complete dependency cycles, conflicts, disabled façades, and colliding build identifiers at compile time.

## 0.1.0-rc.1 - 2026-08-25

### Added

- Standalone C++26 foundation-library identity: `import Miracle;` / `Miracle::Miracle`.
- Source, FetchContent, and installed-package consumption.
- Result/error, types, filesystem, reflection/meta, bitflags, debug formatting, build facts, profiling, concepts, and memory foundations.
- Switch-powered self-tests without a production dependency on Switch.
- Standalone CI, documentation, examples, and release scaffolding.
