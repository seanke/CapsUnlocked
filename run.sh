#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
CONFIG="${1:-Release}"

cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${CONFIG}"
cmake --build "${BUILD_DIR}" --config "${CONFIG}"

BIN_NAME="CapsUnlocked"
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
  EXECUTABLE_PATH="${BUILD_DIR}/${CONFIG}/${BIN_NAME}.exe"
else
  EXECUTABLE_PATH="${BUILD_DIR}/${BIN_NAME}"
fi

echo "Running ${EXECUTABLE_PATH}..."
exec "${EXECUTABLE_PATH}"
