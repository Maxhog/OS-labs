#ifndef STDTHREAD_MULTIPLIER_H
#define STDTHREAD_MULTIPLIER_H

#include "Matrix.h"
#include <thread>
#include <vector>
#include <mutex>

class StdThreadMultiplier {
private:
    long long executionTime;
    int threadCount;

    static void multiplyBlock(const Matrix& A, const Matrix& B, Matrix& C,
        int rowBlock, int colBlock, int blockSize, std::mutex& mtx);

public:
    StdThreadMultiplier();

    Matrix multiply(const Matrix& A, const Matrix& B, int blockSize);

    long long getLastExecutionTime() const;
    int getThreadCount() const;
};

#endif