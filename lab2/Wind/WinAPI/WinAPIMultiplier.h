#ifndef WINAPI_MULTIPLIER_H
#define WINAPI_MULTIPLIER_H

#include "Matrix.h"
#include <windows.h>
#include <vector>

struct ThreadBlockData {
    const Matrix* A;
    const Matrix* B;
    Matrix* C;                 // result matrix
    int blockRow;
    int blockCol;
    int blockSize;
    int matrixSize;
    int numBlocks;

    ThreadBlockData(const Matrix* a, const Matrix* b, Matrix* c,
        int br, int bc, int k, int n)
        : A(a), B(b), C(c), blockRow(br), blockCol(bc),
        blockSize(k), matrixSize(n) {
        numBlocks = (n + k - 1) / k;  // ceil(N/k)
    }
};

class WinAPIMultiplier {
private:
    long long executionTime;
    int threadCount;

    static DWORD WINAPI MultiplyBlockThread(LPVOID lpParam);

public:
    WinAPIMultiplier();

    Matrix multiply(const Matrix& A, const Matrix& B, int blockSize);

    long long getLastExecutionTime() const;
    int getThreadCount() const;
};

#endif // WINAPI_MULTIPLIER_H