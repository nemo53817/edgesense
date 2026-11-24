#!/usr/bin/env bash
# Build and test presets on Linux inside Docker, with gcc and clang.
#
#   scripts/docker-test.sh                 # all compilers x all presets
#   scripts/docker-test.sh gcc tsan        # one compiler, one preset
#   scripts/docker-test.sh clang "debug asan"
#
# The source is copied into the container (minus build/), so host build dirs are untouched.
set -euo pipefail

compilers="${1:-gcc clang}"
presets="${2:-debug release asan tsan}"
image="edgesense-linux-dev"
repo="$(cd "$(dirname "$0")/.." && pwd)"

docker build -q -t "$image" -f "$repo/docker/linux-dev.Dockerfile" "$repo/docker" >/dev/null

docker run --rm -v "$repo:/src:ro" -e COMPILERS="$compilers" -e PRESETS="$presets" "$image" \
  bash -c '
    set -uo pipefail
    failed=()
    for cc in $COMPILERS; do
      case $cc in
        gcc)   export CC=gcc   CXX=g++ ;;
        clang) export CC=clang CXX=clang++ ;;
        *)     echo "unknown compiler: $cc"; exit 2 ;;
      esac
      rm -rf /work && mkdir /work
      tar -C /src --exclude=./build --exclude=./.git -cf - . | tar -C /work -xf -
      cd /work
      for p in $PRESETS; do
        echo "===== linux/$(uname -m) / $cc / $p"
        if cmake --preset "$p" >/dev/null && cmake --build --preset "$p" \
           && ctest --preset "$p"; then :; else failed+=("$cc/$p"); fi
      done
    done
    if ((${#failed[@]})); then echo "FAILED: ${failed[*]}"; exit 1; fi
    echo "ALL PASSED"
  '
