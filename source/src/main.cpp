#include "model/adjacency_parser.h"
#include "model/problem.h"
#include "formulation/mcsm.h"
#include "formulation/linear_ged.h"
#include "solver/glpk_solver.h"
#include "solver/greedy_solver.h"
#include "visualization/graph_canvas.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <chrono>
#include <string>
#include <set>

using namespace gempp;

static void writeSolutionXML(const std::string& filename,
                             Problem* problem,
                             const std::unordered_map<std::string, double>& solution,
                             double objective,
                             bool is_ged)
{
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: cannot open solution file '" << filename << "'" << std::endl;
        return;
    }

    int nVP = problem->getQuery()->getVertexCount();
    int nVT = problem->getTarget()->getVertexCount();
    int nEP = problem->getQuery()->getEdgeCount();
    int nET = problem->getTarget()->getEdgeCount();

    std::vector<int> matched_pattern_vertices(nVP, -1);
    std::vector<int> matched_target_vertices(nVT, -1);
    std::vector<int> matched_pattern_edges(nEP, -1);
    std::vector<int> matched_target_edges(nET, -1);

    auto is_active = [](double v) { return v >= 0.5; };

    for (const auto& kv : solution) {
        if (!is_active(kv.second)) continue;
        const std::string& id = kv.first;
        if (id.rfind("x_", 0) == 0) {
            auto comma = id.find(',');
            int i = std::stoi(id.substr(2, comma - 2));
            int k = std::stoi(id.substr(comma + 1));
            matched_pattern_vertices[i] = k;
            matched_target_vertices[k] = i;
        } else if (id.rfind("y_", 0) == 0) {
            auto comma = id.find(',');
            int ij = std::stoi(id.substr(2, comma - 2));
            int kl = std::stoi(id.substr(comma + 1));
            matched_pattern_edges[ij] = kl;
            matched_target_edges[kl] = ij;
        }
    }

    auto safeCost = [](double v) { return std::isfinite(v) ? v : 0.0; };

    out << "<?xml version=\"1.0\"?>\n";
    out << "<solution>\n";
    out << "  <objective status=\"" << (std::isinf(objective) ? "infeasible" : "optimal")
        << "\" value=\"" << (std::isinf(objective) ? "inf" : std::to_string(objective)) << "\"/>\n";

    // Nodes section
    out << "  <nodes>\n";
    for (int i = 0; i < nVP; ++i) {
        int k = matched_pattern_vertices[i];
        if (k >= 0) {
            double cost = safeCost(problem->getCost(true, i, k));
            out << "    <substitution cost=\"" << cost << "\">\n";
            out << "      <node type=\"query\" index=\"" << i << "\"/>\n";
            out << "      <node type=\"target\" index=\"" << k << "\"/>\n";
            out << "    </substitution>\n";
        }
    }
    for (int i = 0; i < nVP; ++i) {
        if (matched_pattern_vertices[i] < 0) {
            out << "    <insertion cost=\"1\">\n";
            out << "      <node type=\"query\" index=\"" << i << "\"/>\n";
            out << "    </insertion>\n";
        }
    }
    if (is_ged) {
        for (int k = 0; k < nVT; ++k) {
            if (matched_target_vertices[k] < 0) {
                out << "    <deletion cost=\"1\">\n";
                out << "      <node type=\"target\" index=\"" << k << "\"/>\n";
                out << "    </deletion>\n";
            }
        }
    }
    out << "  </nodes>\n";

    // Edges section
    out << "  <edges>\n";
    for (int ij = 0; ij < nEP; ++ij) {
        int kl = matched_pattern_edges[ij];
        if (kl >= 0) {
            double cost = safeCost(problem->getCost(false, ij, kl));
            Edge* qe = problem->getQuery()->getEdge(ij);
            Edge* te = problem->getTarget()->getEdge(kl);
            out << "    <substitution cost=\"" << cost << "\">\n";
            out << "      <edge type=\"query\" from=\"" << qe->getOrigin()->getIndex()
                << "\" to=\"" << qe->getTarget()->getIndex() << "\"/>\n";
            out << "      <edge type=\"target\" from=\"" << te->getOrigin()->getIndex()
                << "\" to=\"" << te->getTarget()->getIndex() << "\"/>\n";
            out << "    </substitution>\n";
        }
    }
    for (int ij = 0; ij < nEP; ++ij) {
        if (matched_pattern_edges[ij] < 0) {
            Edge* qe = problem->getQuery()->getEdge(ij);
            out << "    <insertion cost=\"1\">\n";
            out << "      <edge type=\"query\" from=\"" << qe->getOrigin()->getIndex()
                << "\" to=\"" << qe->getTarget()->getIndex() << "\"/>\n";
            out << "    </insertion>\n";
        }
    }
    if (is_ged) {
        for (int kl = 0; kl < nET; ++kl) {
            if (matched_target_edges[kl] < 0) {
                Edge* te = problem->getTarget()->getEdge(kl);
                out << "    <deletion cost=\"1\">\n";
                out << "      <edge type=\"target\" from=\"" << te->getOrigin()->getIndex()
                    << "\" to=\"" << te->getTarget()->getIndex() << "\"/>\n";
                out << "    </deletion>\n";
            }
        }
    }
    out << "  </edges>\n";
    out << "</solution>\n";
}

int main(int argc, char* argv[]) {
    try {
        bool show_time = false;
        bool use_ged = false;
        bool use_f2lp = false;
        bool approx_minext = false;
        bool first_feasible = false;
        double upper_bound = 1.0;
        std::string output_file;
        std::string input_file;

        // Parse arguments
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--time" || arg == "-t") {
                show_time = true;
            } else if (arg == "--ged" || arg == "-g") {
                use_ged = true;
            } else if (arg == "--f2lp" || arg == "--lp") {
                use_ged = true;
                use_f2lp = true;
            } else if (arg == "--minext-approx" || arg == "--approx-minext") {
                // Approximate minimal extension: GED F2LP with huge deletion penalty
                use_ged = true;
                use_f2lp = true;
                approx_minext = true;
            } else if (arg == "--fast" || arg == "-f") {
                // Stop at first feasible solution (not optimal)
                first_feasible = true;
            } else if (arg == "--up" || arg == "-u") {
                if (i + 1 >= argc) {
                    std::cerr << "Error: missing value after '" << arg << "'" << std::endl;
                    return 1;
                }
                try {
                    upper_bound = std::stod(argv[++i]);
                } catch (const std::exception&) {
                    std::cerr << "Error: invalid upper bound value '" << argv[i] << "'" << std::endl;
                    return 1;
                }
                if (upper_bound <= 0.0 || upper_bound > 1.0) {
                    std::cerr << "Error: upper bound must be in (0,1]" << std::endl;
                    return 1;
                }
            } else if (arg == "--output" || arg == "-o") {
                if (i + 1 >= argc) {
                    std::cerr << "Error: missing value after '" << arg << "'" << std::endl;
                    return 1;
                }
                output_file = argv[++i];
            } else if (input_file.empty()) {
                input_file = arg;
            } else {
                std::cerr << "Error: unexpected argument '" << arg << "'" << std::endl;
                return 1;
            }
        }

        if (input_file.empty()) {
            std::cerr << "Usage: " << argv[0] << " [options] <input_file.txt>" << std::endl;
            std::cerr << std::endl;
            std::cerr << "Input format: text file with two graphs (pattern and target)" << std::endl;
            std::cerr << "  First graph (pattern):" << std::endl;
            std::cerr << "    Line 1: number of vertices" << std::endl;
            std::cerr << "    Following lines: adjacency matrix (non-negative integers)" << std::endl;
            std::cerr << "  Second graph (target):" << std::endl;
            std::cerr << "    Line 1: number of vertices" << std::endl;
            std::cerr << "    Following lines: adjacency matrix (non-negative integers)" << std::endl;
            std::cerr << std::endl;
            std::cerr << "Options:" << std::endl;
            std::cerr << "  --time, -t    Show computation time in milliseconds" << std::endl;
            std::cerr << "  --fast, -f    Use greedy heuristic (fast upper bound)" << std::endl;
            std::cerr << "  --ged,  -g    Compute graph edit distance (penalizes both sides)" << std::endl;
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
            // GED formulation
            LinearGraphEditDistance formulation(&problem);
            if (approx_minext) {
                constexpr double HIGH_DELETION_COST = 1e6;  // Large penalty to discourage deletions
                formulation.setEditCosts(
                    /*vertex_insertion=*/1.0,
                    /*vertex_deletion=*/HIGH_DELETION_COST,
                    /*edge_insertion=*/1.0,
                    /*edge_deletion=*/HIGH_DELETION_COST);
            }
            formulation.init(upper_bound, use_f2lp);

            GLPKSolver solver;
            solver.init(formulation.getLinearProgram(), false, use_f2lp, first_feasible);

            std::unordered_map<std::string, double> solution;
            double objective = solver.solve(solution);

            // End timing
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            int ged_value = std::isinf(objective) ? -1 : static_cast<int>(std::round(objective));
            bool is_isomorphic = (use_f2lp ? (std::abs(objective) < 1e-6) : (ged_value == 0));

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
            if (use_f2lp) {
                if (approx_minext) {
                    std::cout << "GED lower bound (F2LP, high deletion penalty): "
                              << (std::isinf(objective) ? "inf" : std::to_string(objective)) << std::endl;
                } else {
                    std::cout << "GED lower bound (F2LP): "
                              << (std::isinf(objective) ? "inf" : std::to_string(objective)) << std::endl;
                }
            } else {
                std::cout << "GED: " << (std::isinf(objective) ? "inf" : std::to_string(ged_value)) << std::endl;
            }
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

            // Collect and sort edges by (src, dst) preserving direction
            std::vector<std::pair<int, int>> unmatched_pattern_edge_list;
            for (int ij : unmatched_pattern_edges) {
                Edge* e = pattern->getEdge(ij);
                unmatched_pattern_edge_list.push_back({e->getOrigin()->getIndex(), e->getTarget()->getIndex()});
            }
            std::vector<std::pair<int, int>> unmatched_target_edge_list;
            for (int kl : unmatched_target_edges) {
                Edge* e = target->getEdge(kl);
                unmatched_target_edge_list.push_back({e->getOrigin()->getIndex(), e->getTarget()->getIndex()});
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

            if (approx_minext) {
                int approx_extension = static_cast<int>(unmatched_pattern_vertices.size() + unmatched_pattern_edges.size());
                std::cout << "Approx minimal extension (pattern side, count): "
                          << approx_extension << std::endl;
            }

            if (show_time) {
                std::cout << "Time: " << duration.count() << " ms" << std::endl;
            }

            if (!output_file.empty()) {
                writeSolutionXML(output_file, &problem, solution, objective, true);
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

        // Determine vertex matching: vertex_mapping[i] = k means pattern vertex i -> target vertex k
        // -1 means unmatched
        std::vector<int> vertex_mapping(nVP, -1);
        std::vector<int> unmatched_vertices;
        for (int i = 0; i < nVP; ++i) {
            bool matched = false;
            for (int k = 0; k < nVT; ++k) {
                std::string var_id = "x_" + std::to_string(i) + "," + std::to_string(k);
                if (solution.count(var_id) && solution[var_id] >= 0.5) {
                    vertex_mapping[i] = k;
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

        // Compute results
        bool is_subgraph = (objective < 1e-6);
        int minimal_extension = std::isinf(objective) ? -1 : static_cast<int>(std::round(objective));

        // Sort unmatched vertices
        std::sort(unmatched_vertices.begin(), unmatched_vertices.end());

        // Collect and sort edges by (src, dst) preserving direction
        std::vector<std::pair<int, int>> edge_list;
        for (int ij : unmatched_edges) {
            Edge* e = pattern->getEdge(ij);
            int src = e->getOrigin()->getIndex();
            int dst = e->getTarget()->getIndex();
            edge_list.push_back({src, dst});
        }
        std::sort(edge_list.begin(), edge_list.end());

        // Render output
        GraphCanvas::renderMatchingResult(pattern, target,
                                          vertex_mapping, unmatched_vertices, edge_list,
                                          minimal_extension, is_subgraph);

        // Output timing if requested
        if (show_time) {
            std::cout << "Time: " << duration.count() << " ms" << std::endl;
        }

        if (!output_file.empty()) {
            writeSolutionXML(output_file, &problem, solution, objective, false);
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
