#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BIN="${BUILD_DIR}/hello"

mkdir -p "${BUILD_DIR}"

echo "Compiling..."
clang++ -std=c++17 "${SCRIPT_DIR}/src/main.cpp" -o "${BIN}"

echo "Running:"
exec "${BIN}"
