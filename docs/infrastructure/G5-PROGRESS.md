# G5 — Docker-only development: Progress and acceptance

**Updated:** 2026-10-19
**Status:** Maintainer approved revised Docker-only scope; migration validation pending. Codespaces declined, not tested.
**Project:** [Miracle Development](https://github.com/users/spwn02/projects/3)
**Issue:** [#5](https://github.com/spwn02/Miracle/issues/5)
**Contract:** [G5 operating plan](G5-OPERATING-PLAN.md)

## Current authoritative decision (2026-10-19)

The maintainer explicitly reported these manual acceptance results in the G5 continuation:

- **Local interactive devcontainer — PASS (maintainer-reported).** This is human sign-off, not independent observation from GitHub CI. The prior automated Docker and LSP checks are independently documented below.
- **Codespaces — DECLINED / NOT TESTED.** The maintainer does not wish to use paid Codespaces. No Codespace has been created, no cloud-storage/billing/browser acceptance was performed. This check remains **unverified**; it is **removed from the active Docker-only implementation scope**, not represented as a pass.
- **G5 final approval — APPROVED for the revised Docker-only scope.** The maintainer requested replacement of the old Dev Containers configuration by a plain Dockerfile.
- **Current implementation — [Docker-only PR #14](https://github.com/spwn02/Miracle/pull/14).** Migrate the pinned image to [`Dockerfile`](../../Dockerfile), retire `.devcontainer/devcontainer.json` and Dev Containers CLI, add persistent `make docker-dev` and direct Docker workspace/LSP verification. Preserve existing compiler/libc++ checksums and 71 self-tests/3 consumer modes.
- **New Docker-only CI — PENDING**, until the migration branch's clean-image, CMake consumers, source/diagnostic and LSP/persistent bind-mount checks complete successfully.

**Acceptance reconciliation:** The original [Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md) proposed Codespaces support; on 2026-10-19 the maintainer deliberately reduced operational scope to **Docker-only**. The historical acceptance claims below remain point-in-time records. Codespaces cannot be checked as verified. **Keep [issue #5](https://github.com/spwn02/Miracle/issues/5) open until the revised Docker-only implementation is independently revalidated.**

## Historical implementation and verification evidence (2026-10-17)

The following sections preserve the original implementation's evidence and historical status statements. Their mentions of then-pending editor/Codespaces gates are superseded for *current scope* by the decision above; their empirical CI results are unchanged.

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

## Headless devcontainer lifecycle — merged verification (2026-10-17)

- [PR #12](https://github.com/spwn02/Miracle/pull/12) was reviewed and **squash-merged** into `master` at [`a6ffd7b`](https://github.com/spwn02/Miracle/commit/a6ffd7bf4632417194fb4233bb10808da81bedb6). It adds isolated execution of the tracked `.devcontainer/devcontainer.json` via the version-selected Dev Containers CLI, startup/post-create validation, non-root `vscode`, reference toolchain smoke, a CMake compilation database, and persistence across a real container restart. VS Code clangd is configured to read `build/tests/compile_commands.json`.
- [Clean container and lifecycle CI #39003187140](https://github.com/spwn02/Miracle/actions/runs/39003187140) **SUCCESS** on PR head `d6e60de`: no-cache pinned Docker build, **71/71 tests**, **3/3 CMake consumers**, pinned CLI, and the *actual headless devcontainer lifecycle* executed and passed. The CLI logged a nonfatal image-name inspection warning before successfully building and starting the digest-pinned container; monitor compatibility on future CLI upgrades.
- [Reference C++26 CI #39003187065](https://github.com/spwn02/Miracle/actions/runs/39003187065), [mdBook #39003187158](https://github.com/spwn02/Miracle/actions/runs/39003187158), and [code scanning #39003188730](https://github.com/spwn02/Miracle/actions/runs/39003188730) all **SUCCESS** on the same PR revision.
- **Proven:** headless devcontainer startup, post-create commands, workspace permissions and persistence through container restart. **Not proven:** graphical editor attach, interactive clangd diagnostics, browser Codespaces startup, storage/billing behavior, or maintainer sign-off. No paid Codespace/prebuild or image publication was created.

**G5 remains OPEN** under [issue #5](https://github.com/spwn02/Miracle/issues/5); only the automated headless devcontainer gate is newly accepted.

## Headless Neovim / clangd protocol — merged verification (2026-10-17)

[PR #13](https://github.com/spwn02/Miracle/pull/13) was reviewed and **squash-merged** into `master` at [`7b15cee`](https://github.com/spwn02/Miracle/commit/7b15cee763f417b7c13ae8e22bac6a6ba6583124). The existing isolated devcontainer CI lifecycle now executes [`verify-nvim-lsp.lua`](../../scripts/ci/verify-nvim-lsp.lua). Neovim 0.12's built-in LSP client opens `src/private/Error.cxx`, checks that its generated CMake compilation database entry selects the pinned `clang++`, initializes pinned `clangd`, attaches to the C++ buffer, and receives a nonempty `textDocument/documentSymbol` response.

- [Clean container workflow #39005531835](https://github.com/spwn02/Miracle/actions/runs/39005531835) **SUCCESS** on `3ceaf6c`: actual log records `PASS: Neovim LSP attached to pinned clangd and received Miracle C++ source symbols`; full **71/71 CTest**, **3/3 CMake consumers**, and the headless devcontainer restart-persistence probe also passed.
- [Reference C++26 CI #39005531845](https://github.com/spwn02/Miracle/actions/runs/39005531845), [mdBook #39005531833](https://github.com/spwn02/Miracle/actions/runs/39005531833) and [code scanning #39005534763](https://github.com/spwn02/Miracle/actions/runs/39005534763) **SUCCESS** on the same revision.
- **Proven:** actual Neovim↔clangd LSP protocol interaction with nonempty project source-symbol response and pinned compile database/compiler. **Not proven:** interactive keyboard/editor ergonomics, accurate C++26 module/reflection diagnostics in a live session, VS Code extension attachment, real Codespaces startup/storage/billing, or maintainer sign-off.

The LSP evidence upgrades automation coverage, **not** the manual exit gates. G5 issue #5 remains **OPEN**.

## Acceptance matrix — revised Docker-only scope (2026-10-19)

- [x] Immutable reference C++26 compiler/runtime, pinned image, complete 71/71 tests and 3/3 consumer modes — **verified on historical image**, see #39005531835.
- [x] Actual Neovim↔clangd protocol and document-symbol response — **CI verified** on historical devcontainer image #39005531835.
- [x] Interactive local devcontainer usability — **maintainer reported pass** on 2026-10-19 (historical environment).
- [x] Maintainer approves Docker-only scope — **explicitly approved** on 2026-10-19.
- [ ] Clean **root Dockerfile** build, full C++26 suite and consumers after migration — **pending new CI result**.
- [ ] Direct Docker bind-mount persistence and Neovim/clangd protocol on the new Docker-only image — **pending new CI result**.
- [ ] GitHub Codespaces live startup/billing/storage/interactive acceptance — **DECLINED, NOT TESTED, excluded from revised implementation**. Do not mark passed.

**G5 issue #5 remains OPEN** while Docker-only changes are verified. No cloud charges, Codespaces prebuild, registry publication or reference compiler upgrade is required.
