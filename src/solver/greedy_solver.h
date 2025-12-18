#ifndef V2_GREEDY_SOLVER_H
#define V2_GREEDY_SOLVER_H

#include "../model/problem.h"
#include "../model/graph.h"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace gempp {

/**
 * Greedy solver for fast approximation of graph matching.
 * Uses a degree-based greedy heuristic to find a feasible matching quickly.
 *
 * For minimal extension (subgraph isomorphism):
 * - Tries to match each pattern vertex to a target vertex
 * - Greedily selects matches that preserve edge structure
 * - Returns an upper bound on the minimal extension
 */
class GreedySolver {
public:
    struct Result {
        double objective;
        std::unordered_map<std::string, double> solution;
        std::vector<int> vertex_matching;  // vertex_matching[i] = k means pattern vertex i -> target vertex k
        std::vector<int> edge_matching;    // edge_matching[ij] = kl means pattern edge ij -> target edge kl
    };

    GreedySolver(Problem* pb) : pb_(pb) {}

    /**
     * Solve using greedy matching.
     * For MCSM (minimal extension), tries to find a matching that minimizes unmatched elements.
     */
    Result solve() {
        Result result;
        result.objective = 0.0;

        Graph* pattern = pb_->getQuery();
        Graph* target = pb_->getTarget();

        int nVP = pattern->getVertexCount();
        int nVT = target->getVertexCount();
        int nEP = pattern->getEdgeCount();
        int nET = target->getEdgeCount();

        // Initialize matchings as unmatched (-1)
        result.vertex_matching.assign(nVP, -1);
        result.edge_matching.assign(nEP, -1);

        // Track which target vertices/edges are used
        std::vector<bool> target_vertex_used(nVT, false);
        std::vector<bool> target_edge_used(nET, false);

        // Build adjacency for target edges: (src,dst) -> edge index
        std::unordered_map<int, std::unordered_map<int, int>> target_adj;
        for (int kl = 0; kl < nET; ++kl) {
            Edge* e = target->getEdge(kl);
            int k = e->getOrigin()->getIndex();
            int l = e->getTarget()->getIndex();
            target_adj[k][l] = kl;
            if (!target->isDirected()) {
                target_adj[l][k] = kl;  // Undirected: both directions
            }
        }

        // Sort pattern vertices by degree (descending) - match high-degree first
        std::vector<int> pattern_order(nVP);
        for (int i = 0; i < nVP; ++i) pattern_order[i] = i;
        std::sort(pattern_order.begin(), pattern_order.end(), [&](int a, int b) {
            return pattern->getVertex(a)->getDegree() > pattern->getVertex(b)->getDegree();
        });

        // Greedy vertex matching
        for (int i : pattern_order) {
            int best_k = -1;
            int best_score = -1000000;  // Use very low value to ensure we always pick a vertex

            // Find the best available target vertex
            for (int k = 0; k < nVT; ++k) {
                if (target_vertex_used[k]) continue;

                // Score: how many already-matched neighbors can be edge-matched
                int score = 0;
                Vertex* pv = pattern->getVertex(i);
                Vertex* tv = target->getVertex(k);

                // Check edges from this pattern vertex
                for (Edge* pe : pv->getEdges(Vertex::EDGE_IN_OUT)) {
                    int j = (pe->getOrigin()->getIndex() == i)
                            ? pe->getTarget()->getIndex()
                            : pe->getOrigin()->getIndex();

                    if (result.vertex_matching[j] >= 0) {
                        // Neighbor j is already matched to some target vertex l
                        int l = result.vertex_matching[j];
                        // Check if there's an edge k-l in target
                        if (target_adj[k].count(l) || target_adj[l].count(k)) {
                            ++score;
                        }
                    }
                }

                // Degree compatibility: prefer similar degrees
                int degree_diff = std::abs(pv->getDegree() - tv->getDegree());
                int adjusted_score = score * 1000 - degree_diff;

                if (adjusted_score > best_score) {
                    best_score = adjusted_score;
                    best_k = k;
                }
            }

            if (best_k >= 0) {
                result.vertex_matching[i] = best_k;
                target_vertex_used[best_k] = true;

                // Record in solution map
                std::string var_id = "x_" + std::to_string(i) + "," + std::to_string(best_k);
                result.solution[var_id] = 1.0;
            }
        }

        // Now match edges based on vertex matching
        for (int ij = 0; ij < nEP; ++ij) {
            Edge* pe = pattern->getEdge(ij);
            int i = pe->getOrigin()->getIndex();
            int j = pe->getTarget()->getIndex();

            int k = result.vertex_matching[i];
            int l = result.vertex_matching[j];

            if (k >= 0 && l >= 0) {
                // Both endpoints are matched - check if corresponding edge exists
                int kl = -1;
                if (target_adj[k].count(l)) {
                    kl = target_adj[k][l];
                } else if (!target->isDirected() && target_adj[l].count(k)) {
                    kl = target_adj[l][k];
                }

                if (kl >= 0 && !target_edge_used[kl]) {
                    result.edge_matching[ij] = kl;
                    target_edge_used[kl] = true;

                    std::string var_id = "y_" + std::to_string(ij) + "," + std::to_string(kl);
                    result.solution[var_id] = 1.0;
                }
            }
        }

        // Calculate objective: number of unmatched pattern elements
        int unmatched_vertices = 0;
        int unmatched_edges = 0;

        for (int i = 0; i < nVP; ++i) {
            if (result.vertex_matching[i] < 0) {
                ++unmatched_vertices;
            }
        }

        for (int ij = 0; ij < nEP; ++ij) {
            if (result.edge_matching[ij] < 0) {
                ++unmatched_edges;
            }
        }

        result.objective = unmatched_vertices + unmatched_edges;

        return result;
    }

private:
    Problem* pb_;
};

} // namespace gempp

#endif // V2_GREEDY_SOLVER_H
