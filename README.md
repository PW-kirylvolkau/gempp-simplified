# GEM++ v2 - Minimal Extension Calculator

A C++17 implementation for computing **minimal graph extensions** using Integer Linear Programming.

## Documentation

- [TASK.md](docs/TASK.md) - Laboratory task description
- [MATH.md](docs/MATH.md) - Mathematical foundations (definitions, metrics, proofs)
- [ALG.md](docs/ALG.md) - Algorithm description in pseudocode
- [GEMPP.md](docs/GEMPP.md) - Configuration choices, changes from original GEM++, and known limitations

## Requirements

- **CMake** (version 3.10 or higher)
- **C++ Compiler**:
  - Windows: MSVC (`cl.exe`) from Visual Studio Build Tools
  - macOS/Linux: GCC or Clang

No external libraries required - GLPK is bundled.

## Verification (Windows)

Step-by-step instructions for verifying the solution on a Windows machine:

- [ ] Go to the repository (or receive USB drive with the code)
- [ ] Download/extract the project as ZIP
- [ ] Open **Developer Command Prompt for VS** (or any terminal with `cl.exe` in PATH)
- [ ] Navigate to the project folder: `cd path\to\v2`
- [ ] Run `build.bat` to compile
- [ ] Run `test.bat` to execute all tests
- [ ] Verify: all tests should show `PASS`

## Building

### Windows

```batch
build.bat
```

The executable will be created at `build\Release\gempp-v2.exe` or `build\gempp-v2.exe`.

### macOS / Linux

```bash
mkdir build && cd build
cmake ..
make
```

The executable will be: `build/gempp-v2`

## Usage

```bash
./gempp-v2 <pattern_file.txt> <target_file.txt>
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

### Output Format

```
GED: <value>
Is Subgraph: <yes|no>
Minimal Extension: <value>
Vertices to add: <count>
Edges to add: <count>
Unmatched vertices: <list or "none">
Unmatched edges: <list or "none">
```

**Example** (triangle pattern in square target):
```
GED: 1
Is Subgraph: no
Minimal Extension: 1
Vertices to add: 0
Edges to add: 1
Unmatched vertices: none
Unmatched edges: (0,1)
```

## Running Tests

### Windows
```batch
test.bat
```

### macOS / Linux
```bash
chmod +x test.sh
./test.sh
```

## Project Structure

```
v2/
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
├── build.bat                # Windows build script
├── test.bat                 # Windows test runner
├── test.sh                  # Unix test runner
├── docs/
│   ├── TASK.md              # Task description
│   ├── MATH.md              # Mathematical foundations
│   ├── ALG.md               # Algorithm pseudocode
│   └── GEMPP.md             # Configuration, changes, limitations
├── external/
│   └── glpk-5.0.tar.gz      # Bundled GLPK source
├── tests/
│   ├── 01_triangle/         # Test case: exact match
│   ├── 02_edge_needed/      # Test case: one edge to add
│   └── ...                  # More test cases
└── src/
    ├── main.cpp             # CLI entry point
    ├── core/                # Basic types and utilities
    ├── model/               # Graph data structures
    ├── formulation/         # ILP formulation (MCSM)
    └── solver/              # GLPK solver interface
```

## Algorithm Summary

1. Parse pattern and target graphs from adjacency matrices
2. Build Integer Linear Program (ILP) formulation
3. Solve ILP with GLPK to find optimal vertex/edge matching
4. Compute minimal extension = count of unmatched pattern elements

See [ALG.md](docs/ALG.md) for detailed pseudocode.

## License

CeCILL version 2.1 (same as original GEM++)
