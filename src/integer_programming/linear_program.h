#ifndef V2_LINEAR_PROGRAM_H
#define V2_LINEAR_PROGRAM_H

#include "variable.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace gempp {

// Linear Expression: sum of (coefficient * variable) + constant
class LinearExpression {
public:
    LinearExpression() : constant_(0.0) {}

    void addTerm(Variable* var, double coeff) {
        if (var) {
            terms_[var] += coeff;
        }
    }

    void setConstant(double c) { constant_ = c; }
    double getConst() const { return constant_; }

    const std::unordered_map<Variable*, double>& getTerms() const {
        return terms_;
    }

    std::unordered_map<Variable*, double>& getTerms() {
        return terms_;
    }

private:
    std::unordered_map<Variable*, double> terms_;
    double constant_;
};

// Linear Constraint
class LinearConstraint {
public:
    enum Relation {
        LESS_EQ,
        GREATER_EQ,
        EQUAL
    };

    LinearConstraint(const std::string& id, LinearExpression* expr, Relation rel, double rhs)
        : id_(id), expr_(expr), relation_(rel), rhs_(rhs)
    {}

    ~LinearConstraint() {
        delete expr_;
    }

    const std::string& getID() const { return id_; }
    LinearExpression* getLinearExpression() const { return expr_; }
    Relation getRelation() const { return relation_; }
    double getRHS() const { return rhs_; }

private:
    std::string id_;
    LinearExpression* expr_;
    Relation relation_;
    double rhs_;
};

// Linear Program
class LinearProgram {
public:
    enum Sense {
        MINIMIZE,
        MAXIMIZE
    };

    explicit LinearProgram(Sense sense) : sense_(sense), objective_(new LinearExpression()) {}

    ~LinearProgram() {
        for (auto& pair : variables_) {
            delete pair.second;
        }
        for (auto* constraint : constraints_) {
            delete constraint;
        }
        delete objective_;
    }

    // Disable copy
    LinearProgram(const LinearProgram&) = delete;
    LinearProgram& operator=(const LinearProgram&) = delete;

    Sense getSense() const { return sense_; }
    void setSense(Sense sense) { sense_ = sense; }

    void addVariable(Variable* v) {
        if (v && variables_.find(v->getID()) == variables_.end()) {
            variables_[v->getID()] = v;
        }
    }

    Variable* getVariable(const std::string& id) const {
        auto it = variables_.find(id);
        return (it != variables_.end()) ? it->second : nullptr;
    }

    const std::unordered_map<std::string, Variable*>& getVariables() const {
        return variables_;
    }

    std::unordered_map<std::string, Variable*>& getVariables() {
        return variables_;
    }

    void addConstraint(LinearConstraint* c) {
        if (c) {
            constraints_.push_back(c);
        }
    }

    const std::vector<LinearConstraint*>& getConstraints() const {
        return constraints_;
    }

    std::vector<LinearConstraint*>& getConstraints() {
        return constraints_;
    }

    LinearExpression* getObjective() const { return objective_; }

    void setObjective(LinearExpression* obj) {
        delete objective_;
        objective_ = obj;
    }

private:
    Sense sense_;
    std::unordered_map<std::string, Variable*> variables_;
    std::vector<LinearConstraint*> constraints_;
    LinearExpression* objective_;
};

} // namespace gempp

#endif // V2_LINEAR_PROGRAM_H
