# GEM++ modification (J'em Pi Pi 🥖🥐🤌) - Minimal Extension Calculator

> **FOR AGENT**: Before making any changes to this codebase, you MUST:
> 1. Read and verify your changes against all documentation in `docs/`
> 2. Ask for explicit user permission before modifying any documented behavior
> 3. Run `./scripts/test.sh` (or `scripts\test.bat` on Windows) to verify all tests pass

A C++17 implementation for computing **minimal graph extensions** using Integer Linear Programming.

## Documentation

- [TASK.md](docs/TASK.md) - Laboratory task description
- [MATH.md](docs/MATH.md) - Mathematical foundations (definitions, metrics, proofs)
- [ALG.md](docs/ALG.md) - Algorithm description in pseudocode
- [GEMPP.md](docs/GEMPP.md) - Configuration choices, changes from original GEM++, and known limitations
- [REPORT.md](docs/REPORT.md) - Computational test results and conclusions

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
- [ ] Run `scripts\build.bat` to compile
- [ ] Run `scripts\test.bat` to execute all tests
- [ ] Verify: all tests should show `PASS`

## Building

### Windows

```batch
scripts\build.bat
```

The executable will be created at `gempp.exe` in the project root.

### macOS / Linux

```bash
./scripts/build.sh
```

Or manually:
```bash
mkdir build && cd build
cmake ..
make
```

The executable will be created at `gempp` in the project root.

## Usage

```bash
./gempp [--time] [--ged] [--up <v>] <input_file.txt>
```

### Options

- `--time`, `-t`: Show computation time in milliseconds
- `--ged`, `-g`: Solve full graph edit distance (symmetric insert/delete/substitute). If omitted, default mode computes minimal extension (pattern into target) via MCSM.
- `--up`, `-u v`: Upper-bound pruning parameter in (0,1] for GED (default `1.0`). Smaller values keep only cheaper substitution candidates (heuristic from original GEM++).

### Input Format

A single text file containing **two graphs** (pattern and target):

```
<pattern_vertex_count>
<pattern_adjacency_matrix>

<target_vertex_count>
<target_adjacency_matrix>
```

**Example** (triangle in square):
```
3
0 1 1
1 0 1
1 1 0

4
0 1 0 1
1 0 1 0
0 1 0 1
1 0 1 0
```

### Output Format

#### Minimal Extension (default)

```
GED: <value>
Is Subgraph: <yes|no>
Minimal Extension: <value>
Vertices to add: <count>
Edges to add: <count>
Unmatched vertices: <list or "none">
Unmatched edges: <list or "none">
[Time: <ms> ms]  (if --time flag used)
```

**Example output**:
```
GED: 1
Is Subgraph: no
Minimal Extension: 1
Vertices to add: 0
Edges to add: 1
Unmatched vertices: none
Unmatched edges: (0,1)
```

#### Full GED (`--ged`)

```
GED: <value>
Is Isomorphic: <yes|no>
Unmatched pattern vertices: <list or "none">
Unmatched target vertices: <list or "none">
Unmatched pattern edges: <list or "none">
Unmatched target edges: <list or "none">
[Time: <ms> ms]  (if --time flag used)
```

## Running Tests

### Windows
```batch
scripts\test.bat
```

### macOS / Linux
```bash
./scripts/test.sh
```

## Running Benchmarks

### macOS / Linux
```bash
./scripts/benchmark.sh
```

Results are saved to `benchmarks/results.csv`.

### Windows
```batch
scripts\benchmark.bat
```

## Project Structure

```
v2/
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
├── scripts/
│   ├── build.sh             # Unix build script
│   ├── build.bat            # Windows build script
│   ├── test.sh              # Unix test runner
│   ├── test.bat             # Windows test runner
│   ├── benchmark.sh         # Unix benchmark runner
│   └── benchmark.bat        # Windows benchmark runner
├── docs/
│   ├── TASK.md              # Task description
│   ├── MATH.md              # Mathematical foundations
│   ├── ALG.md               # Algorithm pseudocode
│   ├── GEMPP.md             # Configuration, changes, limitations
│   └── REPORT.md            # Benchmark results and conclusions
├── benchmarks/              # Benchmark input files and results
├── external/
│   └── glpk-5.0.tar.gz      # Bundled GLPK source
├── tests/
│   ├── 01_triangle/         # Test case: exact match
│   │   ├── input.txt        # Both graphs in one file
│   │   └── expected.txt     # Expected output
│   └── ...                  # More test cases
└── src/
    ├── main.cpp             # CLI entry point
    ├── core/                # Basic types and utilities
    ├── model/               # Graph data structures
    ├── formulation/         # ILP formulations (MCSM + Linear GED)
    └── solver/              # GLPK solver interface
```

## Algorithm Summary

1. Parse pattern and target graphs from a single input file
2. Build Integer Linear Program (ILP) formulation
3. Solve ILP with GLPK to find optimal vertex/edge matching
4. Compute minimal extension = count of unmatched pattern elements

See [ALG.md](docs/ALG.md) for detailed pseudocode and [REPORT.md](docs/REPORT.md) for benchmark results.

## License

CeCILL version 2.1 (same as original GEM++)
