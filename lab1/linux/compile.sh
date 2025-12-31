#!/bin/bash
echo "=== Компиляция проекта для Linux ==="

# Очистка
rm -f *.o *.a *.so lab_linux

# 1. Компиляция Number (статическая библиотека)
echo "1. Компиляция Number..."
g++ -c Number.cpp -std=c++11 -fPIC
ar rcs libnumber.a Number.o

# 2. Компиляция Vector (динамическая библиотека)  
echo "2. Компиляция Vector..."
g++ -c Vector.cpp -std=c++11 -fPIC -DVECTOR_EXPORTS
g++ -shared -o libDynamicLib.so Vector.o -L. -lnumber

# 3. Компиляция Main и линковка
echo "3. Компиляция и сборка..."
g++ -c Main.cpp -std=c++11
g++ Main.o Vector.o -o lab_linux -L. -lnumber -ldl

echo "=== Сборка завершена ==="
echo "Для запуска: ./lab_linux"
