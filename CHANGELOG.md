# Changelog

All notable user-visible changes to Miracle are documented here.

The project is currently pre-1.0 and follows semantic versioning for release numbering.

## Unreleased

### Added

- Added structural `BasicStaticString<Char, Capacity>` / `StaticString<Capacity>` with NTTP support, searching, trimming, case conversion, replacement/removal, split/join, hashing, compile-time diagnostic formatting, and `std::formatter` integration.
- Exposed executable toolchain probe results as generated `Miracle::capability` compile-time facts for the upcoming Feature engine.

### Performance

- Reused configured capability probe results when generating C++ capability facts instead of introducing duplicate compile-time/compiler probes.

## 0.1.0-rc.1 - 2026-08-25

### Added

- Standalone C++26 foundation-library identity: `import Miracle;` / `Miracle::Miracle`.
- Source, FetchContent, and installed-package consumption.
- Result/error, types, filesystem, reflection/meta, bitflags, debug formatting, build facts, profiling, concepts, and memory foundations.
- Switch-powered self-tests without a production dependency on Switch.
- Standalone CI, documentation, examples, and release scaffolding.
