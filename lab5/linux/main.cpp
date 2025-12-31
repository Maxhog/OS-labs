#include <iostream>
#include <string>
#include <cstdio>

int main() {
    std::cout << "Process chain: M -> A -> P -> S\n";
    std::cout << "M: multiplies by 7\n";
    std::cout << "A: adds 10 (your number)\n"; 
    std::cout << "P: cubes the number (x^3)\n";
    std::cout << "S: sums all numbers\n";
    std::cout << std::endl;
    
    // Get input data
    std::string input;
    std::cout << "Enter numbers separated by space: ";
    std::getline(std::cin, input);
    
    if (input.empty()) {
        input = "1 2 3";
        std::cout << "Using default: " << input << std::endl;
    }
    
    // Build Linux pipeline command
    // This creates a chain: echo -> M -> A -> P -> S
    // Each process output goes directly to next process input
    std::string cmd = "echo '" + input + "' | ./M | ./A | ./P | ./S";
    
    std::cout << std::endl << "Executing pipeline: echo '" << input << "' | M | A | P | S" << std::endl;
    std::cout <<  std::endl;
    
    // Open pipe and execute the entire pipeline
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: Failed to create pipeline" << std::endl;
        return 1;
    }
    
    // Read the final output from process S
    char buffer[256];
    std::string result;
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    
    // Close pipe and get exit status
    int status = pclose(pipe);
    
    // Display final result
    if (!result.empty()) {
        // Remove trailing newline if present
        if (!result.empty() && result[result.length()-1] == '\n') {
            result.erase(result.length()-1);
        }
        std::cout << "Final result from chain: " << result << std::endl;
    } else {
        std::cerr << "Error: Pipeline returned no result" << std::endl;
        return 1;
    }
    
    return 0;
}
