#!/bin/bash
# Test script for gempp (macOS/Linux)

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
TESTS_DIR="$PROJECT_DIR/tests"
EXE="$PROJECT_DIR/gempp"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Build first
echo "=== Building gempp ==="
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. >/dev/null
cmake --build . --parallel >/dev/null
cd "$PROJECT_DIR"

if [ ! -f "$EXE" ]; then
    echo -e "${RED}Build failed: executable not found${NC}"
    exit 1
fi

echo "=== Running tests ==="
PASSED=0
FAILED=0

# Find all test cases by looking for directories containing expected.txt
for test_dir in "$TESTS_DIR"/*/; do
    test_name=$(basename "$test_dir")

    input_file="$test_dir/input.txt"
    expected_file="$test_dir/expected.txt"

    if [ ! -f "$input_file" ] || [ ! -f "$expected_file" ]; then
        echo -e "${RED}SKIP${NC}: $test_name (missing files)"
        continue
    fi

    # Run the test
    actual=$("$EXE" "$input_file" 2>&1) || true
    expected=$(cat "$expected_file")

    # Compare only first 5 lines (GED, Is Subgraph, Minimal Extension, Vertices count, Edges count)
    # The specific vertices/edges can vary between equivalent optimal solutions
    actual_core=$(echo "$actual" | head -5)
    expected_core=$(echo "$expected" | head -5)

    if [ "$actual_core" = "$expected_core" ]; then
        echo -e "${GREEN}PASS${NC}: $test_name"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC}: $test_name"
        echo "  Expected:"
        echo "$expected" | sed 's/^/    /'
        echo "  Actual:"
        echo "$actual" | sed 's/^/    /'
        ((FAILED++))
    fi
done

echo ""
echo "=== Results ==="
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"

if [ $FAILED -gt 0 ]; then
    exit 1
fi
