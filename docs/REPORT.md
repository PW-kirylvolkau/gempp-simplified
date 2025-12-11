# Report: Minimal Graph Extension

This report presents the computational results and conclusions for the minimal graph extension problem.

## 1. Problem Statement

Given two graphs G₁ (pattern) and G₂ (target), find the **minimal extension** of G₂ such that G₁ becomes a subgraph of the extended G₂.

The solution uses an Integer Linear Programming (ILP) formulation based on the GEM++ MCSM algorithm.

## 2. Computational Test Results

### 2.1 Test Environment

- **Platform**: macOS / Windows
- **Solver**: GLPK 5.0 (bundled)
- **Compilation**: C++17, Release mode

### 2.2 Benchmark Results

The following benchmarks were run to characterize the algorithm's performance.

#### 2.2.1 Complete Graph in Complete Graph (K_n in K_m)

These are exact subgraph cases (GED = 0).

| Pattern | Target | Time (ms) | GED |
|---------|--------|-----------|-----|
| K₃ | K₄ | 1 | 0 |
| K₃ | K₅ | 0 | 0 |
| K₃ | K₆ | 0 | 0 |
| K₃ | K₇ | 1 | 0 |
| K₃ | K₈ | 1 | 0 |
| K₄ | K₄ | 0 | 0 |
| K₄ | K₅ | 1 | 0 |
| K₄ | K₆ | 1 | 0 |
| K₄ | K₇ | 3 | 0 |
| K₄ | K₈ | 1 | 0 |
| K₅ | K₅ | 3 | 0 |
| K₅ | K₆ | 1 | 0 |
| K₅ | K₇ | 11 | 0 |
| K₅ | K₈ | 2 | 0 |

#### 2.2.2 Path Graph in Path Graph (P_n in P_m)

| Pattern | Target | Time (ms) | GED |
|---------|--------|-----------|-----|
| P₃ | P₅ | 0 | 0 |
| P₃ | P₁₀ | 0 | 0 |
| P₄ | P₅ | 1 | 0 |
| P₄ | P₁₀ | 1 | 0 |
| P₅ | P₅ | 1 | 0 |
| P₅ | P₁₀ | 2 | 0 |
| P₆ | P₆ | 3 | 0 |
| P₆ | P₁₀ | 32 | 0 |

#### 2.2.3 Cycle Graph in Complete Graph (C_n in K_m)

| Pattern | Target | Time (ms) | GED |
|---------|--------|-----------|-----|
| C₃ | K₄ | 0 | 0 |
| C₃ | K₈ | 1 | 0 |
| C₄ | K₄ | 0 | 0 |
| C₄ | K₈ | 2 | 0 |
| C₅ | K₅ | 1 | 0 |
| C₅ | K₈ | 2 | 0 |
| C₆ | K₆ | 1 | 0 |
| C₆ | K₈ | 11 | 0 |

#### 2.2.4 Complete Graph in Path Graph (Extension Required)

These cases require adding edges to the target (GED > 0).

| Pattern | Target | Time (ms) | GED | Edges to Add |
|---------|--------|-----------|-----|--------------|
| K₃ | P₅ | 2 | 1 | 1 |
| K₃ | P₁₀ | 15 | 1 | 1 |
| K₄ | P₅ | 13 | 3 | 3 |
| K₄ | P₁₀ | 279 | 3 | 3 |
| K₅ | P₅ | 106 | 6 | 6 |
| K₅ | P₁₀ | 3951 | 6 | 6 |

#### 2.2.5 Large Graph Stress Test (K_n in K_n)

| Size | Time (ms) | Notes |
|------|-----------|-------|
| K₈ | 62 | Fast |
| K₁₀ | 518 | < 1 second |
| K₁₂ | 21,732 | ~22 seconds |
| K₁₅ | 229,735 | ~4 minutes |

### 2.3 Performance Analysis

#### Time Complexity Observations

1. **Exact subgraph cases** (GED = 0) are generally fast, even for larger graphs
2. **Extension cases** (GED > 0) are significantly slower due to the need to explore more of the search space
3. **Complete graphs** (densest structure) show exponential growth:
   - K₁₀: ~0.5 seconds
   - K₁₂: ~22 seconds (44× slower)
   - K₁₅: ~230 seconds (10× slower than K₁₂)

#### Scaling Behavior

For complete graphs K_n in K_n:

| n | ILP Variables | Time |
|---|---------------|------|
| 8 | 64 + 784 = 848 | 62 ms |
| 10 | 100 + 2025 = 2125 | 518 ms |
| 12 | 144 + 4356 = 4500 | 21,732 ms |
| 15 | 225 + 11025 = 11250 | 229,735 ms |

The exponential growth confirms the NP-complete nature of the subgraph isomorphism problem.

### 2.4 Correctness Verification

All 15 unit tests pass, covering:
- Exact subgraph matches (GED = 0)
- Edge extension cases (GED > 0 with only edge additions)
- Vertex extension cases (GED > 0 with vertex additions)
- Various graph types (triangles, squares, paths, stars, complete graphs, cycles)

## 3. Conclusions

### 3.1 Algorithm Effectiveness

The ILP-based approach using GLPK:
- **Correctly computes** the minimal extension for all tested cases
- **Performs well** for small to medium graphs (|V| ≤ 12)
- **Becomes impractical** for large dense graphs (|V| > 15)

### 3.2 Practical Limits

Based on the benchmarks, recommended practical limits:
- **Fast** (< 1 second): |V| ≤ 10 for any graph type
- **Moderate** (< 1 minute): |V| ≤ 12 for dense graphs, |V| ≤ 15 for sparse graphs
- **Slow/Impractical**: |V| > 15 for dense graphs

### 3.3 Approximation Considerations

An **optional approximate mode** is available via `--approx-stsm` with an `--upperbound <up>` parameter (0 < up ≤ 1):

- **What it does**: Uses substitution-tolerant subgraph matching and prunes high-cost substitutions, shrinking the ILP.
- **Trade-off**: Smaller `up` ⇒ faster solving, but may prune the optimal mapping. `up = 1.0` keeps all candidates (no pruning).
- **Benchmarks**: Run `./scripts/benchmark_approx.sh <up>` to produce `benchmarks/results_approx.csv`.

For graphs exceeding practical limits, increase `up` toward 1.0 for better accuracy or use heuristic ideas (beam search, graph kernels) if more speed is needed.

### 3.4 Summary

The solution successfully implements the minimal graph extension computation using Integer Linear Programming. The algorithm is exact and correct but has exponential worst-case complexity, which is expected given the NP-completeness of subgraph isomorphism.
