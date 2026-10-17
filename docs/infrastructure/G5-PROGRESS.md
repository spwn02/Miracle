# G5 — Docker, devcontainers and Codespaces: Progress

**Updated:** 2026-10-17
**Status:** Initial implementation committed; clean Docker build and live Codespaces acceptance pending
**Project:** [Miracle Development](https://github.com/users/spwn02/projects/3)
**Issue:** [#5](https://github.com/spwn02/Miracle/issues/5)
**Contract:** [G5 operating plan](G5-OPERATING-PLAN.md)

## Implementation

- Official Ubuntu 22.04 **Linux/amd64** base image pinned by manifest SHA-256.
- Immutable reference compiler \`cxx26-2026.10.09.2\` and source revision from existing reference CI; pinned and verified compiler tarball, release manifest and published SHA256SUMS.
- Matching packaged libc++ runtime and \`import std\` metadata used; no distribution Clang/libstdc++ swap.
- Verified Kitware CMake 4.4.3 archive, Ninja, clangd, Python, Git and Neovim.
- Non-root \`vscode\` devcontainer with a quick post-create toolchain smoke check; no prebuild or registry.
- Source-controlled, read-only [G5 Docker validation workflow](https://github.com/spwn02/Miracle/blob/master/.github/workflows/container.yml) that builds the image **without Docker cache** and runs reference tests/examples and all three consumer modes.
- Self-contained reference toolchain manifest validator and static identity/configuration guard.
- [G5 operating plan](G5-OPERATING-PLAN.md) documents provenance, commands, resource requirements, billing caveats and immutable vs time-dependent dependencies.

## Verification evidence

- [ ] Source/JSON/Dockerfile and workflow configuration pass early reference CI syntax checks.
- [ ] Clean Docker image successfully builds and verifies pinned manifest/checksums.
- [ ] Container test suite, examples, source and installed-package consumers pass.
- [ ] Local Docker devcontainer opens and clangd/Neovim workflow is usable.
- [ ] Maintainer successfully opens Codespaces and checks toolchain and workspace behavior.
- [ ] Maintainer explicitly accepts G5.

**Not yet accepted.** The presence of a Dockerfile or a successful *non-container* CI run does not establish that the image builds or Codespaces works. Keep issue #5 open until end-to-end evidence is collected. Container files cannot establish fixed Codespaces pricing or a fully hermetic APT package closure.
