# G5 — Reproducible Reference-C++26 Developer Environments

**Scope:** Docker CLI development, devcontainers and Codespaces, Linux x86-64
**Authority:** [GitHub Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md), §8/§10
**Tracker:** [Infrastructure issue #5](https://github.com/spwn02/Miracle/issues/5)
**Status:** Clean Docker build and all consumer checks passed; interactive Codespaces/devcontainer acceptance pending
**Progress:** [G5-PROGRESS.md](G5-PROGRESS.md)

## Purpose and authority

Miracle's authoritative C++26 toolchain is a **coherent Clang, libc++, libc++abi and standard-module unit** from a single immutable release. Neither an arbitrary distribution Clang nor a moving compiler development branch is a replacement. This setup does not modify the existing CMake presets, library API, or compiler contract.

The image targets **Linux/amd64** and matches the Ubuntu 22.04 reference CI host ABI. The Docker base image is locked to an official amd64 manifest digest. Current container source:

- [Dockerfile](https://github.com/spwn02/Miracle/blob/master/.devcontainer/Dockerfile)
- [devcontainer.json](https://github.com/spwn02/Miracle/blob/master/.devcontainer/devcontainer.json)
- [clean container validation](https://github.com/spwn02/Miracle/blob/master/.github/workflows/container.yml)
- [toolchain identity checker](https://github.com/spwn02/Miracle/blob/master/scripts/ci/verify-reference-toolchain.py)
- [CMake/consumer test entry point](https://github.com/spwn02/Miracle/blob/master/scripts/ci/verify-container.sh)

## Provenance contract

| Component | Pinned identity |
| --- | --- |
| Operating system base | Ubuntu 22.04, official Linux/amd64 manifest `sha256:08ea48a03a3e78ebc7cd526e6a275053223aadd88bfc09cc49b06d5281525fde` |
| Reference toolchain | `spwn02/clang-cxx26`, snapshot `cxx26-2026.10.09.2` |
| Compiler source | `040df2b05ea338d107fac7387a2e4db835d746e0` |
| Toolchain archive | `sha256:7965d447d32022557fc767bb5b8524e9ca6d9318b5d0c0ba3457b32e6632a1e8` |
| Toolchain manifest | `sha256:178c91d2f7d2bf59532223581563eee5c3babbd136b4c4496f92c69971b6ffb1` |
| Published SHA256SUMS file | `sha256:8ed3810fa391f056ad840c9331a3598bb24257a16fb1f05e9752bbd7301c1b5a` |
| CMake | Official Kitware 4.4.3 Linux x86-64 archive, `sha256:d6c83076c575bc00b823522ac974bda66d0af05d6ddc30e739c12385cf32c6cc` |
| Ninja | Official Ninja 1.13.2 Linux x86-64 zip, `sha256:5749cbc4e668273514150a80e387a957f933c6ed3f5f11e03fb30955e2bbead6` |
| Neovim | Official Neovim 0.12.5 Linux x86-64 tarball, `sha256:bce0f56eda1f1b1db6eee8f4133d7a38813ea07933837dd1777411ca384c6875` |

The same reference snapshot/source/asset identity is used by existing [CI](https://github.com/spwn02/Miracle/blob/master/.github/workflows/ci.yml). Container builds verify all published checksums, parse the manifest, unpack the matching compiler/runtime bundle, compare its installed metadata, and verify the expected module descriptor. Standard-library/compiler consistency is subsequently exercised by the actual test and consumer builds.

**Limitation:** The Docker base digest, compiler archive and CMake archive are immutable, but Ubuntu package repositories provide current packages at build time. Without an APT snapshot, this is **reproducible compiler identity and build recipe**, not a mathematically byte-for-byte identical total filesystem. Maintain security updates explicitly and preserve the pinned reference components; do not claim a fully hermetic dependency closure.

## Root Make orchestration (recommended CLI)

The [root `Makefile`](https://github.com/spwn02/Miracle/blob/master/Makefile) offers a small, discoverable command-line entry point. **CMakePresets.json remains the only build configuration** and `.devcontainer/Dockerfile` remains the image recipe. `make` does not install the compiler, rewrite the selected toolchain, use system Clang as a fallback or generate any release tags.

| Command | Effect |
| --- | --- |
| `make` / `make help` | List supported commands and override variables |
| `make configure [PRESET=tests\|debug\|release]` | Fresh configure with the explicitly selected reference toolchain |
| `make build [PRESET=...]` | Build the selected preset; configure first only if absent |
| `make test` | Build and run the `tests` preset, independent of `PRESET` |
| `make verify` | Fresh reference-toolchain configuration, build and tests |
| `make clean PRESET=release` | CMake's normal `clean` target, **not** a destructive directory deletion |
| `make docker-build` | Build the pinned `linux/amd64` image |
| `make docker-smoke` | Verify installed image's compiler, linker, Ninja, clangd and Neovim |
| `make docker-test` | Run all C++26 tests and consumers with a disposable writable checkout |
| `make docker-verify NO_CACHE=1` | Build without cache, then run complete container tests |
| `make docker-shell` | Temporary writable copy of repository inside container; host checkout read-only |
| `make docs` / `make docs-serve` | Validate or serve the canonical mdBook |

```sh
# Host workflow: activate the coherent installed reference compiler first.
source /path/to/clang-cxx26/share/clang-cxx26/activate.sh
make verify
make PRESET=release configure
make PRESET=release build JOBS=4

# Alternative: no host-side reference compiler installation.
make docker-verify NO_CACHE=1
make docker-shell
```

`TOOLCHAIN=/absolute/path/to/toolchain.cmake` can explicitly override the value exported as `CXX26_CMAKE_TOOLCHAIN_FILE`. An absent toolchain is an **error for host configuration**, not permission to silently switch compilers. The `build` target reuses CMake's already configured cache; if the intended toolchain changes, rerun `make configure` explicitly. `PRESET` is limited to `tests`, `debug` and `release`; `JOBS` defaults to 2. `IMAGE` and `DOCKER` select the Docker tag and CLI; no image is pushed anywhere.

Docker commands need the image already built (or use `docker-verify`). `docker-shell` copies a read-only bind mount into a **disposable** container directory, so edits inside it do not persist; use a proper devcontainer for live editing. On Apple Silicon, these Docker commands select `linux/amd64` and may run under emulation.

The same Make targets are used by the [clean G5 GitHub Actions workflow](https://github.com/spwn02/Miracle/blob/master/.github/workflows/container.yml). The read-only [Make CLI test](https://github.com/spwn02/Miracle/blob/master/scripts/ci/check-makefile.py) checks help, forwarding, Docker commands, safety guards and negative inputs without requiring Docker for every reference-CI run.

## Local Docker

Requirements: Docker or compatible BuildKit with Linux/amd64 support, internet access to Ubuntu APT and the public GitHub releases, enough RAM and free disk space for a C++26 build. Begin with **8 GiB RAM and several GiB free disk**, preferably **16 GiB+** for parallel C++ module builds; these are practical planning recommendations, not measured hard minima. Apple Silicon / ARM hosts need amd64 emulation and may be much slower.

```sh
docker build --platform linux/amd64 -f .devcontainer/Dockerfile -t miracle-cxx26:g5 .
docker run --rm -it --platform linux/amd64 \
  --mount "type=bind,source=$PWD,target=/miracle-source,readonly" \
  miracle-cxx26:g5 bash
```

Inside the running shell, copy source into a writable container path and test:

```sh
cp -R /miracle-source/. /tmp/miracle/
cd /tmp/miracle
bash scripts/ci/verify-container.sh --smoke
bash scripts/ci/verify-container.sh
```

Full validation runs the normal CMake `tests` preset, tests/examples, `add_subdirectory`, `FetchContent`, and installed-package consumers with the activated coherent reference toolchain. The resulting `build/` directory stays in the disposable container in the above procedure.

For regular editing, use a devcontainer (below) so your editor's workspace is bind-mounted and persistent. `nvim`, `clangd`, Git, Ninja, CMake and a non-root `vscode` account are already available. Nothing needs to rebuild LLVM or fetch separate libc++ headers.

## Devcontainer and Codespaces

Local editors that support the dev container specification detect `.devcontainer/devcontainer.json`; GitHub Codespaces uses the same file. The configuration builds this Dockerfile, opens `/workspaces/Miracle` as `vscode`, and runs a **toolchain-only post-create smoke check**, not an expensive compile or an installation script. The devcontainer includes VS Code clangd/CMake extension suggestions; use CLI and Neovim independently of VS Code.

- Codespaces consumes billable compute and storage subject to your GitHub plan and quotas. **No prebuilds are enabled**, no Codespace is auto-created, and no registry push/published image is configured. Check [GitHub Codespaces billing documentation](https://docs.github.com/en/billing/concepts/product-billing/github-codespaces) before creating one.
- The container is deliberately `linux/amd64`. On ARM hosts, image building and module compilation may require emulation or a remote amd64 Docker host. ARM is **not** validated by this phase.
- The Dockerfile creates no credential files or tokens and does not ask for personal access tokens. Codespaces' platform-provided credentials are managed by the hosting platform; do not put them in this repository.
- The actual Codespaces browser onboarding and cost/resource settings **must be verified by the maintainer**. A successful Docker build in CI is not equivalent to having opened a live Codespace.

## Observed clean-container results (2026-10-17)

[Container workflow #39000812865](https://github.com/spwn02/Miracle/actions/runs/39000812865) built the Docker image from scratch using the public pinned archives and ran all C++26 tests/consumers as an unprivileged user. **71/71 CTest tests passed**. The three `add_subdirectory`, `FetchContent`, and installed `find_package` modes compiled, linked and ran. The matching [reference CI #39000813059](https://github.com/spwn02/Miracle/actions/runs/39000813059) was also green.

The first clean-run gate found missing Ubuntu C runtime developer startup objects; installing `build-essential` and `libc6-dev` fixed native C linkage without changing the reference Clang/libc++ selection. The next run uncovered Ubuntu 22.04's outdated Ninja 1.10.1, inadequate for CMake C++ modules. An upstream SHA-256-pinned Ninja **1.13.2** binary resolved that failure. The small container smoke gate now checks real compile/link capability and Ninja >=1.11 before expensive C++26 tests.

This establishes a working, compiler-coherent **Docker** environment. It does not by itself prove that a local editor has attached to the devcontainer or that a billable Codespaces instance has opened and worked interactively.

## Modern Neovim and final regression

Ubuntu 22.04's packaged Neovim 0.6.1 is too old for many current Neovim/LSP workflows. The development image now installs **official Neovim 0.12.5** with an independently verified upstream SHA-256, rather than relying on the distribution version or a mutable `stable` tag. Its smoke test runs both `nvim --version` and `nvim --headless --clean '+quit'`.

The **final** clean [Docker workflow #39001300054](https://github.com/spwn02/Miracle/actions/runs/39001300054) and matching [reference C++26 CI #39001300184](https://github.com/spwn02/Miracle/actions/runs/39001300184), at [commit `7ab69e2`](https://github.com/spwn02/Miracle/commit/7ab69e244d893f7d84e7d8b5d90c0f72ed0ea01a), both succeeded after this editor upgrade. The immutable C++26 compiler/runtime release is unchanged.

## CI and acceptance

The [G5 workflow](https://github.com/spwn02/Miracle/blob/master/.github/workflows/container.yml) is scoped to actual container input changes, manual dispatch and matching PRs. On an Ubuntu 22.04 GitHub runner it:

1. Runs static configuration/security checks.
2. Performs a **clean `linux/amd64` Docker build** from the pinned reference artifacts.
3. Starts a container as the unprivileged user, copies the checkout into a temporary writable path, and runs all CMake and consumer tests.

The workflow publishes no image and requires only `contents: read`. It does not claim Codespaces cost-free operation or silently create billable prebuilds.

Exit gates: passing clean image build, self-tests/examples, all three consumer modes, external editor/devcontainer construction, actual Codespaces open, documentation and maintainer approval. Record skipped/unavailable gates rather than fabricate successes. Track empirical results in [G5-PROGRESS.md](G5-PROGRESS.md).
