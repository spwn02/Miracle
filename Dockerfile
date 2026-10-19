# Miracle G5: Docker-only pinned Linux/x86-64 reference environment.
# The single-platform digest is the official Ubuntu 22.04 amd64 image.
FROM --platform=linux/amd64 ubuntu:22.04@sha256:08ea48a03a3e78ebc7cd526e6a275053223aadd88bfc09cc49b06d5281525fde

ARG DEBIAN_FRONTEND=noninteractive
ARG CXX26_SNAPSHOT=cxx26-2026.10.09.2
ARG CXX26_REVISION=040df2b05ea338d107fac7387a2e4db835d746e0
ARG CXX26_ASSET=clang-cxx26-2026.10.09.2-linux-x86_64
ARG CXX26_ARCHIVE_SHA256=7965d447d32022557fc767bb5b8524e9ca6d9318b5d0c0ba3457b32e6632a1e8
ARG CXX26_MANIFEST_SHA256=178c91d2f7d2bf59532223581563eee5c3babbd136b4c4496f92c69971b6ffb1
ARG CXX26_SUMS_SHA256=8ed3810fa391f056ad840c9331a3598bb24257a16fb1f05e9752bbd7301c1b5a
ARG NVIM_VERSION=0.12.5
ARG NVIM_SHA256=bce0f56eda1f1b1db6eee8f4133d7a38813ea07933837dd1777411ca384c6875
ARG NINJA_VERSION=1.13.2
ARG NINJA_SHA256=5749cbc4e668273514150a80e387a957f933c6ed3f5f11e03fb30955e2bbead6
ARG CMAKE_VERSION=4.4.3
ARG CMAKE_SHA256=d6c83076c575bc00b823522ac974bda66d0af05d6ddc30e739c12385cf32c6cc

# Ubuntu's build-essential/libc6-dev supply the C runtime crt*.o and native
# linker support files, NOT an alternate selected C++ compiler/standard library.
# The reference compiler, its matching libc++ and CMake are SHA-256 pinned.
RUN set -eux; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
      bash build-essential libc6-dev ca-certificates curl git ninja-build python3 python3-venv \
      zstd tar xz-utils sudo \
      libstdc++6 libgcc-s1 libatomic1 zlib1g libzstd1 libxml2 \
      libz3-4 libedit2 libtinfo6 libncurses6 libssl3; \
    rm -rf /var/lib/apt/lists/*

# The checksum, release manifest, and archive are verified independently.
# Toolchain is installed as a coherent unit, without system clang/libstdc++.
COPY scripts/ci/verify-reference-toolchain.py /usr/local/libexec/verify-reference-toolchain.py
RUN set -eux; \
    install -d /opt/cxx26 /tmp/cxx26-download; \
    cd /tmp/cxx26-download; \
    for extension in tar.zst manifest.json SHA256SUMS; do \
      curl --fail --location --retry 3 --silent --show-error \
        "https://github.com/spwn02/clang-cxx26/releases/download/$CXX26_SNAPSHOT/$CXX26_ASSET.$extension" \
        -o "$CXX26_ASSET.$extension"; \
    done; \
    printf '%s  %s\n' "$CXX26_ARCHIVE_SHA256" "$CXX26_ASSET.tar.zst" | sha256sum --check; \
    printf '%s  %s\n' "$CXX26_MANIFEST_SHA256" "$CXX26_ASSET.manifest.json" | sha256sum --check; \
    printf '%s  %s\n' "$CXX26_SUMS_SHA256" "$CXX26_ASSET.SHA256SUMS" | sha256sum --check; \
    sha256sum --check "$CXX26_ASSET.SHA256SUMS"; \
    python3 /usr/local/libexec/verify-reference-toolchain.py \
      --manifest "$CXX26_ASSET.manifest.json" \
      --snapshot "$CXX26_SNAPSHOT" \
      --revision "$CXX26_REVISION"; \
    tar --use-compress-program=unzstd -xf "$CXX26_ASSET.tar.zst" -C /opt/cxx26; \
    root="/opt/cxx26/clang-$CXX26_SNAPSHOT"; \
    cmp "$CXX26_ASSET.manifest.json" "$root/share/clang-cxx26/manifest.json"; \
    python3 /usr/local/libexec/verify-reference-toolchain.py \
      --manifest "$root/share/clang-cxx26/manifest.json" \
      --snapshot "$CXX26_SNAPSHOT" --revision "$CXX26_REVISION" --root "$root"; \
    rm -rf /tmp/cxx26-download

RUN set -eux; \
    curl --fail --location --retry 3 --silent --show-error \
      "https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-linux-x86_64.tar.gz" \
      -o /tmp/cmake.tar.gz; \
    printf '%s  %s\n' "$CMAKE_SHA256" /tmp/cmake.tar.gz | sha256sum --check; \
    install -d /opt/cmake; \
    tar -xzf /tmp/cmake.tar.gz --strip-components=1 -C /opt/cmake; \
    rm /tmp/cmake.tar.gz; \
    /opt/cmake/bin/cmake --version

# Ubuntu 22.04 ships Ninja 1.10, which cannot generate C++ modules.
# Use the SHA-256-pinned upstream Ninja binary needed by CMake 4.4.
RUN set -eux; \
    curl --fail --location --retry 3 --silent --show-error \
      "https://github.com/ninja-build/ninja/releases/download/v$NINJA_VERSION/ninja-linux.zip" \
      -o /tmp/ninja.zip; \
    printf '%s  %s\n' "$NINJA_SHA256" /tmp/ninja.zip | sha256sum --check; \
    install -d /opt/ninja; \
    python3 -m zipfile -e /tmp/ninja.zip /opt/ninja; \
    chmod 0755 /opt/ninja/ninja; \
    rm /tmp/ninja.zip; \
    /opt/ninja/ninja --version

# Modern Neovim is needed for current LSP/plugins; Ubuntu 22.04 ships 0.6.
# Install the verified official release tarball, not a mutable stable tag.
RUN set -eux; \
    curl --fail --location --retry 3 --silent --show-error \
      "https://github.com/neovim/neovim/releases/download/v$NVIM_VERSION/nvim-linux-x86_64.tar.gz" \
      -o /tmp/nvim.tar.gz; \
    printf '%s  %s\n' "$NVIM_SHA256" /tmp/nvim.tar.gz | sha256sum --check; \
    tar -xzf /tmp/nvim.tar.gz -C /opt; \
    rm /tmp/nvim.tar.gz; \
    /opt/nvim-linux-x86_64/bin/nvim --version

ENV CXX26_TOOLCHAIN_ROOT=/opt/cxx26/clang-cxx26-2026.10.09.2
ENV CXX26_CMAKE_TOOLCHAIN_FILE=/opt/cxx26/clang-cxx26-2026.10.09.2/share/clang-cxx26/toolchain.cmake
ENV CC=/opt/cxx26/clang-cxx26-2026.10.09.2/bin/clang
ENV CXX=/opt/cxx26/clang-cxx26-2026.10.09.2/bin/clang++
ENV PATH="/opt/nvim-linux-x86_64/bin:/opt/ninja:/opt/cmake/bin:/opt/cxx26/clang-cxx26-2026.10.09.2/bin:${PATH}"
ENV LD_LIBRARY_PATH=/opt/cxx26/clang-cxx26-2026.10.09.2/lib
ENV LANG=C.UTF-8

# Default non-root Docker user; bind mounts may override to matching host UID/GID.
RUN useradd --create-home --uid 1000 --shell /bin/bash miracle \
    && install -d -o miracle -g miracle /workspace/Miracle
USER miracle
WORKDIR /workspace/Miracle
CMD ["bash"]
