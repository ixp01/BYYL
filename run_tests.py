#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
编译器前端自动化测试脚本
====================

这个脚本可以自动运行编译器的各种测试用例，
并检查输出结果是否符合预期。

使用方法：
    python3 run_tests.py

要求：
    1. 编译器可执行文件位于当前目录下，名为 CompilerFrontend
    2. 测试用例文件位于 test_cases/ 目录下
"""

import os
import sys
import subprocess
import time
import json
from typing import List, Dict, Any

class CompilerTester:
    def __init__(self, compiler_path="./CompilerFrontend"):
        self.compiler_path = compiler_path
        self.test_results = []
        
    def create_test_cases(self):
        """创建测试用例目录和文件"""
        if not os.path.exists("test_cases"):
            os.makedirs("test_cases")
            
        # 基础测试用例
        basic_tests = {
            "simple_assignment.c": """int a = 5;
int b = a;
int c = a + b;""",
            
            "arithmetic.c": """int x = 10;
int y = 20;
int sum = x + y;
int diff = x - y;
int product = x * y;
int quotient = x / y;""",
            
            "variables.c": """int a;
float b = 3.14;
char c = 'A';
bool flag = true;""",
            
            "expressions.c": """int a = 5;
int b = 3;
int c = (a + b) * 2;
int d = a > b ? a : b;""",
            
            "errors.c": """int a = 5;
int b = undeclared_var;  // 错误：未声明变量
int c;
int result = c + 10;     // 警告：未初始化变量""",
            
            "complex.c": """int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    int n = 5;
    int result = factorial(n);
    return result;
}""",
        }
        
        for filename, code in basic_tests.items():
            with open(f"test_cases/{filename}", "w", encoding="utf-8") as f:
                f.write(code)
                
        print(f"创建了 {len(basic_tests)} 个测试用例文件")
        
    def run_single_test(self, test_file: str) -> Dict[str, Any]:
        """运行单个测试用例"""
        print(f"\n=== 测试: {test_file} ===")
        
        if not os.path.exists(test_file):
            return {
                "file": test_file,
                "status": "SKIP",
                "error": "文件不存在"
            }
            
        # 读取测试代码
        with open(test_file, "r", encoding="utf-8") as f:
            test_code = f.read()
            
        print(f"测试代码:\n{test_code}\n")
        
        # 这里应该通过某种方式向编译器传递代码并获取结果
        # 由于当前编译器是GUI程序，我们暂时只是验证编译器能否启动
        result = {
            "file": test_file,
            "code": test_code,
            "status": "MANUAL",
            "message": "需要手动在GUI中测试"
        }
        
        return result
        
    def run_compiler_check(self) -> bool:
        """检查编译器是否可以运行"""
        try:
            if not os.path.exists(self.compiler_path):
                print(f"错误: 编译器文件不存在: {self.compiler_path}")
                return False
                
            # 检查文件是否可执行
            if not os.access(self.compiler_path, os.X_OK):
                print(f"错误: 编译器文件不可执行: {self.compiler_path}")
                return False
                
            print(f"编译器检查通过: {self.compiler_path}")
            return True
            
        except Exception as e:
            print(f"编译器检查失败: {e}")
            return False
            
    def run_all_tests(self):
        """运行所有测试用例"""
        print("开始编译器前端测试")
        print("=" * 50)
        
        # 检查编译器
        if not self.run_compiler_check():
            print("编译器检查失败，测试终止")
            return
            
        # 创建测试用例
        self.create_test_cases()
        
        # 获取所有测试文件
        test_files = []
        if os.path.exists("test_cases"):
            test_files = [f"test_cases/{f}" for f in os.listdir("test_cases") 
                         if f.endswith(".c")]
        
        if not test_files:
            print("没有找到测试用例文件")
            return
            
        # 运行测试
        for test_file in sorted(test_files):
            result = self.run_single_test(test_file)
            self.test_results.append(result)
            
        # 生成报告
        self.generate_report()
        
    def generate_report(self):
        """生成测试报告"""
        print("\n" + "=" * 50)
        print("测试报告")
        print("=" * 50)
        
        total_tests = len(self.test_results)
        manual_tests = len([r for r in self.test_results if r["status"] == "MANUAL"])
        skipped_tests = len([r for r in self.test_results if r["status"] == "SKIP"])
        
        print(f"总测试数: {total_tests}")
        print(f"需要手动测试: {manual_tests}")
        print(f"跳过的测试: {skipped_tests}")
        
        print("\n详细结果:")
        for result in self.test_results:
            status_icon = "📝" if result["status"] == "MANUAL" else "⏭️"
            print(f"{status_icon} {result['file']}: {result['status']}")
            if "message" in result:
                print(f"   {result['message']}")
                
        # 保存JSON格式的详细报告
        with open("test_report.json", "w", encoding="utf-8") as f:
            json.dump(self.test_results, f, ensure_ascii=False, indent=2)
            
        print(f"\n详细报告已保存到: test_report.json")
        
    def print_manual_testing_guide(self):
        """打印手动测试指南"""
        print("\n" + "=" * 50)
        print("手动测试指南")
        print("=" * 50)
        
        guide = """
请按照以下步骤进行手动测试：

1. 启动编译器程序:
   ./CompilerFrontend

2. 对于每个测试用例文件，执行以下操作：
   a) 将测试代码复制到编译器的代码编辑器中
   b) 观察词法分析标签页中的Token序列
   c) 检查语法分析标签页中的AST结构
   d) 查看语义分析标签页中的符号表和错误信息
   e) 检查代码生成标签页中的中间代码

3. 验证要点：
   - 词法分析: Token类型和值是否正确
   - 语法分析: AST结构是否合理
   - 语义分析: 变量是否正确识别，错误是否正确报告
   - 代码生成: 三地址码是否正确生成

4. 特别注意错误处理：
   - errors.c 文件应该能正确识别语义错误
   - 编译器应该能优雅地处理错误而不崩溃

5. 性能测试：
   - complex.c 文件测试递归函数处理能力
   - 观察编译器在处理复杂代码时的响应速度
        """
        
        print(guide)

def main():
    """主函数"""
    print("编译器前端自动化测试工具")
    print("版本: 1.0")
    print("作者: AI Assistant")
    print()
    
    # 创建测试器
    tester = CompilerTester()
    
    # 运行测试
    tester.run_all_tests()
    
    # 显示手动测试指南
    tester.print_manual_testing_guide()
    
    print("\n测试完成！")
    print("建议：将上述测试用例逐一在编译器GUI中运行，验证各个功能模块的正确性。")

if __name__ == "__main__":
    main() 