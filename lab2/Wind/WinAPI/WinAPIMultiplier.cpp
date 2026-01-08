#include "WinAPIMultiplier.h"
#include <iostream>
#include <chrono>
#include <algorithm>

// To avoid conflict with macros min in Windows
#undef min
#undef max

WinAPIMultiplier::WinAPIMultiplier() : executionTime(0), threadCount(0) {
    InitializeCriticalSection(&cs);
}

WinAPIMultiplier::~WinAPIMultiplier() {
    DeleteCriticalSection(&cs);
}

DWORD WINAPI WinAPIMultiplier::MultiplyBlockThread(LPVOID lpParam) {
    ThreadBlockData* data = static_cast<ThreadBlockData*>(lpParam);

    const Matrix& A = *(data->A);
    const Matrix& B = *(data->B);
    Matrix& C = *(data->C);
    int k = data->blockSize;
    int blockRow = data->blockRow;
    int colBlockStart = data->colBlockStart;
    int colBlockEnd = data->colBlockEnd;

    int N = C.getRows(); // Assuming square matrix

    // Process multiple blocks per thread to reduce thread count
    for (int colBlock = colBlockStart; colBlock < colBlockEnd; ++colBlock) {
        int startRow = blockRow * k;
        int startCol = colBlock * k;
        int endRow = std::min(startRow + k, N);
        int endCol = std::min(startCol + k, N);

        // Calculate block
        for (int i = startRow; i < endRow; ++i) {
            for (int j = startCol; j < endCol; ++j) {
                int sum = 0;
                for (int m = 0; m < N; ++m) {
                    sum += A(i, m) * B(m, j);
                }

                // Lock for writing
                EnterCriticalSection(data->cs);
                C(i, j) = sum;
                LeaveCriticalSection(data->cs);
            }
        }
    }

    delete data;
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

    // Calculate number of blocks
    int numBlocksPerDim = (N + blockSize - 1) / blockSize;

    // Limit maximum number of threads (as in your example)
    const int MAX_THREADS = 64;

    // Distribute blocks across threads (each thread processes multiple blocks)
    int totalBlocks = numBlocksPerDim * numBlocksPerDim;
    int numThreads = std::min(totalBlocks, MAX_THREADS);
    threadCount = numThreads;

    // If blocks are too small, use sequential
    if (totalBlocks > 1000) {
        // Too many blocks, use sequential
        auto start = std::chrono::high_resolution_clock::now();
        Matrix result = Matrix::sequentialMultiply(A, B);
        auto end = std::chrono::high_resolution_clock::now();
        executionTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        return result;
    }

    Matrix result(N, N);

    std::vector<HANDLE> threads;
    threads.reserve(numThreads);

    auto start = std::chrono::high_resolution_clock::now();

    // Create threads - each thread processes multiple blocks
    int blocksPerThread = (totalBlocks + numThreads - 1) / numThreads;
    int blockIndex = 0;

    for (int threadId = 0; threadId < numThreads && blockIndex < totalBlocks; ++threadId) {
        int blocksThisThread = std::min(blocksPerThread, totalBlocks - blockIndex);

        // Convert block index to row and column
        int startBlock = blockIndex;
        blockIndex += blocksThisThread;

        ThreadBlockData* data = new ThreadBlockData();
        data->A = &A;
        data->B = &B;
        data->C = &result;
        data->blockSize = blockSize;
        data->cs = &cs;

        // For simplicity, let each thread process blocks from a specific row
        // This is simpler than distributing 2D blocks
        int blocksPerRow = numBlocksPerDim;
        data->blockRow = startBlock / blocksPerRow;
        data->colBlockStart = startBlock % blocksPerRow;
        data->colBlockEnd = std::min(data->colBlockStart + blocksThisThread, blocksPerRow);

        HANDLE thread = CreateThread(
            NULL,
            0,
            MultiplyBlockThread,
            data,
            0,
            NULL
        );

        if (thread == NULL) {
            delete data;
            continue;
        }

        threads.push_back(thread);
    }

    // Wait for all threads to complete
    if (!threads.empty()) {
        WaitForMultipleObjects(
            static_cast<DWORD>(threads.size()),
            threads.data(),
            TRUE,
            INFINITE
        );
    }

    // Close handles
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