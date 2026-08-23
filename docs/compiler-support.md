# Compiler support

Miracle is intentionally developed against the complete C++26-and-earlier standard model rather than the intersection of features implemented by today's mainstream compilers.

## Master contract

`master` represents Miracle as it should exist in a future where the complete C++26 language and standard-library surface, together with every earlier C++ standard facility, is implemented and available.

This is deliberately broader than "the capabilities Miracle currently happens to use." Miracle may adopt any standardized facility through C++26 whenever it improves the design. Missing implementation support is a toolchain problem, not a reason to constrain the architecture of `master`.

The consequences are intentional:

- C++26 is the language baseline.
- Facilities from C++26 and every earlier standard may be used when appropriate.
- `master` does not carry compatibility layers merely to preserve support for incomplete compiler or standard-library implementations.
- `master` does not add fallbacks for older C++ language modes.
- Missing standard facilities may be implemented or ported into the reference toolchain while compiler ecosystems catch up.
- No missing standard-library facility is emulated by injecting declarations or implementations into `namespace std`.
- Compiler-specific workarounds belong outside the primary design whenever they would compromise the public API or implementation model.

The question for `master` is:

> What should this library look like when C++26 is fully available?

not:

> What subset of C++26 can every compiler build today?

## Reference toolchain

The current reference implementation is:

```text
compiler:
  https://github.com/spwn02/clang-p2996
  branch: p2996

standard library:
  the matching libc++ tree from the same fork
```

The fork builds on Bloomberg's Clang/P2996 work and is being extended with a focus on compiler stability, real-world reflection-heavy workloads, and progressively broader C++26 libc++ coverage.

"Reference toolchain" does not mean Miracle is permanently tied to one compiler. It means this is the toolchain against which the complete `master` contract is currently validated.

Other toolchains become eligible for `master` support when they implement the required standardized behavior faithfully. Support is ultimately capability-driven rather than vendor- or version-driven.

The compiler alone is not the reference unit: its matching libc++ headers, binaries, ABI runtime, and C++ module sources/metadata belong to the same validated toolchain build. Deliberately mixing components from unrelated toolchain revisions is unsupported.

The `p2996` branch is the mutable source-development channel, not a CI/release pin. It is planned to introduce immutable `p2996-YYYY.MM.DD` toolchain snapshots. CI and releases will pin those snapshots rather than following the branch.

See [`reference-toolchain.md`](reference-toolchain.md) for the complete identity, provenance, selection, component-coherence, and snapshot contract.

## GCC compatibility branch

The deferred GCC compatibility branch is named:

```text
gcc
```

It is intentionally not named `gcc16`: the branch describes a compiler-family compatibility line and may span GCC 16, 17, 18, and later versions.

The branch may trail `master` for an unlimited amount of time. It advances only as far as Miracle's public semantics can be represented faithfully with the capabilities available in GCC and libstdc++.

Normal synchronization direction is:

```text
master --> gcc
```

The entire `gcc` branch is never merged back into `master`.

A compiler-independent bug discovered while working on `gcc` should be fixed or cherry-picked independently on `master`. GCC/libstdc++ workarounds remain on `gcc`.

## Compatibility rules

The GCC branch adapts implementation details to compiler reality; it does not redefine Miracle.

Compatibility work follows these rules:

1. Preserve Miracle's public concepts and observable semantics.
2. Keep public spelling identical when the compiler can support it faithfully.
3. Prefer internal implementation substitutions over public API divergence.
4. Never inject replacement facilities into `namespace std`.
5. Never introduce backwards-compatibility machinery into `master` solely because GCC/libstdc++ is incomplete.
6. If a facility cannot be emulated faithfully, the `gcc` branch remains behind the `master` commit that requires it.
7. Temporary compatibility code is deleted when the corresponding standard feature becomes sufficiently implemented.
8. Compiler versions are hints; actual capabilities determine support.

For standard-library gaps such as `<hive>` and `<scope>`, a compatibility implementation is acceptable only when it preserves the semantics Miracle relies on and remains behind Miracle's own vocabulary/internal boundary. A fake or observably different implementation is worse than leaving the branch behind.

## Release policy

Official Miracle releases are cut from:

```text
master
```

The `gcc` branch is non-release-bearing for now. There are no `-gcc`, `-gcc16` or parallel compatibility releases.

The intended end state is convergence:

```text
GCC/libstdc++ implementation improves
              |
              v
compatibility delta shrinks
              |
              v
GCC passes the master capability contract
              |
              v
GCC joins master CI
              |
              v
gcc branch is retired
```

The compatibility branch is transitional infrastructure, not a permanent fork of the product.

## Capability-driven support

Compiler support is decided by executable capability probes rather than a compiler-version allowlist. `master` does not inject compiler-specific language or standard-library feature-enablement flags. Those belong to the selected toolchain, which must establish the complete C++26 mode before Miracle configures. Vendor checks in Miracle are limited to compiler-specific diagnostic policy; they do not decide whether a toolchain is accepted.

The current Miracle master revision probes:

```text
import_std
reflection_core
reflection_queries
reflection_annotations
reflection_static_storage
expansion_statements
std_vocabulary
std_hive
```

The probes compile in one nested CMake build so the `std` module can be reused across checks. When the parent build uses a CMake toolchain file, the probe project reuses that same toolchain file without reconstructing or duplicating its compiler flags. Without a toolchain file, the probe project reuses the selected compiler and global C++ flags. This keeps implementation-specific mode selection at the toolchain boundary while testing the same effective C++26 environment as Miracle itself. Every failed target gets its own build log. Configuration also writes a machine-readable `MiracleCapabilities.json` into the Miracle build directory.

These are the executable requirements of the current source revision, not an exhaustive C++26 conformance suite. `master` continues to target the complete standardized C++26-and-earlier model. When `master` adopts another standardized facility, its corresponding executable capability gate is added here.
 

A compiler version alone must never be treated as proof that the required semantics are present.

## Module artifacts

C++ module BMI/PCM artifacts are toolchain-local build products, not Miracle's distribution interface.

Miracle distributes module source and CMake module metadata. Consumers build compiler-specific module artifacts with their own compatible toolchain.

## Nyx and Switch

Miracle's compiler branches remain independent product concerns.

Switch follows the same `master` / `gcc` model and, once the compatibility branches exist, `Switch:gcc` must consume the corresponding `Miracle:gcc` implementation rather than accidentally mixing it with `Miracle:master`.

Nyx does not receive a `gcc` branch as of the current moment. A Nyx compatibility lane can be considered later once Miracle and Switch GCC support is sufficiently mature.
