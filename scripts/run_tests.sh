#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

cmake -S . -B build \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++
cmake --build build
ctest --test-dir build --output-on-failure
