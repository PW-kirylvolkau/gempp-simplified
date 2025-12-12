#ifndef V2_LINEAR_GED_H
#define V2_LINEAR_GED_H

#include "../model/problem.h"
#include "../model/graph.h"
#include "../integer_programming/linear_program.h"
#include "../core/matrix.h"
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

namespace gempp {

// Linear programming formulation for Graph Edit Distance (GED).
// Uses unit insertion/deletion costs and optional substitution costs
// provided through the Problem's cost matrices.
class LinearGraphEditDistance {
public:
    explicit LinearGraphEditDistance(Problem* pb)
        : pb_(pb), lp_(nullptr), relaxed_(false)
    {
        precision_ = 1e-9;
        vertex_cost_ = 1.0;
        edge_cost_ = 1.0;
    }

    ~LinearGraphEditDistance() {
        delete lp_;
    }

    // Build the linear program.
    void init(double up = 1.0, bool relaxed = false) {
        lp_ = new LinearProgram(LinearProgram::MINIMIZE);
        relaxed_ = relaxed;

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
        auto varType = relaxed_ ? Variable::CONTINUOUS : Variable::BINARY;

        // Vertex substitution variables
        x_variables = Matrix<Variable*>(nVP, nVT);
        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                std::string id = "x_" + std::to_string(i) + "," + std::to_string(k);
                auto* v = new Variable(id, varType, 0, 1);
                x_variables.setElement(i, k, v);
                lp_->addVariable(v);
            }
        }

        // Edge substitution variables
        y_variables = Matrix<Variable*>(nEP, nET);
        for (int ij = 0; ij < nEP; ++ij) {
            for (int kl = 0; kl < nET; ++kl) {
                std::string id = "y_" + std::to_string(ij) + "," + std::to_string(kl);
                auto* v = new Variable(id, varType, 0, 1);
                y_variables.setElement(ij, kl, v);
                lp_->addVariable(v);
            }
        }
    }

    void initCosts() {
        // Substitution cost matrices. The GED objective is:
        // constant (delete+insert everything) + sum((substitution - delete - insert) * match)
        x_costs = Matrix<double>(nVP, nVT);
        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                double substitution = pb_->getCost(true, i, k);
                x_costs.setElement(i, k, substitution - vertex_cost_ - vertex_cost_);
            }
        }

        y_costs = Matrix<double>(nEP, nET);
        for (int ij = 0; ij < nEP; ++ij) {
            for (int kl = 0; kl < nET; ++kl) {
                double substitution = pb_->getCost(false, ij, kl);
                y_costs.setElement(ij, kl, substitution - edge_cost_ - edge_cost_);
            }
        }
    }

    void restrictProblem(double up) {
        // Activate all variables (reset bounds).
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

        // When up < 1, keep only the cheapest candidates (same strategy as original GEM++).
        if (up < 1.0) {
            // Filter vertex substitutions by rows
            for (int i = 0; i < nVP; ++i) {
                std::vector<double> v;
                v.reserve(nVT);
                for (int k = 0; k < nVT; ++k) {
                    v.push_back(x_costs.getElement(i, k));
                }
                std::sort(v.begin(), v.end());
                double threshold = v[static_cast<int>(std::floor(nVT * up))];
                for (int k = 0; k < nVT; ++k) {
                    if (x_costs.getElement(i, k) > threshold) {
                        x_variables.getElement(i, k)->deactivate();
                    }
                }
            }

            // Filter vertex substitutions by columns
            for (int k = 0; k < nVT; ++k) {
                std::vector<double> v;
                v.reserve(nVP);
                for (int i = 0; i < nVP; ++i) {
                    v.push_back(x_costs.getElement(i, k));
                }
                std::sort(v.begin(), v.end());
                double threshold = v[static_cast<int>(std::floor(nVP * up))];
                for (int i = 0; i < nVP; ++i) {
                    if (x_costs.getElement(i, k) > threshold) {
                        x_variables.getElement(i, k)->deactivate();
                    }
                }
            }

            // Filter edge substitutions according to active vertex pairs and costs
            for (int ij = 0; ij < nEP; ++ij) {
                int i = pb_->getQuery()->getEdge(ij)->getOrigin()->getIndex();
                int j = pb_->getQuery()->getEdge(ij)->getTarget()->getIndex();
                for (int kl = 0; kl < nET; ++kl) {
                    int k = pb_->getTarget()->getEdge(kl)->getOrigin()->getIndex();
                    int l = pb_->getTarget()->getEdge(kl)->getTarget()->getIndex();
                    if (isDirected) {
                        // y_ij,kl must be 0 if the couple (x_i,k * x_j,l) is inactive
                        if (!(x_variables.getElement(i, k)->isActive() &&
                              x_variables.getElement(j, l)->isActive())) {
                            y_variables.getElement(ij, kl)->deactivate();
                        }
                    } else {
                        // y_ij,kl must be 0 if both (x_i,k * x_j,l) and (x_i,l * x_j,k) are inactive
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
        // Each pattern vertex matched at most once
        for (int i = 0; i < nVP; ++i) {
            auto* expr = new LinearExpression();
            for (int k = 0; k < nVT; ++k) {
                expr->addTerm(x_variables.getElement(i, k), 1.0);
            }
            std::string id = "vertex_" + std::to_string(i);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::LESS_EQ, 1.0);
            lp_->addConstraint(c);
        }

        // Each target vertex matched at most once
        for (int k = 0; k < nVT; ++k) {
            auto* expr = new LinearExpression();
            for (int i = 0; i < nVP; ++i) {
                expr->addTerm(x_variables.getElement(i, k), 1.0);
            }
            std::string id = "target_vertex_" + std::to_string(k);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::LESS_EQ, 1.0);
            lp_->addConstraint(c);
        }

        // Each pattern edge matched at most once
        for (int ij = 0; ij < nEP; ++ij) {
            auto* expr = new LinearExpression();
            for (int kl = 0; kl < nET; ++kl) {
                expr->addTerm(y_variables.getElement(ij, kl), 1.0);
            }
            std::string id = "edge_" + std::to_string(ij);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::LESS_EQ, 1.0);
            lp_->addConstraint(c);
        }

        // Each target edge matched at most once (symmetry for GED)
        for (int kl = 0; kl < nET; ++kl) {
            auto* expr = new LinearExpression();
            for (int ij = 0; ij < nEP; ++ij) {
                expr->addTerm(y_variables.getElement(ij, kl), 1.0);
            }
            std::string id = "target_edge_" + std::to_string(kl);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::LESS_EQ, 1.0);
            lp_->addConstraint(c);
        }

        // Edge consistency constraints (F2)
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

                    if (k_out == k) {
                        e1->addTerm(y_variables.getElement(ij, kl), 1.0);
                    }
                    if (k_in == k) {
                        e2->addTerm(y_variables.getElement(ij, kl), 1.0);
                    }
                }

                e1->addTerm(x_variables.getElement(i, k), -1.0);
                e2->addTerm(x_variables.getElement(j, k), -1.0);

                if (!isDirected) {
                    e1->addTerm(x_variables.getElement(j, k), -1.0);
                    e2->addTerm(x_variables.getElement(i, k), -1.0);
                }

                std::string id1 = "edge_cons_" + std::to_string(ij) + "_" + std::to_string(k) + "_out";
                std::string id2 = "edge_cons_" + std::to_string(ij) + "_" + std::to_string(k) + "_in";

                auto* c1 = new LinearConstraint(id1, e1, LinearConstraint::LESS_EQ, 0.0);
                auto* c2 = new LinearConstraint(id2, e2, LinearConstraint::LESS_EQ, 0.0);

                lp_->addConstraint(c1);
                lp_->addConstraint(c2);
            }
        }
    }

    void initObjective() {
        auto* obj = new LinearExpression();

        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                double coeff = x_costs.getElement(i, k);
                if (std::abs(coeff) > precision_) {
                    obj->addTerm(x_variables.getElement(i, k), coeff);
                }
            }
        }

        for (int ij = 0; ij < nEP; ++ij) {
            for (int kl = 0; kl < nET; ++kl) {
                double coeff = y_costs.getElement(ij, kl);
                if (std::abs(coeff) > precision_) {
                    obj->addTerm(y_variables.getElement(ij, kl), coeff);
                }
            }
        }

        double constant = vertex_cost_ * (nVP + nVT) + edge_cost_ * (nEP + nET);
        obj->setConstant(constant);
        lp_->setObjective(obj);
    }

    Problem* pb_;
    LinearProgram* lp_;
    bool relaxed_;
    double precision_;
    double vertex_cost_;
    double edge_cost_;

    int nVP, nVT, nEP, nET;
    bool isDirected;

    Matrix<Variable*> x_variables;
    Matrix<Variable*> y_variables;
    Matrix<double> x_costs;
    Matrix<double> y_costs;
};

} // namespace gempp

#endif // V2_LINEAR_GED_H

