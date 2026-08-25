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

The reference toolchain is developed in:

```text
compiler:
  https://github.com/spwn02/clang-cxx26
  branch: cxx26

standard library:
  the matching libc++ tree from the same fork
```

`clang-cxx26` preserves upstream LLVM history and the Bloomberg-originated reflection implementation while broadening the fork into a general C++26 toolchain. The current development line is being synchronized with LLVM/Clang 22.

"Reference toolchain" does not mean Miracle is permanently tied to one compiler. It means this is the toolchain against which the complete `master` contract is currently validated.

Other toolchains become eligible for `master` support when they implement the required standardized behavior faithfully. Support is ultimately capability-driven rather than vendor- or version-driven.

The compiler alone is not the reference unit: its matching libc++ headers, binaries, ABI runtime, and C++ module sources/metadata belong to the same validated toolchain build. Deliberately mixing components from unrelated toolchain revisions is unsupported.

The mutable development channel is `clang-cxx26:cxx26`. CI and releases never follow it directly: the currently validated immutable reference remains the historical `p2996-2026.08.23.2` snapshot from source revision `60966cc65acc736637ffd4ba03951932e47f5042`. Its P2996-era name is immutable provenance, not the current scope of the compiler project.

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

## Automated GCC validation and synchronization

The `gcc` line is continuously validated rather than advanced by assumption.

Every push or pull request targeting `gcc` runs the GCC compatibility validator. The validator uses the GCC compatibility toolchain and checks the production/consumer contract that closes the current Miracle GCC support line:

```text
capability probes
Miracle library + quickstart
add_subdirectory consumer
FetchContent consumer
optimized Release build
install + find_package consumer
```

Miracle's Switch-powered self-tests are intentionally not part of this lane. They add a second product/compiler surface beyond Miracle's production and consumer contract; Switch validates its own GCC self-test surface independently.

Every push to `master` also creates a temporary synchronization candidate by merging `master` into the current `gcc` tip. The candidate is validated with the same GCC validator before `gcc` is advanced. A merge conflict, compiler regression, consumer failure, or concurrent `gcc` update fails the workflow and leaves `gcc` unchanged. The final update is a normal non-forced push.

This automation does not change the synchronization policy:

```text
master --> gcc
```

There is no automated `gcc --> master` path. Compiler-independent fixes discovered on `gcc` still move to `master` through an explicit normal fix or cherry-pick.

The GitHub runner currently uses the current Arch Linux GCC/libstdc++ package. The executable capability probes, not the distro package version string, remain the acceptance criterion.

Every successful GCC validation also writes `gcc-convergence.json`, a machine-readable report of the complete `master`/`gcc` tree delta. The report records the exact revisions, merge base, graph distance, changed files, and line delta. Synchronization candidates produce the same report before `gcc` advances, so convergence evidence does not depend on a bot-authored push triggering a second workflow.

An empty tree delta is a **promotion candidate**, not permission to delete the compatibility branch automatically.

## Release policy

Official Miracle releases are cut from:

```text
master
```

A release tag must resolve to a commit contained in `master` history. The release workflow rejects a tag that points only to `gcc` or another side branch even when its version spelling is otherwise valid.

Every GitHub release attaches `release-metadata.json`. The metadata records the exact Miracle source revision, the `master` release branch, the immutable reference-toolchain snapshot and source revision used by CI, and that `gcc` is explicitly non-release-bearing. Release metadata is deterministic and contains no wall-clock fields.

The `gcc` branch is non-release-bearing. There are no `-gcc`, `-gcc16` or parallel compatibility releases.

## GCC convergence and retirement

The intended end state is convergence:

```text
GCC/libstdc++ implementation improves
              |
              v
compatibility delta shrinks
              |
              v
validated gcc tree matches master
              |
              v
same GCC validator runs directly on master
              |
              v
GCC joins master CI
              |
              v
gcc branch is retired explicitly
```

Retirement is deliberately gated:

1. A green GCC validation must report `treeEqual: true` against `master`.
2. The same GCC validator must then pass directly against the unmodified `master` tree.
3. Reference-toolchain CI remains required; GCC supplements it rather than replacing it.
4. At least one subsequent meaningful C++/build change on `master` must pass both the reference lane and the direct GCC lane without recreating a compatibility delta.
5. Only then are the synchronization workflow and branch-specific GCC lane removed and the `gcc` branch deleted explicitly.

No workflow force-pushes, rewrites, or automatically deletes `gcc`. The compatibility branch is transitional infrastructure, not a permanent fork or a second product line.

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
