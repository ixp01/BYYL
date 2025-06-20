# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Directories
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# Source files
LEXER_SOURCES = $(SRC_DIR)/lexer/token.cpp $(SRC_DIR)/lexer/dfa.cpp $(SRC_DIR)/lexer/lexer.cpp $(SRC_DIR)/lexer/minimizer.cpp
PARSER_SOURCES = 
SEMANTIC_SOURCES = 
GUI_SOURCES = 

# Test files
TEST_SOURCES = $(TEST_DIR)/test_main.cpp $(TEST_DIR)/test_lexer.cpp

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

# Test only lexer components
test-lexer: tests/test_lexer.o $(LEXER_OBJECTS)
	@echo "Building lexer test executable..."
	$(CXX) $(CXXFLAGS) -o test_lexer tests/test_lexer.o $(LEXER_OBJECTS)
	@echo "Running lexer tests..."
	./test_lexer

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

# Compile individual source files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -f test_runner test_lexer
	rm -f CompilerFrontend
	rm -f $(LEXER_OBJECTS) $(TEST_OBJECTS)
	rm -f Makefile
	rm -f *.o
	rm -rf build

# Help target
help:
	@echo "Available targets:"
	@echo "  all        - Build Qt application (default)"
	@echo "  test       - Build and run all tests"
	@echo "  test-lexer - Build and run lexer tests only"
	@echo "  qt-build   - Build Qt application"
	@echo "  clean      - Clean build files"
	@echo "  help       - Show this help" 