# GEM++ v2 - Qt-Free Implementation

A clean, modern C++17 implementation of GEM++ (Graph Extraction and Matching) with **NO Qt dependencies**.

## Features

- **Zero Qt dependencies** - Pure C++17 with STL
- **Bundled GLPK** - No external GLPK installation needed
- **Simple CLI** - Just two file paths as input
- **Adjacency matrix format** - Easy text-based input
- **Minimal Extension calculation** - Computes cost of making pattern a subgraph

## Building (macOS)

```bash
cd v2
mkdir build && cd build
cmake ..
make
```

The executable will be: `build/gempp-v2`

## Usage

```bash
./build/gempp-v2 <pattern_file.txt> <target_file.txt>
```

### Input Format

Each file contains an adjacency matrix in plain text:

```
<number_of_vertices>
<row_1_of_adjacency_matrix>
<row_2_of_adjacency_matrix>
...
<row_n_of_adjacency_matrix>
```

**Example** (`pattern.txt` - a triangle):
```
3
0 1 1
1 0 1
1 1 0
```

### Output

The CLI outputs three values:

1. **GED** - Graph Edit Distance (minimal extension cost)
2. **Is Subgraph** - Whether the pattern is an exact subgraph of the target
3. **Minimal Extension** - Number of elements that need to be added to make pattern a subgraph

**Example output:**
```
GED: 0
Is Subgraph: yes
Minimal Extension: 0
```

Or when pattern is NOT an exact subgraph:
```
GED: 1
Is Subgraph: no
Minimal Extension: 1
```

## Project Structure

```
v2/
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
├── external/
│   └── glpk-5.0/            # Bundled GLPK source
└── src/
    ├── main.cpp             # CLI entry point
    ├── core/
    │   ├── types.h          # Basic types and utilities
    │   └── matrix.h         # Matrix template class
    ├── model/
    │   ├── graph.h          # Graph/Vertex/Edge classes
    │   ├── problem.h        # Problem definition
    │   └── adjacency_parser.h  # Parse adjacency matrix files
    ├── integer_programming/
    │   ├── variable.h       # LP variables
    │   └── linear_program.h # Linear program representation
    ├── solver/
    │   └── glpk_solver.h    # GLPK solver integration
    └── formulation/
        └── mcsm.h           # Minimum Cost Subgraph Matching
```

## Algorithm

Uses **Minimum Cost Subgraph Matching (MCSM)** formulation which:
- Allows partial matches (pattern elements can be unmatched)
- Counts unmatched elements as the "minimal extension" cost
- Returns 0 when pattern is an exact subgraph of target
- Returns >0 when some pattern elements cannot be matched

## License

Same as original GEM++ (CeCILL version 2.1)
