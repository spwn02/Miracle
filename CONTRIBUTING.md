# Contributing to Miracle

Miracle is intentionally strict about keeping its foundation-layer boundary clean.

# Toolchain

Development of `master` targets the complete C++26-and-earlier model. The current reference implementation is [`spwn02/clang-p2996:p2996`](https://github.com/spwn02/clang-p2996/tree/p2996) together with the matching libc++ from that same toolchain build.

Use CMake 4.4+ and Ninja. Select the reference compiler before the top-level CMake configuration; do not add machine-specific compiler paths to checked-in presets.

See [`docs/compiler-support.md`](docs/compiler-support.md) and [`docs/reference-toolchain.md`](docs/reference-toolchain.md) before making compiler-compatibility or standard-library fallback changes.

## Invariants

1. Miracle production code must not depend on [Switch](https://github.com/spwn02/Switch) or [Nyx](https://github.com/spwn02/Nyx).
2. Public APIs must not contain repository-relative paths.
3. Source, FetchContent, and installed-package consumers must all see `Miracle::Miracle`.
4. New functionality should be broadly useful foundation function rather than engine policy.
5. C++26 is the baseline; do not add compatibility fallback for older language modes.

## Build and test

```bash
cmake --preset tests --fresh
cmake --build --preset tests
ctest --preset tests
```

Then validate the release build:

```bash
cmake --preset release --fresh
cmake --build --preset release
```

Before submitting a change:

```bash
git diff --check
```

## Tests

Tests live under `tests/` and use Switch as a test-only dependency. Prefer focused tests close to the changed facility.

## Style

- Prefer standard C++26 facilities where they express the intent clearly.
- Preserve module boundaries; do not replace them with header compatibility layers.
- Avoid public macros for configuration that belongs to the compiled target.
- Keep dependencies shallow.

Update `CHANGELOG.md` for user-visible behavior or API changes.
