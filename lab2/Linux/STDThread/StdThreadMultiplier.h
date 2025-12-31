#ifndef STDTHREAD_MULTIPLIER_H
#define STDTHREAD_MULTIPLIER_H

#include "Matrix.h"
#include <thread>
#include <vector>

class StdThreadMultiplier {
private:
    long long executionTime;
    int threadCount;

    struct BlockData {
        const Matrix* A;
        const Matrix* B;
        Matrix* C;
        int rowBlock, colBlock;
        int blockSize, N;
    };

public:
    StdThreadMultiplier();

    Matrix multiply(const Matrix& A, const Matrix& B, int blockSize);

    long long getLastExecutionTime() const;
    int getThreadCount() const;
};

#endif
