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
- Dependencies: ONNX Runtime 1.32.0 (from this repo's `onnxruntime-1.32.0` release, rebuilt from
  NuGet by `scripts/repackage-onnxruntime.sh`; upstream GitHub repo was 404 on 2026-11-25),
  OpenCV (system: Homebrew 5.x on Mac, apt 4.6 on Ubuntu), YOLOX-Nano downloaded to `models/`.
- Every `app/*.cpp` builds into its own executable with `EDGESENSE_MODEL_PATH` defined.

## Conventions
- C++20, headers in include/edgesense/<module>/, sources in src/<module>/
- Namespaces: `edgesense::<module>` matching the directory (core, vision, postprocess,
  inference, concurrency); at most 2 levels. Top-level facades (e.g. Detector) in `edgesense`.
- Library code takes `core::ImageView`, not `cv::Mat`; OpenCV only in app/. ONNX Runtime only
  inside src/inference/ (pimpl), no `Ort::` types in public headers.
- clang-format (Google based, 100 columns); warnings are errors
- No heap allocation in the per-frame hot path; no user callbacks under a lock
