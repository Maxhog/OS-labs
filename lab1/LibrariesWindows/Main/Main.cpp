#include "Number.h"
#include "Vector.h"
#include <iostream>
#include <stdexcept>
#include <windows.h>

void testNumberLibrary() {
    std::cout << "Testing Number Static Library\n";

    Number a(10.0);
    Number b(3.0);

    std::cout << "a + b = " << (a + b).getValue() << "\n";
    std::cout << "a - b = " << (a - b).getValue() << "\n";
    std::cout << "a * b = " << (a * b).getValue() << "\n";
    std::cout << "a / b = " << (a / b).getValue() << "\n";

    try {
        Number zero(0.0);
        std::cout << "a / zero = " << (a / zero).getValue() << "\n";
    }
    catch (const std::runtime_error& e) {
        std::cout << "Division by zero error: " << e.what() << "\n";
    }

    std::cout << "Global ZERO = " << ZERO.getValue() << "\n";
    std::cout << "Global ONE = " << ONE.getValue() << "\n";

    Number c = a;
    std::cout << "c (copy of a) = " << c.getValue() << "\n";
}

void testVectorLibraryDynamic() {
    std::cout << "Testing Vector Dynamic Library (DLL)\n";

    HINSTANCE hDLL = LoadLibrary(L"DynamicLib.dll");
    if (!hDLL) {
        std::cout << "Error: Cannot load DynamicLib.dll\n";
        std::cout << "Make sure Vector.dll is in the same directory as this executable\n";
        return;
    }

    std::cout << "DynamicLib.dll loaded successfully\n";

    typedef Vector* (*AddVectorsFunc)(const Vector*, const Vector*);
    typedef double (*GetLengthFunc)(const Vector*);

    AddVectorsFunc AddVectors = (AddVectorsFunc)GetProcAddress(hDLL, "AddVectors");
    GetLengthFunc CalculateVectorLength = (GetLengthFunc)GetProcAddress(hDLL, "CalculateVectorLength");

    if (!AddVectors || !CalculateVectorLength) {
        std::cout << "Error: Cannot get required functions from DLL\n";
        FreeLibrary(hDLL);
        return;
    }

    std::cout << "All required functions found in DLL\n\n";

    Vector vec1(Number(3.0), Number(4.0));
    Vector vec2(Number(1.0), Number(2.0));

    // Test polar coords
    std::cout << "Testing polar coordinates:\n";
    std::cout << "Vector 1 polar: ";
    vec1.printPolar();
    std::cout << "\n";

    std::cout << "Vector 1 radius: " << vec1.calculateRadius().getValue() << "\n";
    std::cout << "Vector 1 angle: " << vec1.calculateAngle().getValue() << " rad\n";

    std::cout << "Vector 1: (" << vec1.getX().getValue() << ", " << vec1.getY().getValue() << ")\n";
    std::cout << "Vector 2: (" << vec2.getX().getValue() << ", " << vec2.getY().getValue() << ")\n";

    // Calculate lengths DLL
    std::cout << "Calculating lengths\n";
    double len1 = CalculateVectorLength(&vec1);
    double len2 = CalculateVectorLength(&vec2);
    std::cout << "Length of vector 1: " << len1 << "\n";
    std::cout << "Length of vector 2: " << len2 << "\n";

    // Add vectors DLL
    std::cout << "Adding vectors\n";
    Vector* sum = AddVectors(&vec1, &vec2);
    std::cout << "Vector 1 + Vector 2 = (" << sum->getX().getValue() << ", " << sum->getY().getValue() << ")\n";

    // Length of sum
    double sumLength = CalculateVectorLength(sum);
    std::cout << "Length of sum vector: " << sumLength << "\n";

    // Cleanup
    std::cout << "\nCleaning up...\n";
    delete sum;
    std::cout << "Sum vector deleted\n";

    // Unload DLL
    FreeLibrary(hDLL);
    std::cout << "DynamicLib.dll unloaded\n";
}

int main() {
    try {
        testNumberLibrary();
        testVectorLibraryDynamic();

        return 0; 
    }
    catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n";
        return 1; 
    }
}
