# GEM++ Configuration and Modifications

This document explains the configuration choices and modifications made to the original GEM++ implementation.

## 1. Configuration Parameters

### 1.1 Problem Types

- **Default (minimal extension)**: `Problem::SUBGRAPH` with MCSM (partial matching, one-sided).
- **Optional (full GED)**: `Problem::GED` with the Linear GED formulation (symmetric insert/delete/substitute) enabled via `--ged`.

### 1.2 Formulations

- **Minimal extension**: **MCSM**, non-induced.
  ```cpp
  Problem problem(Problem::SUBGRAPH, pattern, target);
  MinimumCostSubgraphMatching formulation(&problem, false);
  ```
  - Partial matching (≤ constraints) keeps the ILP feasible and counts unmatched pattern elements.
- **Full GED (exact)**: **Linear GED** with optional upper-bound pruning `--up v` (v in (0,1], default 1.0).
  ```cpp
  Problem problem(Problem::GED, pattern, target);
  LinearGraphEditDistance formulation(&problem);
  formulation.init(v);
  ```
  - Pruning mirrors original GEM++: keep only cheaper substitution candidates per row/column and deactivate incompatible edge pairs when `v < 1.0`.
- **Full GED (lower bound)**: **F2LP** continuous relaxation of the F2 formulation, enabled with `--f2lp` (alias `--lp`, implies `--ged`).
  ```cpp
  Problem problem(Problem::GED, pattern, target);
  LinearGraphEditDistance formulation(&problem);
  formulation.init(v, /*relaxed=*/true);
  ```
  - Variables are continuous in [0,1]; solved via simplex. Returns a lower bound on GED; extracted matches treat values ≥ 0.5 as active for reporting only.

### 1.3 Why Partial Matching (Not Exact Matching)?

The key insight for computing minimal extension is that we need **partial matching**, not exact matching.

#### The Problem with Exact Matching

An **exact subgraph matching** formulation would require:
```
∀i ∈ V_pattern: Σₖ x_{i,k} = 1   (each pattern vertex MUST match exactly one target vertex)
∀(i,j) ∈ E_pattern: Σ_{kl} y_{ij,kl} = 1   (each pattern edge MUST match exactly one target edge)
```

This approach **fails** for minimal extension because:
1. If the pattern is NOT a subgraph of the target, the ILP becomes **infeasible** (no solution exists)
2. We get no information about HOW MANY elements are missing
3. We cannot determine WHICH elements need to be added

#### The Solution: Partial Matching with MCSM

MCSM uses **relaxed constraints** (≤ instead of =):
```
∀i ∈ V_pattern: Σₖ x_{i,k} ≤ 1   (each pattern vertex matches AT MOST one target vertex)
∀(i,j) ∈ E_pattern: Σ_{kl} y_{ij,kl} ≤ 1   (each pattern edge matches AT MOST one target edge)
```

This means:
1. The ILP is **always feasible** (worst case: nothing matches, all variables = 0)
2. The objective counts **unmatched elements** = minimal extension cost
3. Unmatched elements tell us exactly **what needs to be added** to the target

#### Example

Consider pattern = triangle (3 vertices, 3 edges) and target = path of 3 vertices (3 vertices, 2 edges).

**With exact matching**: ILP is infeasible (triangle has 3 edges, path has only 2).

**With MCSM**:
- All 3 vertices can match (cost contribution: 0)
- Only 2 edges can match (cost contribution: 1 unmatched edge)
- Result: Minimal extension = 1 (add one edge to make triangle a subgraph)

### 1.4 Solver: GLPK

We use the **GLPK solver** (GNU Linear Programming Kit):

```cpp
GLPKSolver solver;
solver.init(formulation.getLinearProgram(), false, /*relaxed=*/false_or_true);
```

**Why**:
- Open source (GPL license)
- No external dependencies (can be bundled)
- Supports Mixed Integer Programming (MIP)
- Cross-platform (Windows, macOS, Linux)

### 1.5 Cost Model: Unit Costs

- Minimal extension: creation costs = 1.0; objective counts unmatched pattern elements.
- GED: insertion/deletion = 1.0; substitution uses provided cost matrices minus insertion/deletion, plus constant term, matching original GEM++ linear model. Applies to both integer (exact) and relaxed (F2LP) modes; the relaxation only affects integrality, not costs.

## 2. Modifications from Original GEM++

### 2.1 Removed Dependencies

| Original | This Version |
|----------|--------------|
| Qt Framework | Pure C++17 STL |
| External GLPK installation | Bundled GLPK source |
| GXL file format | Simple adjacency matrix |
| Multiple solvers (CPLEX, etc.) | GLPK only |

**Rationale**: The original GEM++ is a full-featured GUI application with many options. For this laboratory task, we need only the core ILP formulation and a simple CLI.

### 2.2 Simplified Input Format

Original GEM++ uses GXL (Graph eXchange Language) format. We use plain text adjacency matrices:

```
3
0 1 1
1 0 1
1 1 0
```

**Rationale**: GXL is verbose and complex. Adjacency matrices are:
- Easy to write by hand
- Easy to validate visually
- Compatible with the task specification

### 2.3 Header-Only Implementation

All code is in header files (`.h`), not split into `.h`/`.cpp`:

```
src/
├── core/types.h
├── core/matrix.h
├── model/graph.h
├── model/problem.h
├── model/adjacency_parser.h
├── integer_programming/variable.h
├── integer_programming/linear_program.h
├── formulation/mcsm.h
├── solver/glpk_solver.h
└── main.cpp
```

**Rationale**:
- Simpler build process
- Easier to understand code structure
- No linking issues

### 2.4 Removed Features

The following GEM++ features are **not implemented**:

| Feature | Reason for Removal |
|---------|-------------------|
| Node/Edge labels | Not needed for unlabeled graphs |
| Multiple cost functions | Unit costs sufficient |
| Induced matching | Not required by task |
| GUI | CLI is sufficient |
| Graph visualization | Not required |
| CPLEX solver | Commercial, not portable |

### 2.5 Added Features

| Feature | Purpose |
|---------|---------|
| Detailed extension output | Shows which vertices/edges to add |
| Cross-platform build | CMake with bundled GLPK |
| Test suite | Automated verification |

## 3. Algorithm Equivalence

Despite the modifications, the core algorithm is **mathematically equivalent** to GEM++ MCSM:

1. Same ILP formulation (variables, constraints, objective)
2. Same constraint structure (vertex injection, edge consistency)
3. Same objective function (minimize unmatched elements)

The only differences are:
- Simplified cost model (unit costs)
- Simplified input/output formats
- Removed unused features

## 4. Known Limitations

### 4.1 Multiple Optimal Solutions

The graph edit distance (GED) problem solved by this tool is formulated as an Integer Linear Program (ILP). For many input graphs, there exist **multiple optimal solutions** with the same minimal cost.

GLPK (GNU Linear Programming Kit), the solver used in this project, returns only **one** optimal solution. The specific solution returned depends on:
- Internal solver heuristics
- Floating-point arithmetic differences across platforms
- Compiler optimizations

As a result, running the same test on different platforms (e.g., macOS vs Windows) may produce different specific vertex/edge mappings, even though the **cost is identical**.

#### Test Design

To handle this limitation, the test infrastructure compares only the **deterministic values**:
1. GED (Graph Edit Distance)
2. Is Subgraph (yes/no)
3. Minimal Extension cost
4. Vertices to add (count)
5. Edges to add (count)

The specific vertices and edges listed in the output may vary between platforms but represent equally valid optimal solutions.

### 4.2 Computational Complexity

The subgraph isomorphism problem is NP-complete. For large graphs:
- Solving time grows exponentially
- Memory usage can be significant

Practical limits depend on graph structure, but generally:
- |V| ≤ 20: Fast (< 1 second)
- |V| ≤ 50: Moderate (seconds to minutes)
- |V| > 100: May be slow or infeasible

## References

1. Solnon, C. et al. (2015). "On the complexity of subgraph isomorphism." AAAI.
