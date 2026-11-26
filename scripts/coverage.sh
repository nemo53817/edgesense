#!/usr/bin/env bash
# Build with coverage instrumentation, run the tests, and report coverage of our own code
# (include/ and src/; not tests, GoogleTest or app/).
#
#   scripts/coverage.sh            # report in build/coverage/report/index.html
#
# Outputs (all under build/coverage/):
#   report/index.html   per-file, per-line HTML report
#   summary.md          Markdown table (CI appends it to the job summary)
#
# Needs gcovr (https://gcovr.com). If it is not on PATH, a pinned copy is installed into a
# private virtualenv under build/ (no system-wide install).
set -euo pipefail

gcovr_version="8.8"
repo="$(cd "$(dirname "$0")/.." && pwd)"
cd "$repo"
out="build/coverage"

cmake --preset coverage >/dev/null
find "$out" -name '*.gcda' -delete  # counters from a previous run would be added to this one
cmake --build --preset coverage
ctest --preset coverage

if command -v gcovr >/dev/null && [[ "$(gcovr --version | head -1)" == "gcovr $gcovr_version" ]]; then
  gcovr=gcovr
else
  venv="build/.gcovr-venv"
  [[ -x "$venv/bin/gcovr" ]] || {
    python3 -m venv "$venv"
    "$venv/bin/pip" install -q --disable-pip-version-check "gcovr==$gcovr_version"
  }
  gcovr="$venv/bin/gcovr"
fi

# The gcov tool must match the compiler that produced the .gcno/.gcda files. The compiler ID is
# not in CMakeCache.txt; CMake records it in CMakeFiles/<version>/CMakeCXXCompiler.cmake.
compiler_id="$(sed -n 's/^set(CMAKE_CXX_COMPILER_ID "\(.*\)")$/\1/p' "$out"/CMakeFiles/*/CMakeCXXCompiler.cmake)"
extra=()
case "$compiler_id" in
  GNU) gcov="gcov" ;;
  AppleClang) gcov="xcrun llvm-cov gcov" ;;
  Clang)  # llvm-cov from the same LLVM install as the compiler (on Ubuntu: package llvm-<N>)
    cxx="$(sed -n 's/^CMAKE_CXX_COMPILER:FILEPATH=//p' "$out/CMakeCache.txt")"
    llvm_cov="$(dirname "$(readlink -f "$cxx")")/llvm-cov"
    [[ -x "$llvm_cov" ]] || { echo "coverage: $llvm_cov not found (install the matching llvm package)" >&2; exit 1; }
    gcov="$llvm_cov gcov" ;;
  *) echo "coverage: unsupported compiler '$compiler_id'" >&2; exit 1 ;;
esac
# llvm-cov cannot place files whose code never ran (no .gcda); keep reporting them at 0%.
[[ "$compiler_id" == GNU ]] || extra+=(--gcov-ignore-errors=no_working_dir_found)

mkdir -p "$out/report"
"$gcovr" --root "$repo" --object-directory "$out" --gcov-executable "$gcov" "${extra[@]}" \
  --filter 'include/' --filter 'src/' \
  --exclude-throw-branches --exclude-unreachable-branches \
  --html-details "$out/report/index.html" --html-title "EdgeSense coverage" \
  --markdown "$out/summary.md" --markdown-title "Coverage" \
  --print-summary
echo "HTML report: $out/report/index.html"
