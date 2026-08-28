#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cmake -S "$ROOT" -B "$ROOT/build" -DCMAKE_BUILD_TYPE="${1:-Release}"
cmake --build "$ROOT/build" -j
ctest --test-dir "$ROOT/build" --output-on-failure
echo "Binary: $ROOT/build/src/DrPdf"
