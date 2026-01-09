# Algorithm Description

This document describes the algorithm used for computing the minimal extension, focusing on the GEM++ MCSM (Minimum Cost Subgraph Matching) approach.

## 1. Overview

The algorithm solves the minimal extension problem by formulating it as an **Integer Linear Program (ILP)** and solving it with an ILP solver (GLPK).

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Input Graphs   │───▶│  ILP Building   │───▶│   ILP Solver    │
│  (Pattern, Target)   │  (Variables,     │    │   (GLPK)        │
│                 │    │   Constraints)   │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                      │
                                                      ▼
                              ┌─────────────────────────────────┐
                              │  Minimal Extension + Mapping    │
                              └─────────────────────────────────┘
```

## 2. Main Algorithm

```
ALGORITHM MinimalExtension(G_pattern, G_target)

INPUT:
    G_pattern = (V_P, E_P)  -- Pattern graph
    G_target  = (V_T, E_T)  -- Target graph

OUTPUT:
    cost     -- Minimal extension cost (number of elements to add)
    mapping  -- Vertex and edge mapping from pattern to target

BEGIN
    1. Parse input graphs from adjacency matrices
    2. Build ILP formulation (see Section 3)
    3. Solve ILP with GLPK solver
    4. Extract solution:
       - cost = objective value
       - mapping = {(i,k) | x[i,k] = 1} ∪ {(ij,kl) | y[ij,kl] = 1}
    5. Compute unmatched elements:
       - unmatched_vertices = {i | ∀k: x[i,k] = 0}
       - unmatched_edges = {ij | ∀kl: y[ij,kl] = 0}
    6. Return (cost, unmatched_vertices, unmatched_edges)
END
```

## 3. ILP Construction

```
ALGORITHM BuildILP(G_pattern, G_target)

INPUT:
    V_P, E_P  -- Pattern vertices and edges
    V_T, E_T  -- Target vertices and edges

OUTPUT:
    ILP formulation (variables, constraints, objective)

BEGIN
    // ═══════════════════════════════════════════════════════
    // STEP 1: Create Variables
    // ═══════════════════════════════════════════════════════

    FOR each i in V_P:
        FOR each k in V_T:
            CREATE binary variable x[i,k]
            // x[i,k] = 1 means vertex i maps to vertex k

    FOR each (i,j) in E_P:
        FOR each (k,l) in E_T:
            CREATE binary variable y[ij,kl]
            // y[ij,kl] = 1 means edge (i,j) maps to edge (k,l)

    // ═══════════════════════════════════════════════════════
    // STEP 2: Create Constraints
    // ═══════════════════════════════════════════════════════

    // C1: Each pattern vertex maps to AT MOST one target vertex
    FOR each i in V_P:
        ADD CONSTRAINT: Σ_{k ∈ V_T} x[i,k] ≤ 1

    // C2: Each target vertex receives AT MOST one pattern vertex
    FOR each k in V_T:
        ADD CONSTRAINT: Σ_{i ∈ V_P} x[i,k] ≤ 1

    // C3: Each pattern edge maps to AT MOST one target edge
    FOR each (i,j) in E_P:
        ADD CONSTRAINT: Σ_{(k,l) ∈ E_T} y[ij,kl] ≤ 1

    // C4: Edge consistency (crucial for subgraph matching)
    FOR each (i,j) in E_P:
        FOR each k in V_T:
            // If edge (i,j) maps to an edge starting at k,
            // then either i or j must map to k
            ADD CONSTRAINT:
                Σ_{l:(k,l)∈E_T} y[ij,kl] ≤ x[i,k] + x[j,k]

            // If edge (i,j) maps to an edge ending at k,
            // then either i or j must map to k
            ADD CONSTRAINT:
                Σ_{l:(l,k)∈E_T} y[ij,kl] ≤ x[i,k] + x[j,k]

    // ═══════════════════════════════════════════════════════
    // STEP 3: Create Objective Function
    // ═══════════════════════════════════════════════════════

    // Objective: Minimize unmatched pattern elements
    // = (total pattern elements) - (matched elements)

    constant = |V_P| + |E_P|

    MINIMIZE:
        constant - Σ_{i,k} x[i,k] - Σ_{ij,kl} y[ij,kl]

    // Equivalently: count unmatched vertices + unmatched edges

    RETURN ILP
END
```

## 4. GLPK Solver Interface

```
ALGORITHM SolveWithGLPK(ILP)

INPUT:
    ILP -- Integer Linear Program (variables, constraints, objective)

OUTPUT:
    objective_value -- Optimal objective value
    solution        -- Variable assignments

BEGIN
    // Create GLPK problem
    prob = glp_create_prob()
    glp_set_obj_dir(prob, GLP_MIN)

    // Add columns (variables)
    FOR each variable v in ILP.variables:
        col = glp_add_cols(prob, 1)
        glp_set_col_kind(col, GLP_BV)  // Binary variable
        glp_set_obj_coef(col, v.coefficient)

    // Add rows (constraints)
    FOR each constraint c in ILP.constraints:
        row = glp_add_rows(prob, 1)
        glp_set_row_bnds(row, c.type, c.lower, c.upper)
        glp_set_mat_row(row, c.coefficients)

    // Set solver parameters
    params = glp_iocp()
    glp_init_iocp(params)
    params.presolve = GLP_ON
    params.msg_lev = GLP_MSG_OFF

    // Solve
    glp_intopt(prob, params)

    // Extract solution
    objective_value = glp_mip_obj_val(prob)
    FOR each variable v:
        solution[v.id] = glp_mip_col_val(v.column)

    glp_delete_prob(prob)
    RETURN (objective_value, solution)
END
```

## 5. Solution Extraction

```
ALGORITHM ExtractSolution(solution, G_pattern, G_target)

INPUT:
    solution  -- Variable assignments from ILP solver
    G_pattern -- Pattern graph
    G_target  -- Target graph

OUTPUT:
    unmatched_vertices -- Pattern vertices not mapped
    unmatched_edges    -- Pattern edges not mapped

BEGIN
    unmatched_vertices = []
    unmatched_edges = []

    // Check each pattern vertex
    FOR i = 0 TO |V_P| - 1:
        matched = FALSE
        FOR k = 0 TO |V_T| - 1:
            IF solution["x_" + i + "," + k] == 1:
                matched = TRUE
                BREAK
        IF NOT matched:
            ADD i TO unmatched_vertices

    // Check each pattern edge
    FOR ij = 0 TO |E_P| - 1:
        matched = FALSE
        FOR kl = 0 TO |E_T| - 1:
            IF solution["y_" + ij + "," + kl] == 1:
                matched = TRUE
                BREAK
        IF NOT matched:
            ADD ij TO unmatched_edges

    RETURN (unmatched_vertices, unmatched_edges)
END
```

## 6. Greedy Heuristic (Fast Mode)

For large graphs where ILP solving is impractical, a greedy heuristic provides a fast approximation.

### 6.1 Algorithm Overview

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Input Graphs   │───▶│  Greedy Match   │───▶│  Upper Bound    │
│  (Pattern, Target)   │  (Degree-based)  │    │  Solution       │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### 6.2 Greedy Matching Algorithm

```
ALGORITHM GreedyMinimalExtension(G_pattern, G_target)

INPUT:
    G_pattern = (V_P, E_P)  -- Pattern graph
    G_target  = (V_T, E_T)  -- Target graph

OUTPUT:
    cost     -- Upper bound on minimal extension
    mapping  -- Vertex and edge mapping (may not be optimal)

BEGIN
    // ═══════════════════════════════════════════════════════
    // STEP 1: Sort pattern vertices by degree (highest first)
    // ═══════════════════════════════════════════════════════

    pattern_order = SORT V_P BY degree DESCENDING

    vertex_matching = {} // pattern vertex -> target vertex
    edge_matching = {}   // pattern edge -> target edge
    used_target_vertices = {}
    used_target_edges = {}

    // ═══════════════════════════════════════════════════════
    // STEP 2: Greedily match vertices
    // ═══════════════════════════════════════════════════════

    FOR each i in pattern_order:
        best_k = NULL
        best_score = -∞

        FOR each k in V_T:
            IF k in used_target_vertices:
                CONTINUE

            // Score based on edge compatibility with already-matched neighbors
            score = 0
            FOR each neighbor j of i in G_pattern:
                IF j in vertex_matching:
                    l = vertex_matching[j]
                    IF edge (k,l) exists in G_target:
                        score += 1

            // Prefer similar degree (tie-breaker)
            degree_penalty = |degree(i) - degree(k)|
            adjusted_score = score * 1000 - degree_penalty

            IF adjusted_score > best_score:
                best_score = adjusted_score
                best_k = k

        IF best_k != NULL:
            vertex_matching[i] = best_k
            used_target_vertices.ADD(best_k)

    // ═══════════════════════════════════════════════════════
    // STEP 3: Match edges based on vertex matching
    // ═══════════════════════════════════════════════════════

    FOR each edge (i,j) in E_P:
        IF i in vertex_matching AND j in vertex_matching:
            k = vertex_matching[i]
            l = vertex_matching[j]

            IF edge (k,l) exists in G_target AND (k,l) not in used_target_edges:
                edge_matching[(i,j)] = (k,l)
                used_target_edges.ADD((k,l))

    // ═══════════════════════════════════════════════════════
    // STEP 4: Compute objective (upper bound)
    // ═══════════════════════════════════════════════════════

    unmatched_vertices = |V_P| - |vertex_matching|
    unmatched_edges = |E_P| - |edge_matching|
    cost = unmatched_vertices + unmatched_edges

    RETURN (cost, vertex_matching, edge_matching)
END
```

### 6.3 Properties of the Greedy Heuristic

**Guarantees:**
- Returns a **valid** matching (satisfies all constraints)
- Returns an **upper bound** on the minimal extension
- Runs in **polynomial time**: O(|V_P| × |V_T| × max_degree)

**Limitations:**
- May not find the optimal solution
- Quality depends on graph structure
- Best for graphs with high symmetry (e.g., complete graphs)

### 6.4 When to Use Greedy vs ILP

| Scenario | Recommended Method |
|----------|-------------------|
| Small graphs (|V| ≤ 10) | ILP (exact) |
| Medium graphs (10 < |V| ≤ 15) | ILP with timeout, fallback to greedy |
| Large graphs (|V| > 15) | Greedy (fast) |
| Isomorphic graphs | Greedy finds optimal |
| Highly asymmetric graphs | ILP preferred if feasible |

## 7. Complexity Analysis

### 7.1 ILP Size

For pattern graph with n vertices, m edges and target graph with N vertices, M edges:

| Component | Count |
|-----------|-------|
| Vertex variables (x) | n × N |
| Edge variables (y) | m × M |
| Vertex constraints | n + N |
| Edge constraints | m |
| Consistency constraints | m × N × 2 |

**Total variables**: O(nN + mM)
**Total constraints**: O(n + N + m + mN) = O(mN)

### 7.2 Time Complexity

**ILP Construction**: O(nN + mM + mN) = O(mN) (assuming m = O(n²))

**ILP Solving**: The subgraph isomorphism problem is NP-complete, so:
- **Worst case**: Exponential in the number of variables
- **Average case**: Depends on graph structure and solver heuristics
- **Best case**: Polynomial (when presolve eliminates most variables)

### 7.3 Space Complexity

**O(nN + mM)** for storing the ILP formulation.

## 8. Example Trace

**Input**:
- Pattern: Triangle (3 vertices, 3 edges)
- Target: Square (4 vertices, 4 edges)

**ILP Variables**:
- x[0,0], x[0,1], x[0,2], x[0,3] (vertex 0 mapping options)
- x[1,0], x[1,1], x[1,2], x[1,3] (vertex 1 mapping options)
- x[2,0], x[2,1], x[2,2], x[2,3] (vertex 2 mapping options)
- y[0,0], y[0,1], y[0,2], y[0,3] (edge 0 mapping options)
- y[1,0], y[1,1], y[1,2], y[1,3] (edge 1 mapping options)
- y[2,0], y[2,1], y[2,2], y[2,3] (edge 2 mapping options)

**Optimal Solution**:
- All vertices matched: x[0,0]=1, x[1,1]=1, x[2,2]=1
- Two edges matched: y[0,0]=1, y[1,1]=1
- One edge unmatched: y[2,*]=0 for all *

**Output**:
- GED: 1
- Unmatched edges: 1 (one edge of the triangle cannot map to the square's cycle)
