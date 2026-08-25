#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
toolchain="$repo_root/cmake/toolchains/GCC.cmake"

if [[ ! -f "$toolchain" ]]; then
  echo "GCC compatibility toolchain not found: $toolchain" >&2
  exit 1
fi

cd "$repo_root"

echo "== GCC compatibility toolchain =="
g++ --version
cmake --version
ninja --version

echo "== Miracle debug + example =="
cmake --preset debug --fresh \
  -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
  -DMIRACLE_BUILD_EXAMPLES=ON \
  -DMIRACLE_BUILD_TESTS=OFF
cmake --build --preset debug
"$repo_root/build/debug/examples/MiracleQuickstart"

echo "== add_subdirectory consumer =="
cmake \
  -S tests/consumer/add-subdirectory \
  -B build/gcc-consumer-add \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
  -DMIRACLE_SOURCE_DIR="$repo_root"
cmake --build build/gcc-consumer-add
"$repo_root/build/gcc-consumer-add/miracle_consumer"

echo "== FetchContent consumer =="
cmake \
  -S tests/consumer/fetch-content \
  -B build/gcc-consumer-fetch \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
  -DFETCHCONTENT_SOURCE_DIR_MIRACLE="$repo_root"
cmake --build build/gcc-consumer-fetch
"$repo_root/build/gcc-consumer-fetch/miracle_consumer"

echo "== Release + installed-package consumer =="
cmake --preset release --fresh \
  -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
  -DMIRACLE_BUILD_TESTS=OFF
cmake --build --preset release

install_prefix="$repo_root/build/gcc-install"
rm -rf "$install_prefix"
cmake --install build/release --prefix "$install_prefix"

cmake \
  -S tests/consumer/find-package \
  -B build/gcc-consumer-package \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
  -DCMAKE_PREFIX_PATH="$install_prefix"
cmake --build build/gcc-consumer-package
"$repo_root/build/gcc-consumer-package/miracle_consumer"
