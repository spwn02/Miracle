# Miracle

Miracle is a C++26 foundation library for projects that want modern language facilities without rebuilding the same low-level infrastructure in every codebase.

It is intentionally small at the dependency boundary and currently provides the foundations extracted from [Nyx Engine](https://github.com/spwn02/Nyx.git): result/error handling, common types, filesystem helpers, reflection/meta utilities, bitflags, debug formatting, profiling, memory primitives, concepts, and compile-time build facts.

> **Status:** pre-1.0. The API is usable, but source and ABI compatibility are not guaranteed until the project reaches 1.0.

### Philosophy

Miracle is written in C++, but its API philosophy is strongly influenced by Rust.

The goal is not to disguise C++ as Rust, reproduce Rust's ownership model, or mechanically port Rust's standard library. Instead, Miracle tries to bring some of the qualities that make Rust libraries pleasant to compose into modern C++26: explicit fallibility, strong vocabulary types, predictable resource ownership, small APIs, expressive naming, and useful behavior encoded in types rather than macros or hidden global policy.

A few principles guide the library:

- **Prefer explicit failure over exceptional control flow.** Operations that can reasonably fail should generally make that visible in their return type.
- **Use the standard library as machinery, not necessarily as the final vocabulary.** When C++ already has the right primitive, Miracle prefers a thin alias or composition over rebuilding it.
- **Adopt good ideas regardless of language.** Rust is a major influence, but Miracle is still designed around C++26 modules, reflection, ranges, `std::expected`, RAII, and the rest of the modern C++ model.
- **Keep the happy path compact.** Error handling and resource management should add correctness without drowning ordinary code in ceremony.
- **Stay C++26-first.** Miracle does not carry a backwards-compatibility layer for older language modes when a modern facility expresses the design better.

In short, Miracle asks: *what would a modern C++ foundation library look like if it learned aggressively from Rust, while still embracing what C++26 is uniquely good at?* Miracle took the best of two worlds, and united them with an extremely simple API.

## Rust influence and credits

Several parts of Miracle deliberately borrow vocabulary or API ideas from the Rust ecosystem. These are inspirations rather than ports; the implementations are built on C++26 and the C++ standard library.

- **`Result<T>` and `Error`** are inspired by Rust's `Result<T, E>` model and especially the application-oriented ergonomics of [`anyhow::Result<T>`](https://docs.rs/anyhow/latest/anyhow/type.Result.html). Miracle spells the common case as `Result<T>` and fixes the error side to `Miracle::Error`, implemented with `std::expected<T, Error>`.
- **`bail`** follows the same intent and vocabulary as `anyhow::bail!`: make an early error return concise. Miracle expresses that idea as C++ rather than a macro.
- **`Option<T>` and `None`** adopt Rust's `Option`/`None` vocabulary while remaining thin aliases over `std::optional` and `std::nullopt`.
- **`Vec<T>`, `String`, `usize`, and `isize`** intentionally use familiar Rust vocabulary where the corresponding C++ standard-library type already provides the required semantics. They remain aliases, not replacement containers or numeric types.
- **`fs::OpenOptions`** is directly inspired by [`std::fs::OpenOptions`](https://doc.rust-lang.org/std/fs/struct.OpenOptions.html). Miracle exposes the same core intent - `read`, `write`, `append`, `truncate`, `create`, and `create_new` - and returns a fallible `Result<File>`.

More Rust-inspired APIs may appear over time where the model translates cleanly to C++. The criterion is not similarity for its own sake: an idea belongs in Miracle only when it produces a clearer C++ API.

### Requirements

Miracle currently targets:

- C++26
- CMake 4.4 or newer
- Ninja
- GCC 16 or newer as the verified ordinary-user compiler
- a reflection-enabled Clang toolchain for development
- toolchain support for C++26 `import std;`

The current CI baseline is Linux + GCC 16.

### Quick start

```cpp
import std;
import Miracle;

using namespace Miracle;

auto reciprocal(i32 value) -> Result<f64> {
  if (value == 0)
    return bail({"Cannot divide by zero."});

  return 1.0 / static_cast<f64>(value);
}

auto main() -> int {
  auto value = reciprocal(4);
  if (not value) {
    std::println(std::cerr, "{}", value.error().display());
    return 1;
  }

  std::println("{}", *value);
  return 0;
}
```

The same program lives in [`examples/quickstart.cxx`](examples/quickstart.cxx) and is built by the `tests` preset.

### Public module

The umbrella import is:

```cpp
import Miracle;
```

It currently re-exports:

- `Miracle:Types`
- `Miracle:Error`
- `Miracle:Fs`
- `Miracle:Meta`
- `Miracle:Bitflags`
- `Miracle:Debug`
- `Miracle:Build`
- `Miracle:Concepts`
- `Miracle:Profiling`
- `Miracle:Memory`

The stable CMake target is:

```cmake
Miracle::Miracle
```

## Consuming Miracle

### Existing source checkout

```
add_subdirectory(path/to/Miracle)
target_link_libraries(my_target PRIVATE Miracle::Miracle)
```

### FetchContent

Pin a known revision rather than silently following a moving branch:

```cmake
include(FetchContent)

FetchContent_Declare(
  Miracle
  GIT_REPOSITORY https://github.com/spwn02/Miracle.git
  GIT_TAG <commit-or-release-tag>
)
FetchContent_MakeAvailable(Miracle)

target_link_libraries(my_target PRIVATE Miracle::Miracle)
```

### Installed package

After installing Miracle:

```cmake
find_package(Miracle 0.1.0 CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE Miracle::Miracle)
```

All three modes intentionally expose the same target identity.

### Building Miracle

```bash
cmake --preset debug
cmake --build --preset debug
```

Run the full self-test suite and compile the quickstart example with:

```bash
cmake --preset tests --fresh
cmake --build --preset tests
ctest --preset tests
```

Miracle's production library does **not** depend on [Switch](https://github.com/spwn02/Switch.git). Switch is used only to build Miracle's own tests.

# Configuration

`MIRACLE_OPTIMIZED` accepts `AUTO`, `ON`, or `OFF`.

`MIRACLE_WARNINGS_AS_ERRORS=ON` applies only while compiling Miracle itself.

`MIRACLE_BUILD_EXAMPLES=ON` builds repository examples.

`MIRACLE_BUILD_TESTS=ON` builds the Switch-powered self-test suite.

## Design

See [`docs/architecture.md`](docs/architecture.md) for the product boundary and dependency rules.

## Contributing and security

See [`CONTRIBUTING.md`](CONTRIBUTING.md) and [`SECURITY.md`](SECURITY.md).

## License

Apache License 2.0.
