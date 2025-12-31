#include "WinAPIMultiplier.h"
#include <iostream>
#include <chrono>
#include <algorithm>

WinAPIMultiplier::WinAPIMultiplier() : executionTime(0), threadCount(0) {}

DWORD WINAPI WinAPIMultiplier::MultiplyBlockThread(LPVOID lpParam) {
    ThreadBlockData* data = static_cast<ThreadBlockData*>(lpParam);

    const Matrix& A = *(data->A);
    const Matrix& B = *(data->B);
    Matrix& C = *(data->C);
    int k = data->blockSize;
    int N = data->matrixSize;
    int numBlocks = data->numBlocks;

    int rowStart = data->blockRow * k;
    int colStart = data->blockCol * k;
    int rowEnd = (std::min)(rowStart + k, N);
    int colEnd = (std::min)(colStart + k, N);

    for (int i = rowStart; i < rowEnd; i++) {
        for (int j = colStart; j < colEnd; j++) {
            int sum = 0;
            for (int m = 0; m < N; m++) {
                sum += A(i, m) * B(m, j);
            }
            C(i, j) = sum;
        }
    }

    return 0;
}

Matrix WinAPIMultiplier::multiply(const Matrix& A, const Matrix& B, int blockSize) {
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

    int numBlocks = (N + blockSize - 1) / blockSize;
    threadCount = numBlocks * numBlocks;

    Matrix result(N, N);

    std::vector<ThreadBlockData> threadData;
    std::vector<HANDLE> threads;
    threadData.reserve(threadCount);
    threads.reserve(threadCount);

    auto start = std::chrono::high_resolution_clock::now();

    for (int rowBlock = 0; rowBlock < numBlocks; rowBlock++) {
        for (int colBlock = 0; colBlock < numBlocks; colBlock++) {
            threadData.emplace_back(&A, &B, &result, rowBlock, colBlock, blockSize, N);
            HANDLE thread = CreateThread(
                NULL,
                0,
                MultiplyBlockThread,
                &threadData.back(),
                0,
                NULL
            );

            if (thread == NULL) {
                std::cerr << "Error creating thread for block ["
                    << rowBlock << "][" << colBlock
                    << "]. Error: " << GetLastError() << std::endl;
                threadData.pop_back();
                continue;
            }

            threads.push_back(thread);
        }
    }

    if (!threads.empty()) {
        const DWORD MAX_WAIT = 64;
        for (size_t i = 0; i < threads.size(); i += MAX_WAIT) {
            size_t batchSize = (std::min<size_t>)(MAX_WAIT, threads.size() - i);
            WaitForMultipleObjects(
                static_cast<DWORD>(batchSize),
                &threads[i],
                TRUE,
                INFINITE
            );
        }
    }

    for (HANDLE thread : threads) {
        CloseHandle(thread);
    }

    auto end = std::chrono::high_resolution_clock::now();
    executionTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    return result;
}

long long WinAPIMultiplier::getLastExecutionTime() const {
    return executionTime;
}

int WinAPIMultiplier::getThreadCount() const {
    return threadCount;
}