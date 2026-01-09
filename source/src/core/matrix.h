#ifndef V2_MATRIX_H
#define V2_MATRIX_H

#include <vector>

namespace gempp {

template <class T>
class Matrix {
public:
    Matrix() : rows_(0), cols_(0) {}

    Matrix(int rows, int cols, T init = T()) : rows_(rows), cols_(cols) {
        matrix_.resize(rows);
        for (int i = 0; i < rows; ++i) {
            matrix_[i].resize(cols, init);
        }
    }

    int getRowsNumber() const { return rows_; }
    int getColumnsNumber() const { return cols_; }

    T getElement(int i, int j) const {
        if (i >= 0 && i < rows_ && j >= 0 && j < cols_) {
            return matrix_[i][j];
        }
        return T();
    }

    void setElement(int i, int j, T value) {
        if (i >= 0 && i < rows_ && j >= 0 && j < cols_) {
            matrix_[i][j] = value;
        }
    }

    std::vector<T>& operator[](int i) {
        return matrix_[i];
    }

    const std::vector<T>& operator[](int i) const {
        return matrix_[i];
    }

private:
    int rows_;
    int cols_;
    std::vector<std::vector<T>> matrix_;
};

} // namespace gempp

#endif // V2_MATRIX_H
