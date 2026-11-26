#!/usr/bin/env bash
# G5 Docker-only integration: isolated writable bind-mount + real Neovim/clangd LSP.
# Never alters a developer's live checkout.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
docker="${DOCKER:-docker}"
image="${IMAGE:-miracle-cxx26:g5}"

if ! command -v "$docker" >/dev/null 2>&1; then
  echo "ERROR: Docker CLI required for G5 workspace verification" >&2
  exit 2
fi

workspace_root="$(mktemp -d)"
trap 'rm -rf "$workspace_root"' EXIT
mkdir -p "$workspace_root/Miracle"
cp -R "$repo_root/." "$workspace_root/Miracle/"

uid="$(id -u)"
gid="$(id -g)"
test "$uid" -ne 0 || { echo "ERROR: run workspace acceptance as a non-root host user" >&2; exit 2; }

mount="type=bind,source=$workspace_root/Miracle,target=/workspace/Miracle"

echo "G5: writable, disposable Docker workspace with matching host UID/GID"
"$docker" run --rm --platform linux/amd64 \
  --user "$uid:$gid" --env HOME=/tmp \
  --mount "$mount" --workdir /workspace/Miracle \
  "$image" bash -lc '
    set -euo pipefail
    test "$(pwd -P)" = /workspace/Miracle
    test "$(id -u)" -ne 0
    test "$CXX" = "$CXX26_TOOLCHAIN_ROOT/bin/clang++"
    test -f "$CXX26_CMAKE_TOOLCHAIN_FILE"
    bash scripts/ci/verify-container.sh --smoke
    cmake --preset tests --fresh -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
    test -s build/tests/compile_commands.json
    nvim --headless --clean -c "luafile scripts/ci/verify-nvim-lsp.lua"
    printf "persisted\n" > build/.g5-docker-workspace-marker
  '

echo "G5: second fresh container must see the same writable workspace"
"$docker" run --rm --platform linux/amd64 \
  --user "$uid:$gid" --env HOME=/tmp \
  --mount "$mount" --workdir /workspace/Miracle \
  "$image" bash -lc '
    set -euo pipefail
    test "$(pwd -P)" = /workspace/Miracle
    test "$(id -u)" -ne 0
    test "$(cat build/.g5-docker-workspace-marker)" = persisted
    test -s build/tests/compile_commands.json
  '
test "$(cat "$workspace_root/Miracle/build/.g5-docker-workspace-marker")" = persisted
echo "PASS: writable Docker workspace, pinned compiler, Neovim-clangd LSP and cross-container persistence"
