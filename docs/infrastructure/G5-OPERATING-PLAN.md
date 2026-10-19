# G5 — Reproducible C++26 Docker environment

**Scope:** Docker CLI/Neovim/clangd, Linux/amd64 (local Linux and emulated/remote amd64 on macOS)
**Authority:** [GitHub Infrastructure Contract](GITHUB-INFRASTRUCTURE-CONTRACT.md), §8/§10, plus the maintainer's [2026-10-19 scope decision](G5-PROGRESS.md)
**Tracker:** [Issue #5](https://github.com/spwn02/Miracle/issues/5)
**Status:** G5 Docker-only scope ACCEPTED and CLOSED (2026-10-19); Codespaces DECLINED / NOT TESTED, excluded
**Evidence:** [G5 progress ledger](G5-PROGRESS.md)

## Maintainer scope decision — 2026-10-19

The maintainer reported that the **previous local interactive devcontainer passed**, approved G5 for the **revised Docker-only development workflow**, and explicitly **declined Codespaces because it is paid**. The prior devcontainer test is legitimate historical acceptance evidence. Codespaces startup, storage, billing and browser editor were **not executed**; they are **not passed** and have been removed from the active implementation scope by explicit decision, not waived as test successes.

Miracle now exposes **one [root Dockerfile](../../Dockerfile)**. Neither `.devcontainer/devcontainer.json` nor a Dev Containers CLI is required. There are no Codespaces prebuilds, images pushed to registries, checked-in credentials, or paid cloud instances. This does not change the authoritative C++26 compiler, CMake preset, library API or CI test criteria.

The original infrastructure contract described devcontainer/Codespaces support as **proposed** and the G5 release gate as demonstrated reproducible compiler-compatible builds; the explicit maintainer scope change is recorded in that contract, this plan and [issue #5](https://github.com/spwn02/Miracle/issues/5).

## Reference compiler identity

Miracle's single compiler/runtime identity is Clang + matching libc++/libc++abi and standard-module metadata from one signed-off immutable release. Distribution Clang/libstdc++ must not substitute for the reference unit. Docker targets Ubuntu 22.04 and `linux/amd64`.

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


## CLI workflow

The root [Makefile](../../Makefile) is a convenience interface; **CMakePresets.json remains authoritative**. `make` does not install a compiler on your host, publish an image, build LLVM from source or generate a release.

| Command | Action |
| --- | --- |
| `make docker-build` | Build verified pinned image from the root Dockerfile |
| `make docker-dev` | Open a persistent **writable host-bind workspace** in an interactive Bash container; runs as the host UID/GID |
| `make docker-smoke` | Run reference toolchain compile/link smoke in a disposable container |
| `make docker-test` | Execute all C++26 self-tests and three consumer modes in a disposable copy |
| `make docker-verify NO_CACHE=1` | Clean image build and complete test/consumer verification |
| `make docker-shell` | Temporary **nonpersistent** writable copy of a read-only host checkout |
| `make configure`, `make build`, `make test`, `make verify` | Use the pinned selected toolchain with the canonical CMake presets (inside Docker or activated on host) |
| `make docs`, `make docs-serve` | mdBook build and local preview |

From a normal Linux user account with Docker access:

```sh
git clone https://github.com/spwn02/Miracle.git
cd Miracle
make docker-build
make docker-dev
# Inside the Docker shell, in /workspace/Miracle:
bash scripts/ci/verify-container.sh --smoke
make verify
nvim src/private/Error.cxx
```

`make docker-dev` uses a bind mount of the **real checkout**, not an ephemeral copy. Edits and `build/` output survive container exit because they live on the host. It supplies `--user "$(id -u):$(id -g)"` and `HOME=/tmp` so files are written with host UID/GID rather than root. Avoid using this target as root and do not store credentials under the temporary container home.

**Workspace preflight:** `make docker-dev` now checks for stale `CMakeCache.txt` directories before starting Docker and **rejects `sudo`/root invocation** rather than silently writing root-owned output. `make configure`/`make verify` check their selected CMake preset too. Nested Switch capability caches can retain the old `/workspaces/Miracle` devcontainer path even after `cmake --fresh`, while plain Docker mounts at `/workspace/Miracle`. If the preflight reports incompatible paths, exit the container and **move the affected `build/<preset>` directory to an unused backup name** (for example `mv build/tests build/tests.pre-docker-migration`) before retrying. If an earlier sudo session created root-owned artifacts, repair their ownership explicitly on the host. The preflight never deletes, renames or `chown`s files automatically.

The image defaults to a non-root `miracle` user (UID 1000) and `/workspace/Miracle` directory; the live shell intentionally overrides UID/GID to match your host. Neither `whoami` nor a named passwd entry is guaranteed when your host UID differs from 1000; use `id -u` to verify. This is not a failure.

Equivalent direct Docker command:

```sh
docker build --platform linux/amd64 -f Dockerfile -t miracle-cxx26:g5 .
docker run --rm -it --platform linux/amd64 \
  --user "$(id -u):$(id -g)" --env HOME=/tmp \
  --mount "type=bind,source=$PWD,target=/workspace/Miracle" \
  --workdir /workspace/Miracle miracle-cxx26:g5 bash
```

**Host prerequisites:** Docker with `linux/amd64` support, internet access for the first image build, and sufficient free space/RAM (start with 8 GiB RAM; 16 GiB+ for parallel module work). On Apple Silicon, expect `amd64` emulation overhead; ARM performance has not been validated. Docker-group membership gives root-equivalent privileges on Linux; use a trusted host and do not weaken Docker socket permissions.

The pinned toolchain and tool downloads are checksum-verified and version-fixed. Ubuntu APT packages remain time-dependent; this is compiler/build-recipe reproducibility, **not** a fully hermetic or byte-identical filesystem.

## Docker-only integration gate

The [G5 container workflow](../../.github/workflows/container.yml) performs the no-cache image build and canonical [reference test/consumer suite](../../scripts/ci/verify-container.sh), including **71 CTest cases** and the three `add_subdirectory`, `FetchContent`, and installed `find_package` modes.

The [Docker workspace acceptance script](../../scripts/ci/verify-docker-workspace.sh) additionally creates a disposable **host-side copy** of the checkout, mounts it read/write with matching UID/GID, runs CMake configuration and confirms `build/tests/compile_commands.json`, and executes the [Neovim/clangd LSP probe](../../scripts/ci/verify-nvim-lsp.lua). The probe opens real Miracle C++ source and requests symbols via `textDocument/documentSymbol`. A fresh second Docker container must observe the generated marker and compilation database from the first; the host also verifies persistence. The script removes its temporary copy, **never the developer's checkout**.

Local reproducibility:

```sh
make docker-build NO_CACHE=1
bash scripts/ci/verify-docker-workspace.sh
make docker-test
```

Automated LSP protocol success proves a real source-symbol exchange, **not** accurate live C++26 diagnostics or interactive keyboard ergonomics; these remain separate statements. No Codespaces acceptance is implied.

## Historical evidence and boundaries

Earlier devcontainer integration and manual local acceptance are retained in the [G5 progress ledger](G5-PROGRESS.md), including [PR #12](https://github.com/spwn02/Miracle/pull/12), [PR #13](https://github.com/spwn02/Miracle/pull/13) and the successful [headless Neovim↔clangd container CI](https://github.com/spwn02/Miracle/actions/runs/39005531835) (71/71 CTest, 3/3 consumers). They document how the working image was reached, not a requirement to retain VS Code integration.

**Verified Docker-only run:** [G5 #39192903330](https://github.com/spwn02/Miracle/actions/runs/39192903330) **SUCCESS** at PR revision `6f131985` (**71/71 CTest**, **3/3 consumers**, real writable mount/LSP/persistence). [Reference CI #39192903211](https://github.com/spwn02/Miracle/actions/runs/39192903211), [documentation #39192903142](https://github.com/spwn02/Miracle/actions/runs/39192903142) and [code scanning #39192905595](https://github.com/spwn02/Miracle/actions/runs/39192905595) **SUCCESS**. [PR #14](https://github.com/spwn02/Miracle/pull/14) squash-merged as [`4965346`](https://github.com/spwn02/Miracle/commit/4965346cc1c2ee4d1e52a9929498728a18812558).

**Final acceptance (2026-10-19):** the maintainer reported the local setup is working and explicitly authorized G5 closure. Docker-only technical verification passed CI; local interactive usability and final acceptance are maintainer-reported. [Issue #5](https://github.com/spwn02/Miracle/issues/5) is **closed as completed for the revised Docker-only scope**. Its historical Codespaces checkbox remains **unchecked: declined/not tested**, excluded by explicit maintainer decision; no Codespaces behavior or cost was verified. Do not rewrite the historical evidence or interpret closure as Codespaces acceptance.
