# Changelog

All notable user-visible changes to Miracle are documented here.

The project is currently pre-1.0 and follows semantic versioning for release numbering.

## Unreleased

### Added

- Added the structural compile-time Feature engine with build/semantic descriptors, dependency/implication/conflict resolution, feature groups, capability requirements, canonical local sets, third-party metadata, and reflection-friendly requirements.
- Added a build-system feature that omits disabled heavy module sources and dependencies and generates the typed configured `BuildFeatureSet`, capability universe, and build identity.
- Added structural `BasicStaticString<Char, Capacity>` / `StaticString<Capacity>` with NTTP support, searching, trimming, case conversion, replacement/removal, split/join, hashing, compile-time diagnostic formatting, and `std::formatter` integration.
- Exposed executable toolchain probe results as generated `Miracle::capability` compile-time facts consumed by the Feature engine.

### Changed

- Promoted reference validation and release provenance to the LLVM 22.1.8-synchronized `cxx26-2026.09.05` toolchain snapshot (`6c7ef6afbfd8456c964c7a2625b3ea2aaa7d613f`).

### Performance

- Resolved local feature sets through cached catalog-order graph indices and a flat constexpr traversal stack, avoiding repeated relationship lookup and dependency-depth call recursion.
- Reused configured capability probe results when generating C++ capability facts instead of introducing duplicate compile-time/compiler probes.

### Diagnostics

- Diagnosed duplicate features, missing build/dependency/capability requirements, complete dependency cycles, conflicts, disabled façades, and colliding build identifiers at compile time.

## 0.1.0-rc.1 - 2026-08-25

### Added

- Standalone C++26 foundation-library identity: `import Miracle;` / `Miracle::Miracle`.
- Source, FetchContent, and installed-package consumption.
- Result/error, types, filesystem, reflection/meta, bitflags, debug formatting, build facts, profiling, concepts, and memory foundations.
- Switch-powered self-tests without a production dependency on Switch.
- Standalone CI, documentation, examples, and release scaffolding.
