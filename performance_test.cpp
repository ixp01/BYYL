#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// 包含编译器模块
#include "src/lexer/lexer.h"
#include "src/parser/parser.h"
#include "src/semantic/semantic_analyzer.h"
#include "src/codegen/code_generator.h"

/**
 * @brief 编译器前端性能测试
 */
class PerformanceTest {
public:
    struct TestResult {
        std::string stage;
        double time_ms;
        bool success;
        std::string error_message;
    };

private:
    std::vector<TestResult> results;
    std::string test_code;
    size_t code_size;
    size_t line_count;
    
public:
    PerformanceTest() : code_size(0), line_count(0) {}
    
    bool loadTestFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开测试文件: " << filename << std::endl;
            return false;
        }
        
        std::string line;
        test_code.clear();
        line_count = 0;
        
        while (std::getline(file, line)) {
            test_code += line + "\n";
            line_count++;
        }
        
        code_size = test_code.size();
        std::cout << "加载测试文件: " << filename << std::endl;
        std::cout << "代码大小: " << code_size << " 字节" << std::endl;
        std::cout << "代码行数: " << line_count << " 行" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return true;
    }
    
    TestResult testLexicalAnalysis() {
        std::cout << "开始词法分析性能测试..." << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        TestResult result;
        result.stage = "词法分析";
        result.success = false;
        
        try {
            Lexer lexer;
            lexer.setSource(test_code);
            auto lexical_result = lexer.analyze();
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            result.time_ms = duration.count() / 1000.0;
            result.success = lexical_result.success;
            
            if (result.success) {
                std::cout << "✅ 词法分析完成" << std::endl;
                std::cout << "   Token数量: " << lexical_result.tokens.size() << std::endl;
                std::cout << "   用时: " << result.time_ms << " ms" << std::endl;
                if (result.time_ms > 0) {
                    std::cout << "   速度: " << (lexical_result.tokens.size() / (result.time_ms / 1000.0)) << " tokens/s" << std::endl;
                }
            } else {
                std::cout << "⚠️ 词法分析完成但有错误" << std::endl;
                std::cout << "   错误数量: " << lexical_result.errors.size() << std::endl;
                std::cout << "   用时: " << result.time_ms << " ms" << std::endl;
            }
            
        } catch (const std::exception& e) {
            result.error_message = e.what();
            std::cout << "❌ 词法分析失败: " << e.what() << std::endl;
        }
        
        results.push_back(result);
        return result;
    }
    
    void runAllTests() {
        std::cout << "\n🚀 开始编译器前端性能测试\n" << std::endl;
        testLexicalAnalysis();
        printSummary();
    }
    
    void printSummary() {
        std::cout << "\n📊 性能测试总结" << std::endl;
        std::cout << "========================================" << std::endl;
        
        double total_time = 0;
        int success_count = 0;
        
        for (const auto& result : results) {
            std::cout << std::endl;
            std::cout << "阶段: " << result.stage << std::endl;
            std::cout << "状态: " << (result.success ? "✅ 成功" : "❌ 失败") << std::endl;
            std::cout << "用时: " << result.time_ms << " ms" << std::endl;
            
            if (!result.success && !result.error_message.empty()) {
                std::cout << "错误: " << result.error_message << std::endl;
            }
            
            total_time += result.time_ms;
            if (result.success) success_count++;
        }
        
        std::cout << "\n📈 整体统计" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "总用时: " << total_time << " ms" << std::endl;
        std::cout << "成功率: " << success_count << "/" << results.size() << std::endl;
        std::cout << "代码大小: " << code_size << " 字节" << std::endl;
        std::cout << "处理速度: " << (code_size / (total_time / 1000.0)) << " 字节/秒" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::string test_file = "test_code.c";
    
    if (argc > 1) {
        test_file = argv[1];
    }
    
    PerformanceTest tester;
    
    if (!tester.loadTestFile(test_file)) {
        return 1;
    }
    
    tester.runAllTests();
    
    return 0;
}
