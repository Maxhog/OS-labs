#include "Matrix.h"
#include "StdThreadMultiplier.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <algorithm>
#include <climits> 

std::vector<int> getBlockSizesToTest(int N) {
    std::vector<int> blockSizes;

    if (N < 100) {
        // For small matrices: test ALL block sizes
        for (int i = 1; i <= N; i++) {
            blockSizes.push_back(i);
        }
    }
    else {
        // For large matrices: test key sizes
        blockSizes.push_back(N);

        // Include divisors and common sizes
        for (int k : {N / 2, N / 4, N / 8, 64, 32, 16, 8, 4, 2}) {
            if (k > 0 && k <= N && std::find(blockSizes.begin(), blockSizes.end(), k) == blockSizes.end()) {
                blockSizes.push_back(k);
            }
        }

        // Include some divisors if they exist
        for (int i = 2; i <= N / 10; i++) {
            if (N % i == 0 && std::find(blockSizes.begin(), blockSizes.end(), i) == blockSizes.end()) {
                blockSizes.push_back(i);
            }
        }

        std::sort(blockSizes.begin(), blockSizes.end());
    }

    return blockSizes;
}

void testStdThreadMultiplication(int matrixSize) {
    std::cout << "Matrix size: " << matrixSize << "x" << matrixSize << std::endl;

    Matrix A(matrixSize, matrixSize);
    Matrix B(matrixSize, matrixSize);
    A.randomFill(1, 10);
    B.randomFill(1, 10);

    // Sequential
    auto startSeq = std::chrono::high_resolution_clock::now();
    Matrix seqResult = Matrix::sequentialMultiply(A, B);
    auto endSeq = std::chrono::high_resolution_clock::now();
    auto timeSeq = std::chrono::duration_cast<std::chrono::microseconds>(endSeq - startSeq).count();

    std::cout << "Sequential time: " << timeSeq << " microseconds" << std::endl;
    std::cout << std::endl;

    // Parallel tests
    std::cout << std::setw(10) << "k"
        << std::setw(12) << "Blocks/Row"
        << std::setw(12) << "Total Blocks"
        << std::setw(15) << "Threads"
        << std::setw(20) << "Time (microseconds)"
        << std::setw(15) << "Speedup" << std::endl;
    std::cout << std::string(84, '-') << std::endl;

    StdThreadMultiplier multiplier;
    long long bestTime = LLONG_MAX;
    int bestBlockSize = 1;

    // Get block sizes to test based on matrix size
    std::vector<int> blockSizes = getBlockSizesToTest(matrixSize);

    for (int blockSize : blockSizes) {
        try {
            Matrix parResult = multiplier.multiply(A, B, blockSize);
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
                << std::setw(20) << timePar
                << std::setw(15) << std::fixed << std::setprecision(2) << speedup
                << std::endl;

        }
        catch (const std::exception& e) {
            std::cout << "Error with k=" << blockSize << ": " << e.what() << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "Best parallel time: " << bestTime << " microseconds (k=" << bestBlockSize << ")" << std::endl;
    std::cout << "Maximum speedup: " << std::fixed << std::setprecision(2)
        << static_cast<double>(timeSeq) / bestTime << "x" << std::endl;
}

int main() {
    std::cout << "Matrix multiplication with std::thread" << std::endl;
    std::cout << std::string(84, '-') << std::endl;

    // Test different matrix sizes
    testStdThreadMultiplication(20);
    std::cout << std::endl << std::string(84, '-') << std::endl << std::endl;

    testStdThreadMultiplication(50);
    std::cout << std::endl << std::string(84, '-') << std::endl << std::endl;

    testStdThreadMultiplication(100);
    std::cout << std::endl << std::string(84, '-') << std::endl << std::endl;

    testStdThreadMultiplication(200);

    return 0;
}
