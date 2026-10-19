# Miracle command-line entry point; CMake presets and root Dockerfile remain authoritative.
.DEFAULT_GOAL := help
SHELL := /bin/sh

PRESET ?= tests
TOOLCHAIN ?= $(CXX26_CMAKE_TOOLCHAIN_FILE)
JOBS ?= 2
IMAGE ?= miracle-cxx26:g5
DOCKER ?= docker
NO_CACHE ?= 0

# Deliberate allowlist: a typo must not select a different CMake configuration.
ifneq ($(filter $(PRESET),tests debug release),$(PRESET))
$(error Unknown PRESET '$(PRESET)'; use tests, debug or release)
endif

.PHONY: help configure build test verify clean docker-build docker-dev docker-shell docker-smoke docker-test docker-verify docs docs-serve

help: ## Show the developer commands
	@printf '%s\n' \
	  'Miracle — CMake + Docker entry points' \
	  '' \
	  '  make configure [PRESET=tests|debug|release]  Fresh CMake configure (requires reference toolchain)' \
	  '  make build     [PRESET=tests|debug|release]  Build, configure if missing' \
	  '  make test                                  Build and run tests preset' \
	  '  make verify                                Fresh configure + full local tests' \
	  '  make clean     [PRESET=tests|debug|release]  Clean configured preset output' \
	  '  make docker-build                          Build pinned Linux/amd64 image' \
	  '  make docker-smoke                          Quick reference compiler check in image' \
	  '  make docker-test                           Full container tests + consumers' \
	  '  make docker-verify                         Build image then run full container tests' \
	  '  make docker-dev                            Live writable checkout mounted in Docker' \
	  '  make docker-shell                          Temporary writable checkout in container' \
	  '  make docs                                  Build and verify mdBook' \
	  '  make docs-serve                            Serve mdBook locally' \
	  '' \
	  'Settings: PRESET=tests, JOBS=2, IMAGE=miracle-cxx26:g5, TOOLCHAIN=<path>, NO_CACHE=0' \
	  'Activate the pinned toolchain first, or set TOOLCHAIN to its toolchain.cmake.' \
	  'No reference toolchain installed? Use: make docker-verify'

configure: ## Configure explicitly with coherent pinned C++26 toolchain
	@test -n "$(TOOLCHAIN)" || { printf '%s\n' 'ERROR: reference toolchain not selected. Source share/clang-cxx26/activate.sh or pass TOOLCHAIN=/path/to/toolchain.cmake; otherwise use make docker-verify.' >&2; exit 2; }
	@test -f "$(TOOLCHAIN)" || { printf 'ERROR: missing reference toolchain file: %s\n' '$(TOOLCHAIN)' >&2; exit 2; }
	@python3 scripts/ci/check-build-cache.py --expected-root "$(CURDIR)" --preset "$(PRESET)"
	cmake --preset "$(PRESET)" --fresh -DCMAKE_TOOLCHAIN_FILE="$(TOOLCHAIN)"

build: ## Build selected CMake preset; configure once if required
	@if test ! -f "build/$(PRESET)/CMakeCache.txt"; then \
	  $(MAKE) --no-print-directory configure PRESET="$(PRESET)"; \
	fi
	cmake --build --preset "$(PRESET)" --parallel "$(JOBS)"

test: ## Run the canonical tests preset, regardless of PRESET
	@$(MAKE) --no-print-directory build PRESET=tests
	ctest --preset tests --output-on-failure

verify: ## Fresh configuration and complete reference test run
	@$(MAKE) --no-print-directory configure PRESET=tests
	@$(MAKE) --no-print-directory test

clean: ## Clean configured preset; never remove arbitrary directories
	@test -f "build/$(PRESET)/CMakeCache.txt" || { printf 'ERROR: build/$(PRESET) is not configured\n' >&2; exit 2; }
	cmake --build --preset "$(PRESET)" --target clean

docker-build: ## Build the pinned G5 image for Linux/amd64
	$(DOCKER) build --platform linux/amd64 $(if $(filter 1,$(NO_CACHE)),--no-cache,) -f Dockerfile -t "$(IMAGE)" .

docker-smoke: ## Run toolchain smoke test without compiling Miracle
	$(DOCKER) run --rm --platform linux/amd64 \
	  --mount 'type=bind,source=$(CURDIR),target=/miracle-source,readonly' \
	  "$(IMAGE)" bash -c 'set -eu; mkdir -p /tmp/miracle; cp -R /miracle-source/. /tmp/miracle/; cd /tmp/miracle; bash scripts/ci/verify-container.sh --smoke'

docker-test: ## Run C++26 tests and all 3 consumer modes inside the G5 image
	$(DOCKER) run --rm --platform linux/amd64 \
	  --mount 'type=bind,source=$(CURDIR),target=/miracle-source,readonly' \
	  "$(IMAGE)" bash -c 'set -eu; mkdir -p /tmp/miracle; cp -R /miracle-source/. /tmp/miracle/; cd /tmp/miracle; bash scripts/ci/verify-container.sh'

docker-verify: docker-build ## Build the image and run the complete container suite
	@$(MAKE) --no-print-directory docker-test IMAGE="$(IMAGE)" DOCKER="$(DOCKER)"

docker-dev: ## Run shell with live host checkout mounted writable as host UID/GID
	@python3 scripts/ci/check-build-cache.py --expected-root /workspace/Miracle --require-non-root
	$(DOCKER) run --rm -it --platform linux/amd64 \
	  --user "$$(id -u):$$(id -g)" --env HOME=/tmp \
	  --mount 'type=bind,source=$(CURDIR),target=/workspace/Miracle' \
	  --workdir /workspace/Miracle \
	  "$(IMAGE)" bash

docker-shell: ## Open Bash in temporary writable image workspace (source is read-only)
	$(DOCKER) run --rm -it --platform linux/amd64 \
	  --mount 'type=bind,source=$(CURDIR),target=/miracle-source,readonly' \
	  "$(IMAGE)" bash -c 'set -eu; mkdir -p /tmp/miracle; cp -R /miracle-source/. /tmp/miracle/; cd /tmp/miracle; exec bash'

docs: ## Build and validate canonical mdBook documentation
	mdbook build
	python3 scripts/docs/verify-site.py site

docs-serve: ## Serve mdBook without touching published Pages
	mdbook serve
