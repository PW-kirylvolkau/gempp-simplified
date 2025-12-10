#ifndef V2_VARIABLE_H
#define V2_VARIABLE_H

#include "../core/types.h"
#include <utility>
#include <unordered_map>

namespace gempp {

class Variable;

// Term represents multiplication of a variable by a coefficient
using Term = std::pair<Variable*, double>;

// Quad represents multiplication of two variables
using Quad = std::pair<Variable*, Variable*>;

// QuadTerm represents multiplication of a Quad by a coefficient
using QuadTerm = std::pair<Quad, double>;

class Variable {
public:
    enum Type {
        BOUNDED,    // bounded integer variable
        BINARY,     // binary variable (0 or 1)
        CONTINUOUS  // continuous variable
    };

    Variable(const std::string& id, Type type = BINARY, int lowerBound = 0, int upperBound = 1)
        : id_(id), type_(type), lowerBound_(lowerBound), upperBound_(upperBound), value_(0)
    {
        if (type == BINARY) {
            lowerBound_ = 0;
            upperBound_ = 1;
        }
    }

    const std::string& getID() const { return id_; }
    void setID(const std::string& id) { id_ = id; }

    Type getType() const { return type_; }
    void setType(Type type) { type_ = type; }

    int getLowerBound() const { return lowerBound_; }
    void setLowerBound(int low) { lowerBound_ = low; }

    int getUpperBound() const { return upperBound_; }
    void setUpperBound(int up) { upperBound_ = up; }

    void activate(int low = 0, int up = 1) {
        if (type_ == BINARY) {
            lowerBound_ = 0;
            upperBound_ = 1;
        } else {
            lowerBound_ = low;
            upperBound_ = up;
        }
    }

    void deactivate() {
        lowerBound_ = 0;
        upperBound_ = 0;
    }

    bool isActive() const {
        return !(lowerBound_ == 0 && upperBound_ == 0);
    }

    void addColumn(const std::string& id, double d) {
        columns_[id] = d;
    }

    std::unordered_map<std::string, double>& getColumns() {
        return columns_;
    }

    double getColumn(const std::string& id) const {
        auto it = columns_.find(id);
        return (it != columns_.end()) ? it->second : 0.0;
    }

    void setValue(int val) { value_ = val; }
    int eval() const { return value_; }

    // Conversion operator to Term
    operator Term() const {
        return std::make_pair(const_cast<Variable*>(this), 1.0);
    }

private:
    std::string id_;
    Type type_;
    int lowerBound_;
    int upperBound_;
    int value_;
    std::unordered_map<std::string, double> columns_;
};

// Helper functions
inline bool operator==(const Quad& q1, const Quad& q2) {
    return (q1.first == q2.first && q1.second == q2.second) ||
           (q1.first == q2.second && q1.second == q2.first);
}

inline bool isActive(const QuadTerm& t) {
    return t.first.first->isActive() && t.first.second->isActive();
}

} // namespace gempp

#endif // V2_VARIABLE_H
