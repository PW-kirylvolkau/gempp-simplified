#ifndef V2_PROBLEM_H
#define V2_PROBLEM_H

#include "graph.h"
#include "../core/matrix.h"

namespace gempp {

// Problem class - holds pattern and target graphs with cost matrices
class Problem {
public:
    enum Type {
        SUBGRAPH = 0,
        GED
    };

    Problem(Type type, Graph* query, Graph* target)
        : type_(type), query_(query), target_(target)
    {
        // Initialize cost matrices with zeros (for exact matching)
        int nVP = query_->getVertexCount();
        int nVT = target_->getVertexCount();
        int nEP = query_->getEdgeCount();
        int nET = target_->getEdgeCount();

        vCosts_ = Matrix<double>(nVP, nVT, 0.0);
        eCosts_ = Matrix<double>(nEP, nET, 0.0);
    }

    ~Problem() {
        // Don't delete graphs - they're owned by caller
    }

    Type getType() const { return type_; }
    Graph* getQuery() const { return query_; }
    Graph* getTarget() const { return target_; }

    double getCost(bool isVertex, int queryIndex, int targetIndex) const {
        if (isVertex) {
            return vCosts_.getElement(queryIndex, targetIndex);
        } else {
            return eCosts_.getElement(queryIndex, targetIndex);
        }
    }

    void setCost(bool isVertex, int queryIndex, int targetIndex, double value) {
        if (isVertex) {
            vCosts_.setElement(queryIndex, targetIndex, value);
        } else {
            eCosts_.setElement(queryIndex, targetIndex, value);
        }
    }

private:
    Type type_;
    Graph* query_;   // Pattern graph
    Graph* target_;  // Target graph
    Matrix<double> vCosts_;  // Vertex substitution costs
    Matrix<double> eCosts_;  // Edge substitution costs
};

} // namespace gempp

#endif // V2_PROBLEM_H
