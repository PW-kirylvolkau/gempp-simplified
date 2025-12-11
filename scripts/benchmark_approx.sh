#!/bin/bash
# Benchmark script for gempp (macOS/Linux) using STSM upper-bound approximation
# Usage: ./benchmark_approx.sh [upperbound]
#   upperbound: optional, in (0,1], default 0.5. Must be the only argument.

set -e

if [ "$#" -gt 1 ]; then
    echo "Usage: $0 [upperbound]" >&2
    exit 1
fi

UPPERBOUND="${1:-0.5}"

# Basic validation to match app expectations
awk "BEGIN {exit !($UPPERBOUND > 0 && $UPPERBOUND <= 1)}" >/dev/null 2>&1 || {
    echo "Error: upperbound must be in (0,1], got '$UPPERBOUND'" >&2
    exit 1
}

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
BENCHMARKS_DIR="$PROJECT_DIR/benchmarks"
EXE="$PROJECT_DIR/gempp"
RESULTS_FILE="$BENCHMARKS_DIR/results_approx.csv"

# Build first
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

echo "=== Running approximation benchmarks (upperbound=$UPPERBOUND) ==="
echo "Pattern Size,Target Size,Pattern Type,Target Type,Upperbound,Time (ms),GED" > "$RESULTS_FILE"

# Benchmark 1: Small complete graphs (exact subgraph cases)
echo "--- Small complete graphs ---"
for pattern_size in 3 4 5; do
    for target_size in 4 5 6 7 8; do
        if [ $target_size -ge $pattern_size ]; then
            input_file="$BENCHMARKS_DIR/k${pattern_size}_in_k${target_size}.txt"
            generate_complete_graph $pattern_size > "$input_file"
            echo "" >> "$input_file"
            generate_complete_graph $target_size >> "$input_file"

            output=$("$EXE" --approx-stsm --upperbound "$UPPERBOUND" --time "$input_file" 2>&1)
            time_ms=$(echo "$output" | grep "Time:" | awk '{print $2}')
            ged=$(echo "$output" | grep "GED:" | awk '{print $2}')

            echo "K$pattern_size in K$target_size: ${time_ms}ms, GED=$ged"
            echo "$pattern_size,$target_size,complete,complete,$UPPERBOUND,$time_ms,$ged" >> "$RESULTS_FILE"
        fi
    done
done

# Benchmark 2: Path in path (easy cases)
echo "--- Path graphs ---"
for pattern_size in 3 4 5 6; do
    for target_size in 5 6 7 8 10; do
        if [ $target_size -ge $pattern_size ]; then
            input_file="$BENCHMARKS_DIR/p${pattern_size}_in_p${target_size}.txt"
            generate_path_graph $pattern_size > "$input_file"
            echo "" >> "$input_file"
            generate_path_graph $target_size >> "$input_file"

            output=$("$EXE" --approx-stsm --upperbound "$UPPERBOUND" --time "$input_file" 2>&1)
            time_ms=$(echo "$output" | grep "Time:" | awk '{print $2}')
            ged=$(echo "$output" | grep "GED:" | awk '{print $2}')

            echo "P$pattern_size in P$target_size: ${time_ms}ms, GED=$ged"
            echo "$pattern_size,$target_size,path,path,$UPPERBOUND,$time_ms,$ged" >> "$RESULTS_FILE"
        fi
    done
done

# Benchmark 3: Cycle in complete (harder cases)
echo "--- Cycle in complete graph ---"
for pattern_size in 3 4 5 6; do
    for target_size in 4 5 6 7 8; do
        if [ $target_size -ge $pattern_size ]; then
            input_file="$BENCHMARKS_DIR/c${pattern_size}_in_k${target_size}.txt"
            generate_cycle_graph $pattern_size > "$input_file"
            echo "" >> "$input_file"
            generate_complete_graph $target_size >> "$input_file"

            output=$("$EXE" --approx-stsm --upperbound "$UPPERBOUND" --time "$input_file" 2>&1)
            time_ms=$(echo "$output" | grep "Time:" | awk '{print $2}')
            ged=$(echo "$output" | grep "GED:" | awk '{print $2}')

            echo "C$pattern_size in K$target_size: ${time_ms}ms, GED=$ged"
            echo "$pattern_size,$target_size,cycle,complete,$UPPERBOUND,$time_ms,$ged" >> "$RESULTS_FILE"
        fi
    done
done

# Benchmark 4: Complete in path (extension needed)
echo "--- Complete graph in path (extension cases) ---"
for pattern_size in 3 4 5; do
    for target_size in 5 6 7 8 10; do
        input_file="$BENCHMARKS_DIR/k${pattern_size}_in_p${target_size}.txt"
        generate_complete_graph $pattern_size > "$input_file"
        echo "" >> "$input_file"
        generate_path_graph $target_size >> "$input_file"

        output=$("$EXE" --approx-stsm --upperbound "$UPPERBOUND" --time "$input_file" 2>&1)
        time_ms=$(echo "$output" | grep "Time:" | awk '{print $2}')
        ged=$(echo "$output" | grep "GED:" | awk '{print $2}')

        echo "K$pattern_size in P$target_size: ${time_ms}ms, GED=$ged"
        echo "$pattern_size,$target_size,complete,path,$UPPERBOUND,$time_ms,$ged" >> "$RESULTS_FILE"
    done
done

# Benchmark 5: Larger graphs (stress test)
echo "--- Larger graphs (stress test) ---"
for size in 8 10 12 15; do
    input_file="$BENCHMARKS_DIR/k${size}_in_k${size}.txt"
    generate_complete_graph $size > "$input_file"
    echo "" >> "$input_file"
    generate_complete_graph $size >> "$input_file"

    output=$("$EXE" --approx-stsm --upperbound "$UPPERBOUND" --time "$input_file" 2>&1)
    time_ms=$(echo "$output" | grep "Time:" | awk '{print $2}')
    ged=$(echo "$output" | grep "GED:" | awk '{print $2}')

    echo "K$size in K$size: ${time_ms}ms, GED=$ged"
    echo "$size,$size,complete,complete,$UPPERBOUND,$time_ms,$ged" >> "$RESULTS_FILE"
done

echo ""
echo "=== Benchmark complete ==="
echo "Upperbound used: $UPPERBOUND"
echo "Results saved to: $RESULTS_FILE"

