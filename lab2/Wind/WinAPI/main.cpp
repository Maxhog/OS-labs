#include "Matrix.h"
#include "WinAPIMultiplier.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <vector>
#include <cmath>

void testMatrixWithAllBlockSizes(int matrixSize) {
    std::cout << "Matrix size: " << matrixSize << "x" << matrixSize << std::endl;

    // Create matrices
    Matrix A(matrixSize, matrixSize);
    Matrix B(matrixSize, matrixSize);
    A.randomFill(1, 10);
    B.randomFill(1, 10);

    if (matrixSize <= 8) {
        A.print("Matrix A");
        B.print("Matrix B");
    }

    std::cout << "\n1. Sequential multiplication" << std::endl;
    auto startSeq = std::chrono::high_resolution_clock::now();
    Matrix result = Matrix::sequentialMultiply(A, B);
    auto endSeq = std::chrono::high_resolution_clock::now();
    auto timeSeq = std::chrono::duration_cast<std::chrono::microseconds>(endSeq - startSeq).count();
    std::cout << "Execution time: " << timeSeq << " microseconds" << std::endl;

    if (matrixSize <= 8) {
        result.print("Sequential Result");
    }

    std::cout << "\n2. Parallel multiplication with different block sizes" << std::endl;
    std::cout << std::endl;

    // Table header
    std::cout << std::setw(10) << "k (Block)"
        << std::setw(12) << "Blocks/Row"
        << std::setw(12) << "Total Blocks"
        << std::setw(15) << "Threads"
        << std::setw(18) << "Time (microseconds)"
        << std::setw(12) << "Speedup" << std::endl;
    std::cout << std::string(89, '-') << std::endl;

    WinAPIMultiplier multiplier;
    int bestBlockSize = 1;
    long long bestTime = LLONG_MAX;

    for (int blockSize = 1; blockSize <= matrixSize; blockSize++) {
        try {
            Matrix resultAPI = multiplier.multiply(A, B, blockSize);

            long long timePar = multiplier.getLastExecutionTime();
            int threads = multiplier.getThreadCount();

            // Calculate blocks per dimension: ceil(N/k)
            int blocksPerDim = (matrixSize + blockSize - 1) / blockSize;
            int totalBlocks = blocksPerDim * blocksPerDim;

            double speedup = (timeSeq > 0) ? static_cast<double>(timeSeq) / timePar : 0.0;
            bool correct = result.equals(resultAPI);

            if (timePar < bestTime) {
                bestTime = timePar;
                bestBlockSize = blockSize;
            }

            std::cout << std::setw(10) << blockSize
                << std::setw(12) << blocksPerDim
                << std::setw(12) << totalBlocks
                << std::setw(15) << threads
                << std::setw(18) << timePar
                << std::setw(12) << std::fixed << std::setprecision(2) << speedup
                << std::endl;

        }
        catch (const std::exception& e) {
            std::cout << "Error with k=" << blockSize << ": " << e.what() << std::endl;
        }
    }

    std::cout << "\n3. Performance analysis" << std::endl;
    std::cout << std::string(89, '-') << std::endl;
    std::cout << "Matrix size: " << matrixSize << "x" << matrixSize << std::endl;
    std::cout << "Sequential time: " << timeSeq << " microseconds" << std::endl;
    std::cout << "Best parallel time: " << bestTime << " microseconds (k=" << bestBlockSize << ")" << std::endl;
    std::cout << "Maximum speedup: " << std::fixed << std::setprecision(2)
        << static_cast<double>(timeSeq) / bestTime << "x" << std::endl;

    // Theoretical analysis
    std::cout << "\nTheoretical analysis:" << std::endl;
    std::cout << "- k=1: " << matrixSize << "×" << matrixSize << " = "
        << matrixSize * matrixSize << " threads (max parallelism)" << std::endl;
    std::cout << "- k=" << matrixSize << ": 1×1 = 1 thread (sequential)" << std::endl;
    std::cout << "- Optimal block size found: " << bestBlockSize << std::endl;
}

// Test with selected block sizes for larger matrices
void testMatrixWithSelectedBlocks(int matrixSize) {
    std::cout << std::string(89, '-') << std::endl;
    std::cout << "Matrix size: " << matrixSize << "x" << matrixSize << std::endl;
    std::cout << "Testing SELECTED block sizes" << std::endl;
    std::cout << std::string(89, '-') << std::endl;

    Matrix A(matrixSize, matrixSize);
    Matrix B(matrixSize, matrixSize);
    A.randomFill(1, 10);
    B.randomFill(1, 10);

    // Sequential baseline
    auto startSeq = std::chrono::high_resolution_clock::now();
    Matrix C_seq = Matrix::sequentialMultiply(A, B);
    auto endSeq = std::chrono::high_resolution_clock::now();
    auto timeSeq = std::chrono::duration_cast<std::chrono::microseconds>(endSeq - startSeq).count();

    // Selected block sizes to test
    std::vector<int> blockSizes;

    blockSizes.push_back(matrixSize);

    // Include divisors of matrixSize
    for (int i = 10; i <= matrixSize / 2; i++) {
        if (matrixSize % i == 0) {
            blockSizes.push_back(i);
        }
    }

    if (matrixSize >= 8 && std::find(blockSizes.begin(), blockSizes.end(), 4) == blockSizes.end())
        blockSizes.push_back(4);
    if (matrixSize >= 12 && std::find(blockSizes.begin(), blockSizes.end(), 6) == blockSizes.end())
        blockSizes.push_back(6);

    // Sort and remove duplicates
    std::sort(blockSizes.begin(), blockSizes.end());
    blockSizes.erase(std::unique(blockSizes.begin(), blockSizes.end()), blockSizes.end());

    std::cout << "Testing block sizes: ";
    for (size_t i = 0; i < blockSizes.size(); i++) {
        std::cout << blockSizes[i];
        if (i < blockSizes.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    std::cout << "Sequential time: " << timeSeq << " microseconds" << std::endl;
    std::cout << std::endl;

    // Table
    std::cout << std::setw(10) << "Block k"
        << std::setw(12) << "Blocks/Row"
        << std::setw(12) << "Total Blocks"
        << std::setw(15) << "Threads"
        << std::setw(18) << "Time (microseconds)"
        << std::setw(12) << "Speedup" << std::endl;
    std::cout << std::string(89, '-') << std::endl;

    WinAPIMultiplier multiplier;

    for (int blockSize : blockSizes) {
        try {
            Matrix C_par = multiplier.multiply(A, B, blockSize);

            long long timePar = multiplier.getLastExecutionTime();
            int threads = multiplier.getThreadCount();
            int blocksPerDim = (matrixSize + blockSize - 1) / blockSize;
            int totalBlocks = blocksPerDim * blocksPerDim;

            double speedup = (timeSeq > 0) ? static_cast<double>(timeSeq) / timePar : 0.0;
            bool correct = C_seq.equals(C_par);

            std::cout << std::setw(10) << blockSize
                << std::setw(12) << blocksPerDim
                << std::setw(12) << totalBlocks
                << std::setw(15) << threads
                << std::setw(18) << timePar
                << std::setw(12) << std::fixed << std::setprecision(2) << speedup
                << std::setw(15) << std::endl;

        }
        catch (const std::exception& e) {
            std::cout << "Error with k=" << blockSize << ": " << e.what() << std::endl;
        }
    }
}

int main() {
    std::cout << "Implementation using Windows API (WinAPI)" << std::endl;
    std::cout << std::string(89, '-') << std::endl;
    std::cout << "Output: Time on number of blocks/threads" << std::endl;
    std::cout << std::string(89, '-') << std::endl;

    // Small matrix (will show overhead)
    testMatrixWithAllBlockSizes(20);

    // Medium matrices (should show performance gain)
    testMatrixWithSelectedBlocks(100);

    // Large matrices (should definitely show performance gain)
    testMatrixWithSelectedBlocks(300);

    return 0;
}