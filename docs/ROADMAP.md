# Roadmap

- ✅ **M0 Foundation** (done 2026-11-24): CMake presets (debug/release/asan/tsan), GoogleTest, CI (Linux gcc+clang, macOS),
  clang-format, `SpscRing<T,N>` + 5 tests (empty, full, wrap-around, move-only, 2-thread stress under TSan)
- **M1 Single-image inference on the Mac:** ONNX Runtime CPU, YOLOX-Nano, preprocessing + NMS, golden tests
- **M2 Real-time pipeline:** stage threads, SPSC queues, frame pool, telemetry, clean shutdown
- **M3 Benchmarks + Linux/ARM64 + Docker**
- **M4 Raspberry Pi 5:** INT8, NEON, thread tuning, results table
- **M5 Publish:** README, ADRs, demo, v1.0
