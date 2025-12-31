#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <thread>
#include <cstring>

// Start process and return PID
bool StartProcess(const std::string& command, pid_t& pid) {
    pid = fork();
    
    if (pid == 0) {
        // Child process
        execlp(command.c_str(), command.c_str(), NULL);
        exit(1);
    } else if (pid > 0) {
        return true;
    }
    
    return false;
}

// Run Killer with arguments
void RunKiller(const std::string& args) {
    pid_t pid = fork();
    
    if (pid == 0) {
        std::string command = "./Killer " + args;
        system(command.c_str());
        exit(0);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    }
}

// Check if process is running
bool IsProcessRunning(pid_t pid) {
    return (kill(pid, 0) == 0);
}

int main() {
    std::cout << "Starting Killer tests" << std::endl;
    
    // Set environment variable
    std::string proc_to_kill = "gedit,gnome-calculator";
    setenv("PROC_TO_KILL", proc_to_kill.c_str(), 1);
    std::cout << "Set PROC_TO_KILL = " << proc_to_kill << std::endl;
    
    // Start test processes
    pid_t pid_gedit = 0, pid_calc = 0;
    
    // Start gedit
    if (system("gedit &") == 0) {
        std::cout << "Started gedit" << std::endl;
    }
    
    // Start calculator
    if (system("gnome-calculator &") == 0) {
        std::cout << "Started calculator" << std::endl;
    }
    
    // Wait for processes to start
    sleep(2);
    
    // Find PID of gedit using pgrep
    FILE* pipe = popen("pgrep gedit", "r");
    if (pipe) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            pid_gedit = atoi(buffer);
            std::cout << "Found gedit PID: " << pid_gedit << std::endl;
        }
        pclose(pipe);
    }
    
    // Kill by PID
    std::cout << "Test 1: Kill by PID" << std::endl;
    if (pid_gedit > 0) {
        RunKiller("--id " + std::to_string(pid_gedit));
    }
    
    // Kill by name
    std::cout << "Test 2: Kill by name" << std::endl;
    RunKiller("--name gnome-calculator");
    
    // Kill via environment variable
    std::cout << "Test 3: Kill via PROC_TO_KILL" << std::endl;
    
    // Restart processes
    system("gedit &");
    system("gnome-calculator &");
    sleep(2);
    
    RunKiller("");
    
    // Cleanup
    unsetenv("PROC_TO_KILL");
    std::cout << "Deleted PROC_TO_KILL variable" << std::endl;
    
    // Kill any remaining processes
    system("pkill gedit 2>/dev/null");
    system("pkill gnome-calculator 2>/dev/null");
    
    // Final test
    std::cout << "Final test" << std::endl;
    RunKiller("");
    
    std::cout << "All tests completed" << std::endl;
    
    return 0;
}
