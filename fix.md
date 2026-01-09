# Deep Analysis of main.tex Report

## Summary of Findings

Comprehensive analysis of the report (`report/main.tex`) against the task requirements (`docs/TASK.md`), the codebase implementation, and mathematical/general knowledge. Issues organized by severity.

---

## CRITICAL ISSUES

### 1. Duplicate Bibliography Section (Lines 778-821)

**Problem:** The report has TWO separate `\begin{thebibliography}` sections:
- First bibliography at lines 778-798 (contains Bunke1997, RiesenBunke2009, Solnon2015)
- Second bibliography at lines 803-821 (contains riesen2009, lerougeBLP, lerougeMCSM)

**Impact:** LaTeX will produce compilation warnings/errors. The first bibliography uses `\bibitem{RiesenBunke2009}` and the second has `\bibitem{riesen2009}` - these are duplicate entries for the same paper with different keys.

**Fix Required:** Merge into a single bibliography section.

---

### 2. Self-Loop Contradiction (Implementation vs. Report)

**Report Claims (Lines 47-49, 58):**
- "we keep the usual restriction of no self-loops for simplicity"
- "diagonal entries are zero because self-loops are disallowed"

**Implementation Reality:**
- `src/model/adjacency_parser.h` line 127: "Create directed edges **(supports multigraphs and self-loops)**"
- Test cases `tests/18_multigraph_self_loop/` and `tests/20_multigraph_loop_and_parallel/` demonstrate self-loop handling with non-zero diagonal entries

**Fix Required:** Either update the report to reflect that self-loops ARE supported, or clarify the discrepancy.

---

### 3. Undirected vs. Directed Graph Inconsistency

**Report Inconsistencies:**
- Introduction (line 27): "finite simple **undirected graphs**"
- Section 2.1 (line 47): "finite **directed multigraphs**"
- Formulation constraints (lines 241, 319): Uses undirected set notation `{i,j}` for edges

**Implementation Reality:**
- ALL graphs are created as **DIRECTED** (`adjacency_parser.h` line 86: "Create graph as DIRECTED")
- Edge consistency constraints use `getOrigin()` and `getTarget()` (directed semantics)

**Fix Required:** Make the report consistent - either clarify that directed graphs are used throughout, or explain both undirected and directed modes.

---

## MAJOR ISSUES

### 4. Empty Results Table (Lines 617-632)

**Problem:** Table 6 (exp-results-large) has all values as `--`:
```latex
\texttt{exp6\_grid4\_in\_grid5}    & -- & -- & -- & -- & -- & -- \\
\texttt{exp7\_k8\_in\_k10}         & -- & -- & -- & -- & -- & -- \\
\texttt{exp8\_tree12\_in\_path15}  & -- & -- & -- & -- & -- & -- \\
```

**Impact:** This is incomplete content that makes Section 5.11 ("Outcomes of the experiments") meaningless.

**Fix Required:** Run the experiments and fill in the actual values, or remove the section.

---

### 5. Bibliography Citation Errors

**Problems:**
1. `riesen2009` (line 806-809): Author is "**P.** Riesen" but should be "**K.** Riesen" (Kaspar Riesen)
2. `lerougeBLP` (line 814): Published in "Pattern Recognition Letters, 2015" but the full paper was actually published in **Pattern Recognition, 2017** (72:254-265). The 2015 date is for the arXiv preprint.
3. Duplicate citation: Same Riesen & Bunke 2009 paper appears twice with different keys (`RiesenBunke2009` and `riesen2009`)
4. `Bunke1997`, `RiesenBunke2009`, `Solnon2015` are defined but never cited in the document

---

### 6. Section Reference Missing Label

**Problem (Line 638):**
```latex
\cite{lerougeMCSM}
```
This citation appears in Section 6.1 referring to "the laboratory task specification" but `lerougeMCSM` is the MCSM paper about subgraph matching - this should reference the actual lab task document, not a research paper.

---

## MODERATE ISSUES

### 7. Triangle Inequality Justification - Confusing Wording (Lines 156-158)

**Current Text:**
> "concatenating optimal edit paths from F to G and from G to H yields an edit path from F to H whose cost is **at least** d_GED(F,H)"

**Problem:** The wording "at least" is backwards/confusing. The concatenated path has cost d(F,G) + d(G,H), and since it's a valid (but possibly non-optimal) path, we have d(F,H) ≤ d(F,G) + d(G,H).

**Fix:** Rephrase to: "...yields an edit path from F to H whose cost equals d(F,G) + d(G,H), which upper-bounds d_GED(F,H), thus establishing d(F,H) ≤ d(F,G) + d(G,H)."

---

### 8. F2 Formulation Description Uses Undirected Notation

**Problem (Lines 241-245):** The edge consistency constraints are written using undirected graph notation:
```latex
\sum_{f\in E_2:\; k\in f} y_{\{i,j\},f} \le x_{i,k}+x_{j,k}
```

But the implementation uses directed graph constraints with separate handling for edge origin and target vertices.

**Fix:** Either add directed constraint formulation or clarify that the undirected version is shown for simplicity.

---

### 9. Introduction Mentions "simple graphs" but Implementation Handles Multigraphs

**Problem (Line 27):** "finite simple undirected graphs" - but:
- The implementation explicitly supports multigraphs (parallel edges)
- Test cases `tests/16_multigraph_exact`, `tests/17_multigraph_extension`, etc. demonstrate this

---

## MINOR ISSUES

### 10. Missing Conclusion Content

**Problem:** The Conclusions section (lines 732-774) is reasonably complete but lacks:
- A summary of whether the TASK.md requirements were met
- Discussion of the advanced version (multigraphs) vs. basic version

---

### 11. Comment in Wrong Language (Line 5)

```latex
\usepackage[english]{babel} % albo [polish], jeśli raport ma być po polsku
```

Polish comment in an English document - minor but inconsistent.

---

### 12. Expected GED Calculations Could Be Clearer (Lines 608-612)

**Problem:** The expected GED values are stated without clarifying that they assume the pattern is an induced subgraph of the target. For example:
- "exp6: GED should equal...25" assumes the 4x4 grid embeds naturally into the 5x5 grid
- This assumption should be explicitly stated

---

### 13. Reference to Non-Existent Greedy in Discussion (Lines 405-406)

**Problem:** Section 4.3 mentions:
> "the matching-based approach described above does not guarantee..."

But the "approach described above" refers to a bipartite matching that was never actually described in Section 4.2 (which describes greedy matching). The text seems to conflate different approximation methods.

---

## TASK.md COMPLIANCE CHECK

| Requirement | Status | Notes |
|-------------|--------|-------|
| Graph size definition | OK | Defined in Section 2.2 |
| Metric in set of graphs | OK | GED defined in Section 2.3 |
| Minimal extension definition | OK | Defined in Section 2.5 |
| Descriptions with justifications | OK | Provided throughout |
| Algorithm descriptions | OK | Sections 3 and 4 |
| Computational complexity analysis | OK | Section 3.2 |
| Exponential + polynomial approx | OK | Both exact and approx provided |
| Computational test results | PARTIAL | Table 6 is empty |
| Time characteristics | OK | Tables include ms timings |
| Technical description | OK | Section 6 |
| Compiling instructions | OK | Section 6.2 |
| Conclusions | OK | Section 7 |
| Advanced version (multigraphs) | INCONSISTENT | Claimed but report text contradicts |

---

## RECOMMENDED FIXES (Priority Order)

1. **Merge duplicate bibliographies** into one section
2. **Fix author name** "P. Riesen" -> "K. Riesen"
3. **Update lerougeBLP citation** to Pattern Recognition 2017
4. **Remove unused bibliography entries** (Bunke1997, Solnon2015) or add citations
5. **Fix Section 6.1 citation** - don't cite lerougeMCSM for "lab task specification"
6. **Fill in Table 6** with actual experiment results OR remove the section
7. **Resolve directed/undirected inconsistency** - update Introduction to say "directed multigraphs"
8. **Fix self-loop claim** - either say "self-loops are supported" or clarify
9. **Reword triangle inequality justification** for clarity
10. **Remove Polish comment** from line 5

---

## VERIFIED MATHEMATICAL CLAIMS

The following mathematical claims were verified and found to be **correct**:

- Graph size definition ||G|| = |V(G)| + |E(G)| with all 4 properties (non-negativity, monotonicity, additivity, complexity relevance)
- GED metric axioms (non-negativity, identity of indiscernibles, symmetry, triangle inequality)
- F2 formulation objective function and constraints (per Lerouge et al.)
- Complexity bound O(P(n_H, n_G) * n_G^2) for exhaustive subgraph isomorphism
- Expected GED values for exp6 (25) and exp7 (19) - arithmetically correct

---

## REFERENCES VERIFICATION

- [Lerouge et al. F2 formulation](https://arxiv.org/abs/1505.05740) - Paper exists, arXiv 2015, full publication Pattern Recognition 2017
- [Lerouge et al. MCSM](https://www.sciencedirect.com/science/article/abs/pii/S0167865515004250) - Paper exists, Pattern Recognition Letters 2016
- Riesen & Bunke 2009 - Author first name is **Kaspar (K.)**, not P.
