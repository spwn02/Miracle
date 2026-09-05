# Build capabilities

Miracle validates its selected C++26 toolchain with executable capability probes. For now it exposes those already-computed results to C++ code without leaking build system macros into the public API.

## Module surface

`import Miracle;` re-exports the generated `Miracle:Capabilities` partition. Capability values live in:

```cpp
Miracle::capability
```

and are `inline constexpr bool` values:

```text
importStd
reflectionCore
reflectionQueries
reflectionAnnotations
reflectionStaticStorage
expansionStatements
stdVocabulary
stdHive
```

Example:

```cpp
import Miracle;

static_assert(Miracle::capability::reflectionCore);
```

## Why the values are generated

CMake probes the selected compiler/toolchain once and stores the results in its capability cache plus `MiracleCapabilities.json`. The normal Miracle configure step then translates those booleans into the generated module partition.

C++ consumers therefore reason about ordinary types constants rather than:

- compiler-version allowlists;
- vendor-specific feature macros;
- duplicated compile probes;
- public preprocessor configuration;

This is the bootstrap bridge consumed by the Feature engine. Feature resolution adapts these facts into `feature::CapabilitySet` and combines them with dependency, conflict, group, and build-universe reasoning. See [`feature.md`](feature.md).

## Current master behavior

Existing pre-contract Miracle modules still require the capabilities their current implementations use. The Feature engine does not retroactively make legacy modules optional before their dedicated redesign phases. New build-selectable facilities can register capability requirements and heavy sources through the Feature engine now.

A feature capability failure is therefore distinct from a capability that remains a hard requirement of an existing required module. The former is diagnosed by feature resolution; the latter continues to reject the selected toolchain during configuration.

## Build identity

Capability results describe the exact configured Miracle build. They are generated from the same probe run as the build and are not recomputed in consuming translation units. The generated `feature::buildIdentity` now hashes the canonical build-feature universe together with these capability results so incompatible configurations do not share the same generated module source identity.
