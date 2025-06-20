#include <iostream>
#include <fstream>
#include "src/lexer/lexer.h"

int main() {
    std::ifstream file("test_code.c");
    if (!file.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        return 1;
    }
    
    std::string code((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    
    Lexer lexer;
    lexer.setSource(code);
    auto result = lexer.analyze();
    
    std::cout << "=== 词法分析结果 ===" << std::endl;
    std::cout << "成功: " << (result.success ? "是" : "否") << std::endl;
    std::cout << "Token数量: " << result.tokens.size() << std::endl;
    std::cout << "错误数量: " << result.errors.size() << std::endl;
    
    if (!result.errors.empty()) {
        std::cout << "\n=== 错误列表 (前10个) ===" << std::endl;
        for (size_t i = 0; i < std::min(size_t(10), result.errors.size()); ++i) {
            const auto& error = result.errors[i];
            std::cout << "错误 " << (i+1) << ": 行" << error.line << " 列" << error.column 
                      << " - " << error.message << std::endl;
        }
    }
    
    if (!result.tokens.empty()) {
        std::cout << "\n=== Token列表 (前20个) ===" << std::endl;
        for (size_t i = 0; i < std::min(size_t(20), result.tokens.size()); ++i) {
            const auto& token = result.tokens[i];
            std::cout << "Token " << (i+1) << ": " << token.toString() << std::endl;
        }
    }
    
    return 0;
} 