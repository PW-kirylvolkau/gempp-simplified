#!/bin/bash
# Benchmark script for gempp --fast mode (greedy heuristic)
# Compares ILP vs greedy performance on larger graphs

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
BENCHMARKS_DIR="$PROJECT_DIR/benchmarks"
EXE="$PROJECT_DIR/gempp"
RESULTS_FILE="$BENCHMARKS_DIR/results_fast.csv"

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

echo "=== Running --fast benchmarks (greedy heuristic) ==="
echo "Pattern Size,Target Size,Pattern Type,Target Type,Fast Time (ms),Fast GED,ILP Time (ms),ILP GED,Speedup" > "$RESULTS_FILE"

# Benchmark 1: Complete graphs K_n in K_n (isomorphic)
echo ""
echo "--- Complete graphs K_n in K_n (isomorphic) ---"
for size in 5 8 10 12 15 20 25 30; do
    input_file="$BENCHMARKS_DIR/fast_k${size}_in_k${size}.txt"
    generate_complete_graph $size > "$input_file"
    echo "" >> "$input_file"
    generate_complete_graph $size >> "$input_file"

    # Run with --fast
    output_fast=$("$EXE" --fast --time "$input_file" 2>&1)
    time_fast=$(echo "$output_fast" | grep "Time:" | awk '{print $2}')
    ged_fast=$(echo "$output_fast" | grep "GED:" | awk '{print $2}')

    # Run without --fast (ILP) only for smaller sizes
    if [ $size -le 10 ]; then
        output_ilp=$("$EXE" --time "$input_file" 2>&1)
        time_ilp=$(echo "$output_ilp" | grep "Time:" | awk '{print $2}')
        ged_ilp=$(echo "$output_ilp" | grep "GED:" | awk '{print $2}')
        if [ "$time_fast" -gt 0 ]; then
            speedup=$(echo "scale=1; $time_ilp / $time_fast" | bc 2>/dev/null || echo "inf")
        else
            speedup="inf"
        fi
    else
        time_ilp="skipped"
        ged_ilp="skipped"
        speedup="N/A"
    fi

    echo "K$size in K$size: fast=${time_fast}ms (GED=$ged_fast), ILP=${time_ilp}ms (GED=$ged_ilp), speedup=${speedup}x"
    echo "$size,$size,complete,complete,$time_fast,$ged_fast,$time_ilp,$ged_ilp,$speedup" >> "$RESULTS_FILE"
done

# Benchmark 2: Complete in path (non-isomorphic, extension needed)
echo ""
echo "--- Complete graph in path (extension cases) ---"
for pattern_size in 3 4 5 6 8 10; do
    target_size=$((pattern_size * 2))
    input_file="$BENCHMARKS_DIR/fast_k${pattern_size}_in_p${target_size}.txt"
    generate_complete_graph $pattern_size > "$input_file"
    echo "" >> "$input_file"
    generate_path_graph $target_size >> "$input_file"

    # Run with --fast
    output_fast=$("$EXE" --fast --time "$input_file" 2>&1)
    time_fast=$(echo "$output_fast" | grep "Time:" | awk '{print $2}')
    ged_fast=$(echo "$output_fast" | grep "GED:" | awk '{print $2}')

    # Run without --fast (ILP)
    if [ $pattern_size -le 6 ]; then
        output_ilp=$("$EXE" --time "$input_file" 2>&1)
        time_ilp=$(echo "$output_ilp" | grep "Time:" | awk '{print $2}')
        ged_ilp=$(echo "$output_ilp" | grep "GED:" | awk '{print $2}')
        if [ "$time_fast" -gt 0 ]; then
            speedup=$(echo "scale=1; $time_ilp / $time_fast" | bc 2>/dev/null || echo "inf")
        else
            speedup="inf"
        fi
    else
        time_ilp="skipped"
        ged_ilp="skipped"
        speedup="N/A"
    fi

    echo "K$pattern_size in P$target_size: fast=${time_fast}ms (GED=$ged_fast), ILP=${time_ilp}ms (GED=$ged_ilp), speedup=${speedup}x"
    echo "$pattern_size,$target_size,complete,path,$time_fast,$ged_fast,$time_ilp,$ged_ilp,$speedup" >> "$RESULTS_FILE"
done

# Benchmark 3: Cycle in complete (subgraph cases)
echo ""
echo "--- Cycle in complete graph ---"
for pattern_size in 4 6 8 10 12 15; do
    target_size=$pattern_size
    input_file="$BENCHMARKS_DIR/fast_c${pattern_size}_in_k${target_size}.txt"
    generate_cycle_graph $pattern_size > "$input_file"
    echo "" >> "$input_file"
    generate_complete_graph $target_size >> "$input_file"

    # Run with --fast
    output_fast=$("$EXE" --fast --time "$input_file" 2>&1)
    time_fast=$(echo "$output_fast" | grep "Time:" | awk '{print $2}')
    ged_fast=$(echo "$output_fast" | grep "GED:" | awk '{print $2}')

    # Run without --fast (ILP)
    if [ $pattern_size -le 8 ]; then
        output_ilp=$("$EXE" --time "$input_file" 2>&1)
        time_ilp=$(echo "$output_ilp" | grep "Time:" | awk '{print $2}')
        ged_ilp=$(echo "$output_ilp" | grep "GED:" | awk '{print $2}')
        if [ "$time_fast" -gt 0 ]; then
            speedup=$(echo "scale=1; $time_ilp / $time_fast" | bc 2>/dev/null || echo "inf")
        else
            speedup="inf"
        fi
    else
        time_ilp="skipped"
        ged_ilp="skipped"
        speedup="N/A"
    fi

    echo "C$pattern_size in K$target_size: fast=${time_fast}ms (GED=$ged_fast), ILP=${time_ilp}ms (GED=$ged_ilp), speedup=${speedup}x"
    echo "$pattern_size,$target_size,cycle,complete,$time_fast,$ged_fast,$time_ilp,$ged_ilp,$speedup" >> "$RESULTS_FILE"
done

# Benchmark 4: Very large graphs (fast mode only)
echo ""
echo "--- Very large graphs (fast mode only) ---"
for size in 50 75 100; do
    input_file="$BENCHMARKS_DIR/fast_k${size}_in_k${size}.txt"
    generate_complete_graph $size > "$input_file"
    echo "" >> "$input_file"
    generate_complete_graph $size >> "$input_file"

    output_fast=$("$EXE" --fast --time "$input_file" 2>&1)
    time_fast=$(echo "$output_fast" | grep "Time:" | awk '{print $2}')
    ged_fast=$(echo "$output_fast" | grep "GED:" | awk '{print $2}')

    echo "K$size in K$size: fast=${time_fast}ms (GED=$ged_fast)"
    echo "$size,$size,complete,complete,$time_fast,$ged_fast,skipped,skipped,N/A" >> "$RESULTS_FILE"
done

echo ""
echo "=== Benchmark complete ==="
echo "Results saved to: $RESULTS_FILE"
