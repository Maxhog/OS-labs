# Базовые настройки
CXX = g++
RELEASE_FLAGS = -std=c++11 -pthread -Wall -Wextra -O2 -Wno-sign-compare
DEBUG_FLAGS = -std=c++11 -pthread -Wall -Wextra -g -Wno-sign-compare

# По умолчанию
CXXFLAGS = $(RELEASE_FLAGS)
TARGET = matrix_multiply_pthread

# Объектные файлы
OBJS = main.o Matrix.o PThreadMultiplier.o

# Паттерн rule для компиляции .cpp в .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Зависимости заголовочных файлов
main.o: Matrix.h PThreadMultiplier.h
Matrix.o: Matrix.h
PThreadMultiplier.o: PThreadMultiplier.h Matrix.h

# Основная цель
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# Отдельные цели для разных типов сборки
debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: $(TARGET)

release: CXXFLAGS = $(RELEASE_FLAGS)
release: clean $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run debug release
