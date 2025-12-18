#include "model/adjacency_parser.h"
#include "model/problem.h"
#include "formulation/mcsm.h"
#include "formulation/linear_ged.h"
#include "solver/glpk_solver.h"
#include "solver/greedy_solver.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
#include <chrono>
#include <string>

using namespace gempp;

int main(int argc, char* argv[]) {
    try {
        bool show_time = false;
        bool use_ged = false;
        bool first_feasible = false;
        std::string input_file;

        // Parse arguments
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--time" || arg == "-t") {
                show_time = true;
            } else if (arg == "--ged" || arg == "-g") {
                use_ged = true;
            } else if (arg == "--fast" || arg == "-f") {
                // Use greedy heuristic (polynomial time upper bound)
                first_feasible = true;
            } else if (input_file.empty()) {
                input_file = arg;
            } else {
                std::cerr << "Error: unexpected argument '" << arg << "'" << std::endl;
                return 1;
            }
        }

        if (input_file.empty()) {
            std::cerr << "Usage: " << argv[0] << " [--time] <input_file.txt>" << std::endl;
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
            std::cerr << "  --ged, -g     Solve full graph edit distance (default: minimal extension)" << std::endl;
            std::cerr << "  --fast, -f    Use greedy heuristic (polynomial time, upper bound)" << std::endl;
            return 1;
        }

        // Start timing
        auto start_time = std::chrono::high_resolution_clock::now();

        // Parse input file containing both graphs
        auto graphs = AdjacencyMatrixParser::parseFile(input_file);
        Graph* pattern = graphs.first;
        Graph* target = graphs.second;

        // Create problem
        Problem::Type problem_type = use_ged ? Problem::GED : Problem::SUBGRAPH;
        Problem problem(problem_type, pattern, target);

        int nVP = pattern->getVertexCount();
        int nVT = target->getVertexCount();
        int nEP = pattern->getEdgeCount();
        int nET = target->getEdgeCount();

        if (use_ged) {
            // GED formulation (exact ILP)
            LinearGraphEditDistance formulation(&problem);
            formulation.init(1.0, false);

            GLPKSolver solver;
            solver.init(formulation.getLinearProgram(), false, false, first_feasible);

            std::unordered_map<std::string, double> solution;
            double objective = solver.solve(solution);

            // End timing
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            int ged_value = std::isinf(objective) ? -1 : static_cast<int>(std::round(objective));
            bool is_isomorphic = (ged_value == 0);

            // Unmatched vertices (pattern and target)
            std::vector<int> unmatched_pattern_vertices;
            std::vector<int> unmatched_target_vertices;
            for (int i = 0; i < nVP; ++i) {
                bool matched = false;
                for (int k = 0; k < nVT; ++k) {
                    std::string var_id = "x_" + std::to_string(i) + "," + std::to_string(k);
                    if (solution.count(var_id) && solution[var_id] >= 0.5) {
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    unmatched_pattern_vertices.push_back(i);
                }
            }
            for (int k = 0; k < nVT; ++k) {
                bool matched = false;
                for (int i = 0; i < nVP; ++i) {
                    std::string var_id = "x_" + std::to_string(i) + "," + std::to_string(k);
                    if (solution.count(var_id) && solution[var_id] >= 0.5) {
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    unmatched_target_vertices.push_back(k);
                }
            }

            // Unmatched edges (pattern and target)
            std::vector<int> unmatched_pattern_edges;
            std::vector<int> unmatched_target_edges;
            for (int ij = 0; ij < nEP; ++ij) {
                bool matched = false;
                for (int kl = 0; kl < nET; ++kl) {
                    std::string var_id = "y_" + std::to_string(ij) + "," + std::to_string(kl);
                    if (solution.count(var_id) && solution[var_id] >= 0.5) {
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    unmatched_pattern_edges.push_back(ij);
                }
            }
            for (int kl = 0; kl < nET; ++kl) {
                bool matched = false;
                for (int ij = 0; ij < nEP; ++ij) {
                    std::string var_id = "y_" + std::to_string(ij) + "," + std::to_string(kl);
                    if (solution.count(var_id) && solution[var_id] == 1) {
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    unmatched_target_edges.push_back(kl);
                }
            }

            // Output GED results
            std::cout << "GED: " << (std::isinf(objective) ? "inf" : std::to_string(ged_value)) << std::endl;
            std::cout << "Is Isomorphic: " << (is_isomorphic ? "yes" : "no") << std::endl;

            // Sorted unmatched elements for deterministic output
            std::sort(unmatched_pattern_vertices.begin(), unmatched_pattern_vertices.end());
            std::sort(unmatched_target_vertices.begin(), unmatched_target_vertices.end());
            std::sort(unmatched_pattern_edges.begin(), unmatched_pattern_edges.end());
            std::sort(unmatched_target_edges.begin(), unmatched_target_edges.end());

            std::cout << "Unmatched pattern vertices:";
            if (unmatched_pattern_vertices.empty()) {
                std::cout << " none";
            } else {
                for (int v : unmatched_pattern_vertices) {
                    std::cout << " " << v;
                }
            }
            std::cout << std::endl;

            std::cout << "Unmatched target vertices:";
            if (unmatched_target_vertices.empty()) {
                std::cout << " none";
            } else {
                for (int v : unmatched_target_vertices) {
                    std::cout << " " << v;
                }
            }
            std::cout << std::endl;

            // Collect and sort edges by (src, dst)
            auto normalize_edge = [](Edge* e) {
                int src = e->getOrigin()->getIndex();
                int dst = e->getTarget()->getIndex();
                if (src > dst) std::swap(src, dst);
                return std::make_pair(src, dst);
            };

            std::vector<std::pair<int, int>> unmatched_pattern_edge_list;
            for (int ij : unmatched_pattern_edges) {
                unmatched_pattern_edge_list.push_back(normalize_edge(pattern->getEdge(ij)));
            }
            std::vector<std::pair<int, int>> unmatched_target_edge_list;
            for (int kl : unmatched_target_edges) {
                unmatched_target_edge_list.push_back(normalize_edge(target->getEdge(kl)));
            }

            std::sort(unmatched_pattern_edge_list.begin(), unmatched_pattern_edge_list.end());
            std::sort(unmatched_target_edge_list.begin(), unmatched_target_edge_list.end());

            std::cout << "Unmatched pattern edges:";
            if (unmatched_pattern_edge_list.empty()) {
                std::cout << " none";
            } else {
                for (const auto& e : unmatched_pattern_edge_list) {
                    std::cout << " (" << e.first << "," << e.second << ")";
                }
            }
            std::cout << std::endl;

            std::cout << "Unmatched target edges:";
            if (unmatched_target_edge_list.empty()) {
                std::cout << " none";
            } else {
                for (const auto& e : unmatched_target_edge_list) {
                    std::cout << " (" << e.first << "," << e.second << ")";
                }
            }
            std::cout << std::endl;

            if (show_time) {
                std::cout << "Time: " << duration.count() << " ms" << std::endl;
            }

            // Cleanup
            delete pattern;
            delete target;
            return 0;
        }

        std::unordered_map<std::string, double> solution;
        double objective;

        if (first_feasible) {
            // Use fast greedy solver for approximation
            GreedySolver greedy(&problem);
            auto result = greedy.solve();
            solution = std::move(result.solution);
            objective = result.objective;
        } else {
            // Create MCSM formulation (allows partial matches)
            MinimumCostSubgraphMatching formulation(&problem, false);
            formulation.init();

            // Solve with GLPK
            GLPKSolver solver;
            solver.init(formulation.getLinearProgram(), false, false, false);

            objective = solver.solve(solution);
        }

        // End timing
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Determine which pattern vertices are unmatched
        std::vector<int> unmatched_vertices;
        for (int i = 0; i < nVP; ++i) {
            bool matched = false;
            for (int k = 0; k < nVT; ++k) {
                std::string var_id = "x_" + std::to_string(i) + "," + std::to_string(k);
                if (solution.count(var_id) && solution[var_id] >= 0.5) {
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
                if (solution.count(var_id) && solution[var_id] >= 0.5) {
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

        if (first_feasible) {
            std::cout << "Mode: greedy (upper bound)" << std::endl;
        }
        std::cout << "GED: " << (std::isinf(objective) ? "inf" : std::to_string(minimal_extension)) << std::endl;
        std::cout << "Is Subgraph: " << (is_subgraph ? "yes" : "no") << std::endl;
        std::cout << "Minimal Extension: " << (std::isinf(objective) ? "inf" : std::to_string(minimal_extension)) << std::endl;

        // Output extension counts (deterministic across platforms)
        std::cout << "Vertices to add: " << unmatched_vertices.size() << std::endl;
        std::cout << "Edges to add: " << unmatched_edges.size() << std::endl;

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
