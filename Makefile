# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Directories
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# Source files
LEXER_SOURCES = $(SRC_DIR)/lexer/token.cpp
PARSER_SOURCES = 
SEMANTIC_SOURCES = 
GUI_SOURCES = 

# Test files
TEST_SOURCES = $(TEST_DIR)/test_main.cpp

# Object files
LEXER_OBJECTS = $(LEXER_SOURCES:.cpp=.o)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

# Targets
.PHONY: all test clean qt-build qt-test

all: qt-build

# Test only the core components (without Qt)
test: $(TEST_OBJECTS) $(LEXER_OBJECTS)
	@echo "Building test executable..."
	$(CXX) $(CXXFLAGS) -o test_runner $(TEST_OBJECTS) $(LEXER_OBJECTS)
	@echo "Running tests..."
	./test_runner

# Build Qt application
qt-build:
	@echo "Building Qt application..."
	qmake complier.pro
	make

# Build and run Qt test
qt-test:
	@echo "Building Qt test..."
	qmake complier.pro CONFIG+=test
	make
	./test_runner

# Compile source files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -f test_runner
	rm -f CompilerFrontend
	rm -f $(LEXER_OBJECTS) $(TEST_OBJECTS)
	rm -f Makefile
	rm -f *.o
	rm -rf $(BUILD_DIR)

# Create directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Help
help:
	@echo "Available targets:"
	@echo "  all      - Build Qt application (default)"
	@echo "  test     - Build and run core component tests"
	@echo "  qt-build - Build Qt application"
	@echo "  qt-test  - Build and run Qt tests"
	@echo "  clean    - Clean all build files"
	@echo "  help     - Show this help message" 