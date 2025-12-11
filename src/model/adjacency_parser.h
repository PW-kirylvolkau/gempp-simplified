#ifndef V2_ADJACENCY_PARSER_H
#define V2_ADJACENCY_PARSER_H

#include "graph.h"
#include "../core/types.h"
#include <fstream>
#include <sstream>
#include <utility>

namespace gempp {

class AdjacencyMatrixParser {
public:
    static std::pair<Graph*, Graph*> parseFile(const std::string& filename, bool multigraph = false) {
        // Read entire file
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw Exception("Cannot open file: " + filename);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string data = buffer.str();

        return parseData(data, multigraph);
    }

    static std::pair<Graph*, Graph*> parseData(const std::string& data, bool multigraph = false) {
        // Split into lines
        std::vector<std::string> lines = StringUtils::split(data, '\n', false);

        // Remove empty lines and trim whitespace
        std::vector<std::string> trimmed_lines;
        for (const auto& line : lines) {
            std::string trimmed = StringUtils::trim(line);
            if (!trimmed.empty()) {
                trimmed_lines.push_back(trimmed);
            }
        }

        if (trimmed_lines.size() < 2) {
            throw Exception("File must contain at least two graphs (vertex count lines)");
        }

        // Parse first graph
        auto firstGraphResult = parseSingleGraph(trimmed_lines, 0, 0, multigraph);
        Graph* graph1 = firstGraphResult.first;
        int nextLine = firstGraphResult.second;

        // Parse second graph
        auto secondGraphResult = parseSingleGraph(trimmed_lines, nextLine, 1, multigraph);
        Graph* graph2 = secondGraphResult.first;

        return std::make_pair(graph1, graph2);
    }

private:
    static std::pair<Graph*, int> parseSingleGraph(
        const std::vector<std::string>& lines,
        int startLine,
        int graphIndex,
        bool multigraph = false)
    {
        if (startLine >= (int)lines.size()) {
            throw Exception("Unexpected end of file while parsing graph " +
                          std::to_string(graphIndex + 1));
        }

        // Read number of vertices
        bool ok;
        int vertexCount = StringUtils::toInt(lines[startLine], &ok);
        if (!ok || vertexCount <= 0) {
            throw Exception("Invalid vertex count '" + lines[startLine] +
                          "' for graph " + std::to_string(graphIndex + 1));
        }

        int currentLine = startLine + 1;

        // Read adjacency matrix
        if (currentLine + vertexCount > (int)lines.size()) {
            throw Exception("Not enough lines for adjacency matrix of graph " +
                          std::to_string(graphIndex + 1));
        }

        // Create graph (undirected for symmetric adjacency matrices)
        Graph* graph = new Graph(Graph::UNDIRECTED);
        graph->setID("graph_" + std::to_string(graphIndex));

        // Add vertices
        for (int i = 0; i < vertexCount; ++i) {
            Vertex* v = new Vertex();
            graph->addVertex(v, std::to_string(i));
        }

        // Parse adjacency matrix and create edges
        // For undirected graphs with symmetric matrices, only create edge when i < j
        for (int i = 0; i < vertexCount; ++i) {
            std::string line = lines[currentLine + i];
            std::vector<std::string> values = StringUtils::split(line, ' ', true);

            if ((int)values.size() != vertexCount) {
                throw Exception("Adjacency matrix row " + std::to_string(i + 1) +
                              " of graph " + std::to_string(graphIndex + 1) +
                              " has " + std::to_string(values.size()) +
                              " values, expected " + std::to_string(vertexCount));
            }

            // Only process upper triangle (j > i) for undirected graphs
            for (int j = i + 1; j < vertexCount; ++j) {
                bool ok;
                int weight = StringUtils::toInt(values[j], &ok);
                if (!ok) {
                    throw Exception("Invalid adjacency matrix value '" + values[j] +
                                  "' at position (" + std::to_string(i + 1) + "," +
                                  std::to_string(j + 1) + ") in graph " +
                                  std::to_string(graphIndex + 1));
                }

                // Create edges based on weight
                if (weight != 0) {
                    if (weight < 0) {
                        throw Exception("Adjacency matrix value " + std::to_string(weight) +
                                      " at position (" + std::to_string(i + 1) + "," +
                                      std::to_string(j + 1) + ") in graph " +
                                      std::to_string(graphIndex + 1) + " is negative");
                    }

                    if (!multigraph && weight != 1) {
                        throw Exception("Adjacency matrix value " + std::to_string(weight) +
                                      " at position (" + std::to_string(i + 1) + "," +
                                      std::to_string(j + 1) + ") in graph " +
                                      std::to_string(graphIndex + 1) + " is not 0 or 1 (use --multigraph flag for multigraphs)");
                    }

                    // For multigraphs, create 'weight' edges; for simple graphs, create 1 edge
                    int edge_count = multigraph ? weight : 1;
                    for (int m = 0; m < edge_count; ++m) {
                        Edge* edge = new Edge();
                        edge->setOrigin(graph->getVertex(i));
                        edge->setTarget(graph->getVertex(j));
                        graph->addEdge(edge);

                        // Connect the edge to vertices (both directions for undirected)
                        graph->getVertex(i)->addEdge(edge, Vertex::EDGE_IN_OUT);
                        graph->getVertex(j)->addEdge(edge, Vertex::EDGE_IN_OUT);
                    }
                }
            }
        }

        return std::make_pair(graph, currentLine + vertexCount);
    }
};

} // namespace gempp

#endif // V2_ADJACENCY_PARSER_H
