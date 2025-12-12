#ifndef V2_GLPK_SOLVER_H
#define V2_GLPK_SOLVER_H

#include "../integer_programming/linear_program.h"
#include "../core/types.h"
#include <glpk.h>
#include <unordered_map>

namespace gempp {

class GLPKSolver {
public:
    enum Status {
        OPTIMAL,
        SUBOPTIMAL,
        INFEASIBLE,
        UNBOUNDED,
        NOT_SOLVED
    };

    GLPKSolver() : model_(nullptr), first_feasible_(false) {}

    ~GLPKSolver() {
        if (model_) {
            glp_delete_prob(model_);
            glp_free_env();
        }
    }

    // Callback to terminate after first feasible integer solution
    static void firstFeasibleCallback(glp_tree* tree, void* info) {
        GLPKSolver* solver = static_cast<GLPKSolver*>(info);
        if (solver && solver->first_feasible_) {
            int reason = glp_ios_reason(tree);
            if (reason == GLP_IBINGO) {
                // Found an integer feasible solution - terminate immediately
                glp_ios_terminate(tree);
            }
        }
    }

    void init(LinearProgram* lp, bool verbose = false, bool relaxed = false,
              bool first_feasible = false) {
        if (model_) {
            glp_delete_prob(model_);
        }

        lp_ = lp;
        model_ = glp_create_prob();
        glp_set_prob_name(model_, "gempp");
        relaxed_ = relaxed;
        first_feasible_ = first_feasible;

        glp_init_iocp(&config_);
        config_.msg_lev = verbose ? GLP_MSG_ALL : GLP_MSG_OFF;
        config_.tm_lim = INT_MAX;
        config_.mip_gap = 1e-9;
        config_.presolve = GLP_ON;

        // Set up callback for first-feasible mode
        if (first_feasible_ && !relaxed_) {
            config_.cb_func = firstFeasibleCallback;
            config_.cb_info = this;
            // Keep presolve enabled - it helps find solutions faster
        }

        buildModel();
    }

    double solve(std::unordered_map<std::string, double>& solution) {
        if (!model_) {
            throw Exception("GLPK solver must be initialized before solving");
        }

        Status status = NOT_SOLVED;
        double obj = (lp_->getSense() == LinearProgram::MINIMIZE) ? INFINITY : -INFINITY;

        if (relaxed_) {
            glp_smcp smcp;
            glp_init_smcp(&smcp);
            smcp.msg_lev = config_.msg_lev;
            smcp.tm_lim = config_.tm_lim;

            if (!glp_simplex(model_, &smcp)) {
                switch (glp_get_status(model_)) {
                    case GLP_OPT:
                        status = OPTIMAL;
                        break;
                    case GLP_FEAS:
                        status = SUBOPTIMAL;
                        break;
                    case GLP_INFEAS:
                        status = INFEASIBLE;
                        break;
                    case GLP_UNBND:
                        status = UNBOUNDED;
                        break;
                    default:
                        status = NOT_SOLVED;
                        break;
                }
            }
            if (status == OPTIMAL || status == SUBOPTIMAL) {
                obj = glp_get_obj_val(model_);
                for (const auto& pair : var_order_) {
                    const std::string& var_id = pair.first;
                    int col_idx = pair.second;
                    double value = glp_get_col_prim(model_, col_idx);
                    solution[var_id] = value;
                }
            }
        } else {
            int ret = glp_intopt(model_, &config_);
            // Handle both normal completion (ret==0) and early termination (GLP_ESTOP)
            if (ret == 0 || ret == GLP_ESTOP) {
                switch (glp_mip_status(model_)) {
                    case GLP_OPT:
                        status = OPTIMAL;
                        break;
                    case GLP_FEAS:
                        status = SUBOPTIMAL;
                        break;
                    case GLP_NOFEAS:
                        status = INFEASIBLE;
                        break;
                    case GLP_UNDEF:
                        status = NOT_SOLVED;
                        break;
                }
            }

            if (status == OPTIMAL || status == SUBOPTIMAL) {
                obj = glp_mip_obj_val(model_);

                // Extract solution
                for (const auto& pair : var_order_) {
                    const std::string& var_id = pair.first;
                    int col_idx = pair.second;
                    double value = glp_mip_col_val(model_, col_idx);
                    solution[var_id] = value;
                }
            }
        }

        return obj;
    }

private:
    void buildModel() {
        // Add variables
        for (auto& pair : lp_->getVariables()) {
            Variable* v = pair.second;
            addVariable(v);
        }

        // Add constraints
        for (auto* c : lp_->getConstraints()) {
            addConstraint(c);
        }

        // Set objective
        setObjective();

        // Load matrix
        initMatrix();
    }

    void addVariable(Variable* v) {
        glp_add_cols(model_, 1);
        int idx = var_order_.size() + 1;
        var_order_[v->getID()] = idx;

        glp_set_col_name(model_, idx, v->getID().c_str());
        glp_set_col_bnds(model_, idx, GLP_DB, v->getLowerBound(), v->getUpperBound());

        switch (v->getType()) {
            case Variable::BINARY:
                glp_set_col_kind(model_, idx, GLP_BV);
                break;
            case Variable::BOUNDED:
                glp_set_col_kind(model_, idx, GLP_IV);
                break;
            case Variable::CONTINUOUS:
                glp_set_col_kind(model_, idx, GLP_CV);
                break;
        }
    }

    void addConstraint(LinearConstraint* c) {
        glp_add_rows(model_, 1);
        int idx = const_order_.size() + 1;
        const_order_[c->getID()] = idx;

        glp_set_row_name(model_, idx, c->getID().c_str());

        double bound = c->getRHS() - c->getLinearExpression()->getConst();
        switch (c->getRelation()) {
            case LinearConstraint::LESS_EQ:
                glp_set_row_bnds(model_, idx, GLP_UP, 0, bound);
                break;
            case LinearConstraint::GREATER_EQ:
                glp_set_row_bnds(model_, idx, GLP_LO, bound, 0);
                break;
            case LinearConstraint::EQUAL:
                glp_set_row_bnds(model_, idx, GLP_FX, bound, bound);
                break;
        }

        nz_ += c->getLinearExpression()->getTerms().size();
    }

    void setObjective() {
        switch (lp_->getSense()) {
            case LinearProgram::MAXIMIZE:
                glp_set_obj_dir(model_, GLP_MAX);
                break;
            case LinearProgram::MINIMIZE:
                glp_set_obj_dir(model_, GLP_MIN);
                break;
        }

        LinearExpression* obj = lp_->getObjective();
        for (const auto& pair : obj->getTerms()) {
            Variable* var = pair.first;
            double coeff = pair.second;
            glp_set_obj_coef(model_, var_order_[var->getID()], coeff);
        }
        glp_set_obj_coef(model_, 0, obj->getConst());
    }

    void initMatrix() {
        int* ia = (int*)calloc(1 + nz_, sizeof(int));
        int* ja = (int*)calloc(1 + nz_, sizeof(int));
        double* ar = (double*)calloc(1 + nz_, sizeof(double));

        int cpt = 0;
        for (auto* c : lp_->getConstraints()) {
            LinearExpression* le = c->getLinearExpression();
            for (const auto& pair : le->getTerms()) {
                ++cpt;
                ia[cpt] = const_order_[c->getID()];
                ja[cpt] = var_order_[pair.first->getID()];
                ar[cpt] = pair.second;
            }
        }

        glp_load_matrix(model_, nz_, ia, ja, ar);

        free(ia);
        free(ja);
        free(ar);
    }

    LinearProgram* lp_;
    glp_prob* model_;
    glp_iocp config_;
    bool relaxed_ = false;
    bool first_feasible_ = false;
    std::unordered_map<std::string, int> var_order_;
    std::unordered_map<std::string, int> const_order_;
    int nz_ = 0;
};

} // namespace gempp

#endif // V2_GLPK_SOLVER_H
