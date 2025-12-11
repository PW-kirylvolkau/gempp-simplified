#ifndef V2_STSM_H
#define V2_STSM_H

#include "../model/problem.h"
#include "../model/graph.h"
#include "../integer_programming/linear_program.h"
#include "../core/matrix.h"
#include <vector>
#include <algorithm>
#include <cmath>

namespace gempp {

// Substitution-Tolerant Subgraph Matching (STSM)
// Matches every pattern vertex/edge while allowing label substitutions.
// Supports an upper-bound approximation (up in (0, 1]) that prunes
// high-cost substitutions to speed up solving.
class SubstitutionTolerantSubgraphMatching {
public:
    SubstitutionTolerantSubgraphMatching(Problem* pb, bool induced = false)
        : pb_(pb), induced_(induced), lp_(nullptr) {
        precision_ = 1e-9;
    }

    ~SubstitutionTolerantSubgraphMatching() {
        delete lp_;
    }

    void init(double up = 1.0) {
        if (up <= 0.0 || up > 1.0) {
            throw Exception("Upperbound must be in (0, 1].");
        }

        lp_ = new LinearProgram(LinearProgram::MINIMIZE);

        nVP = pb_->getQuery()->getVertexCount();
        nVT = pb_->getTarget()->getVertexCount();
        nEP = pb_->getQuery()->getEdgeCount();
        nET = pb_->getTarget()->getEdgeCount();
        isDirected = pb_->getQuery()->isDirected();

        initVariables();
        initCosts();
        restrictProblem(up);
        initConstraints();
        initObjective();
    }

    LinearProgram* getLinearProgram() { return lp_; }
    const Matrix<Variable*>& getXVariables() const { return x_variables; }
    const Matrix<Variable*>& getYVariables() const { return y_variables; }

private:
    void initVariables() {
        x_variables = Matrix<Variable*>(nVP, nVT);
        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                std::string id = "x_" + std::to_string(i) + "," + std::to_string(k);
                Variable* v = new Variable(id, Variable::BINARY);
                x_variables.setElement(i, k, v);
                lp_->addVariable(v);
            }
        }

        y_variables = Matrix<Variable*>(nEP, nET);
        for (int ij = 0; ij < nEP; ++ij) {
            for (int kl = 0; kl < nET; ++kl) {
                std::string id = "y_" + std::to_string(ij) + "," + std::to_string(kl);
                Variable* v = new Variable(id, Variable::BINARY);
                y_variables.setElement(ij, kl, v);
                lp_->addVariable(v);
            }
        }
    }

    void initCosts() {
        x_costs = Matrix<double>(nVP, nVT);
        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                x_costs.setElement(i, k, pb_->getCost(true, i, k));
            }
        }

        y_costs = Matrix<double>(nEP, nET);
        for (int ij = 0; ij < nEP; ++ij) {
            for (int kl = 0; kl < nET; ++kl) {
                y_costs.setElement(ij, kl, pb_->getCost(false, ij, kl));
            }
        }
    }

    static double computeThreshold(const std::vector<double>& costs, int total, double up) {
        int keep = std::max(1, static_cast<int>(std::floor(total * up)));
        std::vector<double> sorted = costs;
        std::sort(sorted.begin(), sorted.end());
        return sorted[keep - 1];
    }

    void restrictProblem(double up) {
        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                x_variables.getElement(i, k)->activate();
            }
        }
        for (int ij = 0; ij < nEP; ++ij) {
            for (int kl = 0; kl < nET; ++kl) {
                y_variables.getElement(ij, kl)->activate();
            }
        }

        // Vertex pruning based on substitution costs
        for (int i = 0; i < nVP; ++i) {
            std::vector<double> row;
            row.reserve(nVT);
            for (int k = 0; k < nVT; ++k) {
                row.push_back(x_costs.getElement(i, k));
            }
            double threshold = computeThreshold(row, nVT, up);
            for (int k = 0; k < nVT; ++k) {
                if (x_costs.getElement(i, k) - threshold > precision_) {
                    x_variables.getElement(i, k)->deactivate();
                }
            }
        }

        // Edge pruning based on substitution costs and active vertex mappings
        for (int ij = 0; ij < nEP; ++ij) {
            int i = pb_->getQuery()->getEdge(ij)->getOrigin()->getIndex();
            int j = pb_->getQuery()->getEdge(ij)->getTarget()->getIndex();

            std::vector<double> row;
            row.reserve(nET);
            for (int kl = 0; kl < nET; ++kl) {
                row.push_back(y_costs.getElement(ij, kl));
            }
            double threshold = computeThreshold(row, nET, up);

            for (int kl = 0; kl < nET; ++kl) {
                if (y_costs.getElement(ij, kl) - threshold > precision_) {
                    y_variables.getElement(ij, kl)->deactivate();
                    continue;
                }

                int k = pb_->getTarget()->getEdge(kl)->getOrigin()->getIndex();
                int l = pb_->getTarget()->getEdge(kl)->getTarget()->getIndex();

                if (isDirected) {
                    bool active = x_variables.getElement(i, k)->isActive() &&
                                  x_variables.getElement(j, l)->isActive();
                    if (!active) {
                        y_variables.getElement(ij, kl)->deactivate();
                    }
                } else {
                    bool option1 = x_variables.getElement(i, k)->isActive() &&
                                   x_variables.getElement(j, l)->isActive();
                    bool option2 = x_variables.getElement(i, l)->isActive() &&
                                   x_variables.getElement(j, k)->isActive();
                    if (!(option1 || option2)) {
                        y_variables.getElement(ij, kl)->deactivate();
                    }
                }
            }
        }
    }

    void initConstraints() {
        // Each pattern vertex maps to exactly one target vertex
        for (int i = 0; i < nVP; ++i) {
            LinearExpression* expr = new LinearExpression();
            for (int k = 0; k < nVT; ++k) {
                if (x_variables.getElement(i, k)->isActive()) {
                    expr->addTerm(x_variables.getElement(i, k), 1.0);
                }
            }
            std::string id = "vertex_" + std::to_string(i);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::EQUAL, 1.0);
            lp_->addConstraint(c);
        }

        // Each target vertex maps to at most one pattern vertex
        for (int k = 0; k < nVT; ++k) {
            LinearExpression* expr = new LinearExpression();
            for (int i = 0; i < nVP; ++i) {
                if (x_variables.getElement(i, k)->isActive()) {
                    expr->addTerm(x_variables.getElement(i, k), 1.0);
                }
            }
            std::string id = "target_vertex_" + std::to_string(k);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::LESS_EQ, 1.0);
            lp_->addConstraint(c);
        }

        // Each pattern edge maps to exactly one target edge
        for (int ij = 0; ij < nEP; ++ij) {
            LinearExpression* expr = new LinearExpression();
            for (int kl = 0; kl < nET; ++kl) {
                if (y_variables.getElement(ij, kl)->isActive()) {
                    expr->addTerm(y_variables.getElement(ij, kl), 1.0);
                }
            }
            std::string id = "edge_" + std::to_string(ij);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::EQUAL, 1.0);
            lp_->addConstraint(c);
        }

        // Edge consistency constraints
        for (int ij = 0; ij < nEP; ++ij) {
            int i = pb_->getQuery()->getEdge(ij)->getOrigin()->getIndex();
            int j = pb_->getQuery()->getEdge(ij)->getTarget()->getIndex();

            for (int k = 0; k < nVT; ++k) {
                LinearExpression* e1 = new LinearExpression();
                LinearExpression* e2 = new LinearExpression();

                for (int kl = 0; kl < nET; ++kl) {
                    Edge* target_edge = pb_->getTarget()->getEdge(kl);
                    int k_out = target_edge->getOrigin()->getIndex();
                    int k_in = target_edge->getTarget()->getIndex();

                    if (k_out == k && y_variables.getElement(ij, kl)->isActive()) {
                        e1->addTerm(y_variables.getElement(ij, kl), 1.0);
                    }
                    if (k_in == k && y_variables.getElement(ij, kl)->isActive()) {
                        e2->addTerm(y_variables.getElement(ij, kl), 1.0);
                    }
                }

                if (x_variables.getElement(i, k)->isActive()) {
                    e1->addTerm(x_variables.getElement(i, k), -1.0);
                }
                if (x_variables.getElement(j, k)->isActive()) {
                    e2->addTerm(x_variables.getElement(j, k), -1.0);
                }

                if (!isDirected) {
                    if (x_variables.getElement(j, k)->isActive()) {
                        e1->addTerm(x_variables.getElement(j, k), -1.0);
                    }
                    if (x_variables.getElement(i, k)->isActive()) {
                        e2->addTerm(x_variables.getElement(i, k), -1.0);
                    }
                }

                std::string id1 = "edge_cons_" + std::to_string(ij) + "_" + std::to_string(k) + "_out";
                std::string id2 = "edge_cons_" + std::to_string(ij) + "_" + std::to_string(k) + "_in";

                auto* c1 = new LinearConstraint(id1, e1, LinearConstraint::LESS_EQ, 0.0);
                auto* c2 = new LinearConstraint(id2, e2, LinearConstraint::LESS_EQ, 0.0);

                lp_->addConstraint(c1);
                lp_->addConstraint(c2);
            }
        }

        // Induced subgraph constraints (optional)
        if (induced_) {
            for (int kl = 0; kl < nET; ++kl) {
                int k = pb_->getTarget()->getEdge(kl)->getOrigin()->getIndex();
                int l = pb_->getTarget()->getEdge(kl)->getTarget()->getIndex();

                LinearExpression* expr = new LinearExpression();

                for (int i = 0; i < nVP; ++i) {
                    if (x_variables.getElement(i, k)->isActive()) {
                        expr->addTerm(x_variables.getElement(i, k), 1.0);
                    }
                }
                for (int i = 0; i < nVP; ++i) {
                    if (x_variables.getElement(i, l)->isActive()) {
                        expr->addTerm(x_variables.getElement(i, l), 1.0);
                    }
                }
                for (int ij = 0; ij < nEP; ++ij) {
                    if (y_variables.getElement(ij, kl)->isActive()) {
                        expr->addTerm(y_variables.getElement(ij, kl), -1.0);
                    }
                }

                std::string id = "induced_" + std::to_string(kl);
                auto* c = new LinearConstraint(id, expr, LinearConstraint::LESS_EQ, 1.0);
                lp_->addConstraint(c);
            }
        }
    }

    void initObjective() {
        LinearExpression* obj = new LinearExpression();

        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                if (x_variables.getElement(i, k)->isActive()) {
                    double cost = x_costs.getElement(i, k);
                    if (cost > 0) {
                        obj->addTerm(x_variables.getElement(i, k), cost);
                    }
                }
            }
        }

        for (int ij = 0; ij < nEP; ++ij) {
            for (int kl = 0; kl < nET; ++kl) {
                if (y_variables.getElement(ij, kl)->isActive()) {
                    double cost = y_costs.getElement(ij, kl);
                    if (cost > 0) {
                        obj->addTerm(y_variables.getElement(ij, kl), cost);
                    }
                }
            }
        }

        lp_->setObjective(obj);
    }

    Problem* pb_;
    bool induced_;
    LinearProgram* lp_;
    double precision_;

    int nVP, nVT, nEP, nET;
    bool isDirected;

    Matrix<Variable*> x_variables;
    Matrix<Variable*> y_variables;
    Matrix<double> x_costs;
    Matrix<double> y_costs;
};

} // namespace gempp

#endif // V2_STSM_H

