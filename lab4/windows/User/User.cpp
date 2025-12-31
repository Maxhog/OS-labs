#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cctype>

std::string WideToNarrow(const wchar_t* wideStr) {
    if (!wideStr) return "";
    int size = WideCharToMultiByte(CP_ACP, 0, wideStr, -1, NULL, 0, NULL, NULL);
    if (size == 0) return "";
    char* buffer = new char[size];
    WideCharToMultiByte(CP_ACP, 0, wideStr, -1, buffer, size, NULL, NULL);
    std::string result(buffer);
    delete[] buffer;
    return result;
}

std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool StartProcess(const std::string& path, DWORD& pid) {
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    char* cmdLine = new char[path.length() + 1];
    strcpy_s(cmdLine, path.length() + 1, path.c_str());

    if (CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        pid = pi.dwProcessId;
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        delete[] cmdLine;
        return true;
    }

    delete[] cmdLine;
    return false;
}

bool IsProcessRunningByID(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProcess) return false;

    DWORD exitCode;
    GetExitCodeProcess(hProcess, &exitCode);
    CloseHandle(hProcess);

    return (exitCode == STILL_ACTIVE);
}

bool IsProcessRunningByName(const std::string& name, DWORD& foundPid) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            std::string processName = WideToNarrow(pe.szExeFile);
            if (ToLower(processName).find(ToLower(name)) != std::string::npos) {
                foundPid = pe.th32ProcessID;
                CloseHandle(hSnapshot);
                return true;
            }
        } while (Process32NextW(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return false;
}

bool RunKiller(const std::string& args) {
    std::string command = "Killer.exe " + args;

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    char* cmdLine = new char[command.length() + 1];
    strcpy_s(cmdLine, command.length() + 1, command.c_str());

    bool success = false;
    if (CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        success = true;
    }

    delete[] cmdLine;
    return success;
}

int main() {
    std::cout << "Starting Killer tests..." << std::endl;

    std::string proc_to_kill = "notepad.exe,Calculator";
    SetEnvironmentVariableA("PROC_TO_KILL", proc_to_kill.c_str());
    std::cout << "Set PROC_TO_KILL = " << proc_to_kill << std::endl;

    DWORD pid_notepad = 0, pid_calc = 0;

    if (StartProcess("notepad.exe", pid_notepad)) {
        std::cout << "Started notepad.exe (PID: " << pid_notepad << ")" << std::endl;
    }

    if (StartProcess("calc.exe", pid_calc)) {
        std::cout << "Started calc.exe (PID: " << pid_calc << ")" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    //Kill by ID
    std::cout << "Test 1: Kill by PID..." << std::endl;
    if (pid_notepad > 0) {
        RunKiller("--id " + std::to_string(pid_notepad));
        if (!IsProcessRunningByID(pid_notepad)) {
            std::cout << "Success: Notepad terminated by PID" << std::endl;
        }
    }

    // Kill by name
    std::cout << "Test 2: Kill by name..." << std::endl;
    RunKiller("--name Calculator");

    // Kill via environment variable
    std::cout << "Test 3: Kill via PROC_TO_KILL..." << std::endl;

    // Restart processes
    StartProcess("notepad.exe", pid_notepad);
    StartProcess("calc.exe", pid_calc);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    RunKiller("");

    SetEnvironmentVariableA("PROC_TO_KILL", NULL);
    std::cout << "Deleted PROC_TO_KILL variable" << std::endl;

    std::cout << "Final test (should show help)..." << std::endl;
    RunKiller("");

    std::cout << "All tests completed" << std::endl;
    return 0;
}