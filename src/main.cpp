#include "model/adjacency_parser.h"
#include "model/problem.h"
#include "formulation/mcsm.h"
#include "solver/glpk_solver.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace gempp;

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: " << argv[0] << " <pattern_file.txt> <target_file.txt>" << std::endl;
            std::cerr << std::endl;
            std::cerr << "Input format: adjacency matrix text files" << std::endl;
            std::cerr << "  First line: number of vertices" << std::endl;
            std::cerr << "  Following lines: adjacency matrix (0 or 1)" << std::endl;
            return 1;
        }

        std::string pattern_file = argv[1];
        std::string target_file = argv[2];

        // Read pattern file
        std::ifstream pfile(pattern_file);
        if (!pfile.is_open()) {
            throw Exception("Cannot open file: " + pattern_file);
        }
        std::stringstream pbuffer;
        pbuffer << pfile.rdbuf();
        std::string pdata = pbuffer.str();
        pfile.close();

        // Parse as two copies (workaround for parser expecting 2 graphs)
        std::string pattern_data = pdata + "\n" + pdata;
        auto pattern_graphs = AdjacencyMatrixParser::parseData(pattern_data);
        Graph* pattern = pattern_graphs.first;
        delete pattern_graphs.second;

        // Read target file
        std::ifstream tfile(target_file);
        if (!tfile.is_open()) {
            throw Exception("Cannot open file: " + target_file);
        }
        std::stringstream tbuffer;
        tbuffer << tfile.rdbuf();
        std::string tdata = tbuffer.str();
        tfile.close();

        // Parse as two copies (workaround for parser expecting 2 graphs)
        std::string target_data = tdata + "\n" + tdata;
        auto target_graphs = AdjacencyMatrixParser::parseData(target_data);
        Graph* target = target_graphs.first;
        delete target_graphs.second;

        // Create problem
        Problem problem(Problem::SUBGRAPH, pattern, target);

        int nVP = pattern->getVertexCount();
        int nVT = target->getVertexCount();
        int nEP = pattern->getEdgeCount();
        int nET = target->getEdgeCount();

        // Create MCSM formulation (allows partial matches)
        MinimumCostSubgraphMatching formulation(&problem, false);
        formulation.init();

        // Solve with GLPK
        GLPKSolver solver;
        solver.init(formulation.getLinearProgram(), false);

        std::unordered_map<std::string, int> solution;
        double objective = solver.solve(solution);

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
