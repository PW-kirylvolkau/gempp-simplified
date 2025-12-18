# Implementation Notes

## Graph Support

The implementation technically supports:
- **Directed graphs**: Adjacency matrix entries are treated as directed edges (i→j)
- **Undirected graphs**: Symmetric adjacency matrices are interpreted as undirected edges
- **Multigraphs**: Matrix entries > 1 represent multiple parallel edges

However, the **report and experiments focus exclusively on simple undirected graphs** (0/1 adjacency matrices, symmetric).

### Why the broader support exists

The underlying ILP formulations (F2, MCSM) and GLPK solver can handle integer edge multiplicities and directed edges without modification. The adjacency matrix parser reads integer values, so multigraph support comes "for free."

### Tested configurations

Only simple undirected graphs have been thoroughly tested:
- All 15 unit tests use simple undirected graphs
- All benchmark instances use simple undirected graphs
- The report's experimental evaluation uses simple undirected graphs

### Recommendations

If you need directed or multigraph support:
1. Verify correctness with additional test cases
2. Check that edge consistency constraints in the ILP handle directedness correctly
3. Consider whether the greedy heuristic needs adjustment for directed graphs
