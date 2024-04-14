# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20

# Source files
SRCS = src/main.cpp


# Executable name
EXEC = bin/gp_threading

# Default target
all: build run

# Create bin directory if it doesn't exist, compile the program
build: $(SRCS)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $(EXEC) $^

# Run the program
run:
	./$(EXEC)

# Remove the compiled files and bin directory
clean:
	rm -rf bin