# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Directories
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# Source files
LEXER_SOURCES = $(SRC_DIR)/lexer/token.cpp $(SRC_DIR)/lexer/dfa.cpp $(SRC_DIR)/lexer/lexer.cpp $(SRC_DIR)/lexer/minimizer.cpp
PARSER_SOURCES = $(SRC_DIR)/parser/grammar.cpp $(SRC_DIR)/parser/lalr.cpp $(SRC_DIR)/parser/parser.cpp $(SRC_DIR)/parser/ast.cpp
SEMANTIC_SOURCES = $(SRC_DIR)/semantic/symbol_table.cpp $(SRC_DIR)/semantic/semantic_analyzer.cpp
CODEGEN_SOURCES = $(SRC_DIR)/codegen/intermediate_code.cpp $(SRC_DIR)/codegen/code_generator.cpp
GUI_SOURCES = 

# Test files
TEST_SOURCES = $(TEST_DIR)/test_main.cpp $(TEST_DIR)/test_lexer.cpp $(TEST_DIR)/test_parser.cpp $(TEST_DIR)/test_semantic.cpp $(TEST_DIR)/test_codegen.cpp

# Object files
LEXER_OBJECTS = $(LEXER_SOURCES:.cpp=.o)
PARSER_OBJECTS = $(PARSER_SOURCES:.cpp=.o)
SEMANTIC_OBJECTS = $(SEMANTIC_SOURCES:.cpp=.o)
CODEGEN_OBJECTS = $(CODEGEN_SOURCES:.cpp=.o)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

# Targets
.PHONY: all test test-parser clean qt-build qt-test

all: qt-build

# Test only the core components (without Qt)
test: $(TEST_OBJECTS) $(LEXER_OBJECTS) $(PARSER_OBJECTS) $(SEMANTIC_OBJECTS) $(CODEGEN_OBJECTS)
	@echo "Building test executable..."
	$(CXX) $(CXXFLAGS) -o test_runner $(TEST_OBJECTS) $(LEXER_OBJECTS) $(PARSER_OBJECTS) $(SEMANTIC_OBJECTS) $(CODEGEN_OBJECTS)
	@echo "Running tests..."
	./test_runner

# Test only lexer components
test-lexer: tests/test_lexer.o $(LEXER_OBJECTS)
	@echo "Building lexer test executable..."
	$(CXX) $(CXXFLAGS) -o test_lexer tests/test_lexer.o $(LEXER_OBJECTS)
	@echo "Running lexer tests..."
	./test_lexer

# Test only parser components
test-parser: tests/test_parser.o $(PARSER_OBJECTS) $(LEXER_OBJECTS)
	@echo "Building parser test executable..."
	$(CXX) $(CXXFLAGS) -o test_parser tests/test_parser.o $(PARSER_OBJECTS) $(LEXER_OBJECTS)
	@echo "Running parser tests..."
	./test_parser

# Test only semantic components
test-semantic: tests/test_semantic.o $(SEMANTIC_OBJECTS) $(PARSER_OBJECTS) $(LEXER_OBJECTS)
	@echo "Building semantic test executable..."
	$(CXX) $(CXXFLAGS) -o test_semantic tests/test_semantic.o $(SEMANTIC_OBJECTS) $(PARSER_OBJECTS) $(LEXER_OBJECTS)
	@echo "Running semantic tests..."
	./test_semantic

# Test only codegen components
test-codegen: tests/test_codegen.o $(CODEGEN_OBJECTS) $(SEMANTIC_OBJECTS) $(PARSER_OBJECTS) $(LEXER_OBJECTS)
	@echo "Building codegen test executable..."
	$(CXX) $(CXXFLAGS) -o test_codegen tests/test_codegen.o $(CODEGEN_OBJECTS) $(SEMANTIC_OBJECTS) $(PARSER_OBJECTS) $(LEXER_OBJECTS)
	@echo "Running codegen tests..."
	./test_codegen

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
	rm -f test_runner test_lexer test_parser test_semantic test_codegen
	rm -f CompilerFrontend
	rm -f $(LEXER_OBJECTS) $(PARSER_OBJECTS) $(SEMANTIC_OBJECTS) $(CODEGEN_OBJECTS) $(TEST_OBJECTS)
	rm -f *.o
	rm -rf build

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build Qt application (default)"
	@echo "  test          - Build and run all tests"
	@echo "  test-lexer    - Build and run lexer tests only"
	@echo "  test-parser   - Build and run parser tests only"
	@echo "  test-semantic - Build and run semantic tests only"
	@echo "  test-codegen  - Build and run codegen tests only"
	@echo "  qt-build      - Build Qt application"
	@echo "  clean         - Clean build files"
	@echo "  help          - Show this help" 