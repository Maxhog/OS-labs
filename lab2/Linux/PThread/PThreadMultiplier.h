#ifndef PTHREAD_MULTIPLIER_H
#define PTHREAD_MULTIPLIER_H

#include "Matrix.h"
#include <pthread.h>
#include <vector>

class PThreadMultiplier {
private:
    long long executionTime;
    int threadCount;
    
    struct ThreadData {
        const Matrix* A;
        const Matrix* B;
        Matrix* C;
        int blockSize;
        int N;
        int numBlocks;
        pthread_mutex_t* mutex;
        int* nextBlock;
    };
    
    static void* threadFunction(void* arg);
    static void computeBlock(const Matrix& A, const Matrix& B, Matrix& C,
                            int rowBlock, int colBlock, int blockSize, int N);

public:
    PThreadMultiplier();
    ~PThreadMultiplier();
    
    Matrix multiply(const Matrix& A, const Matrix& B, int blockSize);
    
    long long getLastExecutionTime() const;
    int getThreadCount() const;
};

#endif // PTHREAD_MULTIPLIER_H
