# Miracle documentation

**Miracle** is a C++26 foundation library for reusable, explicit and composable low-level facilities. Its interfaces draw on Rust's library ergonomics while retaining C++26 modules, standard-library integration and modern C++ ownership semantics.

> **Status: pre-1.0.** The library and its API may change before 1.0. The [`master` branch](https://github.com/spwn02/Miracle/tree/master) targets the project's pinned C++26 reference toolchain; GCC support is maintained separately.

This site is **generated from the canonical [`docs/` directory](https://github.com/spwn02/Miracle/tree/master/docs)**. The authoritative technical contracts, test evidence, and source remain in the Git repository; the website is a read-only presentation of those files, not another place where they are edited.

## Getting started

Use the [README quickstart and build instructions](https://github.com/spwn02/Miracle#quick-start) to import `Miracle` and build the library with CMake. For development, start with:

- [Compiler support](compiler-support.md): the C++26-first `master` policy and GCC compatibility scope.
- [Reference toolchain](reference-toolchain.md): immutable snapshot identity, component coherence and provenance.
- [Build capabilities](capabilities.md): the compile-time capability vocabulary.

The public umbrella module is `import Miracle;`, and the CMake target is `Miracle::Miracle`.

## Existing API guides

These are **maintained prose and usage documentation**, not automatically extracted API declarations.

| Area | Documentation |
| --- | --- |
| Diagnostics and errors | [Diagnostics and panic](diagnostics.md) |
| Reflection | [Meta](meta.md) |
| Iteration | [Iter](iter.md) |
| Ranges | [Range](range.md) |
| Compile-time strings | [Static string](static-string.md) |
| Build-time configuration | [Feature engine](feature.md) |

Documentation does not imply that every proposed or experimental C++26 compiler facility is supported on mainstream toolchains. Refer to the reference compiler requirements and actual CI evidence before relying on a feature.

## Architecture and development

Read [Architecture](architecture.md) for the product boundary. The [Development Protocol v1.0](DEVELOPMENT_PROTOCOL.md) governs future contract-driven engineering phases.

The [GitHub Infrastructure Contract](infrastructure/GITHUB-INFRASTRUCTURE-CONTRACT.md) establishes the repository-governance and publication boundaries. See [Miracle Development](https://github.com/users/spwn02/projects/3) for current tasks, or [infrastructure progress](infrastructure/G2-PROGRESS.md) for the ongoing G0–G6 governance work.

## Contributing, releases and security

The repository is authoritative for [contribution instructions](https://github.com/spwn02/Miracle/blob/master/CONTRIBUTING.md), [support](https://github.com/spwn02/Miracle/blob/master/SUPPORT.md), [security reporting](https://github.com/spwn02/Miracle/blob/master/SECURITY.md), and the [changelog](https://github.com/spwn02/Miracle/blob/master/CHANGELOG.md). New reports and proposals can use the [issue chooser](https://github.com/spwn02/Miracle/issues/new/choose).

**Contract preservation:** Verified historical locked texts are indexed in the [frozen contract archive](contracts/index.md), with byte-level provenance for native Markdown imports and an explicitly labeled Phase 5.4 DOCX reading edition. Historical approvals are not reissued by publication; missing source artifacts and final migration acceptance remain tracked in [Issue #7](https://github.com/spwn02/Miracle/issues/7).
