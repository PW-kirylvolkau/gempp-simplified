Here is the Markdown conversion of the provided file.

# Graph Edit Distance: A New Binary Linear Programming Formulation

[cite_start]**Authors:** Julien Lerouge, Zeina Abu-Aisheh, Romain Raveaux, Pierre Héroux, and Sébastien Adam[cite: 3].
[cite_start]**Date:** May 22, 2015[cite: 4].

---

## Abstract

[cite_start]Graph edit distance (GED) is a powerful and flexible graph matching paradigm that can be used to address different tasks in structural pattern recognition, machine learning, and data mining[cite: 6]. [cite_start]In this paper, some new binary linear programming formulations for computing the exact GED between two graphs are proposed[cite: 7]. [cite_start]A major strength of the formulations lies in their genericity since the GED can be computed between directed or undirected fully attributed graphs (i.e. with attributes on both vertices and edges)[cite: 8]. [cite_start]Moreover, a relaxation of the domain constraints in the formulations provides efficient lower bound approximations of the GED[cite: 9]. [cite_start]A complete experimental study comparing the proposed formulations with 4 state-of-the-art algorithms for exact and approximate graph edit distances is provided[cite: 10]. [cite_start]By considering both the quality of the proposed solution and the efficiency of the algorithms as performance criteria, the results show that none of the compared methods dominates the others in the Pareto sense[cite: 11]. [cite_start]As a consequence, faced to a given real-world problem, a trade-off between quality and efficiency has to be chosen w.r.t. the application constraints[cite: 12, 13].

---

## 1. Introduction

[cite_start]Graphs are data structures able to describe complex entities through their elementary components (the vertices of the graph) and the relational properties between them (the edges of the graph)[cite: 15]. [cite_start]For attributed graphs, both vertices and edges can be characterized by attributes that can vary from nominal labels to more complex descriptions such as strings or feature vectors, leading to very powerful representations[cite: 16]. [cite_start]Consequently, graph representations have become popular in domains such as computer vision, biology, chemistry, and pattern recognition[cite: 17, 19].

[cite_start]A major issue related to graph-based algorithms is the computation of a (dis)similarity measure between two graphs[cite: 20]. [cite_start]Algorithms for this can be categorized as embedding-based methods vs. matching-based methods[cite: 22].
* [cite_start]**Embedding-based methods:** The key idea is to project the input graphs into a vector space[cite: 23]. [cite_start]While computationally effective, they do not take into account the complete relational properties and do not provide matching between vertices and edges[cite: 26, 27].
* [cite_start]**Matching-based methods:** These compute the "best" matching between graphs[cite: 29]. [cite_start]Examples include maximum common subgraph methods [cite: 32] [cite_start]and error-tolerant graph matching[cite: 33].

[cite_start]Graph edit distance (GED) is a well-known error-tolerant matching-based method[cite: 36]. [cite_start]The dissimilarity computation is linked to a set of graph edit operations (e.g., node insertion, deletion), each characterized by a cost[cite: 37, 38]. [cite_start]The GED is the total cost of the least expensive sequence of edit operations transforming one graph into the other[cite: 38]. [cite_start]A major advantage is its applicability to arbitrarily structured and attributed graphs[cite: 39]. [cite_start]However, its computational complexity is exponential in the number of nodes[cite: 42].

This paper tackles the GED problem by proposing two main contributions:
1.  [cite_start]Solving the GED problem with binary linear programming (BLP) via two original exact formulations[cite: 48, 49]. [cite_start]These are general (directed/undirected, fully attributed)[cite: 50]. [cite_start]Relaxations of these formulations provide efficient lower bound approximations[cite: 51].
2.  [cite_start]A complete comparative study comparing eight algorithms on graph datasets[cite: 52].

---

## 2. Problem Statement

### Definition 1: Attributed Graph
[cite_start]An attributed graph $G$ is a 4-tuple $G=(V,E,\mu,\xi)$ where[cite: 62]:
* [cite_start]$V$ is a set of vertices[cite: 62].
* [cite_start]$E$ is a set of edges, such that $\forall e=(i,j)\in E$, $i\in V$ and $j\in V$[cite: 63].
* [cite_start]$\mu:V\rightarrow L_{V}$ is a vertex labeling function[cite: 64].
* [cite_start]$\xi:E\rightarrow L_{E}$ is an edge labeling function[cite: 65].

[cite_start]The label spaces $L_V$ and $L_E$ may be composed of numeric, symbolic, or string attributes[cite: 66]. [cite_start]The definition handles simple graphs or multigraphs, and directed or undirected graphs [cite: 68-73].

### Definition 2: Graph Edit Distance
The graph edit distance $d(.,.)$ is a function $d:\mathcal{G}\times\mathcal{G}\rightarrow\mathbb{R}^{+}$ defined as:
$$d(G_{1},G_{2}) = \min_{a=(o_{1},...,o_{k})\in\Gamma(G_{1},G_{2})}\sum_{i=1}^{k}c(o_{i})$$
[cite_start][cite: 77-79]

[cite_start]Where $\Gamma(G_{1},G_{2})$ is the set of all edit paths transforming $G_1$ into $G_2$[cite: 80]. [cite_start]Elementary edit operations $o_i$ include vertex substitution ($v_1 \rightarrow v_2$), edge substitution ($e_1 \rightarrow e_2$), vertex deletion ($v_1 \rightarrow \epsilon$), edge deletion ($e_1 \rightarrow \epsilon$), vertex insertion ($\epsilon \rightarrow v_2$), and edge insertion ($\epsilon \rightarrow e_2$)[cite: 81].

[cite_start]The cost function $c(.)$ must satisfy metric properties [cite: 83-87][cite_start], and to guarantee symmetry, costs must be defined symmetrically (e.g., $c(v_1 \rightarrow v_2) = c(v_2 \rightarrow v_1)$) [cite: 88-91].

---

## 3. Related Work

[cite_start]The GED problem is NP-hard[cite: 101]. [cite_start]Approaches are distinguished into exact methods and approximations[cite: 100].

### 3.1 Exact Approaches
* [cite_start]**$A^*$ Algorithm:** Relies on exploring the tree of solutions[cite: 104]. [cite_start]Exploration is guided by an estimation of the future cost[cite: 108]. [cite_start]If the estimation is lower than or equal to the real cost, an optimal path is guaranteed[cite: 110].
* [cite_start]**Binary Linear Programming (BLP):** Almohamad and Duffuaa [cite: 117] [cite_start]and Justice and Hero [cite: 118] proposed BLP formulations. Justice and Hero's formulation minimizes:
    $$d(G_{1},G_{2})=\min_{P}\sum_{i=1}^{n}\sum_{j=1}^{n}C_{i,j}P_{i,j}+\frac{1}{2}||A_{1}-PA_{2}P^{T}||_{1}$$
    [cite_start][cite: 120]
    [cite_start]However, this formulation uses adjacency matrices, restricting it to simple graphs and not integrating edge labels[cite: 119, 122].

### 3.2 Approximations
* [cite_start]**Justice and Hero:** Proposed a lower bound computable in $\mathcal{O}(n^{7})$ by relaxing variables to $[0, 1]$ and an upper bound in $\mathcal{O}(n^{3})$ using the Hungarian method [cite: 125-127].
* [cite_start]**Riesen et al.:** Proposed determining vertex assignment using the Munkres algorithm on a cost matrix, providing an upper bound[cite: 129, 130].
* [cite_start]**$A^*$-Beamsearch:** Prunes the tree of solutions by limiting concurrent partial solutions to a parameter $q$[cite: 132, 134].
* [cite_start]**$A^*$-Pathlength:** Prioritizes long partial edit paths[cite: 138].
* [cite_start]**Genetic Algorithms:** Use vertex assignments from bipartite matching as initialization[cite: 140].
* [cite_start]**Hausdorff Edit Distance:** Integrates a heuristic based on modified Hausdorff distance into the $A^*$ algorithm[cite: 149].

---

## 4. Graph Edit Distance Using Binary Linear Programming

[cite_start]The problem is modeled as a Binary Linear Program (BLP) of the form: $\min c^T x$ subject to $Ax \le b$ and $x \in \{0,1\}^n$ [cite: 162-165].

### 4.1 Modelling the GED Problem

We consider graphs $G_1$ and $G_2$. [cite_start]The formulations can apply to multigraphs and undirected graphs with slight modifications[cite: 178].

#### 4.1.1 Variables and Cost Functions
Binary variables are defined for operations:
* [cite_start]**Substitution:** $x_{i,k} = 1$ if vertex $i \in V_1$ is substituted with $k \in V_2$ [cite: 183-185]. [cite_start]$y_{ij,kl} = 1$ if edge $ij \in E_1$ is substituted with $kl \in E_2$ [cite: 186-189].
* [cite_start]**Deletion:** $u_i$ (vertex $i$ deletion), $e_{ij}$ (edge $ij$ deletion)[cite: 188, 192, 194, 196].
* [cite_start]**Insertion:** $v_k$ (vertex $k$ insertion), $f_{kl}$ (edge $kl$ insertion)[cite: 193, 200, 198, 199].

[cite_start]Costs are denoted as $c(i \rightarrow k)$, $c(ij \rightarrow kl)$, $c(i \rightarrow \epsilon)$, $c(\epsilon \rightarrow k)$, etc. [cite: 204-206].

#### 4.1.2 Objective Function
The objective is to minimize the total cost:
$$
\min_{x,y,u,v,e,f} \left( \sum_{i\in V_{1}}\sum_{k\in V_{2}}c(i\rightarrow k)\cdot x_{i,k} + \sum_{ij\in E_{1}}\sum_{kl\in E_{2}}c(ij\rightarrow kl)\cdot y_{ij,kl} + \sum_{i\in V_{1}}c(i\rightarrow\epsilon)\cdot u_{i}+\sum_{k\in V_{2}}c(\epsilon\rightarrow k)\cdot v_{k} + \sum_{ij\in E_{1}}c(ij\rightarrow\epsilon)\cdot e_{ij}+\sum_{kl\in E_{2}}c(\epsilon\rightarrow kl)\cdot f_{kl} \right)
$$
[cite_start][cite: 215-217]

#### 4.1.3 Constraints
Admissible edit paths must satisfy:
1.  [cite_start]**Vertex Matching:** Each vertex of $G_1$ is matched or deleted; each vertex of $G_2$ is matched or inserted [cite: 227-229].
    * [cite_start]$u_{i}+\sum_{k\in V_{2}}x_{i,k}=1 \quad \forall i\in V_{1}$ [cite: 230]
    * [cite_start]$v_{k}+\sum_{i\in V_{1}}x_{i,k}=1 \quad \forall k\in V_{2}$ [cite: 230]
2.  [cite_start]**Edge Matching:** similar constraints for edges involving $e_{ij}$ and $f_{kl}$ [cite: 233-236].
3.  [cite_start]**Topological Constraints:** An edge $ij$ can be matched to $kl$ iff head vertices $(i, k)$ and tail vertices $(j, l)$ are matched[cite: 240]. Linearly expressed as:
    * [cite_start]$y_{ij,kl}\le x_{i,k}$ [cite: 242]
    * [cite_start]$y_{ij,kl}\le x_{j,l}$ [cite: 245]

#### 4.1.4 Straightforward Formulation (F1)
[cite_start]Putting these together yields formulation **(F1)**, which uses $|V_{1}|+|V_{2}|+|E_{1}|+|E_{2}|+2\cdot|E_{1}|\cdot|E_{2}|$ constraints (excluding domain constraints) [cite: 249-282].

### 4.2 Reducing the Size of the Formulation

#### 4.2.1 Reducing Variables
Variables $u, v, e, f$ can be removed. Vertex matching constraints become inequalities:
[cite_start]$\sum_{k\in V_{2}}x_{i,k}\le1$ and $\sum_{i\in V_{1}}x_{i,k}\le1$[cite: 292, 293].
[cite_start]The objective function is rewritten using only substitution variables and a constant $C$ representing the cost if all elements were deleted/inserted [cite: 302-304].

#### 4.2.2 Reducing Constraints
[cite_start]The quadratic number of topological constraints in F1 ($|E_1| \cdot |E_2|$) can be reduced[cite: 310, 311]. Constraints (8) and (9) are replaced by:
* [cite_start]$\sum_{kl\in E_{2}}y_{ij,kl}\le x_{i,k} \quad \forall k\in V_{2}, \forall ij\in E_{1}$ [cite: 316]
* [cite_start]$\sum_{kl\in E_{2}}y_{ij,kl}\le x_{j,l} \quad \forall l\in V_{2}, \forall ij\in E_{1}$ [cite: 319]

[cite_start]**Proposition 2** proves that the set of admissible edit paths remains unchanged [cite: 322-341]. The number of topological constraints becomes $|E_1| [cite_start]\cdot |V_2|$, growing linearly with graph density[cite: 342].

#### 4.2.3 Simplified Formulation (F2)
The simplified formulation **(F2)** is:
**Objective:**
$$
\min_{x,y} \left( \sum_{i\in V_{1}}\sum_{k\in V_{2}}(c(i\rightarrow k)-c(i\rightarrow\epsilon)-c(\epsilon\rightarrow k))\cdot x_{i,k} + \sum_{ij\in E_{1}}\sum_{kl\in E_{2}}(c(ij\rightarrow kl)-c(ij\rightarrow\epsilon)-c(\epsilon\rightarrow kl))\cdot y_{ij,kl} + C \right)
$$
[cite_start][cite: 364-366]

**Subject to:**
* [cite_start]$\sum_{k\in V_{2}}x_{i,k}\le1 \quad \forall i\in V_{1}$ [cite: 368]
* [cite_start]$\sum_{i\in V_{1}}x_{i,k}\le1 \quad \forall k\in V_{2}$ [cite: 370]
* [cite_start]$\sum_{kl\in E_{2}}y_{ij,kl}\le x_{i,k} \quad \forall k\in V_{2}, \forall ij\in E_{1}$ [cite: 372]
* [cite_start]$\sum_{kl\in E_{2}}y_{ij,kl}\le x_{j,l} \quad \forall l\in V_{2}, \forall ij\in E_{1}$ [cite: 374]

### 4.4 Lower Bounding with Continuous Relaxation
[cite_start]Continuous relaxation (replacing $\{0,1\}$ with $[0,1]$) provides a lower bound solvable in polynomial time $\mathcal{O}(n^{3.5})$ [cite: 390-392]. [cite_start]We call **F1LP** and **F2LP** the continuous relaxations of F1 and F2[cite: 395].

---

## 5. Experiments

### 5.1 Studied Methods
[cite_start]The study compares four proposed approaches (F1, F2, F1LP, F2LP) with four existing algorithms[cite: 404]:
1.  [cite_start]**$A^*$ ([25]):** Exact, using bipartite heuristic[cite: 406, 423].
2.  [cite_start]**BP ([21]):** Upper bound, bipartite graph matching[cite: 412, 429].
3.  [cite_start]**BS-$q$ ([22]):** Upper bound, $A^*$ beam search[cite: 411, 431].
4.  [cite_start]**H ([40]):** Lower bound, Modified Hausdorff distance[cite: 414, 433].

### 5.2 Datasets
[cite_start]Algorithms are applied to three real-world datasets and one synthetic dataset[cite: 451]. [cite_start]Subsets were built where all graphs have the same number of vertices[cite: 454].

* **GREC:** Architectural/electronic symbols. [cite_start]Undirected, labeled with types and coordinates [cite: 460-464].
* **MUTA (Mutagenicity):** Molecules (atoms and bonds). [cite_start]Undirected, chemical symbols and valences [cite: 481-484].
* **PROT (Protein):** Secondary structure elements. [cite_start]Undirected, amino acid sequences and distances [cite: 494-500].
* [cite_start]**ILPISO:** Synthetic directed graphs with noise[cite: 520, 530].

[cite_start]**Table 3: Subsets Decomposition** [cite: 502]

| Dataset | Subsets (#vertices) |
| :--- | :--- |
| PROT | 20, 30, 40 |
| ILPISO | 10, 25, 50 |
| GREC | 5, 10, 15, 20 |

[cite_start]**Table 5: Cost Function Meta Parameters** [cite: 534]

| | $\tau_{vertex}$ | $\tau_{edge}$ | $\alpha$ | Vertex Substitution | Edge Substitution |
| :--- | :--- | :--- | :--- | :--- | :--- |
| GREC | 90 | 15 | 0.5 | Ext. Euclidean | Dirac |
| PROT | 11 | 1 | 0.75 | Ext. String Edit | Dirac |
| MUTA | 11 | 1.1 | 0.25 | Dirac | Dirac |
| ILPISO | 66.6 | 66.6 | 0.5 | L1 norm | L1 norm |

### 5.3 Protocol
* [cite_start]Max time: 300 seconds per distance computation[cite: 542].
* Metrics:
    * [cite_start]**Deviation:** Error relative to the reference solution (optimal or best found)[cite: 552].
    * [cite_start]**Speed:** Normalized computation time[cite: 582].
* Implementation: CPLEX 12.6 for F1/F2; [cite_start]Java for others[cite: 593, 594].

### 5.4 Results

**Accuracy (Deviation):**
* **F2 vs F1:** F2 outperforms F1 on all datasets. [cite_start]The gap reaches 20% on MUTA[cite: 602].
* [cite_start]**Lower Bounds:** F2LP is the most accurate lower bound[cite: 603]. [cite_start]On GREC, F2LP is near-optimal[cite: 614].
* [cite_start]**$A^*$:** Fails on graphs larger than 10 vertices due to memory saturation (deviation > 30%)[cite: 625, 626].
* [cite_start]**Approximate Methods:** F2LP is 6% more accurate on average than BS (the second best)[cite: 631].

**Speed:**
* **F2 vs F1:** F2 is always faster. [cite_start]On GREC, F2 can be 100 times faster[cite: 633, 634].
* [cite_start]**Literature:** H and BP are the fastest (less than 3 seconds)[cite: 651, 670]. [cite_start]BS is faster than F2 but less accurate[cite: 669].

[cite_start]**Table 6: Mean deviation gap between methods (%)** [cite: 660]

| % | F1 | F2 | F1LP | F2LP | BP | BS | H |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **F1** | 0 | 3 | 8.3 | 4.6 | 29.5 | 12.6 | 39.7 |
| **F2** | 3 | 0 | 11.4 | 7.7 | 32.5 | 15.7 | 42.8 |
| **F2LP** | 4.6 | 7.7 | 3.7 | 0 | 24.8 | 8 | 35.1 |

**Summary:**
[cite_start]F2 is 15% more accurate on average than the best literature method[cite: 684]. [cite_start]F2LP is 8% more accurate than BS but requires 13% more time[cite: 685].

---

## 6. Conclusion

[cite_start]Two exact BLP formulations (F1, F2) for GED were presented, capable of handling complex attributed graphs[cite: 674, 675]. [cite_start]F2 condenses variables and constraints, proving to be faster and more accurate than F1[cite: 679, 682]. [cite_start]Continuous relaxations (F1LP, F2LP) provide lower bounds[cite: 680]. [cite_start]F2 is shown to be highly accurate, and F2LP offers a strong trade-off between speed and accuracy compared to Beam Search[cite: 684, 685]. [cite_start]Future work includes investigating binary quadratic programming and optimizing nearest neighbor search using these bounds[cite: 687, 688].

---

## Appendices

### A. Extension to Undirected Graphs
For undirected graphs, edge $ij$ and $ji$ refer to the same edge. [cite_start]The formulation (F2u) adapts F2 by modifying topological constraints [cite: 691-693].
Constraints (16) and (17) are replaced by:
$$\sum_{kl\in E_{2}}y_{ij,kl}\le x_{i,k}+x_{j,k} \quad \forall k\in V_{2}, \forall ij\in E_{1}$$
[cite_start][cite: 694, 702]
[cite_start]Because $x_{i,k}$ and $x_{j,k}$ cannot be simultaneously 1, the sum is at most 1[cite: 696].