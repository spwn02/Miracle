# Miracle architecture

## Product boundary

Miracle is an independent foundation library. [Nyx Engine](https://github.com/spwn02/Nyx.git) may keep an editable Miracle checkout as a Git submodule, but Nyx-specific build infrastructure is not part of Miracle's public contract.

Canonical identities:

```cpp
import Miracle;
```

```cmake
Miracle::Miracle
```

Miracle does not provide `Nyx::` compatibility aliases.

## Philosophy

Miracle is deliberately influenced by Rust's library design, but it is not an attempt to emulate Rust wholesale.

The project takes a selective approach: identify APIs and conventions that make systems code easier to reason about, then express the same underlying idea using the strongest C++26 mechanism available. Sometimes that means a vocabulary alias such as `Option<T>` or `Vec<T>`. Sometimes it means building on a native C++ facility such as `std::expected`. Sometimes the C++ design should differ completely because modules, reflection, RAII, templates, or ranges provide a better fit.

The guiding rules are:

1. Prefer explicit values and types over hidden control flow.
2. Prefer thin abstractions over replacement implementations when the standard library already has the required semantics.
3. Optimize APIs for composition and readable call sites rather than historical C++ convention.
4. Keep ownership and lifetime behavior unsurprising and RAII-native.
5. Treat C++26 as the baseline instead of designing around older-language compatibility.
6. Borrow from Rust only when the idea survives translation into idiomatic modern C++.

This is why Miracle can intentionally look Rust-like in places without trying to become a Rust compatibility layer.

## Language and toolchain philosophy

`master` targets the complete standardized C++26-and-earlier capability set, not merely the subset implemented by today's compilers or the subset Miracle currently happens to use.

Miracle may adopt any standardized facility through C++26 whenever it produces a better design. If the current reference toolchain lacks that facility, the preferred response is to implement or port the missing standard behavior into the toolchain rather than constrain `master` around an incomplete implementation.

The reference toolchain is developed in [`spwn02/clang-cxx26:cxx26`](https://github.com/spwn02/clang-cxx26/tree/cxx26) together with its matching libc++. The fork is synchronized with LLVM/Clang 22.1.8, and CI pins the immutable `cxx26-2026.09.05` snapshot. GCC compatibility is a deferred implementation concern and may trail `master`.

The complete branch and compiler policy is defined in [`compiler-support.md`](compiler-support.md).

## Influences and attribution

Current deliberate Rust influences include:

- `Result<T>` / `Error`: Rust's explicit `Result<T, E>` model and the single-error-type ergonomics popularized by `anyhow::Result<T>`. Miracle implements the concept using `std::expected<T, Miracle::Error>`.
- `bail`: inspired by `anyhow::bail!` as concise vocabulary for returning a failure.
- `Option<T>` / `None`: Rust vocabulary implemented as aliases over `std::optional` / `std::nullopt`.
- `Vec<T>`, `String`, `usize`, and `isize`: Rust-familiar names mapped directly to suitable standard C++ types rather than reimplemented.
- `fs::OpenOptions`: modeled after Rust's `std::fs::OpenOptions`, including the `create`, `create_new`, `read`, `write`, `append`, and `truncate` configuration model.

These projects and APIs deserve explicit credit for demonstrating that low-level can expose concise, strongly typed interfaces without sacrificing control. Miracle adopts those lessons while remaining a C++ library with its own implementation and design constraints.

## Dependency direction

Production code has no dependency on [Switch](https://github.com/spwn02/Switch.git):

```text
Miracle
```

Miracle's self-tests may use Switch:

```text
MiracleTests -> Switch::Switch -> Miracle::Miracle
```

That is a test-target graph, not a production cycle. When Miracle already exists in the parent build, Switch reuses the existing `Miracle::Miracle` target.

## Public modules

`Miracle` is the umbrella module. Granular partitions currently provide types, error/results, filesystem utilities, reflection/meta utilities, bitflags, debug formatting, build facts, concepts, profiling, and memory primitives.

New facilities should remain low-level enough to be useful independently of Nyx and Switch.

## Build facts

`Miracle:Build` reports immutable properties of the compiled Miracle target: platform, CPU architecture, and whether the build is optimized.

CMake chooses the policy; C++ reports the resulting configuration. Public macros are not used as a per-consumer configuration channel for an already-build module.

## Package identity

Source checkout, FetchContent, and installed-package consumers all receive `Miracle::Miracle`. The installed package exports its C++ module file set and CMake module metadata.

## Compatibility

Before 1.0, source and ABI compatibility may change between minor version. Breaking changes should be recorded in `CHANGELOG.md` and accompanied by focused tests.

Compiler compatibility follows a separate rule: `master` defines the C++26-first product, while the non-release-bearing `gcc` compatibility line adapts that design to the capabilities available in GCC/libstdc++. Compatibility work flows from `master` toward `gcc`, not back into the primary architecture.
