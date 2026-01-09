#!/bin/bash
# Build script for gempp (macOS/Linux)

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"

echo "=== Building gempp ==="
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake ..
cmake --build . --parallel

if [ -f "$PROJECT_DIR/gempp" ]; then
    echo "=== Build successful ==="
    echo "Executable: $PROJECT_DIR/gempp"
else
    echo "=== Build failed ==="
    exit 1
fi
