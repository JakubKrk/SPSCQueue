#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

BUILD_DIR="build-memtest"

cmake -S . -B "${BUILD_DIR}" -DSANITIZER=memory -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DENABLE_TESTS=OFF

cmake --build "${BUILD_DIR}" --parallel

echo "=============================="
echo " Running memtest with MSan"
echo "=============================="

"${BUILD_DIR}/memtest/memtest"

echo "[PASS] memtest"
