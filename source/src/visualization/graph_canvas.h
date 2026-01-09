#ifndef GRAPH_CANVAS_H
#define GRAPH_CANVAS_H

#include "../model/graph.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/color.hpp>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <sstream>

namespace gempp {

class GraphCanvas {
public:
    /**
     * Render the matching result.
     * @param pattern The pattern graph
     * @param target The target graph
     * @param vertexMapping vertexMapping[i] = k means pattern vertex i maps to target vertex k (-1 if unmatched)
     * @param unmatchedPatternVertices List of pattern vertex indices that are unmatched
     * @param unmatchedPatternEdges List of pattern edges (src,dst) that are unmatched
     * @param ged The graph edit distance / minimal extension cost
     * @param isSubgraph True if pattern is already a subgraph of target
     */
    static void renderMatchingResult(Graph* pattern, Graph* target,
                                     const std::vector<int>& vertexMapping,
                                     const std::vector<int>& unmatchedPatternVertices,
                                     const std::vector<std::pair<int,int>>& unmatchedPatternEdges,
                                     int ged, bool isSubgraph) {
        using namespace ftxui;

        int nP = pattern->getVertexCount();
        int nT = target->getVertexCount();
        int eP = pattern->getEdgeCount();
        int eT = target->getEdgeCount();

        // Build adjacency matrices
        std::vector<std::vector<int>> patternAdj(nP, std::vector<int>(nP, 0));
        for (int i = 0; i < eP; ++i) {
            Edge* edge = pattern->getEdge(i);
            patternAdj[edge->getOrigin()->getIndex()][edge->getTarget()->getIndex()]++;
        }

        std::vector<std::vector<int>> targetAdj(nT, std::vector<int>(nT, 0));
        for (int i = 0; i < eT; ++i) {
            Edge* edge = target->getEdge(i);
            targetAdj[edge->getOrigin()->getIndex()][edge->getTarget()->getIndex()]++;
        }

        // Create mapping from pattern vertices to solution vertices
        // Matched pattern vertices map to their matched target vertices (from vertexMapping)
        // Unmatched pattern vertices get new indices at the end (nT, nT+1, ...)
        std::map<int, int> patternToSolution;

        int nextNewIndex = nT;
        std::set<int> unmatchedSet(unmatchedPatternVertices.begin(), unmatchedPatternVertices.end());

        for (int i = 0; i < nP; ++i) {
            if (unmatchedSet.count(i) || vertexMapping[i] < 0) {
                patternToSolution[i] = nextNewIndex++;
            } else {
                // Use the actual mapping from the solver
                patternToSolution[i] = vertexMapping[i];
            }
        }

        // Build solution (target + new vertices at the end)
        int nSol = nT + static_cast<int>(unmatchedPatternVertices.size());
        std::vector<std::vector<int>> solutionAdj(nSol, std::vector<int>(nSol, 0));
        
        // Copy target edges
        for (int i = 0; i < nT; ++i) {
            for (int j = 0; j < nT; ++j) {
                solutionAdj[i][j] = targetAdj[i][j];
            }
        }
        
        // Add unmatched edges with remapped vertices
        std::set<std::pair<int,int>> addedEdges;
        for (const auto& e : unmatchedPatternEdges) {
            int solSrc = patternToSolution[e.first];
            int solDst = patternToSolution[e.second];
            addedEdges.insert({solSrc, solDst});
            solutionAdj[solSrc][solDst]++;
        }
        
        // Track which solution vertices are new
        std::set<int> newVertices;
        for (int v : unmatchedPatternVertices) {
            newVertices.insert(patternToSolution[v]);
        }

        // Create pattern card
        Element patternCard = createGraphCard("PATTERN GRAPH", patternAdj, nP, eP, {}, {});
        
        // Create target card
        Element targetCard = createGraphCard("TARGET GRAPH", targetAdj, nT, eT, {}, {});

        // Create solution card with highlighted additions
        std::string solTitle = "SOLUTION";
        Element solutionCard = createGraphCard(solTitle, solutionAdj, nSol, 
                                                eT + static_cast<int>(unmatchedPatternEdges.size()),
                                                addedEdges, 
                                                newVertices);

        // Create result card
        Elements resultContent;
        resultContent.push_back(hbox({text("GED:             "), text(std::to_string(ged)) | bold}));
        resultContent.push_back(hbox({text("Is Subgraph:     "), text(isSubgraph ? "yes" : "no")}));
        resultContent.push_back(hbox({text("Vertices to add: "), text(std::to_string(unmatchedPatternVertices.size()))}));
        resultContent.push_back(hbox({text("Edges to add:    "), text(std::to_string(unmatchedPatternEdges.size()))}));
        
        if (!addedEdges.empty()) {
            std::string edgeStr;
            bool first = true;
            for (const auto& e : addedEdges) {
                if (!first) edgeStr += ", ";
                edgeStr += std::to_string(e.first) + "-->" + std::to_string(e.second);
                first = false;
            }
            resultContent.push_back(hbox({text("New edges:       "), text(edgeStr) | color(Color::Green)}));
        }
        
        if (!newVertices.empty()) {
            std::string vertStr;
            bool first = true;
            for (int v : newVertices) {
                if (!first) vertStr += ", ";
                vertStr += std::to_string(v);
                first = false;
            }
            resultContent.push_back(hbox({text("New vertices:    "), text(vertStr) | color(Color::Green)}));
        }

        Element resultCard = vbox({
            text("RESULT") | bold | center,
            separator(),
            vbox(resultContent),
        }) | border;

        // Layout: Pattern and Target side by side, then Solution, then Result
        Element layout = vbox({
            hbox({
                patternCard | flex,
                targetCard | flex,
            }),
            solutionCard,
            resultCard,
        });

        auto screen = Screen::Create(Dimension::Fit(layout));
        Render(screen, layout);
        std::cout << std::endl;
        screen.Print();
        std::cout << std::endl;
    }

private:
    static ftxui::Element createGraphCard(const std::string& title,
                                           const std::vector<std::vector<int>>& adj,
                                           int vertexCount, int edgeCount,
                                           const std::set<std::pair<int,int>>& highlightEdges,
                                           const std::set<int>& highlightVertices) {
        using namespace ftxui;

        Elements content;
        
        // Vertices line
        std::string vertLine = "Vertices: " + std::to_string(vertexCount);
        if (!highlightVertices.empty()) {
            vertLine += " (+" + std::to_string(highlightVertices.size()) + " new)";
        }
        content.push_back(text(vertLine));

        // Edges line  
        std::string edgeLine = "Edges: " + std::to_string(edgeCount);
        if (!highlightEdges.empty()) {
            edgeLine = "Edges: " + std::to_string(edgeCount - static_cast<int>(highlightEdges.size())) + 
                       " (+" + std::to_string(highlightEdges.size()) + " new)";
        }
        content.push_back(text(edgeLine));
        
        content.push_back(text(""));
        content.push_back(text("Adjacency Matrix:"));

        int n = static_cast<int>(adj.size());
        
        // Header row with colored new vertices
        Elements headerElements;
        headerElements.push_back(text("    ") | dim);
        for (int j = 0; j < n; ++j) {
            std::string colLabel = std::to_string(j) + " ";
            if (highlightVertices.count(j)) {
                headerElements.push_back(text(colLabel) | color(Color::Green));
            } else {
                headerElements.push_back(text(colLabel) | dim);
            }
        }
        content.push_back(hbox(headerElements));

        // Matrix rows
        for (int i = 0; i < n; ++i) {
            Elements rowElements;
            
            // Row label
            if (highlightVertices.count(i)) {
                rowElements.push_back(text(std::to_string(i) + " ") | color(Color::Green));
            } else {
                rowElements.push_back(text(std::to_string(i) + " ") | dim);
            }
            
            rowElements.push_back(text("[ ") | dim);
            
            for (int j = 0; j < n; ++j) {
                std::string val = std::to_string(adj[i][j]) + " ";
                if (highlightEdges.count({i, j})) {
                    rowElements.push_back(text(val) | color(Color::Green) | bold);
                } else {
                    rowElements.push_back(text(val));
                }
            }
            
            rowElements.push_back(text("]") | dim);
            content.push_back(hbox(rowElements));
        }

        return vbox({
            text(title) | bold | center,
            separator(),
            vbox(content),
        }) | border;
    }
};

} // namespace gempp

#endif // GRAPH_CANVAS_H
