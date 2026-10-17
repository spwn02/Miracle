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
  --snapshot cxx26-2026.10.09.2 \
  --revision 040df2b05ea338d107fac7387a2e4db835d746e0 \
  --root "$CXX26_TOOLCHAIN_ROOT"
cmake --version
"$CXX" --version
clangd --version
ninja --version
nvim --version | head -n 1
if [[ "${1:-}" == "--smoke" ]]; then
  echo "PASS: reference C++26 container toolchain smoke test"
  exit 0
fi

cmake --preset tests --fresh -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
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
