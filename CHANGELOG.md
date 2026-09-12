# Changelog

All notable user-visible changes to Miracle are documented here.

The project is currently pre-1.0 and follows semantic versioning for release numbering.

## Unreleased

### Added

- Added structural finite half-open `Range<T>` views with safe signed sizing, standard range interoperability, and conservative endpoint deduction.
- Added reflected enum diagnostic domains, structured move-only diagnostics/spans, source-aware rendering, Error and C++26 contract adapters, and terminal `panic()` integration with stacktraces and debugger breakpoints.
- Added executable capability probes for contracts, debugging, stacktraces, UTF-8 literal encoding, and typed reflection-annotation extraction.
- Added the structural compile-time Feature engine with build/semantic descriptors, dependency/implication/conflict resolution, feature groups, capability requirements, canonical local sets, third-party metadata, and reflection-friendly requirements.
- Added a build-system feature that omits disabled heavy module sources and dependencies and generates the typed configured `BuildFeatureSet`, capability universe, and build identity.
- Added structural `BasicStaticString<Char, Capacity>` / `StaticString<Capacity>` with NTTP support, searching, trimming, case conversion, replacement/removal, split/join, hashing, compile-time diagnostic formatting, and `std::formatter` integration.
- Exposed executable toolchain probe results as generated `Miracle::capability` compile-time facts consumed by the Feature engine.

### Changed

- Promoted reference validation and release provenance to the LLVM 22.1.8-synchronized `cxx26-2026.09.05` toolchain snapshot (`6c7ef6afbfd8456c964c7a2625b3ea2aaa7d613f`).

### Performance

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
