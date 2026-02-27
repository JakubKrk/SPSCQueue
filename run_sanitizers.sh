#!/usr/bin/env bash
set -euo pipefail

SANITIZERS=(address thread undefined)
FAILED=()

for san in "${SANITIZERS[@]}"; do
    build_dir="build-sanitizer-${san}"
    echo "=============================="
    echo " Sanitizer: ${san}"
    echo "=============================="

    cmake -S . -B "${build_dir}" -DSANITIZER="${san}" -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++

    cmake --build "${build_dir}" --parallel

    if ctest --test-dir "${build_dir}" --output-on-failure; then
        echo "[PASS] ${san}"
    else
        echo "[FAIL] ${san}"
        FAILED+=("${san}")
    fi

    echo ""
done

echo "=============================="
if [ ${#FAILED[@]} -eq 0 ]; then
    echo " All sanitizer runs passed."
else
    echo " Failed sanitizers: ${FAILED[*]}"
    exit 1
fi
