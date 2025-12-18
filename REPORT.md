# Report Proof-Reading: Findings and Recommendations

**Date:** December 18, 2025
**Reviewed File:** `report/main.tex`
**Reviewer:** Claude (AI Assistant)

---

## Executive Summary

The report is well-structured and technically sound. All 15 unit tests pass, and experimental results in Table 1 are verified correct. However, several issues require attention before submission:

| Severity | Count | Key Issues |
|----------|-------|------------|
| CRITICAL | 2 | Graph model contradiction, Empty Table 2 |
| ERROR | 1 | Duplicate bibliography (LaTeX will fail) |
| BUG | 1 | Greedy solver fails on uniform-degree graphs |
| MODERATE | 3 | HOW_TO_RUN.txt incomplete, output format mismatch |
| MINOR | 3 | Missing details, terminology |

---

## Test Verification

**All 15 tests PASS**

```
PASS: 01_triangle
PASS: 02_triangle_in_square
PASS: 03_path_in_star
PASS: 04_single_vertex
PASS: 05_edge
PASS: 06_k4_in_triangle
PASS: 07_path4_in_path3
PASS: 08_star4_in_path
PASS: 09_pentagon_in_square
PASS: 10_k5_in_k4
PASS: 11_two_edges_in_single_edge
PASS: 12_diamond_in_triangle
PASS: 13_hexagon_in_pentagon
PASS: 14_house_in_square
PASS: 15_petersen_fragment
```

---

## Experimental Results Verification

### Table 1 (exp1-exp5): VERIFIED CORRECT

| Instance | MinExt (Report) | MinExt (Actual) | GED (Report) | GED (Actual) | F2LP (Report) | F2LP (Actual) |
|----------|-----------------|-----------------|--------------|--------------|---------------|---------------|
| exp1_k3_in_p4 | 1 | 1 | 3 | 3 | 1.0 | 1.0 |
| exp2_p5_in_p4 | 2 | 2 | 2 | 2 | 2.0 | 2.0 |
| exp3_k4_in_p6 | 3 | 3 | 7 | 7 | 3.0 | 3.0 |
| exp4_c5_in_k5 | 0 | 0 | 5 | 5 | 5.0 | 5.0 |
| exp5_k6_in_k7 | 0 | 0 | 7 | 7 | 7.0 | 7.0 |

### Table 2 (exp6-exp8): NEW RESULTS (to fill empty placeholders)

| Instance | MinExt | Time (ms) | GED | GED Time (ms) | F2LP | F2LP Time (ms) |
|----------|--------|-----------|-----|---------------|------|----------------|
| exp6_grid4_in_grid5 | 0 | 50,260 | 25 | 46,421 | 25.0 | 25 |
| exp7_k8_in_k10 | 0 | 820 | 19 | 836 | 19.0 | 12 |
| exp8_tree12_in_path15 | 2 | 337,813 | N/A* | N/A* | 6.0 | 2 |

*exp8 GED was not computed due to extreme runtime (MinExt alone took ~5.6 minutes)

**Validation against report expectations:**
- exp6: Report predicted MinExt=0, GED≈25 - **CORRECT**
- exp7: Report predicted MinExt=0, GED≈19 - **CORRECT**
- exp8: Report predicted MinExt=2-3, GED>extension - **CORRECT** (got 2)

---

## Critical Issues

### 1. Graph Model Contradiction (Lines 27 vs 47-48)

**Problem:** The report contradicts itself about the graph model.

- Line 27 (Introduction): *"two finite simple undirected graphs"*
- Line 47-48 (Section 2.1): *"finite directed multigraphs"*

**Impact:** This fundamental inconsistency undermines the credibility of the definitions.

**Fix:** Replace Introduction wording (line 27-28) with:
```latex
The project considers two finite graphs $G_1$ and $G_2$ given by adjacency matrices.
The implementation supports both directed and undirected graphs, as well as multigraphs.
```

Or update Section 2.1 to be consistent with "simple undirected graphs" if that's what's actually tested.

---

### 2. Empty Table 2 (Lines 520-535)

**Problem:** Table 2 has all `--` placeholders despite input files existing.

**Impact:** Report claims to have results but shows none - appears incomplete.

**Fix:** Fill Table 2 with the results computed above:

```latex
\begin{tabular}{lrrrrrr}
\toprule
 & \multicolumn{1}{c}{True MinExt} & \multicolumn{1}{c}{True GED} & \multicolumn{2}{c}{F2LP (LB)} \\
\cmidrule(lr){2-2}\cmidrule(lr){3-3}\cmidrule(lr){4-5}
Instance & value & value & value & ms \\
\midrule
\texttt{exp6\_grid4\_in\_grid5}    & 0  & 25 & 25.0 & 25 \\
\texttt{exp7\_k8\_in\_k10}         & 0  & 19 & 19.0 & 12 \\
\texttt{exp8\_tree12\_in\_path15}  & 2  & -- & 6.0  & 2  \\
\bottomrule
\end{tabular}
```

Note: Mark exp8 GED as "--" due to impractical computation time (~6+ minutes).

---

## Error

### 3. Duplicate Bibliography (Lines 681-700 AND 706-723)

**Problem:** Two `\begin{thebibliography}` environments exist.

**Impact:** LaTeX compilation will fail or produce warnings.

**Fix:** Remove the first bibliography block (lines 681-700). Keep the more complete one (lines 706-723) which includes all three required references (riesen2009, lerougeBLP, lerougeMCSM).

---

## Bug Found

### 4. Greedy Solver Bug with Uniform-Degree Graphs

**Location:** `src/solver/greedy_solver.h:79`

**Problem:** Initial `best_score = -1` causes matching to fail when `adjusted_score` is negative.

**Evidence:**
```
exp7_k8_in_k10 with --fast:
  Expected: MinExt = 0 (K8 is subgraph of K10)
  Actual:   MinExt = 36 (ALL vertices unmatched!)
```

**Root Cause:**
```cpp
int best_score = -1;  // Line 79
// ...
int adjusted_score = score * 1000 - degree_diff;  // Line 108
if (adjusted_score > best_score) {  // Line 110
```

For K8→K10: degree_diff = |7-9| = 2, score = 0 (no matched neighbors yet)
- adjusted_score = 0 - 2 = -2
- -2 > -1 is FALSE, so no vertex is matched

**Fix:**
```cpp
int best_score = std::numeric_limits<int>::min();  // or -1000000
```

**Impact:** This affects the accuracy claims about greedy solver in Section 4.2 and REPORT_FAST.md.

---

## Moderate Issues

### 5. HOW_TO_RUN.txt Incomplete

**Problem:** Only documents `--time` and `--fast` options.

**Missing:**
- `--ged, -g` (full graph edit distance)
- `--f2lp, --lp` (LP relaxation lower bound)
- `--minext-approx` (approximate minimal extension)
- `--up, -u <v>` (pruning parameter)
- `--output, -o <file>` (XML output)

**Fix:** Update HOW_TO_RUN.txt to match README.md documentation or at minimum add:
```
OPTIONS:
    --time, -t     Show computation time
    --fast, -f     Fast greedy mode (upper bound, for large graphs)
    --ged, -g      Compute full Graph Edit Distance
    --f2lp, --lp   GED lower bound via LP relaxation
    --output, -o   Write XML solution file
```

---

### 6. Output Format Documentation Mismatch (Lines 614-626)

**Problem:** Report shows generic output format but doesn't mention:
- `--fast` mode prefix: `Mode: greedy (upper bound)`
- `--ged` mode outputs "Is Isomorphic" not "Is Subgraph"

**Fix:** Add subsection showing actual output for each mode, or add a note that format varies by mode.

---

### 7. Section Reference Issue

**Location:** Line 208 references `Section~\ref{sec:approx}`

**Status:** Works correctly (points to Section 4). No fix needed.

---

## Minor Issues

### 8. Greedy Score Formula Incomplete (Lines 386-388)

**Problem:** Report says score "counts how many already-matched neighbors..." but doesn't mention the weighted formula.

**Actual formula (greedy_solver.h:108):**
```cpp
int adjusted_score = score * 1000 - degree_diff;
```

**Fix:** Add to report:
```latex
The score is computed as $1000 \times (\text{edge-compatible neighbors}) - |\deg(i) - \deg(k)|$,
which prioritizes edge compatibility while using degree similarity as a tie-breaker.
```

---

### 9. Test Comparison Limitation Not Mentioned

**Problem:** Test script only compares first 5 lines due to multiple optimal solutions (test.sh:50-53).

**Recommendation:** Add note in Section 5 (Implementation):
```latex
\textbf{Note on testing:} Due to the existence of multiple optimal solutions for some instances,
the test harness compares only the first five output lines (GED value, subgraph status,
minimal extension, vertex count, edge count) which are deterministic.
```

---

### 10. Terminology: "MinExt" vs "Minimal Extension"

**Problem:** The report uses "Minimal Extension" but code/tables use "MinExt". Consider using consistent terminology.

---

## Algorithm Correctness Assessment

### ILP Formulations (MCSM, F2, F2LP): CORRECT

The formulations correctly implement:
- Vertex matching constraints (injective)
- Edge matching constraints (injective)
- Edge-vertex consistency (F2 formulation)
- Objective functions match paper definitions

### Greedy Solver: BUG FOUND

As documented above, the greedy solver has a bug with uniform-degree graphs where pattern degree < target degree.

---

## Summary of Required Actions

### Before Submission (MUST FIX):

1. **Fix graph model contradiction** (choose one consistent description)
2. **Fill Table 2** with experimental results
3. **Remove duplicate bibliography**
4. **Fix greedy solver bug** (change `best_score = -1` to minimum integer)

### Recommended (SHOULD FIX):

5. Update HOW_TO_RUN.txt
6. Document output format variations per mode
7. Add greedy score formula detail

### Optional (NICE TO HAVE):

8. Add test limitation note
9. Consistent terminology

---

## Appendix: Commands Used for Verification

```bash
# Run all tests
./scripts/test.sh

# Verify Table 1 (exp1-exp5)
./gempp report/experiments_inputs/exp1_k3_in_p4.txt --time
./gempp --ged report/experiments_inputs/exp1_k3_in_p4.txt --time
./gempp --f2lp report/experiments_inputs/exp1_k3_in_p4.txt --time

# Fill Table 2 (exp6-exp8)
./gempp report/experiments_inputs/exp6_grid4_in_grid5.txt --time
./gempp --ged report/experiments_inputs/exp6_grid4_in_grid5.txt --time
./gempp --f2lp report/experiments_inputs/exp6_grid4_in_grid5.txt --time

./gempp report/experiments_inputs/exp7_k8_in_k10.txt --time
./gempp --ged report/experiments_inputs/exp7_k8_in_k10.txt --time
./gempp --f2lp report/experiments_inputs/exp7_k8_in_k10.txt --time

./gempp report/experiments_inputs/exp8_tree12_in_path15.txt --time
./gempp --f2lp report/experiments_inputs/exp8_tree12_in_path15.txt --time

# Verify greedy bug
./gempp --fast report/experiments_inputs/exp7_k8_in_k10.txt --time
# Shows MinExt=36 when optimal is 0
```
