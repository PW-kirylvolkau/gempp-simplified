#ifndef V2_SUBGRAPH_ISOMORPHISM_H
#define V2_SUBGRAPH_ISOMORPHISM_H

#include "../model/problem.h"
#include "../model/graph.h"
#include "../integer_programming/linear_program.h"
#include "../core/matrix.h"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace gempp {

// Complete Subgraph Isomorphism Formulation
class SubgraphIsomorphism {
public:
    SubgraphIsomorphism(Problem* pb, bool induced = false)
        : pb_(pb), induced_(induced), lp_(nullptr)
    {
        precision_ = 1e-9;
    }

    ~SubgraphIsomorphism() {
        // Clean up variables (they're owned by linear program)
        delete lp_;
    }

    // Initialize the formulation
    void init(double up = 1.0) {
        // Create linear program
        lp_ = new LinearProgram(LinearProgram::MINIMIZE);

        // Set up problem dimensions
        nVP = pb_->getQuery()->getVertexCount();
        nVT = pb_->getTarget()->getVertexCount();
        nEP = pb_->getQuery()->getEdgeCount();
        nET = pb_->getTarget()->getEdgeCount();
        isDirected = pb_->getQuery()->isDirected();

        // Initialize all components
        initVariables();
        initCosts();
        restrictProblem(up);
        initConstraints();
        initObjective();
    }

    LinearProgram* getLinearProgram() { return lp_; }

    // Get variable mapping for solution extraction
    const Matrix<Variable*>& getXVariables() const { return x_variables; }
    const Matrix<Variable*>& getYVariables() const { return y_variables; }

private:
    void initVariables() {
        // Create x variables (vertex matching): x[i][k] = 1 if vertex i maps to k
        x_variables = Matrix<Variable*>(nVP, nVT);
        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                std::string id = "x_" + std::to_string(i) + "," + std::to_string(k);
                Variable* v = new Variable(id, Variable::BINARY);
                x_variables.setElement(i, k, v);
                lp_->addVariable(v);
            }
        }

        // Create y variables (edge matching): y[ij][kl] = 1 if edge ij maps to kl
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
        // Initialize cost matrices from problem
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
        // For exact subgraph isomorphism (up == 1.0):
        // Deactivate variables where cost > precision
        if (up < 1) {
            throw Exception("Upper-bound approximation not supported in this version");
        }

        // Activate all variables first
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

        // Deactivate vertex variables with high cost
        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                if (x_costs.getElement(i, k) > precision_) {
                    x_variables.getElement(i, k)->deactivate();
                }
            }
        }

        // Deactivate edge variables based on vertex variables and cost
        for (int ij = 0; ij < nEP; ++ij) {
            int i = pb_->getQuery()->getEdge(ij)->getOrigin()->getIndex();
            int j = pb_->getQuery()->getEdge(ij)->getTarget()->getIndex();

            for (int kl = 0; kl < nET; ++kl) {
                if (y_costs.getElement(ij, kl) > precision_) {
                    y_variables.getElement(ij, kl)->deactivate();
                } else {
                    int k = pb_->getTarget()->getEdge(kl)->getOrigin()->getIndex();
                    int l = pb_->getTarget()->getEdge(kl)->getTarget()->getIndex();

                    if (isDirected) {
                        // y_ij,kl must be 0 if (x_i,k * x_j,l) is inactive
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
        // Constraint 1: Each pattern vertex maps to exactly one target vertex
        for (int i = 0; i < nVP; ++i) {
            LinearExpression* expr = new LinearExpression();
            for (int k = 0; k < nVT; ++k) {
                expr->addTerm(x_variables.getElement(i, k), 1.0);
            }
            std::string id = "vertex_" + std::to_string(i);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::EQUAL, 1.0);
            lp_->addConstraint(c);
        }

        // Constraint 2: Each target vertex maps to at most one pattern vertex
        for (int k = 0; k < nVT; ++k) {
            LinearExpression* expr = new LinearExpression();
            for (int i = 0; i < nVP; ++i) {
                expr->addTerm(x_variables.getElement(i, k), 1.0);
            }
            std::string id = "target_vertex_" + std::to_string(k);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::LESS_EQ, 1.0);
            lp_->addConstraint(c);
        }

        // Constraint 3: Each pattern edge maps to exactly one target edge
        for (int ij = 0; ij < nEP; ++ij) {
            LinearExpression* expr = new LinearExpression();
            for (int kl = 0; kl < nET; ++kl) {
                expr->addTerm(y_variables.getElement(ij, kl), 1.0);
            }
            std::string id = "edge_" + std::to_string(ij);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::EQUAL, 1.0);
            lp_->addConstraint(c);
        }

        // Constraint 4 (F2): Edge consistency constraints
        for (int ij = 0; ij < nEP; ++ij) {
            int i = pb_->getQuery()->getEdge(ij)->getOrigin()->getIndex();
            int j = pb_->getQuery()->getEdge(ij)->getTarget()->getIndex();

            for (int k = 0; k < nVT; ++k) {
                LinearExpression* e1 = new LinearExpression();
                LinearExpression* e2 = new LinearExpression();

                // Sum over edges incident to vertex k
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

        // Constraint 5: Induced subgraph constraints (if required)
        if (induced_) {
            for (int kl = 0; kl < nET; ++kl) {
                int k = pb_->getTarget()->getEdge(kl)->getOrigin()->getIndex();
                int l = pb_->getTarget()->getEdge(kl)->getTarget()->getIndex();

                LinearExpression* expr = new LinearExpression();

                // Sum all pattern vertices mapped to k
                for (int i = 0; i < nVP; ++i) {
                    expr->addTerm(x_variables.getElement(i, k), 1.0);
                }

                // Sum all pattern vertices mapped to l
                for (int i = 0; i < nVP; ++i) {
                    expr->addTerm(x_variables.getElement(i, l), 1.0);
                }

                // Subtract edge mappings
                for (int ij = 0; ij < nEP; ++ij) {
                    expr->addTerm(y_variables.getElement(ij, kl), -1.0);
                }

                std::string id = "induced_" + std::to_string(kl);
                auto* c = new LinearConstraint(id, expr, LinearConstraint::LESS_EQ, 1.0);
                lp_->addConstraint(c);
            }
        }
    }

    void initObjective() {
        LinearExpression* obj = new LinearExpression();

        // Add vertex substitution costs
        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                double cost = x_costs.getElement(i, k);
                if (cost > 0) {
                    obj->addTerm(x_variables.getElement(i, k), cost);
                }
            }
        }

        // Add edge substitution costs
        for (int ij = 0; ij < nEP; ++ij) {
            for (int kl = 0; kl < nET; ++kl) {
                double cost = y_costs.getElement(ij, kl);
                if (cost > 0) {
                    obj->addTerm(y_variables.getElement(ij, kl), cost);
                }
            }
        }

        lp_->setObjective(obj);
    }

    // Member variables
    Problem* pb_;
    LinearProgram* lp_;
    bool induced_;
    double precision_;

    // Problem dimensions
    int nVP, nVT, nEP, nET;
    bool isDirected;

    // Variable and cost matrices
    Matrix<Variable*> x_variables;
    Matrix<Variable*> y_variables;
    Matrix<double> x_costs;
    Matrix<double> y_costs;
};

} // namespace gempp

#endif // V2_SUBGRAPH_ISOMORPHISM_H
