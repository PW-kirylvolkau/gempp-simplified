Here is the Markdown conversion of the provided file "Minimum cost subgraph matching using a binary linear program.pdf".

# Minimum Cost Subgraph Matching Using a Binary Linear Program

**Authors:** Julien Lerouge, Maroua Hammami, Pierre Héroux, Sébastien Adam
**Date:** May 27, 2019 (Submitted)
[cite_start]**Journal:** Pattern Recognition Letters, 2016, 71, pp.45-51 [cite: 5, 8]

---

## Abstract

This article presents a binary linear program (BLP) for the Minimum Cost Subgraph Matching (MCSM) problem. MCSM extends the subgraph isomorphism problem by tolerating attribute substitutions and graph structure modifications. The proposed objective function handles rich attributes (e.g., vectors mixing nominal and numerical values) on both vertices and edges. [cite_start]Experimental results on symbol spotting in technical drawings show the approach outperforms a previous substitution-only tolerant method[cite: 18, 19, 20, 21].

---

## 1. Introduction

[cite_start]Solving the subgraph isomorphism problem aims to determine if a pattern graph is isomorphic to a subgraph of a target graph[cite: 24]. [cite_start]It has applications in biosciences, chemistry, knowledge management, and image scene analysis[cite: 25]. [cite_start]In structural pattern recognition, it helps simultaneously segment and recognize objects[cite: 26].

However, subgraph isomorphism has two main drawbacks:
1.  [cite_start]**Computational Complexity:** It is an NP-complete problem, making large graph processing untractable[cite: 27, 28].
2.  [cite_start]**Strict Matching Requirement:** Real-world patterns often have distortions (noise, digitization errors), requiring error-tolerant matching rather than strict isomorphism[cite: 29, 30].

[cite_start]The problem thus shifts from a decision problem (isomorphism) to an optimization problem: **Minimum Cost Subgraph Matching (MCSM)**, which searches for the target subgraph minimizing a matching cost[cite: 35, 36].

This paper contributes a BLP formulation for MCSM that:
* [cite_start]Tolerates both structural (vertex/edge deletions) and attribute distortions[cite: 37, 39].
* [cite_start]Handles rich attributes on vertices and edges[cite: 40].
* [cite_start]Outperforms an existing substitution-only tolerant approach in symbol spotting experiments[cite: 42, 43].

---

## 2. Minimum Cost Subgraph Matching

### Definitions

* [cite_start]**Attributed Simple Graph:** A 4-tuple $G=(\mathcal{V},\mathcal{E},\mu,\nu)$ where $\mathcal{V}$ is a set of vertices, $\mathcal{E}$ is a set of edges, and $\mu, \nu$ are labeling functions for vertices and edges, respectively [cite: 47-52].
* [cite_start]**Subgraph:** $G_1 \subseteq G_2$ if $\mathcal{V}_1 \subseteq \mathcal{V}_2$, $\mathcal{E}_1 \subseteq \mathcal{E}_2$, and labels match [cite: 60-67].
* [cite_start]**Subgraph Isomorphism:** An injective function $f: \mathcal{V}_1 \rightarrow \mathcal{V}_2$ where $G_1$ is isomorphic to a subgraph $S \subseteq G_2$[cite: 69].

### Problem Statement
Due to noise, exact matching is often impossible. The goal is to find a subgraph $G \subseteq G_2$ that minimizes the distance to the pattern graph $G_1$:
$$G = \text{argmin}_{G_i \subseteq G_2} d(G_1, G_i)$$
[cite_start][cite: 76, 77]

The **Graph Edit Distance (GED)** is used as the dissimilarity measure:
$$d_{GED}(G_1, G_2) = \min_{o \in \mathcal{O}} \sum_{i} c(o_i)$$
[cite_start][cite: 81]


[cite_start]Elementary edit operations $o_i$ include substitutions ($v_1 \rightarrow v_2, e_1 \rightarrow e_2$), deletions ($v_1 \rightarrow \epsilon, e_1 \rightarrow \epsilon$), and insertions ($\epsilon \rightarrow v_2, \epsilon \rightarrow e_2$)[cite: 84]. [cite_start]The problem is simplified to finding the minimum cost edit path where insertion costs are zero, as insertions effectively just complete the subgraph in the target[cite: 96, 97].

---

## 3. Formulation as a Binary Linear Program

### 3.1 Binary Linear Programming (BLP)
[cite_start]A BLP minimizes a linear objective function $c^T x$ subject to linear constraints $Ax \le b$, where variables $x$ are binary [cite: 103-108]. [cite_start]Solving BLP is NP-hard, typically handled by solvers using branch-and-cut algorithms [cite: 115-118].

### 3.2 Formulation of the Problem
The formulation uses four sets of binary variables:
* [cite_start]$x_{i,k}$: Vertex substitution ($i \in \mathcal{V}_1 \rightarrow k \in \mathcal{V}_2$)[cite: 122].
* [cite_start]$\alpha_{i}$: Vertex deletion ($i \in \mathcal{V}_1 \rightarrow \epsilon$)[cite: 124].
* [cite_start]$y_{ij,kl}$: Edge substitution ($ij \in \mathcal{E}_1 \rightarrow kl \in \mathcal{E}_2$)[cite: 125].
* [cite_start]$\beta_{ij}$: Edge deletion ($ij \in \mathcal{E}_1 \rightarrow \epsilon$)[cite: 128].

**Objective Function:**
Minimize the sum of edit costs:
$$\min_{x,y,\alpha,\beta} \left( \sum_{i,k} x_{i,k} c(i \rightarrow k) + \sum_{i} \alpha_{i} c(i \rightarrow \epsilon) + \sum_{ij,kl} y_{ij,kl} c(ij \rightarrow kl) + \sum_{ij} \beta_{ij} c(ij \rightarrow \epsilon) \right)$$
[cite_start][cite: 130, 131]

**Constraints:**
1.  **Vertex Matching:** A vertex in $G_1$ matches at most one in $G_2$ (or is deleted).
    [cite_start]$\sum_{k} x_{i,k} \le 1 \quad \forall i \in \mathcal{V}_1$ [cite: 136]
2.  **Target Integrity:** A vertex in $G_2$ matches at most one in $G_1$.
    [cite_start]$\sum_{i} x_{i,k} \le 1 \quad \forall k \in \mathcal{V}_2$ [cite: 141]
3.  **Edge Matching:** An edge $ij$ in $G_1$ matches $kl$ in $G_2$ only if their head/tail vertices match.
    [cite_start]$\sum_{l, kl} y_{ij,kl} \le x_{i,k}$ and $\sum_{k, kl} y_{ij,kl} \le x_{j,l}$ [cite: 145, 149]

**Reduced Formulation:**
[cite_start]Deletion variables $\alpha$ and $\beta$ can be removed by substituting their expressions (e.g., $\alpha_i = 1 - \sum x_{i,k}$), reducing the problem size [cite: 153-156].

### 3.3 & 3.4 Extensions
* [cite_start]**Induced Subgraphs:** Requires constraints ensuring all edges in the matched target subgraph have a corresponding edge in $G_1$ [cite: 221-225].
* **Undirected Graphs:** Edge matching constraints are modified because $ij$ and $ji$ refer to the same edge. [cite_start]The constraint becomes $\sum y_{ij,kl} \le x_{i,k} + x_{j,k}$ [cite: 227-235].

### 3.5 Multiple Instances
To find multiple instances of a pattern, the model is called iteratively. [cite_start]After finding an optimal solution, a constraint is added to discard the vertices used in that solution for the next iteration[cite: 242, 248].

---

## 4. Experiments and Results

[cite_start]The approach is evaluated on **symbol spotting** in technical drawings using an application-dependent dataset[cite: 253, 256].

### 4.1 Graph Dataset
* [cite_start]**Data:** Region Adjacency Graphs (RAG) extracted from architectural floorplans[cite: 259].
* [cite_start]**Attributes:** Vertices have 26 attributes (area + 25 Zernike moments); edges have 2 attributes (relative scale, relative distance)[cite: 260, 261].
* [cite_start]**Distortions:** Pattern symbols are synthetically distorted (vectorial noise parameter $r$) to impact graph topology [cite: 272-276].

### 4.2 Protocol
* **Setup:** 50 documents for tuning, 50 for testing. 11 symbol models.
* **Costs:** Weighted Euclidean distance for substitutions. [cite_start]Deletion costs $C$ tested at 5, 10, 20, 40, 80 [cite: 297-301].
* [cite_start]**Comparison:** Compared against a Substitution Only Tolerant Subgraph Matching (**SOTSM**) approach[cite: 312].

### 4.3 Results
* **Accuracy (F1-score):** MCSM maintains high performance at higher noise levels ($r=8$) where SOTSM fails. [cite_start]For example, for class `sink3`, MCSM achieves an F1-score of 0.85 versus 0.18 for SOTSM [cite: 316-322, 333].
* [cite_start]**Robustness:** MCSM handles spurious vertices/edges generated by noise by allowing deletions, whereas SOTSM cannot[cite: 325].
* [cite_start]**Speed:** MCSM with optimal deletion cost ($C=40$) is approximately **10 times faster** than SOTSM (e.g., 1.14s vs 10.05s at $r=8$)[cite: 329, 336]. [cite_start]This is attributed to better initialization bounds in the branch-and-cut algorithm[cite: 334].

### 4.4 Synthetic Dataset Results
Experiments on synthetic graphs confirm scalability and speed advantages. [cite_start]MCSM is consistently faster than SOTSM, especially as graph size and density increase[cite: 355, 364].

---

## 5. Conclusions

The paper proposes a BLP for Minimum Cost Subgraph Matching that handles structural and attribute distortions. It outperforms substitution-only approaches in both accuracy and speed for symbol spotting tasks. [cite_start]Future work involves embedded learning of edit costs [cite: 361-368].

---

## References

[1] H. A. Almohamad and S. O. Duffuaa. A linear programming approach for the weighted graph matching problem. IEEE Trans. Pattern Anal. Mach. [cite_start]Intell., 15(5):522-525, May 1993. [cite: 370]
[12] Derek Justice and Alfred Hero. A binary linear programming formulation of the graph edit distance. IEEE Trans. Pattern Anal. Mach. [cite_start]Intell., 28(8):1200-1214, August 2006. [cite: 394]
[13] Pierre Le Bodic et al. [cite_start]An integer linear program for substitution-tolerant subgraph isomorphism... Pattern Recognition, 45(12):4214-4224, 2012. [cite: 396]
(See full list in source document pages 16-18)