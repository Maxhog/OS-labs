#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

// Process P: reads numbers from stdin, calculates x^3, writes to stdout
int main() {
    std::string line;

    // Read until end of file
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        int number;

        // Process all numbers in the line
        while (iss >> number) {
            int result = number * number * number;  // x^3
            std::cout << result << " ";
        }
    }
    
    std::cout << std::endl;

    return 0;
}
