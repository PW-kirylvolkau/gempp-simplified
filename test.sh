#!/bin/bash
# Test script for gempp-v2 (macOS/Linux)

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
TESTS_DIR="$SCRIPT_DIR/tests"
EXE="$BUILD_DIR/gempp-v2"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "=== Building gempp-v2 ==="
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. >/dev/null
cmake --build . --parallel >/dev/null
cd "$SCRIPT_DIR"

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

    pattern_file="$test_dir/pattern.txt"
    target_file="$test_dir/target.txt"
    expected_file="$test_dir/expected.txt"

    if [ ! -f "$pattern_file" ] || [ ! -f "$target_file" ] || [ ! -f "$expected_file" ]; then
        echo -e "${RED}SKIP${NC}: $test_name (missing files)"
        continue
    fi

    # Run the test
    actual=$("$EXE" "$pattern_file" "$target_file" 2>&1) || true
    expected=$(cat "$expected_file")

    if [ "$actual" = "$expected" ]; then
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
