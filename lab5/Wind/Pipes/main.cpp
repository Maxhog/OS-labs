#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

bool runProcess(const std::wstring& cmd, HANDLE hStdIn,
    HANDLE hStdOut, PROCESS_INFORMATION& pi) {
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = hStdIn;
    si.hStdOutput = hStdOut;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    ZeroMemory(&pi, sizeof(pi));

    // Create modifiable copy of command line
    std::vector<wchar_t> cmdCopy(cmd.begin(), cmd.end());
    cmdCopy.push_back(L'\0');

    BOOL ok = CreateProcessW(
        NULL,
        cmdCopy.data(),
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (!ok) {
        std::cerr << "Failed to create process: " << cmd.c_str()
            << " Error: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

int main() {
    // Security attributes for inheritable handles
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    // Process information structures
    PROCESS_INFORMATION piM, piA, piP, piS;

    // Pipe handles: Main->M, M->A, A->P, P->S, S->Main
    HANDLE hMainToM_Read, hMainToM_Write;
    HANDLE hMtoA_Read, hMtoA_Write;
    HANDLE hAtoP_Read, hAtoP_Write;
    HANDLE hPtoS_Read, hPtoS_Write;
    HANDLE hStoMain_Read, hStoMain_Write;

    // Create pipe for Main -> M
    if (!CreatePipe(&hMainToM_Read, &hMainToM_Write, &sa, 0)) {
        std::cerr << "Failed to create pipe Main->M" << std::endl;
        return 1;
    }
    SetHandleInformation(hMainToM_Write, HANDLE_FLAG_INHERIT, 0);

    // Create pipe for M -> A
    if (!CreatePipe(&hMtoA_Read, &hMtoA_Write, &sa, 0)) {
        std::cerr << "Failed to create pipe M->A" << std::endl;
        return 1;
    }

    // Start process M (reads from Main, writes to A)
    if (!runProcess(L"M.exe", hMainToM_Read, hMtoA_Write, piM)) {
        return 1;
    }
    CloseHandle(hMainToM_Read);
    CloseHandle(hMtoA_Write);

    // Create pipe for A -> P
    if (!CreatePipe(&hAtoP_Read, &hAtoP_Write, &sa, 0)) {
        std::cerr << "Failed to create pipe A->P" << std::endl;
        return 1;
    }

    // Start process A (reads from M, writes to P)
    if (!runProcess(L"A.exe", hMtoA_Read, hAtoP_Write, piA)) {
        return 1;
    }
    CloseHandle(hMtoA_Read);
    CloseHandle(hAtoP_Write);

    // Create pipe for P -> S
    if (!CreatePipe(&hPtoS_Read, &hPtoS_Write, &sa, 0)) {
        std::cerr << "Failed to create pipe P->S" << std::endl;
        return 1;
    }

    // Start process P (reads from A, writes to S)
    if (!runProcess(L"P.exe", hAtoP_Read, hPtoS_Write, piP)) {
        return 1;
    }
    CloseHandle(hAtoP_Read);
    CloseHandle(hPtoS_Write);

    // Create pipe for S -> Main
    if (!CreatePipe(&hStoMain_Read, &hStoMain_Write, &sa, 0)) {
        std::cerr << "Failed to create pipe S->Main" << std::endl;
        return 1;
    }
    SetHandleInformation(hStoMain_Read, HANDLE_FLAG_INHERIT, 0);

    // Start process S (reads from P, writes to Main)
    if (!runProcess(L"S.exe", hPtoS_Read, hStoMain_Write, piS)) {
        return 1;
    }
    CloseHandle(hPtoS_Read);
    CloseHandle(hStoMain_Write);

    // Get input data
    std::string inputData;
    std::cout << "Enter numbers separated by space: ";
    std::getline(std::cin, inputData);

    if (inputData.empty()) {
        inputData = "1 2 3";
        std::cout << "Using default: " << inputData << std::endl;
    }
    inputData += '\n';

    // Send data to process M
    DWORD bytesWritten;
    if (!WriteFile(hMainToM_Write, inputData.c_str(),
        (DWORD)inputData.length(), &bytesWritten, NULL)) {
        std::cerr << "Failed to send data to M" << std::endl;
    }
    CloseHandle(hMainToM_Write);

    // Read result from process S
    std::string result;
    char buffer[256];
    DWORD bytesRead;

    // Read all available data
    while (ReadFile(hStoMain_Read, buffer, sizeof(buffer) - 1,
        &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result += buffer;
    }
    CloseHandle(hStoMain_Read);

    // Wait for all processes to complete
    WaitForSingleObject(piM.hProcess, INFINITE);
    WaitForSingleObject(piA.hProcess, INFINITE);
    WaitForSingleObject(piP.hProcess, INFINITE);
    WaitForSingleObject(piS.hProcess, INFINITE);

    // Clean up trailing whitespace and display result
    if (!result.empty()) {
        size_t end = result.find_last_not_of(" \n\r\t");
        if (end != std::string::npos) {
            result = result.substr(0, end + 1);
        }
        std::cout << "Result: " << result << std::endl;
    }
    else {
        std::cerr << "Processing chain returned no result" << std::endl;
    }

    // Clean up process handles
    CloseHandle(piM.hProcess);
    CloseHandle(piM.hThread);
    CloseHandle(piA.hProcess);
    CloseHandle(piA.hThread);
    CloseHandle(piP.hProcess);
    CloseHandle(piP.hThread);
    CloseHandle(piS.hProcess);
    CloseHandle(piS.hThread);

    return 0;
}