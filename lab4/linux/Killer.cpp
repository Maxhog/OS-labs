#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <fstream>
#include <cstring>

// Helper function to convert string to lowercase
std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Kill process by PID
bool KillProcessById(pid_t pid) {
    if (kill(pid, SIGKILL) == 0) {
        return true;
    }
    return false;
}

// Get actual process name from /proc filesystem
std::string GetRealProcessName(pid_t pid) {
    // Try to read process name from /proc/[pid]/comm
    std::string commPath = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream commFile(commPath);
    if (commFile.is_open()) {
        std::string name;
        if (std::getline(commFile, name)) {
            return name;
        }
    }
    
    return "";
}

// Kill all processes with matching name
void KillProcessByName(const std::string& targetName) {
    DIR* dir = opendir("/proc");
    if (!dir) return;
    
    struct dirent* entry;
    std::string targetLower = ToLower(targetName);
    bool foundAny = false;
    
    while ((entry = readdir(dir)) != NULL) {
        // Check if directory name is a number (PID)
        if (entry->d_type == DT_DIR) {
            char* endptr;
            long pid = strtol(entry->d_name, &endptr, 10);
            
            if (*endptr == '\0' && pid > 0) {
                std::string procName = GetRealProcessName(pid);
                
                if (!procName.empty()) {
                    std::string procLower = ToLower(procName);
                    
                    // Check if process name contains target or target contains process name
                    if (procLower.find(targetLower) != std::string::npos || 
                        targetLower.find(procLower) != std::string::npos) {
                        
                        if (KillProcessById(pid)) {
                            foundAny = true;
                            std::cout << "Killed: " << procName << " (PID: " << pid << ")" << std::endl;
                        }
                    }
                }
            }
        }
    }
    
    closedir(dir);
    
    if (!foundAny) {
        std::cout << "No process found: " << targetName << std::endl;
    }
}

// Split string by delimiter
std::vector<std::string> SplitString(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Remove whitespace from string
std::string TrimString(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc == 3) {
        std::string param = argv[1];
        
        if (param == "--id") {
            pid_t pid = atoi(argv[2]);
            if (KillProcessById(pid)) {
                std::cout << "Process " << pid << " terminated" << std::endl;
            } else {
                std::cout << "Failed to terminate process " << pid << std::endl;
            }
        } 
        else if (param == "--name") {
            std::string name = argv[2];
            KillProcessByName(name);
        } 
        else {
            std::cout << "Usage: ./Killer --id <PID> or --name <process_name>" << std::endl;
        }
    } 
    else if (argc == 1) {
        // No arguments - use environment variable
        char* proc_to_kill_env = getenv("PROC_TO_KILL");
        
        if (proc_to_kill_env != NULL) {
            std::string proc_to_kill = proc_to_kill_env;
            
            std::vector<std::string> names = SplitString(proc_to_kill, ',');
            for (const std::string& name : names) {
                std::string trimmed = TrimString(name);
                if (!trimmed.empty()) {
                    KillProcessByName(trimmed);
                }
            }
        } else {
            std::cout << "PROC_TO_KILL not found" << std::endl;
            std::cout << "Usage: ./Killer --id <PID> or --name <process_name>" << std::endl;
        }
    } 
    else {
        std::cout << "Usage: ./Killer --id <PID> or --name <process_name>" << std::endl;
    }
    
    return 0;
}
