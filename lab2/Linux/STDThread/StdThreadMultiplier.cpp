#include "StdThreadMultiplier.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <vector>

StdThreadMultiplier::StdThreadMultiplier() : executionTime(0), threadCount(0) {}

static void multiplySubBlocks(const Matrix& A, const Matrix& B, Matrix& C,
                             int iStart, int iEnd, int jStart, int jEnd,
                             int kStart, int kEnd) {
    for (int i = iStart; i < iEnd; i++) {
        for (int k = kStart; k < kEnd; k++) {
            int aik = A(i, k);
            for (int j = jStart; j < jEnd; j++) {
                C(i, j) += aik * B(k, j);
            }
        }
    }
}

Matrix StdThreadMultiplier::multiply(const Matrix& A, const Matrix& B, int blockSize) {
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

    Matrix result(N, N);
    
    unsigned int hardwareThreads = std::thread::hardware_concurrency();
    if (hardwareThreads == 0) hardwareThreads = 4;  
    
    int numBlocks = (N + blockSize - 1) / blockSize;
    threadCount = std::min(static_cast<int>(hardwareThreads), numBlocks * numBlocks);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    if (threadCount == 1) {
        for (int iBlock = 0; iBlock < numBlocks; iBlock++) {
            for (int jBlock = 0; jBlock < numBlocks; jBlock++) {
                int iStart = iBlock * blockSize;
                int jStart = jBlock * blockSize;
                int iEnd = std::min(iStart + blockSize, N);
                int jEnd = std::min(jStart + blockSize, N);
                
                for (int kBlock = 0; kBlock < numBlocks; kBlock++) {
                    int kStart = kBlock * blockSize;
                    int kEnd = std::min(kStart + blockSize, N);
                    
                    multiplySubBlocks(A, B, result, iStart, iEnd, jStart, jEnd, kStart, kEnd);
                }
            }
        }
    } else {
        std::vector<std::thread> threads;
        int blocksPerThread = (numBlocks * numBlocks + threadCount - 1) / threadCount;
        
        for (int t = 0; t < threadCount; t++) {
            threads.emplace_back([&, t, N, blockSize, numBlocks, blocksPerThread]() {
                int startBlock = t * blocksPerThread;
                int endBlock = std::min(startBlock + blocksPerThread, numBlocks * numBlocks);
                
                for (int blockIdx = startBlock; blockIdx < endBlock; blockIdx++) {
                    int iBlock = blockIdx / numBlocks;
                    int jBlock = blockIdx % numBlocks;
                    
                    int iStart = iBlock * blockSize;
                    int jStart = jBlock * blockSize;
                    int iEnd = std::min(iStart + blockSize, N);
                    int jEnd = std::min(jStart + blockSize, N);
                    
                    for (int kBlock = 0; kBlock < numBlocks; kBlock++) {
                        int kStart = kBlock * blockSize;
                        int kEnd = std::min(kStart + blockSize, N);
                        
                        multiplySubBlocks(A, B, result, iStart, iEnd, jStart, jEnd, kStart, kEnd);
                    }
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    executionTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    return result;
}

long long StdThreadMultiplier::getLastExecutionTime() const {
    return executionTime;
}

int StdThreadMultiplier::getThreadCount() const {
    return threadCount;
}
