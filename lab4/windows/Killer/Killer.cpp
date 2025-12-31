#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
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

bool KillProcessById(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProcess) return false;
    bool result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    return result;
}

void KillProcessByName(const std::string& name) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cout << "Error: Cannot create process snapshot" << std::endl;
        return;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);
    bool found = false;

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            std::string processName = WideToNarrow(pe.szExeFile);
            std::string lowerName = ToLower(processName);
            std::string lowerTarget = ToLower(name);

            if (lowerName.find(lowerTarget) != std::string::npos) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess) {
                    if (TerminateProcess(hProcess, 0)) {
                        std::cout << "Terminated: " << processName << " (PID: " << pe.th32ProcessID << ")" << std::endl;
                        found = true;
                    }
                    CloseHandle(hProcess);
                }
            }
        } while (Process32NextW(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);

    if (!found) {
        std::cout << "No processes found: " << name << std::endl;
    }
}

std::vector<std::string> SplitString(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string TrimString(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        std::string param = argv[1];
        if (param == "--id") {
            DWORD pid = atoi(argv[2]);
            std::cout << "Terminating process PID: " << pid << std::endl;
            if (KillProcessById(pid)) {
                std::cout << "Success: Process " << pid << " terminated" << std::endl;
            }
            else {
                std::cout << "Error: Failed to terminate process " << pid << std::endl;
            }
        }
        else if (param == "--name") {
            std::string name = argv[2];
            std::cout << "Terminating all processes: " << name << std::endl;
            KillProcessByName(name);
            std::cout << "Finished" << std::endl;
        }
        else {
            std::cout << "Usage: Killer.exe --id <PID> or --name <process_name>" << std::endl;
        }
    }
    else if (argc == 1) {
        char buffer[1024];
        DWORD len = GetEnvironmentVariableA("PROC_TO_KILL", buffer, sizeof(buffer));

        if (len > 0) {
            std::string proc_to_kill = buffer;
            std::cout << "Using PROC_TO_KILL: " << proc_to_kill << std::endl;

            std::vector<std::string> names = SplitString(proc_to_kill, ',');
            for (const std::string& name : names) {
                std::string trimmed = TrimString(name);
                if (!trimmed.empty()) {
                    KillProcessByName(trimmed);
                }
            }
            std::cout << "Finished processing" << std::endl;
        }
        else {
            std::cout << "PROC_TO_KILL not found" << std::endl;
            std::cout << "Usage: Killer.exe --id <PID> or --name <process_name>" << std::endl;
        }
    }
    else {
        std::cout << "Usage: Killer.exe --id <PID> or --name <process_name>" << std::endl;
    }

    return 0;
}