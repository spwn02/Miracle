# G5 — Docker, devcontainers and Codespaces: Progress

**Updated:** 2026-10-17
**Status:** Docker build/test acceptance VERIFIED; interactive Codespaces and editor workflow pending
**Project:** [Miracle Development](https://github.com/users/spwn02/projects/3)
**Issue:** [#5](https://github.com/spwn02/Miracle/issues/5)
**Contract:** [G5 operating plan](G5-OPERATING-PLAN.md)

## Implementation

- Official Ubuntu 22.04 **Linux/amd64** base image pinned by manifest SHA-256.
- Immutable reference compiler `cxx26-2026.10.09.2` and source revision from existing reference CI; pinned and verified compiler tarball, release manifest and published SHA256SUMS.
- Matching packaged libc++ runtime and `import std` metadata used; no distribution Clang/libstdc++ swap.
- Verified Kitware CMake 4.4.3 archive, Ninja, clangd, Python, Git and Neovim.
- Non-root `vscode` devcontainer with a quick post-create toolchain smoke check; no prebuild or registry.
- Source-controlled, read-only [G5 Docker validation workflow](https://github.com/spwn02/Miracle/blob/master/.github/workflows/container.yml) that builds the image **without Docker cache** and runs reference tests/examples and all three consumer modes.
- Self-contained reference toolchain manifest validator and static identity/configuration guard.
- [G5 operating plan](G5-OPERATING-PLAN.md) documents provenance, commands, resource requirements, billing caveats and immutable vs time-dependent dependencies.

## Clean Docker CI results (2026-10-17)

- [Run #39000812865](https://github.com/spwn02/Miracle/actions/runs/39000812865) **SUCCESS** on [`7eb2536`](https://github.com/spwn02/Miracle/commit/7eb2536529b2ed273fabd568ba5acace23ba8539): clean, no-cache Ubuntu 22.04/amd64 Docker build; integrity-checked compiler snapshot, manifest and checksum list; independent SHA-256 pins for CMake 4.4.3 and Ninja 1.13.2.
- Image starts Clang C++26, clangd, CMake, Ninja and Neovim. Real compile-and-link smoke test passed. Matching standard module metadata and exact source provenance were verified.
- **71 of 71 CTest cases passed**, including compile-fail and reflection/meta hardening tests.
- **3 of 3 consumer modes passed**: `add_subdirectory`, `FetchContent` and installed `find_package`; each compiled, linked and ran the consumer executable.
- [Reference CI #39000813059](https://github.com/spwn02/Miracle/actions/runs/39000813059) on the same code also succeeded.

**Validation history:** [Initial run #39000489439](https://github.com/spwn02/Miracle/actions/runs/39000489439) exposed missing Ubuntu C runtime developer libraries; `build-essential`/`libc6-dev` resolved that. [Run #39000708216](https://github.com/spwn02/Miracle/actions/runs/39000708216) exposed Ninja 1.10.1's lack of C++ modules; pinned Ninja 1.13.2 resolved it. Failures are retained as real evidence, not silently represented as successes.

**Unverified:** interactive local editor attach, Codespaces startup/working directory/retention/billing, ARM/emulation performance, and fully immutable Ubuntu APT dependencies. No Codespace, prebuild or public image was created.

## Verification evidence

- [x] Source/JSON/Dockerfile and workflow configuration passed reference CI checks.
- [x] Clean Docker image built and passed pinned archive/manifest/checksum verification (run #39000812865).
- [x] C++26 test suite/examples and all three consumer modes passed inside the image (71/71 tests).
- [ ] Local Docker devcontainer opens and clangd/Neovim workflow is usable.
- [ ] Maintainer successfully opens Codespaces and checks toolchain and workspace behavior.
- [ ] Maintainer explicitly accepts G5.

**G5 remains open.** The Docker portion is proven by a clean image build and full C++26/consumer run; local editor and Codespaces acceptance, plus maintainer sign-off, remain outstanding. The image does not guarantee fixed Codespaces pricing or a fully hermetic APT package closure.
