#include <iostream>
#include <string>
#include <sstream>

int main() {
    std::string line;
    long long total = 0;

    if (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        long long number;

        while (iss >> number) {
            total += number;
        }
    }

    std::cout << total << std::endl;
    return 0;
}