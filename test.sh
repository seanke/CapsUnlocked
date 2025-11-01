#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
CONFIG="${1:-Debug}"

cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${CONFIG}" -DBUILD_TESTING=ON
cmake --build "${BUILD_DIR}" --config "${CONFIG}" --target caps_core_tests

ctest --test-dir "${BUILD_DIR}" --output-on-failure --build-config "${CONFIG}"
