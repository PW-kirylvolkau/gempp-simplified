# Solution Notes

## Known Limitations

### Multiple Optimal Solutions

The graph edit distance (GED) problem solved by this tool is formulated as an Integer Linear Program (ILP). For many input graphs, there exist **multiple optimal solutions** with the same minimal cost.

GLPK (GNU Linear Programming Kit), the solver used in this project, returns only **one** optimal solution. The specific solution returned depends on:
- Internal solver heuristics
- Floating-point arithmetic differences across platforms
- Compiler optimizations

As a result, running the same test on different platforms (e.g., macOS vs Windows) may produce different specific vertex/edge mappings, even though the **cost is identical**.

### Test Design

To handle this limitation, the test infrastructure compares only the **deterministic values**:
1. GED (Graph Edit Distance)
2. Is Subgraph (yes/no)
3. Minimal Extension cost
4. Vertices to add (count)
5. Edges to add (count)

The specific vertices and edges listed in the output may vary between platforms but represent equally valid optimal solutions.
