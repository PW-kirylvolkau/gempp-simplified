#ifndef V2_GRAPH_H
#define V2_GRAPH_H

#include "../core/types.h"
#include <algorithm>
#include <vector>

namespace gempp {

class Edge;
class Vertex;

// Minimal Vertex class
class Vertex {
public:
    enum Direction {
        EDGE_IN = 0,
        EDGE_OUT,
        EDGE_IN_OUT
    };

    Vertex() : index_(-1) {}

    void setIndex(int index) { index_ = index; }
    int getIndex() const { return index_; }

    void setID(const std::string& id) { id_ = id; }
    const std::string& getID() const { return id_; }

    void addEdge(Edge* e, Direction d) {
        edges_[d].push_back(e);
    }

    void removeEdge(Edge* e) {
        for (int d = 0; d < 3; ++d) {
            auto& vec = edges_[d];
            vec.erase(std::remove(vec.begin(), vec.end(), e), vec.end());
        }
    }

    const std::vector<Edge*>& getEdges(Direction d) const {
        return edges_[d];
    }

    int getDegree() const {
        return edges_[EDGE_IN].size() + edges_[EDGE_OUT].size() + edges_[EDGE_IN_OUT].size();
    }

private:
    int index_;
    std::string id_;
    std::vector<Edge*> edges_[3]; // IN, OUT, IN_OUT
};

// Minimal Edge class
class Edge {
public:
    Edge() : origin_(nullptr), target_(nullptr), index_(-1) {}

    void setIndex(int index) { index_ = index; }
    int getIndex() const { return index_; }

    void setID(const std::string& id) { id_ = id; }
    const std::string& getID() const { return id_; }

    void setOrigin(Vertex* v) { origin_ = v; }
    Vertex* getOrigin() const { return origin_; }

    void setTarget(Vertex* v) { target_ = v; }
    Vertex* getTarget() const { return target_; }

private:
    Vertex* origin_;
    Vertex* target_;
    int index_;
    std::string id_;
};

// Minimal Graph class
class Graph {
public:
    enum Type {
        DIRECTED = 0,
        UNDIRECTED
    };

    explicit Graph(Type type = DIRECTED) : type_(type) {}

    ~Graph() {
        for (auto v : vertices_) delete v;
        for (auto e : edges_) delete e;
    }

    // Disable copy
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    void setID(const std::string& id) { id_ = id; }
    const std::string& getID() const { return id_; }

    Type getType() const { return type_; }
    bool isDirected() const { return type_ == DIRECTED; }

    const std::vector<Vertex*>& getVertices() const { return vertices_; }
    const std::vector<Edge*>& getEdges() const { return edges_; }

    Vertex* getVertex(int i) const {
        if (i >= 0 && i < (int)vertices_.size()) {
            return vertices_[i];
        }
        return nullptr;
    }

    Vertex* getVertex(const std::string& id) const {
        auto it = vertex_map_.find(id);
        if (it != vertex_map_.end()) {
            return getVertex(it->second);
        }
        return nullptr;
    }

    Edge* getEdge(int i) const {
        if (i >= 0 && i < (int)edges_.size()) {
            return edges_[i];
        }
        return nullptr;
    }

    void addVertex(Vertex* v, const std::string& id = "") {
        v->setIndex(vertices_.size());
        std::string sid = !id.empty() ? id : std::to_string(vertices_.size());
        if (v->getID().empty()) {
            v->setID(sid);
        }
        vertex_map_[sid] = vertices_.size();
        vertices_.push_back(v);
    }

    void addEdge(Edge* e) {
        e->setIndex(edges_.size());
        edges_.push_back(e);
    }

    int getVertexCount() const { return vertices_.size(); }
    int getEdgeCount() const { return edges_.size(); }

private:
    Type type_;
    std::string id_;
    std::vector<Vertex*> vertices_;
    std::vector<Edge*> edges_;
    std::unordered_map<std::string, int> vertex_map_;
};

} // namespace gempp

#endif // V2_GRAPH_H
