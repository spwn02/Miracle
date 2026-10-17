#!/usr/bin/env bash
# G5 headless integration gate for the actual devcontainer.json lifecycle.
# This is not a VS Code editor-attach or GitHub Codespaces acceptance test.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
cli="${DEVCONTAINER_CLI:-devcontainer}"

if ! command -v "$cli" >/dev/null 2>&1; then
  echo "ERROR: install the pinned @devcontainers/cli or set DEVCONTAINER_CLI" >&2
  exit 2
fi
if ! command -v docker >/dev/null 2>&1; then
  echo "ERROR: Docker CLI is required for devcontainer lifecycle verification" >&2
  exit 2
fi

# Use an isolated label: never attach to or destroy a developer's existing container.
test_id="g5-$(date +%s)-$$"
label="miracle.g5.lifecycle=$test_id"
marker="build/.g5-devcontainer-$test_id"

cleanup() {
  rm -f "$repo_root/$marker"
  # Only the container created with this unique test label can be removed.
  local ids
  ids="$(docker ps -aq --filter "label=$label" 2>/dev/null || true)"
  if [[ -n "$ids" ]]; then
    docker rm -f $ids >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT

echo "G5: start the real devcontainer configuration (isolated test container)"
up_output="$("$cli" up --workspace-folder "$repo_root" --id-label "$label" --no-lockfile)"
printf '%s\n' "$up_output"
container_id="$(printf '%s\n' "$up_output" | python3 -c '
import json, sys
result = json.load(sys.stdin)
if result.get("outcome") != "success" or not result.get("containerId"):
    raise SystemExit("ERROR: devcontainer up did not return a successful container ID")
if result.get("remoteWorkspaceFolder") != "/workspaces/Miracle":
    raise SystemExit("ERROR: devcontainer workspace path differs from the contract")
if result.get("remoteUser") != "vscode":
    raise SystemExit("ERROR: devcontainer remoteUser differs from the contract")
print(result["containerId"])
')"

# postCreateCommand is a non-blocking lifecycle step by default. Explicitly wait
# for it (or execute it when not already run), so a background failure fails CI.
"$cli" run-user-commands --workspace-folder "$repo_root" --id-label "$label"

echo "G5: exercise remoteUser, live workspace mount, toolchain, and clangd input"
"$cli" exec --workspace-folder "$repo_root" --id-label "$label" \
  bash -lc '
    set -euo pipefail
    test "$(pwd -P)" = /workspaces/Miracle
    test "$(id -un)" = vscode
    test "$(id -u)" -ne 0
    test "$CXX" = "$CXX26_TOOLCHAIN_ROOT/bin/clang++"
    test -f "$CXX26_CMAKE_TOOLCHAIN_FILE"
    test -f CMakePresets.json
    bash scripts/ci/verify-container.sh --smoke
    cmake --preset tests --fresh -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
    test -s build/tests/compile_commands.json
    clangd --version
    nvim --headless --clean -c "luafile scripts/ci/verify-nvim-lsp.lua"
    mkdir -p build
    printf "persisted\n" > "$1"
  ' bash "$marker"

# An actual Docker restart, followed by remote exec, tests that workspace data
# stays accessible after reattachment. This is not a UI/editor interaction.
docker restart "$container_id" >/dev/null
"$cli" exec --workspace-folder "$repo_root" --id-label "$label" \
  bash -lc '
    set -euo pipefail
    test "$(pwd -P)" = /workspaces/Miracle
    test "$(id -un)" = vscode
    test "$(cat "$1")" = persisted
    test -s build/tests/compile_commands.json
  ' bash "$marker"
test "$(cat "$repo_root/$marker")" = persisted

echo "PASS: real headless devcontainer startup, postCreate, workspace, non-root user, configure, and restart persistence"
