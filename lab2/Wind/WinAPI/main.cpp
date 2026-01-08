#include "Matrix.h"
#include "WinAPIMultiplier.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <vector>
#include <climits>

// To avoid conflict with min/max macros in Windows
#undef min
#undef max

void testMatrixWithAllBlockSizes(int matrixSize) {
    std::cout << "Matrix size: " << matrixSize << "x" << matrixSize << std::endl;

    Matrix A(matrixSize, matrixSize);
    Matrix B(matrixSize, matrixSize);
    A.randomFill(1, 10);
    B.randomFill(1, 10);

    std::cout << "\n1. Sequential multiplication" << std::endl;
    auto startSeq = std::chrono::high_resolution_clock::now();
    Matrix result = Matrix::sequentialMultiply(A, B);
    auto endSeq = std::chrono::high_resolution_clock::now();
    auto timeSeq = std::chrono::duration_cast<std::chrono::microseconds>(endSeq - startSeq).count();
    std::cout << "Execution time: " << timeSeq << " microseconds" << std::endl;

    std::cout << "\n2. Parallel multiplication with different block sizes" << std::endl;
    std::cout << std::endl;

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

            int blocksPerDim = (matrixSize + blockSize - 1) / blockSize;
            int totalBlocks = blocksPerDim * blocksPerDim;

            double speedup = (timeSeq > 0) ? static_cast<double>(timeSeq) / timePar : 0.0;

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
    std::cout << "Best parallel time: " << bestTime << " microseconds (k=" << bestBlockSize << ")" << std::endl;
    std::cout << "Maximum speedup: " << std::fixed << std::setprecision(2)
        << static_cast<double>(timeSeq) / bestTime << "x" << std::endl;
}

int main() {
    std::cout << "Implementation using Windows API (WinAPI)" << std::endl;
    std::cout << std::string(89, '-') << std::endl;

    testMatrixWithAllBlockSizes(20);
    testMatrixWithAllBlockSizes(100);
    testMatrixWithAllBlockSizes(300);

    return 0;
}