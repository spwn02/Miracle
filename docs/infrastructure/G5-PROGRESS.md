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
- Verified Kitware CMake 4.4.3, pinned Ninja 1.13.2 and **official pinned Neovim 0.12.5** archives, with clangd, Python and Git.
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

## Final G5 image validation (Neovim 0.12.5)

[Commit `7ab69e2`](https://github.com/spwn02/Miracle/commit/7ab69e244d893f7d84e7d8b5d90c0f72ed0ea01a) upgraded the prior Ubuntu Neovim 0.6.1 binary to an **official SHA-256-pinned Neovim 0.12.5** release, and hardened the version/headless startup smoke check.

- [Clean Docker workflow #39001300054](https://github.com/spwn02/Miracle/actions/runs/39001300054): **success** — immutable toolchain/CMake/Ninja/Neovim assets verified, container started, full C++26 test/consumer job passed.
- [Reference CI #39001300184](https://github.com/spwn02/Miracle/actions/runs/39001300184): **success**.
- The previously validated clean run recorded **71/71 tests and all three CMake consumers**. The final post-Neovim upgrade Docker job completed the same validation step successfully.
- **No compiler/runtime revision changed** and no Codespaces prebuild, user secret, image registry or paid Codespace was created.

**G5's source, clean Docker build and full container tests are implemented and accepted as technical evidence.** The **phase issue remains open** until an actual editor/devcontainer attach, Codespaces open and maintainer final sign-off. Do not conflate container CI with interactive Codespaces acceptance.

## Developer Make orchestrator (2026-10-17)

[Commit `182385c`](https://github.com/spwn02/Miracle/commit/182385c3818d8458b71dceeb25f2191d2b54a644) added the root Makefile, and a dedicated dry-run/negative-fixture test for its CLI. It delegates host operations to existing CMake presets and G5 image operations to the exact Dockerfile and `verify-container.sh`; it refuses missing reference toolchain configuration. There are no new C++ build rules or release operations.

The G5 workflow now runs `make docker-build NO_CACHE=1` and `make docker-test` as separate diagnosable steps, instead of maintaining another copy of the Docker invocation. It continues using a read-only host checkout and a disposable writable container copy, and it never publishes an image. Both the reference and G5 workflow run `check-makefile.py` in their early verification phase.

**Verified on the implementation revision:** [reference CI #39002203719](https://github.com/spwn02/Miracle/actions/runs/39002203719) **passed**, including the new Make CLI tests. [G5 clean Docker #39002203712](https://github.com/spwn02/Miracle/actions/runs/39002203712) **passed**, including `make docker-build NO_CACHE=1`, `make docker-test`, **71/71 CTest cases**, and all three consumer modes. The [documentation build #39002266448](https://github.com/spwn02/Miracle/actions/runs/39002266448) also passed and deployed the Make usage guide. These are actual GitHub Actions results, not dry-run claims. The Make orchestration implementation is **verified**; interactive Codespaces/devcontainer acceptance remains separate.

## Verification evidence

- [x] Source/JSON/Dockerfile and workflow configuration passed reference CI checks.
- [x] Clean Docker image built and passed pinned archive/manifest/checksum verification (run #39000812865).
- [x] C++26 test suite/examples and all three consumer modes passed inside the image (71/71 tests).
- [ ] Local Docker devcontainer opens and clangd/Neovim workflow is usable.
- [ ] Maintainer successfully opens Codespaces and checks toolchain and workspace behavior.
- [ ] Maintainer explicitly accepts G5.

**G5 remains open.** The Docker portion is proven by a clean image build and full C++26/consumer run; local editor and Codespaces acceptance, plus maintainer sign-off, remain outstanding. The image does not guarantee fixed Codespaces pricing or a fully hermetic APT package closure.
