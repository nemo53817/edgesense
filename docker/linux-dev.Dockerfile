# Linux toolchain matching the CI runners (ubuntu-24.04, gcc + clang), used by
# scripts/docker-test.sh. On Apple silicon this builds an arm64 image, like the Pi 5.
FROM ubuntu:24.04

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      cmake ninja-build g++ clang libclang-rt-18-dev git ca-certificates \
 && rm -rf /var/lib/apt/lists/*
