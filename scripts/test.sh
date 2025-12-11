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
YELLOW='\033[0;33m'
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

PASSED=0
FAILED=0

run_test() {
    local test_dir="$1"
    local use_multigraph="$2"
    local test_name=$(basename "$test_dir")
    local category=$(basename "$(dirname "$test_dir")")

    local input_file="$test_dir/input.txt"
    local expected_file="$test_dir/expected.txt"

    if [ ! -f "$input_file" ] || [ ! -f "$expected_file" ]; then
        echo -e "${YELLOW}SKIP${NC}: $category/$test_name (missing files)"
        return
    fi

    # Run the test
    if [ "$use_multigraph" = "true" ]; then
        actual=$("$EXE" --multigraph "$input_file" 2>&1) || true
    else
        actual=$("$EXE" "$input_file" 2>&1) || true
    fi
    expected=$(cat "$expected_file")

    # Compare key output lines (GED, Is Subgraph, Vertices to add, Edges to add)
    # Filter out Mode/Objective/Upperbound lines which are informational
    actual_core=$(echo "$actual" | grep -E "^(GED|Is Subgraph|Vertices to add|Edges to add):" | head -4)
    expected_core=$(echo "$expected" | grep -E "^(GED|Is Subgraph|Vertices to add|Edges to add):" | head -4)

    if [ "$actual_core" = "$expected_core" ]; then
        echo -e "${GREEN}PASS${NC}: $category/$test_name"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}FAIL${NC}: $category/$test_name"
        echo "  Expected:"
        echo "$expected" | sed 's/^/    /'
        echo "  Actual:"
        echo "$actual" | sed 's/^/    /'
        FAILED=$((FAILED + 1))
    fi
}

# Run simple graph tests
echo "=== Running simple graph tests ==="
if [ -d "$TESTS_DIR/graphs" ]; then
    for test_dir in "$TESTS_DIR"/graphs/*/; do
        run_test "$test_dir" "false"
    done
fi

# Run multigraph tests
echo ""
echo "=== Running multigraph tests ==="
if [ -d "$TESTS_DIR/multigraphs" ]; then
    for test_dir in "$TESTS_DIR"/multigraphs/*/; do
        run_test "$test_dir" "true"
    done
fi

echo ""
echo "=== Results ==="
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"

if [ $FAILED -gt 0 ]; then
    exit 1
fi
