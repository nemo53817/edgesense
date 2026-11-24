# EdgeSense: notes for Claude Code

## What this project is
Real-time object detection pipeline in modern C++20 for edge devices
(Mac M3 for development, Raspberry Pi 5 as target). ONNX Runtime for inference.

## Learning project: how to help
- I write the core C++ (everything in include/ and src/) myself.
  Do NOT write or change code there unless I explicitly ask. Review it, point out bugs,
  explain trade-offs, and give hints rather than full solutions.
- You may write build files, CI, scripts and docs when I ask; keep diffs small and explain them.

## Build and test
- Configure/build/test: `cmake --preset <p> && cmake --build --preset <p> && ctest --preset <p>`
- Presets: debug, release, asan (ASan+UBSan), tsan
- Every concurrency change must pass the `tsan` preset.
- Linux check on the Mac: `scripts/docker-test.sh [compilers] [presets]` (ubuntu:24.04, arm64)
- Minimum toolchain (full C++20 library incl. `std::jthread`): GCC >= 12, Clang >= 18 with
  libstdc++, LLVM libc++ >= 20 / Xcode >= 26. Do not downgrade code for older toolchains.

## Conventions
- C++20, namespace `edgesense`, headers in include/edgesense/<module>/
- clang-format (Google based, 100 columns); warnings are errors
- No heap allocation in the per-frame hot path; no user callbacks under a lock
