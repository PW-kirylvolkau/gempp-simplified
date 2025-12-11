#include "model/adjacency_parser.h"
#include "model/problem.h"
#include "formulation/mcsm.h"
#include "formulation/stsm.h"
#include "solver/glpk_solver.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <chrono>
#include <memory>

using namespace gempp;

int main(int argc, char* argv[]) {
    try {
        bool show_time = false;
        std::string input_file;
        bool use_stsm = false;
        bool use_exact = false;
        double upperbound = 1.0;
        double requested_upperbound = 1.0;
        bool upperbound_explicit = false;

        // Parse arguments
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--time" || arg == "-t") {
                show_time = true;
            } else if (arg == "--approx-stsm" || arg == "--stsm") {
                use_stsm = true;
            } else if (arg == "--exact" || arg == "-e") {
                use_exact = true;
            } else if (arg == "--upperbound" || arg == "-u") {
                if (i + 1 >= argc) {
                    std::cerr << "Error: --upperbound requires a value in (0,1]" << std::endl;
                    return 1;
                }
                try {
                    requested_upperbound = std::stod(argv[++i]);
                    upperbound = requested_upperbound;
                    upperbound_explicit = true;
                } catch (const std::exception&) {
                    std::cerr << "Error: invalid upperbound value '" << argv[i] << "'" << std::endl;
                    return 1;
                }
            } else if (input_file.empty()) {
                input_file = arg;
            } else {
                std::cerr << "Error: unexpected argument '" << arg << "'" << std::endl;
                return 1;
            }
        }

        if (use_stsm && use_exact) {
            std::cerr << "Error: --exact cannot be combined with --approx-stsm" << std::endl;
            return 1;
        }

        if (upperbound_explicit && (requested_upperbound <= 0.0 || requested_upperbound > 1.0)) {
            std::cerr << "Error: upperbound must be in (0,1]" << std::endl;
            return 1;
        }

        if (use_exact && upperbound_explicit && requested_upperbound < 1.0 - 1e-12) {
            std::cerr << "Warning: --upperbound ignored in --exact mode (using 1.0)" << std::endl;
        }
        if (use_exact) {
            upperbound = 1.0;
        } else {
            upperbound = requested_upperbound;
        }

        if (input_file.empty()) {
            std::cerr << "Usage: " << argv[0] << " [--exact|--approx-stsm] [--upperbound <0-1>] [--time] <input_file.txt>" << std::endl;
            std::cerr << std::endl;
            std::cerr << "Input format: text file with two graphs (pattern and target)" << std::endl;
            std::cerr << "  First graph (pattern):" << std::endl;
            std::cerr << "    Line 1: number of vertices" << std::endl;
            std::cerr << "    Following lines: adjacency matrix (0 or 1)" << std::endl;
            std::cerr << "  Second graph (target):" << std::endl;
            std::cerr << "    Line 1: number of vertices" << std::endl;
            std::cerr << "    Following lines: adjacency matrix (0 or 1)" << std::endl;
            std::cerr << std::endl;
            std::cerr << "Options:" << std::endl;
            std::cerr << "  --time, -t    Show computation time in milliseconds" << std::endl;
            std::cerr << "  --exact, -e   Force exact MCSM (disable pruning)" << std::endl;
            std::cerr << "  --approx-stsm Use Substitution-Tolerant Subgraph Matching (approx)" << std::endl;
            std::cerr << "  --upperbound, -u <val>  Upper-bound pruning (0<val<=1)" << std::endl;
            return 1;
        }

        if (upperbound <= 0.0 || upperbound > 1.0) {
            std::cerr << "Error: upperbound must be in (0,1]" << std::endl;
            return 1;
        }

        // Start timing
        auto start_time = std::chrono::high_resolution_clock::now();

        // Parse input file containing both graphs
        auto graphs = AdjacencyMatrixParser::parseFile(input_file);
        Graph* pattern = graphs.first;
        Graph* target = graphs.second;

        // Create problem
        Problem problem(Problem::SUBGRAPH, pattern, target);

        int nVP = pattern->getVertexCount();
        int nVT = target->getVertexCount();
        int nEP = pattern->getEdgeCount();
        int nET = target->getEdgeCount();

        // Choose formulation
        LinearProgram* lp = nullptr;
        std::unique_ptr<SubstitutionTolerantSubgraphMatching> stsm_formulation;
        std::unique_ptr<MinimumCostSubgraphMatching> mcsm_formulation;

        if (use_stsm) {
            stsm_formulation = std::make_unique<SubstitutionTolerantSubgraphMatching>(&problem, false);
            stsm_formulation->init(upperbound);
            lp = stsm_formulation->getLinearProgram();
        } else {
            // Default: Minimum Cost Subgraph Matching (allows partial matches)
            mcsm_formulation = std::make_unique<MinimumCostSubgraphMatching>(&problem, false);
            mcsm_formulation->init(upperbound);
            lp = mcsm_formulation->getLinearProgram();
        }

        // Solve with GLPK
        GLPKSolver solver;
        solver.init(lp, false);

        std::unordered_map<std::string, int> solution;
        double objective = solver.solve(solution);

        // End timing
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Determine which pattern vertices are unmatched
        std::vector<int> unmatched_vertices;
        for (int i = 0; i < nVP; ++i) {
            bool matched = false;
            for (int k = 0; k < nVT; ++k) {
                std::string var_id = "x_" + std::to_string(i) + "," + std::to_string(k);
                if (solution.count(var_id) && solution[var_id] == 1) {
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                unmatched_vertices.push_back(i);
            }
        }

        // Determine which pattern edges are unmatched
        std::vector<int> unmatched_edges;
        for (int ij = 0; ij < nEP; ++ij) {
            bool matched = false;
            for (int kl = 0; kl < nET; ++kl) {
                std::string var_id = "y_" + std::to_string(ij) + "," + std::to_string(kl);
                if (solution.count(var_id) && solution[var_id] == 1) {
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                unmatched_edges.push_back(ij);
            }
        }

        // Output results
        bool is_subgraph = (objective < 1e-6);
        int minimal_extension = std::isinf(objective) ? -1 : static_cast<int>(std::round(objective));

        std::cout << "GED: " << (std::isinf(objective) ? "inf" : std::to_string(minimal_extension)) << std::endl;
        std::cout << "Is Subgraph: " << (is_subgraph ? "yes" : "no") << std::endl;
        std::cout << "Minimal Extension: " << (std::isinf(objective) ? "inf" : std::to_string(minimal_extension)) << std::endl;
        std::cout << "Vertices to add: " << unmatched_vertices.size() << std::endl;
        std::cout << "Edges to add: " << unmatched_edges.size() << std::endl;

        std::string mode_desc;
        if (use_stsm) {
            mode_desc = (upperbound < 1.0) ? "STSM (approximation)" : "STSM (exact)";
        } else {
            mode_desc = (upperbound < 1.0) ? "MCSM (approximation)" : "MCSM (exact)";
        }
        std::cout << "Mode: " << mode_desc << std::endl;
        if (use_stsm || upperbound < 1.0) {
            std::cout << "Upperbound: " << upperbound << std::endl;
        }
        std::cout << "Objective Cost: " << (std::isinf(objective) ? "inf" : std::to_string(objective)) << std::endl;

        // Output detailed extension info (sorted for consistency, but may vary across platforms)
        std::sort(unmatched_vertices.begin(), unmatched_vertices.end());
        std::cout << "Unmatched vertices:";
        if (unmatched_vertices.empty()) {
            std::cout << " none";
        } else {
            for (int v : unmatched_vertices) {
                std::cout << " " << v;
            }
        }
        std::cout << std::endl;

        // Collect and sort edges by (src, dst)
        std::vector<std::pair<int, int>> edge_list;
        for (int ij : unmatched_edges) {
            Edge* e = pattern->getEdge(ij);
            int src = e->getOrigin()->getIndex();
            int dst = e->getTarget()->getIndex();
            if (src > dst) std::swap(src, dst);  // Normalize undirected edge
            edge_list.push_back({src, dst});
        }
        std::sort(edge_list.begin(), edge_list.end());

        std::cout << "Unmatched edges:";
        if (edge_list.empty()) {
            std::cout << " none";
        } else {
            for (const auto& e : edge_list) {
                std::cout << " (" << e.first << "," << e.second << ")";
            }
        }
        std::cout << std::endl;

        // Output timing if requested
        if (show_time) {
            std::cout << "Time: " << duration.count() << " ms" << std::endl;
        }

        // Cleanup
        delete pattern;
        delete target;

        return 0;

    } catch (const Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
}
