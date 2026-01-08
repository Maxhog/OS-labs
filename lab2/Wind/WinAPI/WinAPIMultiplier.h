#ifndef WINAPI_MULTIPLIER_H
#define WINAPI_MULTIPLIER_H

#include "Matrix.h"
#include <windows.h>
#include <vector>

struct ThreadBlockData {
    const Matrix* A;
    const Matrix* B;
    Matrix* C;
    int blockSize;
    int blockRow;
    int colBlockStart;
    int colBlockEnd;
    CRITICAL_SECTION* cs;

    ThreadBlockData() = default;
};

class WinAPIMultiplier {
private:
    long long executionTime;
    int threadCount;
    CRITICAL_SECTION cs;

    static DWORD WINAPI MultiplyBlockThread(LPVOID lpParam);

public:
    WinAPIMultiplier();
    ~WinAPIMultiplier();

    Matrix multiply(const Matrix& A, const Matrix& B, int blockSize);

    long long getLastExecutionTime() const;
    int getThreadCount() const;
};

#endif // WINAPI_MULTIPLIER_H