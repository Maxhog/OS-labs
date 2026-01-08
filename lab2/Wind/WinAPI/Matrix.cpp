#include "Matrix.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <stdexcept>
#include <algorithm>

// To avoid conflict with min/max macros in Windows
#undef min
#undef max

Matrix::Matrix() : rows(0), cols(0) {}

Matrix::Matrix(int r, int c) : rows(r), cols(c) {
    if (r < 0 || c < 0) {
        throw std::invalid_argument("Matrix dimensions must be non-negative");
    }
    data.resize(r, std::vector<int>(c, 0));
}

Matrix::Matrix(const std::vector<std::vector<int>>& d) : data(d) {
    rows = d.size();
    if (rows > 0) {
        cols = d[0].size();
        for (const auto& row : data) {
            if (row.size() != cols) {
                throw std::invalid_argument("All matrix rows must have equal length");
            }
        }
    }
    else {
        cols = 0;
    }
}

int Matrix::getRows() const { return rows; }

int Matrix::getCols() const { return cols; }

int& Matrix::operator()(int i, int j) {
    if (i < 0 || i >= rows || j < 0 || j >= cols) {
        throw std::out_of_range("Matrix index out of bounds");
    }
    return data[i][j];
}

const int& Matrix::operator()(int i, int j) const {
    if (i < 0 || i >= rows || j < 0 || j >= cols) {
        throw std::out_of_range("Matrix index out of bounds");
    }
    return data[i][j];
}

void Matrix::randomFill(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            data[i][j] = distrib(gen);
        }
    }
}

void Matrix::print(const std::string& name, int limit) const {
    if (!name.empty()) {
        std::cout << name << " (" << rows << "x" << cols << "):\n";
    }

    int displayRows = std::min(rows, limit);
    int displayCols = std::min(cols, limit);

    for (int i = 0; i < displayRows; i++) {
        for (int j = 0; j < displayCols; j++) {
            std::cout << std::setw(4) << data[i][j];
        }
        if (displayCols < cols) std::cout << " ...";
        std::cout << std::endl;
    }
    if (displayRows < rows) std::cout << "...\n";
}

bool Matrix::equals(const Matrix& other) const {
    if (rows != other.rows || cols != other.cols) return false;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (data[i][j] != other(i, j)) return false;
        }
    }
    return true;
}

Matrix Matrix::sequentialMultiply(const Matrix& A, const Matrix& B) {
    if (A.cols != B.rows) {
        throw std::invalid_argument("Incompatible matrix dimensions for multiplication");
    }

    int M = A.rows;
    int K = A.cols;
    int N = B.cols;

    Matrix result(M, N);

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < K; k++) {
                sum += A(i, k) * B(k, j);
            }
            result(i, j) = sum;
        }
    }

    return result;
}