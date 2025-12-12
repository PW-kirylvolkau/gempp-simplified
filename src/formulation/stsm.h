#ifndef V2_STSM_H
#define V2_STSM_H

#include "../model/problem.h"
#include "../model/graph.h"
#include "../integer_programming/linear_program.h"
#include "../core/matrix.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace gempp {

// Substitution-tolerant subgraph matching (STSM)
// Matches every pattern vertex/edge exactly once, allows substitutions with costs,
// and supports heuristic pruning via the `up` parameter.
class SubstitutionTolerantSubgraphMatching {
public:
    SubstitutionTolerantSubgraphMatching(Problem* pb, bool induced = false)
        : pb_(pb), induced_(induced), lp_(nullptr)
    {
        precision_ = 1e-9;
    }

    ~SubstitutionTolerantSubgraphMatching() {
        delete lp_;
    }

    void init(double up = 1.0) {
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
                auto* v = new Variable(id, Variable::BINARY);
                x_variables.setElement(i, k, v);
                lp_->addVariable(v);
            }
        }

        y_variables = Matrix<Variable*>(nEP, nET);
        for (int ij = 0; ij < nEP; ++ij) {
            for (int kl = 0; kl < nET; ++kl) {
                std::string id = "y_" + std::to_string(ij) + "," + std::to_string(kl);
                auto* v = new Variable(id, Variable::BINARY);
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

    void restrictProblem(double up) {
        // Activate everything first
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

        if (up < 1.0) {
            // Keep only the cheapest fraction of vertex candidates (row-wise)
            for (int i = 0; i < nVP; ++i) {
                std::vector<double> vals;
                vals.reserve(nVT);
                for (int k = 0; k < nVT; ++k) vals.push_back(x_costs.getElement(i, k));
                std::sort(vals.begin(), vals.end());
                double threshold = vals[static_cast<int>(std::floor(nVT * up))];
                for (int k = 0; k < nVT; ++k) {
                    if (x_costs.getElement(i, k) > threshold) {
                        x_variables.getElement(i, k)->deactivate();
                    }
                }
            }

            // Keep only the cheapest fraction of vertex candidates (column-wise)
            for (int k = 0; k < nVT; ++k) {
                std::vector<double> vals;
                vals.reserve(nVP);
                for (int i = 0; i < nVP; ++i) vals.push_back(x_costs.getElement(i, k));
                std::sort(vals.begin(), vals.end());
                double threshold = vals[static_cast<int>(std::floor(nVP * up))];
                for (int i = 0; i < nVP; ++i) {
                    if (x_costs.getElement(i, k) > threshold) {
                        x_variables.getElement(i, k)->deactivate();
                    }
                }
            }

            // Deactivate edge substitutions inconsistent with active vertex pairs
            for (int ij = 0; ij < nEP; ++ij) {
                int i = pb_->getQuery()->getEdge(ij)->getOrigin()->getIndex();
                int j = pb_->getQuery()->getEdge(ij)->getTarget()->getIndex();
                for (int kl = 0; kl < nET; ++kl) {
                    int k = pb_->getTarget()->getEdge(kl)->getOrigin()->getIndex();
                    int l = pb_->getTarget()->getEdge(kl)->getTarget()->getIndex();
                    if (isDirected) {
                        if (!(x_variables.getElement(i, k)->isActive() &&
                              x_variables.getElement(j, l)->isActive())) {
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
    }

    void initConstraints() {
        // Each pattern vertex maps to exactly one target vertex
        for (int i = 0; i < nVP; ++i) {
            auto* expr = new LinearExpression();
            for (int k = 0; k < nVT; ++k) {
                expr->addTerm(x_variables.getElement(i, k), 1.0);
            }
            auto* c = new LinearConstraint("vertex_" + std::to_string(i), expr, LinearConstraint::EQUAL, 1.0);
            lp_->addConstraint(c);
        }

        // Each target vertex receives at most one pattern vertex
        for (int k = 0; k < nVT; ++k) {
            auto* expr = new LinearExpression();
            for (int i = 0; i < nVP; ++i) {
                expr->addTerm(x_variables.getElement(i, k), 1.0);
            }
            auto* c = new LinearConstraint("target_vertex_" + std::to_string(k), expr, LinearConstraint::LESS_EQ, 1.0);
            lp_->addConstraint(c);
        }

        // Each pattern edge maps to exactly one target edge
        for (int ij = 0; ij < nEP; ++ij) {
            auto* expr = new LinearExpression();
            for (int kl = 0; kl < nET; ++kl) {
                expr->addTerm(y_variables.getElement(ij, kl), 1.0);
            }
            auto* c = new LinearConstraint("edge_" + std::to_string(ij), expr, LinearConstraint::EQUAL, 1.0);
            lp_->addConstraint(c);
        }

        // Edge consistency constraints (F2 style)
        for (int ij = 0; ij < nEP; ++ij) {
            int i = pb_->getQuery()->getEdge(ij)->getOrigin()->getIndex();
            int j = pb_->getQuery()->getEdge(ij)->getTarget()->getIndex();

            for (int k = 0; k < nVT; ++k) {
                auto* e1 = new LinearExpression();
                auto* e2 = new LinearExpression();

                for (int kl = 0; kl < nET; ++kl) {
                    Edge* target_edge = pb_->getTarget()->getEdge(kl);
                    int k_out = target_edge->getOrigin()->getIndex();
                    int k_in = target_edge->getTarget()->getIndex();

                    if (k_out == k) e1->addTerm(y_variables.getElement(ij, kl), 1.0);
                    if (k_in == k) e2->addTerm(y_variables.getElement(ij, kl), 1.0);
                }

                e1->addTerm(x_variables.getElement(i, k), -1.0);
                e2->addTerm(x_variables.getElement(j, k), -1.0);

                if (!isDirected) {
                    e1->addTerm(x_variables.getElement(j, k), -1.0);
                    e2->addTerm(x_variables.getElement(i, k), -1.0);
                }

                lp_->addConstraint(new LinearConstraint(
                    "edge_cons_" + std::to_string(ij) + "_" + std::to_string(k) + "_out",
                    e1,
                    LinearConstraint::LESS_EQ,
                    0.0));
                lp_->addConstraint(new LinearConstraint(
                    "edge_cons_" + std::to_string(ij) + "_" + std::to_string(k) + "_in",
                    e2,
                    LinearConstraint::LESS_EQ,
                    0.0));
            }
        }

        // Induced subgraph constraints (optional)
        if (induced_) {
            for (int kl = 0; kl < nET; ++kl) {
                int k = pb_->getTarget()->getEdge(kl)->getOrigin()->getIndex();
                int l = pb_->getTarget()->getEdge(kl)->getTarget()->getIndex();

                auto* expr = new LinearExpression();
                for (int i = 0; i < nVP; ++i) {
                    expr->addTerm(x_variables.getElement(i, k), 1.0);
                    expr->addTerm(x_variables.getElement(i, l), 1.0);
                }
                for (int ij = 0; ij < nEP; ++ij) {
                    expr->addTerm(y_variables.getElement(ij, kl), -1.0);
                }

                lp_->addConstraint(new LinearConstraint(
                    "induced_" + std::to_string(kl),
                    expr,
                    LinearConstraint::LESS_EQ,
                    1.0));
            }
        }
    }

    void initObjective() {
        auto* obj = new LinearExpression();

        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                double cost = x_costs.getElement(i, k);
                if (std::abs(cost) > precision_) {
                    obj->addTerm(x_variables.getElement(i, k), cost);
                }
            }
        }

        for (int ij = 0; ij < nEP; ++ij) {
            for (int kl = 0; kl < nET; ++kl) {
                double cost = y_costs.getElement(ij, kl);
                if (std::abs(cost) > precision_) {
                    obj->addTerm(y_variables.getElement(ij, kl), cost);
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

