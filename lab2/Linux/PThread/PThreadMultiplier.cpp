#include "PThreadMultiplier.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>
#include <cstring>
#include <thread>

PThreadMultiplier::PThreadMultiplier() : executionTime(0), threadCount(0) {}

PThreadMultiplier::~PThreadMultiplier() {}

void PThreadMultiplier::computeBlock(const Matrix& A, const Matrix& B, Matrix& C,
                                     int rowBlock, int colBlock, int blockSize, int N) {
    int rowStart = rowBlock * blockSize;
    int colStart = colBlock * blockSize;
    int rowEnd = std::min(rowStart + blockSize, N);
    int colEnd = std::min(colStart + blockSize, N);
    
    int numBlocks = (N + blockSize - 1) / blockSize;
    
    std::vector<std::vector<int>> tempBlock(rowEnd - rowStart, 
                                            std::vector<int>(colEnd - colStart, 0));
    
    for (int kBlock = 0; kBlock < numBlocks; ++kBlock) {
        int kStart = kBlock * blockSize;
        int kEnd = std::min(kStart + blockSize, N);
        
        for (int i = rowStart; i < rowEnd; ++i) {
            for (int k = kStart; k < kEnd; ++k) {
                int aik = A(i, k);
                for (int j = colStart; j < colEnd; ++j) {
                    tempBlock[i - rowStart][j - colStart] += aik * B(k, j);
                }
            }
        }
    }
    
    for (int i = rowStart; i < rowEnd; ++i) {
        for (int j = colStart; j < colEnd; ++j) {
            C(i, j) = tempBlock[i - rowStart][j - colStart];
        }
    }
}

void* PThreadMultiplier::threadFunction(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    const Matrix& A = *(data->A);
    const Matrix& B = *(data->B);
    Matrix& C = *(data->C);
    int blockSize = data->blockSize;
    int N = data->N;
    int numBlocks = data->numBlocks;
    
    int blockIdx;
    while (true) {
        pthread_mutex_lock(data->mutex);
        blockIdx = *(data->nextBlock);
        *(data->nextBlock) = blockIdx + 1;
        pthread_mutex_unlock(data->mutex);
        
        if (blockIdx >= numBlocks * numBlocks) {
            break;
        }
        
        int rowBlock = blockIdx / numBlocks;
        int colBlock = blockIdx % numBlocks;
        
        computeBlock(A, B, C, rowBlock, colBlock, blockSize, N);
    }
    
    return nullptr;
}

Matrix PThreadMultiplier::multiply(const Matrix& A, const Matrix& B, int blockSize) {
    if (A.getCols() != B.getRows()) {
        throw std::invalid_argument("Incompatible matrix sizes");
    }

    if (blockSize <= 0) {
        throw std::invalid_argument("Block size must be positive");
    }

    int N = A.getRows();
    if (B.getCols() != N || A.getCols() != N) {
        throw std::invalid_argument("Matrices must be square");
    }

    int numBlocks = (N + blockSize - 1) / blockSize;
    int totalBlocks = numBlocks * numBlocks;
    
    Matrix result(N, N);
    
    threadCount = totalBlocks;
    
    unsigned int maxThreads = std::thread::hardware_concurrency() * 2;
    if (maxThreads == 0) maxThreads = 8;
    
    if (threadCount > maxThreads) {
        threadCount = maxThreads;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, nullptr);
    int nextBlock = 0;
    
    std::vector<ThreadData> threadData(threadCount);
    std::vector<pthread_t> threads(threadCount);
    
    for (int i = 0; i < threadCount; i++) {
        threadData[i].A = &A;
        threadData[i].B = &B;
        threadData[i].C = &result;
        threadData[i].blockSize = blockSize;
        threadData[i].N = N;
        threadData[i].numBlocks = numBlocks;
        threadData[i].mutex = &mutex;
        threadData[i].nextBlock = &nextBlock;
        
        if (pthread_create(&threads[i], nullptr, threadFunction, &threadData[i]) != 0) {
            throw std::runtime_error("Failed to create thread");
        }
    }
    
    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    pthread_mutex_destroy(&mutex);
    
    auto end = std::chrono::high_resolution_clock::now();
    executionTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    return result;
}

long long PThreadMultiplier::getLastExecutionTime() const {
    return executionTime;
}

int PThreadMultiplier::getThreadCount() const {
    return threadCount;
}
