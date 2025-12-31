#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <string>

class Matrix {
private:
    std::vector<std::vector<int>> data;
    int rows;
    int cols;

public:
    Matrix();
    Matrix(int r, int c);
    Matrix(const std::vector<std::vector<int>>& d);

    int getRows() const;
    int getCols() const;

    // Access element at position (i,j) for modification
    int& operator()(int i, int j);

    // Access element at position (i,j) for reading only
    const int& operator()(int i, int j) const;

    void randomFill(int min = 1, int max = 10);
    void print(const std::string& name = "", int limit = 6) const;
    bool equals(const Matrix& other) const;
    static Matrix sequentialMultiply(const Matrix& A, const Matrix& B);
};

#endif // MATRIX_H