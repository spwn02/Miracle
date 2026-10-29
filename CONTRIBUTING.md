# Contributing to Miracle

Thank you for considering a contribution to Miracle, a C++26 foundation library. Miracle is intentionally strict about its foundation-layer boundary, public semantics, and compiler/toolchain provenance. Contributions of bug reports, minimal reproducers, documentation, design reviews, tests, and code are welcome.

Miracle is pre-1.0. An issue or pull request is a proposal for review, **not** an accepted design contract, development priority, or release commitment.

## Before contributing

- Read the [Development Protocol](docs/DEVELOPMENT_PROTOCOL.md) for the project's design, review, and acceptance process. Existing locked contracts remain authoritative unless explicitly amended.
- Browse [existing issues](https://github.com/spwn02/Miracle/issues), [Discussions](https://github.com/spwn02/Miracle/discussions) for community questions and exploratory ideas, and the [documentation](docs/).
- Use the [New Issue chooser](https://github.com/spwn02/Miracle/issues/new/choose) for an actionable library bug, compiler/toolchain problem, concrete feature proposal, architectural change, or infrastructure request. For unanswered community questions, visit [Q&A](https://github.com/spwn02/Miracle/discussions/categories/q-a); for preliminary brainstorming, visit [Ideas](https://github.com/spwn02/Miracle/discussions/categories/ideas). A Discussion is not an approved contract.
- For general support routes, consult [SUPPORT.md](SUPPORT.md); for conduct concerns, consult [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
- **Do not disclose security vulnerabilities in a public issue or pull request.** Follow [SECURITY.md](SECURITY.md).
- Accessibility barriers in the documentation or contribution workflow can be reported as infrastructure requests; see [ACCESSIBILITY.md](ACCESSIBILITY.md).

A complete reproducer helps investigation, but if one is not available, explain the isolation status rather than inventing missing evidence.

## Reference toolchain

`master` targets the complete standardized C++26-and-earlier language and library model. The reference implementation is [`spwn02/clang-cxx26`](https://github.com/spwn02/clang-cxx26), development branch `cxx26`, **together with its matching libc++ and C++ module support**.

The current Miracle reference CI configuration pins:

| Component | Verified CI identity |
| --- | --- |
| Immutable toolchain snapshot | `cxx26-2026.10.27` |
| Source revision | `b56ae4e61cf1f46495f0fb08f2b9fc270aad162c` |
| Linux x86_64 asset | `clang-cxx26-2026.10.27-linux-x86_64` |
| CI CMake version | `4.4.3` |

These values describe the repository's **currently pinned CI snapshot**, not a promise that the mutable toolchain branch or latest published compiler release will always have the same revision. The source of truth for future CI changes is [`.github/workflows/ci.yml`](.github/workflows/ci.yml).

Use **CMake 4.4+** and Ninja. Install or select the reference compiler and its matching libc++ *before* the top-level CMake configuration. Do not check machine-specific compiler paths into project presets.

For a correctly installed reference toolchain, a typical shell setup is:

```bash
source /path/to/reference/share/clang-cxx26/activate.sh

cmake --preset tests --fresh \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
cmake --build --preset tests
ctest --preset tests
```

Substitute the actual installation path. The environment setup must use a coherent compiler, standard library, ABI runtime, and module sources; a system Clang/libstdc++ combination is not a drop-in substitute.

Read [compiler support](docs/COMPILER_SUPPORT.md) and the [reference-toolchain contract](docs/REFERENCE_TOOLCHAIN.md) before proposing compiler workarounds or changing the selected standard library.

### GCC compatibility

The `gcc` branch is a capability-driven compatibility line and may lag `master`. It is not a second release branch. Do not introduce compiler workarounds into `master` simply to make an incomplete GCC/libstdc++ version accept the library. Compiler-specific adaptations belong on the compatibility line if they preserve the library's public semantics.

## Repository invariants

1. Miracle production code must not depend on [Switch](https://github.com/spwn02/Switch) or [Nyx](https://github.com/spwn02/Nyx).
2. Public APIs must not contain repository-relative paths.
3. Source, FetchContent, and installed-package consumers must all expose the `Miracle::Miracle` CMake target.
4. New functionality should be broadly useful foundation functionality, not engine-specific policy.
5. C++26 is the baseline; avoid compatibility fallbacks for older language modes on `master`.
6. Preserve module boundaries; do not replace them with public header compatibility layers or introduce unapproved API changes.

## Local build and verification

With the reference compiler already selected:

```bash
cmake --preset tests --fresh
cmake --build --preset tests
ctest --preset tests

cmake --preset release --fresh
cmake --build --preset release

git diff --check
```

The `tests` preset builds the Switch-powered test suite and examples. Switch is a **test-only** dependency, not a dependency of the production Miracle library. Focused tests belong under `tests/`, and executable API examples belong under `examples/`.

Where relevant, include coverage of failure paths, lifetime and ownership rules, compile-time behavior, and the affected consumer integration mode (`add_subdirectory`, `FetchContent`, or installed `find_package`). Do not claim results for commands you have not actually run.

## Design and pull requests

Discuss changes to public APIs, significant algorithms, ownership models, module boundaries, or locked architectural invariants **before** implementing a conflicting design. Use a [Feature Proposal](https://github.com/spwn02/Miracle/issues/new?template=feature_proposal.yml) or [Architecture Proposal](https://github.com/spwn02/Miracle/issues/new?template=architecture_proposal.yml) as appropriate.

For review style and history-preserving integration, follow the [Miracle Pull Request Workflow](docs/infrastructure/G4-PR-WORKFLOW.md). It explains line-specific comments, merge commits, and when an explicit local `git merge --ff-only` can safely advance `master` without rewriting commits. Dependabot PRs use the same review and verification standards.

When opening a pull request:

- Explain the motivation, change scope, affected contract/ADR (if any), and compatibility impact.
- Fill out the repository's [PR template](.github/PULL_REQUEST_TEMPLATE.md), recording **Passed**, **Failed**, **Not run**, or **Not applicable** for each relevant check, with evidence.
- Update relevant documentation and examples; update `CHANGELOG.md` for user-visible API or behavior changes.
- Keep patches focused and avoid including machine-local artifacts, compiler caches, generated build files, credentials, or sensitive logs.
- Follow CI and review findings through to verification; a passing CI job supports only the checks it actually executed.

## Style

- Prefer standard C++26 facilities where they make intent clearer.
- Keep module interfaces and dependencies narrow.
- Prefer explicit fallibility, predictable ownership, and stable vocabulary names.
- Avoid public configuration macros where the build target can express the requirement.
- Keep implementation and documentation changes consistent with the locked contracts.

See [Miracle's architecture](docs/ARCHITECTURE.md) and [support policy](SUPPORT.md) for additional context.
