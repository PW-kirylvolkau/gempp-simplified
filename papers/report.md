Here is the converted markdown document based on the provided report.

# [cite_start]Algorithms and Computability - Project Report [cite: 1]

[cite_start]**Authors:** Remigiusz Iwanik, Bartosz Kaczmarek, Ignacy Walużenicz, Jakub Muszyński [cite: 2, 3, 4]
[cite_start]**Date:** October 2025 [cite: 3]

-----

## 1\. Introduction

[cite_start]Graphs, specifically directed multigraphs with parallel arcs and self-loops, serve as natural models for transportation, communication, biochemical, and workflow systems[cite: 6]. In these contexts, three related questions often arise:

1.  [cite_start]How to compare two networks[cite: 7].
2.  [cite_start]Whether one network embeds into another[cite: 8].
3.  [cite_start]How to minimally augment a host network so that an embedding becomes possible[cite: 8].

[cite_start]This report addresses these questions under a framework tailored to directed multigraphs[cite: 9].

### Problem Setting

Given two (multi)graphs $G=(V,E)$ and $H=(U,F)$ with $|V|\le|U|$, the report addresses:

  * [cite_start]**Size Measure:** A measure reflecting structural scope and connectivity, defined as $|G|=|V(G)|+|E(G)|$, where edges are counted with multiplicity[cite: 11, 12].
  * [cite_start]**Distance:** Graph Edit Distance (GED) is used as the primary notion of dissimilarity[cite: 13].
  * [cite_start]**Subgraph Detection:** Determining if there exists an injective mapping $\phi:V(G)\rightarrow V(H)$ such that $mult_G((u,v))\le mult_{H}((\phi(u),\phi(v)))$ for all $u, v \in V(G)$, and enumerating up to $N$ such embeddings[cite: 14, 15].
  * [cite_start]**Minimal Extension:** Determining the smallest augmentation $H^{\prime}\supseteq H$ that admits an embedding of $G$ (and the $N$-minimal extension problem for $N$ embeddings)[cite: 16].

### Approach and Contributions

  * [cite_start]**Foundations:** GED is formalized for directed multigraphs, introducing a "deficit" view to quantify edge additions needed for an injective mapping[cite: 19, 21].
    [cite_start]$$deficit(\phi)=\sum_{u,v\in V(G)}max(0,mult_{G}((u,v))-mult_{H}((\phi(u),\phi(v))))$$ [cite: 20]
  * [cite_start]**Exact Algorithms:** Based on Ullmann's backtracking method, adapted for multiplicities to enumerate embeddings and compute deficit patterns[cite: 23]. [cite_start]These deficits are combined to find the smallest joint augmentation for the $N$-minimal extension[cite: 24].
  * [cite_start]**Approximation Strategies:** A selection-function approach evaluates only $K$ promising injective mappings[cite: 26]. [cite_start]It utilizes a degree-/multiplicity-aware $K$-best assignments scheme using the Hungarian algorithm and Murty's method[cite: 28].
  * [cite_start]**Complexity:** As subgraph isomorphism is NP-complete and counting embeddings is \#P-complete, exact procedures target small graphs while approximations offer controllable accuracy via $K$[cite: 29, 30].

-----

## 2\. Mathematical Foundations

### 2.1 Graphs

  * [cite_start]**Graph:** A structure with a finite set of vertices $V(G)$ and edges $E(G)\subseteq V(G)\times V(G)$[cite: 36].
  * [cite_start]**Directed Graph:** Edges have orientation (arcs), represented as ordered pairs $(u,v)$ where $u$ is the tail and $v$ is the head[cite: 37, 38, 39]. [cite_start]It utilizes indegree and outdegree functions[cite: 40].
  * [cite_start]**Multigraph:** Two vertices can be joined by multiple edges[cite: 41]. [cite_start]$E(G)$ is a multiset, or a set of ordered pairs $(e, mult_G(e))$ where multiplicity is the count of singular edges[cite: 42, 45].
  * [cite_start]**Directed Multigraph:** The general concept used in this report, permitting parallel arcs and self-loops[cite: 47, 48].

### 2.2 Size of a Graph

[cite_start]The size is defined as the sum of the number of vertices and edges: $|G|=|V(G)|+|E(G)|$[cite: 57].

[cite_start]**Algorithm: GRAPHSIZE(G)** [cite: 66]

```text
1: v <- |V(G)|
2: e <- 0
3: for all ordered pairs {u,v} in V(G) such that u != v do
4:    e <- e + mult_G((u,v))
5: end for
6: for all vertices u in V(G) do
7:    e <- e + mult_G((u,u))
8: end for
9: return v + e
```

### 2.3 Distance between Graphs

[cite_start]Distance is defined as Graph Edit Distance (GED), the length of the minimal edit path transforming $G$ into $G'$ such that $G' \simeq H$[cite: 69]. The operations are:

1.  [cite_start]Vertex addition ($addv(v)$)[cite: 71].
2.  [cite_start]Vertex deletion ($delv(v)$)[cite: 72].
3.  [cite_start]Edge addition ($adde(e)$)[cite: 73].
4.  [cite_start]Edge deletion ($dele(e)$)[cite: 74].

GED is proposed as a proper metric satisfying:

1.  [cite_start]$ged(G, H) = 0 \iff G \simeq H$[cite: 78, 85].
2.  [cite_start]$ged(G, H) > 0 \iff G \not\simeq H$[cite: 80, 87].
3.  [cite_start]$ged(G, H) = ged(H, G)$ (Symmetry)[cite: 82, 90].
4.  [cite_start]$ged(F, G) + ged(G, H) \ge ged(F, H)$ (Triangle Inequality)[cite: 83, 103].

### 2.4 Graph Isomorphism

[cite_start]An isomorphism is a bijective mapping $\phi:V(G)\rightarrow V(H)$ such that $\forall_{u,v\in V(G)}mult_{G}((u,v))=mult_{H}((\phi(u),\phi(v)))$[cite: 109]. [cite_start]This preserves exact adjacency and edge multiplicity[cite: 110].

### 2.5 Subgraph Relation

$G$ is a subgraph of $H$ ($G \subseteq H$) if there exists an injective mapping $\phi:V(G)\rightarrow V(H)$ such that:
[cite_start]$$mult_{G}((u,v)) \le mult_{H}((\phi(u),\phi(v))) \quad \forall u,v \in V(G)$$[cite: 113, 114].

If not a subgraph, the **deficit** of a mapping $\phi$ is:
[cite_start]$$deficit(\phi)=\sum_{u,v\in V(G)}max(0,mult_{G}((u,v))-mult_{H}((\phi(u),\phi(v))))$$[cite: 117].

### 2.6 Minimal Extension

[cite_start]The minimal extension $H^{\prime}$ consists of $H$ plus the minimal set of added edges ($F_{added}$) required to satisfy $G \subseteq H^{\prime}$[cite: 120, 122]. [cite_start]The size of $F_{added}$ corresponds to the minimum deficit over all mappings $\phi$[cite: 123].

-----

## 3\. Checking for N Distinct Subgraph Isomorphisms

### 3.1 Algorithmic Methodology

[cite_start]The method solves the subgraph isomorphism problem (NP-complete) by enumerating $N$ distinct subgraphs of $H$ isomorphic to $G$[cite: 128, 131].

1.  **Search Space Reduction via Refinement:**
      * [cite_start]**Candidate Matrix ($M$):** Initialized such that $M_{ij}=1$ only if $indeg_G(i) \le indeg_H(j)$ and $outdeg_G(i) \le outdeg_H(j)$[cite: 133, 136].
      * **Refinement:** Iteratively prunes $M$. [cite_start]A candidate match $(i, j)$ is removed if neighbors of $i$ cannot be matched to neighbors of $j$ with sufficient multiplicity[cite: 138, 140].
2.  **Search via Backtracking:**
      * [cite_start]**Heuristic Ordering:** Vertices of $G$ are sorted by descending degree to prioritize constrained vertices[cite: 147, 148].
      * **Recursive Search ($DFS$):** Extends a partial injective mapping. [cite_start]It iterates through candidates $j$ where $M_{ij}=1$[cite: 151, 153].
      * [cite_start]**Consistency Check:** Validates if the new mapping preserves adjacency/multiplicity with previously mapped vertices[cite: 154].
      * [cite_start]**Termination:** Returns true if $N$ solutions are found[cite: 162].

### [cite_start]3.2 Pseudocode: FINDNSUBGRAPHS [cite: 166]

```text
procedure FINDNSUBGRAPHS(G, H, N)
    if |V(G)| = 0 or |V(G)| > |V(H)| then return true
    M <- INITIAL_CANDIDATES(G, H)
    if not REFINE_CANDIDATES_MULTIPLICITY(G, H, M) then return false
    order <- SORTVERTICES_BY_DESCDEGREE(G)
    used_H <- empty set
    mapping <- empty map
    count <- 0

    function DFS(t)
        if count >= N then return true
        if t = |V(G)| then
            count <- count + 1
            return false
        end if
        i <- order[t]
        for all j in V(H) where M[i][j] and j not in used_H do
            if CONSISTENTPARTIAL(mapping U {i->j}) then
                mapping[i] <- j
                used_H <- used_H U {j}
                if DFS(t+1) then return true
                used_H <- used_H \ {j}
                remove mapping[i]
            end if
        end for
        return false
    end function

    DFS(0)
end procedure
```

### 3.3 Complexity

  * [cite_start]Worst-case complexity is roughly $O(n_{G}\cdot P(n_{H},n_{G}))$, where $P$ is the permutation function[cite: 178].
  * [cite_start]It is exponential in $n_G$, the size of the pattern graph[cite: 179].

-----

## 4\. Methodology for N Minimal Mappings

[cite_start]This procedure finds a minimal composite extension of $H$ that accommodates $N$ distinct embeddings of $G$ simultaneously[cite: 181].

### 4.1 Enumeration

  * [cite_start]Performs an exhaustive DFS to generate all injective mappings[cite: 185].
  * [cite_start]**Deficit Matrix:** For each mapping $\phi$, computes matrix $E_{\phi}$ where $E_{\phi}(i,j)$ is the number of edges needed between $\phi(a)$ and $\phi(b)$ in $H$[cite: 186, 187].
  * [cite_start]All matrices are stored in set $\mathcal{E}$[cite: 188].

### 4.2 Combination Search

  * [cite_start]**Matrix Combination:** Merges matrices via element-wise max: $(A+B)[i, j] = \max(A[i, j], B[i, j])$[cite: 191, 192].
  * [cite_start]**Cost:** Sum of all entries in the combined matrix[cite: 195].
  * [cite_start]**Recursive Selection:** Selects $N$ distinct deficits to minimize the combined cost using a DFS approach (`edge_dfs`)[cite: 197].

### [cite_start]4.3 Pseudocode: MINIMAL EXTENSION N [cite: 202]

```text
function MINIMAL_EXTENSION_N(G, H, N)
    order <- SORTVERTICES_BY_DESCDEGREE(G)
    all_edgesets <- empty list

    function FULL_EDGESET(mapping)
        local_edgeset <- zero matrix |V(H)|x|V(H)|
        for all pairs (a,b) in V(G) do
             j_a <- mapping[a]; j_b <- mapping[b]
             total <- max(0, mult_G((a,b)) - mult_H((j_a, j_b)))
             if total > 0 then local_edgeset[j_a][j_b] <- total
        end for
        all_edgesets.add(local_edgeset)
    end function

    function DFS(t)
        if t = |V(G)| then FULL_EDGESET(mapping); return; end if
        i <- order[t]
        for all j in V(H) where j not used do
            mapping[i] <- j; used <- used U {j}
            DFS(t+1)
            used <- used \ {j}; remove mapping[i]
        end for
    end function
    
    DFS(0) 
    
    best_cost <- infinity
    best_edgeset <- zero matrix
    used_in_sum <- zero array[N]
    sum <- zero matrix

    function EDGEDFS(level)
        cost <- EDGESETCOST(sum)
        if level = N then
            if cost < best_cost then
                best_cost <- cost; best_edgeset <- COPY(sum)
            end if
            return
        end if
        if cost > best_cost then return end if
        
        start_index <- (level == 0) ? 0 : used_in_sum[level-1] + 1
        for i from start_index to |all_edgesets| do
             used_in_sum[level] <- i
             temp_sum <- sum
             sum <- ADJMATRIXADD(sum, all_edgesets[i])
             EDGEDFS(level + 1)
             sum <- temp_sum
        end for
    end function
    
    EDGEDFS(0)
    return (best_cost, best_edgeset)
end function
```

[cite_start]*(Note: Pseudocode logic adapted from text description and source code table for clarity)* [cite: 202-290].

### 4.4 Complexity

  * [cite_start]**Enumeration:** $O(P(n_{H},n_{G})\cdot n_{H}^{2})$[cite: 298].
  * [cite_start]**Combination:** Upper bound involves combinations of $N$ matrices: $O(\binom{P(n_{H},n_{G})}{N}\cdot n_{H}^{2})$[cite: 301].
  * [cite_start]Overall complexity is factorial: $O(n_{H}!)$[cite: 305].

-----

## 5\. Approximation Algorithms

[cite_start]Exact algorithms are unfit for large graphs due to exponential complexity[cite: 307]. [cite_start]Approximation strategies rely on limiting the search to $K$ promising mappings using a selection function[cite: 310, 313].

### 5.1 Mapping Extension Function

[cite_start]Constructs the minimum edge set to extend $H$ given a mapping $\phi$[cite: 323].

  * [cite_start]**Algorithm:** Iterates edges of $G$, calculating deficit $max(mult_G - mult_H, 0)$ and adding to an edge set[cite: 324].
  * [cite_start]**Complexity:** $O(|V(G)|^2)$ (assuming adjacency matrix implementation)[cite: 327, 328].

### 5.2 Isomorphism Verification Function

[cite_start]Similar to extension but returns `false` immediately upon detecting a deficit[cite: 332, 333].

  * [cite_start]**Complexity:** $O(|V(G)|^2)$ with $O(1)$ space[cite: 336, 337].

### 5.3 Approximation for Existence of N Isomorphisms

  * **Algorithm:** Generates $K$ mappings using a selection function. Checks each for isomorphism. [cite_start]Returns `true` if count $\ge N$[cite: 340, 343].
  * [cite_start]**Complexity:** $O(f(K,G,H)) + O(K \cdot |V(G)|^2)$[cite: 346, 348].
  * **Interpretation:** "Yes" is reliable; [cite_start]"No" implies "I do not know" (potential false negative)[cite: 367, 369].

### 5.4 Approximation for Extension of H

  * **Algorithm:** Selects $K$ mappings. For each, constructs the deficit edge set. [cite_start]Merges all edge sets by taking the maximum multiplicity for each edge[cite: 378].
  * [cite_start]**Complexity:** $O(f(K,G,H) + K \cdot |V(G)|^2)$[cite: 383].

### 5.5 Selection Algorithm

  * [cite_start]**Requirements:** Must be less than exponential, estimate mapping quality (likelihood of isomorphism), and be deterministic[cite: 398, 399, 401].
  * [cite_start]**Strategy:** Treat as a $K$-best assignments problem[cite: 407].
  * **Implementation:**
      * [cite_start]**Murty's Algorithm** based on the **Hungarian Method**[cite: 408].
      * [cite_start]Hungarian method (optimal assignment) runs in polynomial time $O(|V(H)|^3)$ or $O(|V(H)|^4)$[cite: 411].
      * [cite_start]Murty's algorithm finds $K$ best assignments with complexity roughly $O(K \cdot |V(H)|^4)$[cite: 418].
  * **Cost Function:** Based on degree difference. [cite_start]Quality increases if $deg_H(v) - deg_G(u)$ is large[cite: 419].
    [cite_start]$$cost[u,v] = deg_G(u) - deg_H(v)$$[cite: 420].

-----

## References

1.  [cite_start]Algorithms and Computability - Project Report [cite: 1]
2.  [cite_start]Remigiusz Iwanik [cite: 2]
3.  [cite_start]Bartosz Kaczmarek [cite: 3]
4.  [cite_start]Jakub Muszyński [cite: 4]
5.  [cite_start]Ignacy Walużenicz [cite: 3]
6.  Cordella, Luigi P., Pasquale Foggia, Carlo Sansone, and Mario Vento. 2004. "A (sub)graph isomorphism algorithm for matching large graphs." [cite_start]IEEE Transactions on Pattern Analysis and Machine Intelligence 26 (10): 1367-1372. [cite: 422]
7.  Garey, Michael R., and David S. Johnson. 1979. Computers and Intractability: A Guide to the Theory of NP-Completeness. [cite_start]W. H. Freeman. [cite: 424]
8.  Kuhn, H. W. 1955. "The Hungarian method for the assignment problem." [cite_start]Naval Research Logistics Quarterly 2 (1-2): 83-97. [cite: 426]
9.  Murty, K. G. 1968. "An algorithm for ranking all the assignments in order of increasing cost." [cite_start]Operations Research 16 (3): 682-687. [cite: 427]
10. Sanfeliu, A., and K.-S. Fu. 1983. "A distance measure between attributed relational graphs for pattern recognition." [cite_start]IEEE Transactions on Systems, Man, and Cybernetics SMC-13 (3): 353-362. [cite: 429]
11. Ullmann, J. R. 1976. "An algorithm for subgraph isomorphism." [cite_start]Journal of the ACM (JACM) 23 (1): 31-42. [cite: 431]
12. Valiant, Leslie G. 1979. "The Complexity of Computing the Permanent." [cite_start]Theoretical Computer Science 8 (2): 189-201. [cite: 432]