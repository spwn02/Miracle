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

This is the bootstrap bridge for the Feature engine implemented in the future. The Feature engine will add dependency/conflict/group reasoning and optional feature selection. For now it intentionally does not pretend those semantics already exist.


## Current master behavior

The current pre-Feature Miracle source still requires the complete set of capabilities used by its existing public modules. A missing required probe therefore continues to fail configuration exactly as before.

This is deliberate: making reflection-backed modules optional before the Feature engine can consistently select their sources and dependencies would create a half-configured library. For now its only establishes the typed capability substrate; In the future it will consume it to implement real feature selection.

## Build identity

Capability results describe the exact configured Miracle build. They are generated from the same probe run as the build and are not recomputed in consuming translation units. Future configured FeatureSets will become part of the build/BMI identity on top of this substrate.
