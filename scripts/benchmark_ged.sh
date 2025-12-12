#!/bin/bash
# Benchmark script for gempp (macOS/Linux) - GED mode
# Usage: ./scripts/benchmark_ged.sh [upper_bound]
# upper_bound is the pruning parameter in (0,1]; defaults to 1.0

set -e

UP=${1:-1}

# Basic validation (best-effort, accepts 0.x or 1)
UP_OK=$(UP="$UP" python3 - <<'PY'
import os, sys
up = os.environ.get("UP", "1")
try:
    v = float(up)
    if not (0 < v <= 1):
        raise ValueError
except Exception:
    sys.stderr.write("Error: upper_bound must be in (0,1], got %s\n" % up)
    sys.exit(1)
print(up)
PY
) || exit 1

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
BENCHMARKS_DIR="$PROJECT_DIR/benchmarks"
EXE="$PROJECT_DIR/gempp"
RESULTS_FILE="$BENCHMARKS_DIR/results_ged.csv"

echo "=== Building gempp ==="
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. >/dev/null
cmake --build . --parallel >/dev/null
cd "$PROJECT_DIR"

if [ ! -f "$EXE" ]; then
    echo "Build failed: executable not found"
    exit 1
fi

mkdir -p "$BENCHMARKS_DIR"

# Generate a complete graph (K_n) adjacency matrix
generate_complete_graph() {
    local n=$1
    echo "$n"
    for ((i=0; i<n; i++)); do
        row=""
        for ((j=0; j<n; j++)); do
            if [ $i -eq $j ]; then
                row+="0"
            else
                row+="1"
            fi
            if [ $j -lt $((n-1)) ]; then
                row+=" "
            fi
        done
        echo "$row"
    done
}

# Generate a path graph (P_n) adjacency matrix
generate_path_graph() {
    local n=$1
    echo "$n"
    for ((i=0; i<n; i++)); do
        row=""
        for ((j=0; j<n; j++)); do
            if [ $((j - i)) -eq 1 ] || [ $((i - j)) -eq 1 ]; then
                row+="1"
            else
                row+="0"
            fi
            if [ $j -lt $((n-1)) ]; then
                row+=" "
            fi
        done
        echo "$row"
    done
}

# Generate a cycle graph (C_n) adjacency matrix
generate_cycle_graph() {
    local n=$1
    echo "$n"
    for ((i=0; i<n; i++)); do
        row=""
        for ((j=0; j<n; j++)); do
            if [ $((j - i)) -eq 1 ] || [ $((i - j)) -eq 1 ] || \
               ([ $i -eq 0 ] && [ $j -eq $((n-1)) ]) || \
               ([ $i -eq $((n-1)) ] && [ $j -eq 0 ]); then
                row+="1"
            else
                row+="0"
            fi
            if [ $j -lt $((n-1)) ]; then
                row+=" "
            fi
        done
        echo "$row"
    done
}

echo "=== Running GED benchmarks (up=$UP_OK) ==="
echo "Pattern Size,Target Size,Pattern Type,Target Type,Time (ms),GED" > "$RESULTS_FILE"

# Benchmark 1: Small complete graphs (exact cases)
echo "--- Small complete graphs ---"
for pattern_size in 3 4 5; do
    for target_size in 4 5 6 7 8; do
        input_file="$BENCHMARKS_DIR/ged_k${pattern_size}_in_k${target_size}.txt"
        generate_complete_graph $pattern_size > "$input_file"
        echo "" >> "$input_file"
        generate_complete_graph $target_size >> "$input_file"

        output=$("$EXE" --ged --up "$UP_OK" --time "$input_file" 2>&1)
        time_ms=$(echo "$output" | grep "Time:" | awk '{print $2}')
        ged=$(echo "$output" | grep "^GED:" | awk '{print $2}')

        echo "K$pattern_size in K$target_size: ${time_ms}ms, GED=$ged"
        echo "$pattern_size,$target_size,complete,complete,$time_ms,$ged" >> "$RESULTS_FILE"
    done
done

# Benchmark 2: Path in path
echo "--- Path graphs ---"
for pattern_size in 3 4 5 6; do
    for target_size in 5 6 7 8 10; do
        input_file="$BENCHMARKS_DIR/ged_p${pattern_size}_in_p${target_size}.txt"
        generate_path_graph $pattern_size > "$input_file"
        echo "" >> "$input_file"
        generate_path_graph $target_size >> "$input_file"

        output=$("$EXE" --ged --up "$UP_OK" --time "$input_file" 2>&1)
        time_ms=$(echo "$output" | grep "Time:" | awk '{print $2}')
        ged=$(echo "$output" | grep "^GED:" | awk '{print $2}')

        echo "P$pattern_size in P$target_size: ${time_ms}ms, GED=$ged"
        echo "$pattern_size,$target_size,path,path,$time_ms,$ged" >> "$RESULTS_FILE"
    done
done

# Benchmark 3: Cycle in complete
echo "--- Cycle in complete graph ---"
for pattern_size in 3 4 5 6; do
    for target_size in 4 5 6 7 8; do
        input_file="$BENCHMARKS_DIR/ged_c${pattern_size}_in_k${target_size}.txt"
        generate_cycle_graph $pattern_size > "$input_file"
        echo "" >> "$input_file"
        generate_complete_graph $target_size >> "$input_file"

        output=$("$EXE" --ged --up "$UP_OK" --time "$input_file" 2>&1)
        time_ms=$(echo "$output" | grep "Time:" | awk '{print $2}')
        ged=$(echo "$output" | grep "^GED:" | awk '{print $2}')

        echo "C$pattern_size in K$target_size: ${time_ms}ms, GED=$ged"
        echo "$pattern_size,$target_size,cycle,complete,$time_ms,$ged" >> "$RESULTS_FILE"
    done
done

# Benchmark 4: Complete in path (extension)
echo "--- Complete graph in path (extension cases) ---"
for pattern_size in 3 4 5; do
    for target_size in 5 6 7 8 10; do
        input_file="$BENCHMARKS_DIR/ged_k${pattern_size}_in_p${target_size}.txt"
        generate_complete_graph $pattern_size > "$input_file"
        echo "" >> "$input_file"
        generate_path_graph $target_size >> "$input_file"

        output=$("$EXE" --ged --up "$UP_OK" --time "$input_file" 2>&1)
        time_ms=$(echo "$output" | grep "Time:" | awk '{print $2}')
        ged=$(echo "$output" | grep "^GED:" | awk '{print $2}')

        echo "K$pattern_size in P$target_size: ${time_ms}ms, GED=$ged"
        echo "$pattern_size,$target_size,complete,path,$time_ms,$ged" >> "$RESULTS_FILE"
    done
done

# Benchmark 5: Larger graphs (stress)
echo "--- Larger graphs (stress test) ---"
for size in 8 10 12 15; do
    input_file="$BENCHMARKS_DIR/ged_k${size}_in_k${size}.txt"
    generate_complete_graph $size > "$input_file"
    echo "" >> "$input_file"
    generate_complete_graph $size >> "$input_file"

    output=$("$EXE" --ged --up "$UP_OK" --time "$input_file" 2>&1)
    time_ms=$(echo "$output" | grep "Time:" | awk '{print $2}')
    ged=$(echo "$output" | grep "^GED:" | awk '{print $2}')

    echo "K$size in K$size: ${time_ms}ms, GED=$ged"
    echo "$size,$size,complete,complete,$time_ms,$ged" >> "$RESULTS_FILE"
done

echo ""
echo "=== GED benchmark complete ==="
echo "Results saved to: $RESULTS_FILE"

