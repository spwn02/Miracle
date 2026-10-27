#!/usr/bin/env bash
# Runs inside the G5 developer image, from a Miracle repository checkout.
set -euo pipefail

if [[ $# -gt 1 || ( $# -eq 1 && "$1" != "--smoke" ) ]]; then
  echo "Usage: bash scripts/ci/verify-container.sh [--smoke]" >&2
  exit 2
fi

test -n "${CXX26_TOOLCHAIN_ROOT:-}"
test -n "${CXX26_CMAKE_TOOLCHAIN_FILE:-}"
test -x "$CXX"
test -f "$CXX26_CMAKE_TOOLCHAIN_FILE"
# The archive's activate.sh selects the exact compiler, libc++ and ABI unit.
# shellcheck disable=SC1090
source "$CXX26_TOOLCHAIN_ROOT/share/clang-cxx26/activate.sh"
test "$CXX" = "$CXX26_TOOLCHAIN_ROOT/bin/clang++"
python3 scripts/ci/verify-reference-toolchain.py \
  --manifest "$CXX26_TOOLCHAIN_ROOT/share/clang-cxx26/manifest.json" \
  --snapshot cxx26-2026.10.27 \
  --revision b56ae4e61cf1f46495f0fb08f2b9fc270aad162c \
  --root "$CXX26_TOOLCHAIN_ROOT"
cmake --version
"$CXX" --version
clangd --version
ninja --version
python3 -c 'import subprocess; v = subprocess.check_output(["ninja", "--version"], text=True).strip(); assert tuple(map(int, v.split(".")[:2])) >= (1, 11), v'
nvim --version | head -n 1
nvim --headless --clean '+quit'
python3 -c 'import subprocess; v = subprocess.check_output(["nvim", "--version"], text=True).splitlines()[0]; assert v.startswith("NVIM v0.12.5"), v'
# Compile and LINK against the configured libc++ and Ubuntu C runtime.
# This also detects missing crt*.o and glibc development libraries before
# the much heavier CMake module build starts.
probe_dir="$(mktemp -d)"
trap 'rm -rf "$probe_dir"' EXIT
printf 'int main() { return 0; }\n' > "$probe_dir/main.cxx"
"$CXX" -std=c++26 -stdlib=libc++ -fuse-ld=lld \
  "$probe_dir/main.cxx" -o "$probe_dir/link-probe"
"$probe_dir/link-probe"
if [[ "${1:-}" == "--smoke" ]]; then
  echo "PASS: reference C++26 container toolchain smoke test"
  exit 0
fi

if ! cmake --preset tests --fresh -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"; then
  echo "ERROR: container CMake configuration failed; capability probe logs follow:" >&2
  find build/tests/CMakeFiles/MiracleCapabilities -name configure.log -type f \
    -print -exec tail -n 160 {} \; 2>/dev/null || true
  exit 1
fi
cmake --build --preset tests --parallel 2
ctest --preset tests --output-on-failure

cmake -S tests/consumer/add-subdirectory -B build/g5-consumer-add -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE" \
  -DMIRACLE_SOURCE_DIR="$PWD"
cmake --build build/g5-consumer-add --parallel 2
./build/g5-consumer-add/miracle_consumer

cmake -S tests/consumer/fetch-content -B build/g5-consumer-fetch -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE" \
  -DFETCHCONTENT_SOURCE_DIR_MIRACLE="$PWD"
cmake --build build/g5-consumer-fetch --parallel 2
./build/g5-consumer-fetch/miracle_consumer

cmake --preset release --fresh -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
cmake --build --preset release --parallel 2
cmake --install build/release --prefix "$PWD/build/g5-install"
cmake -S tests/consumer/find-package -B build/g5-consumer-package -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE" \
  -DCMAKE_PREFIX_PATH="$PWD/build/g5-install"
cmake --build build/g5-consumer-package --parallel 2
./build/g5-consumer-package/miracle_consumer
echo "PASS: all reference C++26 container self-tests, examples and consumers"
