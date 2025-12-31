#include <iostream>
#include <string>
#include <sstream>

// Process A: reads numbers from stdin, adds 10, writes to stdout
int main() {
    std::string line;

    // Read until end of file
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        int number;

        // Process all numbers in the line
        while (iss >> number) {
            int result = number + 10;
            std::cout << result << " ";
        }
    }
    
    std::cout << std::endl;

    return 0;
}
