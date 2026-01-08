#include "StdThreadMultiplier.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <vector>
#include <mutex>

StdThreadMultiplier::StdThreadMultiplier() : executionTime(0), threadCount(0) {}

void StdThreadMultiplier::multiplyBlock(const Matrix& A, const Matrix& B, Matrix& C,
    int rowBlock, int colBlock, int blockSize, std::mutex& mtx) {

    int N = A.getRows();
    int startRow = rowBlock * blockSize;
    int startCol = colBlock * blockSize;
    int endRow = std::min(startRow + blockSize, N);
    int endCol = std::min(startCol + blockSize, N);

    for (int i = startRow; i < endRow; ++i) {
        for (int j = startCol; j < endCol; ++j) {
            int sum = 0;
            for (int k = 0; k < N; ++k) {
                sum += A(i, k) * B(k, j);
            }

            // Lock for writing
            std::lock_guard<std::mutex> lock(mtx);
            C(i, j) = sum;
        }
    }
}

Matrix StdThreadMultiplier::multiply(const Matrix& A, const Matrix& B, int blockSize) {
    if (A.getCols() != B.getRows()) {
        throw std::invalid_argument("Incompatible matrix sizes for multiplication");
    }

    if (blockSize <= 0) {
        throw std::invalid_argument("Block size must be positive");
    }

    int N = A.getRows();
    if (B.getCols() != N || A.getCols() != N) {
        throw std::invalid_argument("Matrices must be square and same size");
    }

    Matrix result(N, N);
    std::mutex mtx;

    // Calculate number of blocks
    int numRowBlocks = (N + blockSize - 1) / blockSize;
    int numColBlocks = (N + blockSize - 1) / blockSize;
    int totalBlocks = numRowBlocks * numColBlocks;

    // Limit maximum threads to reasonable number
    unsigned int maxHardwareThreads = std::thread::hardware_concurrency();
    if (maxHardwareThreads == 0) maxHardwareThreads = 4;

    // Don't create too many threads for small blocks
    if (totalBlocks > 1000) {
        // Too many blocks, use sequential for this test
        auto start = std::chrono::high_resolution_clock::now();
        result = Matrix::sequentialMultiply(A, B);
        auto end = std::chrono::high_resolution_clock::now();
        executionTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        threadCount = 1;
        return result;
    }

    auto start = std::chrono::high_resolution_clock::now();

    if (totalBlocks <= static_cast<int>(maxHardwareThreads)) {
        // Create one thread per block
        threadCount = totalBlocks;
        std::vector<std::thread> threads;
        threads.reserve(totalBlocks);

        for (int rowBlock = 0; rowBlock < numRowBlocks; ++rowBlock) {
            for (int colBlock = 0; colBlock < numColBlocks; ++colBlock) {
                threads.emplace_back(multiplyBlock,
                    std::cref(A), std::cref(B), std::ref(result),
                    rowBlock, colBlock, blockSize, std::ref(mtx));
            }
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }
    else {
        // Distribute blocks among limited number of threads
        threadCount = maxHardwareThreads;
        std::vector<std::thread> threads;
        threads.reserve(threadCount);

        int blocksPerThread = (totalBlocks + threadCount - 1) / threadCount;

        for (int t = 0; t < threadCount; ++t) {
            threads.emplace_back([&, t, blocksPerThread, numRowBlocks, numColBlocks]() {
                int startBlock = t * blocksPerThread;
                int endBlock = std::min(startBlock + blocksPerThread, totalBlocks);

                for (int blockIdx = startBlock; blockIdx < endBlock; ++blockIdx) {
                    int rowBlock = blockIdx / numColBlocks;
                    int colBlock = blockIdx % numColBlocks;

                    int startRow = rowBlock * blockSize;
                    int startCol = colBlock * blockSize;
                    int endRow = std::min(startRow + blockSize, N);
                    int endCol = std::min(startCol + blockSize, N);

                    for (int i = startRow; i < endRow; ++i) {
                        for (int j = startCol; j < endCol; ++j) {
                            int sum = 0;
                            for (int k = 0; k < N; ++k) {
                                sum += A(i, k) * B(k, j);
                            }

                            std::lock_guard<std::mutex> lock(mtx);
                            result(i, j) = sum;
                        }
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