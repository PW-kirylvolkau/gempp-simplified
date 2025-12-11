# Mathematical Foundations

This document describes the mathematical concepts and definitions used in the solution.

## 1. Graph Definitions

### 1.1 Graph

A **graph** is a pair G = (V, E) where:
- V is a finite set of **vertices** (nodes)
- E ⊆ V × V is a set of **edges** (connections between vertices)

For **undirected graphs**, edges are unordered pairs: {u, v} ∈ E implies {v, u} ∈ E.

### 1.2 Graph Size

We define the **size** of a graph G = (V, E) as:

```
|G| = |V| + |E|
```

This definition counts both vertices and edges, which is consistent with:
- The storage complexity of graph representations
- The edit distance metric (each vertex/edge can be added or removed)
- Common usage in graph theory literature

### 1.3 Adjacency Matrix

For a graph G = (V, E) with n = |V| vertices, the **adjacency matrix** A is an n × n matrix where:

```
A[i][j] = 1  if (i, j) ∈ E
A[i][j] = 0  otherwise
```

For undirected graphs, A is symmetric: A[i][j] = A[j][i].

## 2. Graph Metric

### 2.1 Graph Edit Distance (GED)

The **Graph Edit Distance** between two graphs G₁ and G₂ is the minimum cost of transforming G₁ into G₂ using elementary edit operations:

1. **Vertex insertion**: Add a new vertex
2. **Vertex deletion**: Remove an existing vertex
3. **Edge insertion**: Add a new edge
4. **Edge deletion**: Remove an existing edge

Formally:

```
GED(G₁, G₂) = min { cost(P) | P is an edit path from G₁ to G₂ }
```

where cost(P) is the sum of costs of all operations in path P.

### 2.2 Metric Properties

GED satisfies all metric axioms **when edit costs are properly defined**:

1. **Non-negativity**: GED(G₁, G₂) ≥ 0 (requires non-negative costs)
2. **Identity**: GED(G₁, G₂) = 0 ⟺ G₁ ≅ G₂ (isomorphic)
3. **Symmetry**: GED(G₁, G₂) = GED(G₂, G₁) (requires symmetric costs)
4. **Triangle inequality**: GED(G₁, G₃) ≤ GED(G₁, G₂) + GED(G₂, G₃)

**Note**: The triangle inequality holds when edit costs satisfy certain conditions. With unit costs (all operations cost 1.0), as used in this implementation, GED is a valid metric. See [Serratosa (2019)](https://www.sciencedirect.com/science/article/abs/pii/S0031320319300639) for detailed analysis of when GED forms a metric.

## 3. Subgraph Isomorphism

### 3.1 Definition

Graph G₁ = (V₁, E₁) is a **subgraph** of G₂ = (V₂, E₂), written G₁ ⊆ G₂, if there exists an **injective mapping** φ: V₁ → V₂ such that:

```
∀(u, v) ∈ E₁: (φ(u), φ(v)) ∈ E₂
```

The mapping φ is called a **subgraph isomorphism**.

### 3.2 Computational Complexity

The subgraph isomorphism problem is **NP-complete**. This means:
- No known polynomial-time algorithm exists
- Exhaustive search has complexity O(n!)
- Practical solutions use constraint propagation and branch-and-bound

## 4. Minimal Extension

### 4.1 Definition

Given graphs G₁ (pattern) and G₂ (target), the **minimal extension** of G₂ with respect to G₁ is the minimum number of elements (vertices and edges) that must be added to G₂ such that G₁ becomes a subgraph of the extended G₂.

Formally:

```
MinExt(G₁, G₂) = min { |V'| + |E'| | G₁ ⊆ (V₂ ∪ V', E₂ ∪ E') }
```

### 4.2 Relation to GED

The minimal extension equals the GED when we only allow insertions into G₂:

```
MinExt(G₁, G₂) = GED_{insert-only}(G₁ → G₂)
```

### 4.3 Properties

- MinExt(G₁, G₂) = 0 ⟺ G₁ is a subgraph of G₂
- MinExt(G₁, G₂) ≤ |G₁| (at most all pattern elements need to be added)

## 5. Integer Linear Programming Formulation

### 5.1 Variables

For pattern graph G₁ = (V₁, E₁) and target graph G₂ = (V₂, E₂):

- **Vertex matching**: x_{i,k} ∈ {0, 1} for i ∈ V₁, k ∈ V₂
  - x_{i,k} = 1 means pattern vertex i maps to target vertex k

- **Edge matching**: y_{ij,kl} ∈ {0, 1} for (i,j) ∈ E₁, (k,l) ∈ E₂
  - y_{ij,kl} = 1 means pattern edge (i,j) maps to target edge (k,l)

### 5.2 Constraints

**C1. Vertex injection (pattern side)**: Each pattern vertex maps to at most one target vertex
```
∀i ∈ V₁: Σₖ x_{i,k} ≤ 1
```

**C2. Vertex injection (target side)**: Each target vertex receives at most one pattern vertex
```
∀k ∈ V₂: Σᵢ x_{i,k} ≤ 1
```

**C3. Edge injection**: Each pattern edge maps to at most one target edge
```
∀(i,j) ∈ E₁: Σ_{(k,l)} y_{ij,kl} ≤ 1
```

**C4. Edge consistency**: Edge matchings must be consistent with vertex matchings
```
∀(i,j) ∈ E₁, ∀k ∈ V₂:
  Σ_{l:(k,l)∈E₂} y_{ij,kl} ≤ x_{i,k} + x_{j,k}  (for undirected)
```

### 5.3 Objective Function

Minimize the number of unmatched pattern elements:

```
minimize: Σᵢ(1 - Σₖ x_{i,k}) + Σ_{ij}(1 - Σ_{kl} y_{ij,kl})
```

Equivalently:
```
minimize: (|V₁| + |E₁|) - Σᵢₖ x_{i,k} - Σ_{ij,kl} y_{ij,kl}
```

The optimal value equals MinExt(G₁, G₂).

## References

1. Bunke, H. (1997). "On a relation between graph edit distance and maximum common subgraph." Pattern Recognition Letters.
2. Riesen, K., & Bunke, H. (2009). "Approximate graph edit distance computation by means of bipartite graph matching." Image and Vision Computing.
