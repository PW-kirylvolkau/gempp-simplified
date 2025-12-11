#ifndef V2_MCSM_H
#define V2_MCSM_H

#include "../model/problem.h"
#include "../model/graph.h"
#include "../integer_programming/linear_program.h"
#include "../core/matrix.h"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace gempp {

// Minimum Cost Subgraph Matching (MCSM) Formulation
// This allows partial matches - pattern elements can be unmatched (with cost)
// The objective is the "minimal extension" - cost of unmatched pattern elements
class MinimumCostSubgraphMatching {
public:
    MinimumCostSubgraphMatching(Problem* pb, bool induced = false)
        : pb_(pb), lp_(nullptr), induced_(induced)
    {
        precision_ = 1e-9;
        // Default creation cost for unlabeled elements
        default_creation_cost_ = 1.0;
    }

    ~MinimumCostSubgraphMatching() {
        delete lp_;
    }

    void init(double /* up */ = 1.0) {
        lp_ = new LinearProgram(LinearProgram::MINIMIZE);

        nVP = pb_->getQuery()->getVertexCount();
        nVT = pb_->getTarget()->getVertexCount();
        nEP = pb_->getQuery()->getEdgeCount();
        nET = pb_->getTarget()->getEdgeCount();
        isDirected = pb_->getQuery()->isDirected();

        initVariables();
        initCosts();
        initConstraints();
        initObjective();
    }

    LinearProgram* getLinearProgram() { return lp_; }
    const Matrix<Variable*>& getXVariables() const { return x_variables; }
    const Matrix<Variable*>& getYVariables() const { return y_variables; }

    // Get creation costs for extracting unmatched elements
    double getVertexCreationCost(int i) const { return vertex_creation_costs_[i]; }
    double getEdgeCreationCost(int ij) const { return edge_creation_costs_[ij]; }

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
        // Initialize substitution cost matrices
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

        // Initialize creation costs (cost of NOT matching an element)
        vertex_creation_costs_.resize(nVP, default_creation_cost_);
        edge_creation_costs_.resize(nEP, default_creation_cost_);
    }

    void initConstraints() {
        // Constraint 1: Each pattern vertex maps to AT MOST one target vertex
        // (LESS_EQ instead of EQUAL - allows unmatched pattern vertices)
        for (int i = 0; i < nVP; ++i) {
            LinearExpression* expr = new LinearExpression();
            for (int k = 0; k < nVT; ++k) {
                expr->addTerm(x_variables.getElement(i, k), 1.0);
            }
            std::string id = "vertex_" + std::to_string(i);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::LESS_EQ, 1.0);
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

        // Constraint 3: Each pattern edge maps to AT MOST one target edge
        // (LESS_EQ instead of EQUAL - allows unmatched pattern edges)
        for (int ij = 0; ij < nEP; ++ij) {
            LinearExpression* expr = new LinearExpression();
            for (int kl = 0; kl < nET; ++kl) {
                expr->addTerm(y_variables.getElement(ij, kl), 1.0);
            }
            std::string id = "edge_" + std::to_string(ij);
            auto* c = new LinearConstraint(id, expr, LinearConstraint::LESS_EQ, 1.0);
            lp_->addConstraint(c);
        }

        // Constraint 4 (F2): Edge consistency constraints
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

                for (int i = 0; i < nVP; ++i) {
                    expr->addTerm(x_variables.getElement(i, k), 1.0);
                }
                for (int i = 0; i < nVP; ++i) {
                    expr->addTerm(x_variables.getElement(i, l), 1.0);
                }
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

        // For MCSM: objective = constant - (matched elements' creation costs)
        // constant = sum of all query element creation costs
        // When an element is matched, we subtract its creation cost
        // So: obj = constant + sum(substitution_costs - creation_costs) for matched elements

        // Start with constant term: sum of all creation costs
        double constant = 0.0;
        for (int i = 0; i < nVP; ++i) {
            constant += vertex_creation_costs_[i];
        }
        for (int ij = 0; ij < nEP; ++ij) {
            constant += edge_creation_costs_[ij];
        }
        obj->setConstant(constant);

        // Add vertex terms: (substitution_cost - creation_cost) * x_ik
        // When x_ik = 1 (matched), we add substitution_cost and subtract creation_cost
        for (int i = 0; i < nVP; ++i) {
            for (int k = 0; k < nVT; ++k) {
                double sub_cost = x_costs.getElement(i, k);
                double create_cost = vertex_creation_costs_[i];
                double coeff = sub_cost - create_cost;
                if (std::abs(coeff) > precision_) {
                    obj->addTerm(x_variables.getElement(i, k), coeff);
                }
            }
        }

        // Add edge terms: (substitution_cost - creation_cost) * y_ij_kl
        for (int ij = 0; ij < nEP; ++ij) {
            for (int kl = 0; kl < nET; ++kl) {
                double sub_cost = y_costs.getElement(ij, kl);
                double create_cost = edge_creation_costs_[ij];
                double coeff = sub_cost - create_cost;
                if (std::abs(coeff) > precision_) {
                    obj->addTerm(y_variables.getElement(ij, kl), coeff);
                }
            }
        }

        lp_->setObjective(obj);
    }

    Problem* pb_;
    LinearProgram* lp_;
    bool induced_;
    double precision_;
    double default_creation_cost_;

    int nVP, nVT, nEP, nET;
    bool isDirected;

    Matrix<Variable*> x_variables;
    Matrix<Variable*> y_variables;
    Matrix<double> x_costs;
    Matrix<double> y_costs;

    std::vector<double> vertex_creation_costs_;
    std::vector<double> edge_creation_costs_;
};

} // namespace gempp

#endif // V2_MCSM_H
