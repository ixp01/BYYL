#include <iostream>
#include <cassert>
#include <memory>
#include <sstream>
#include <chrono>
#include "../src/codegen/intermediate_code.h"
#include "../src/codegen/code_generator.h"

void testOperand() {
    std::cout << "Testing Operand..." << std::endl;
    
    // 测试变量操作数
    auto var = OperandUtils::createVariable("x", IRDataType::INT);
    assert(var->isVariable());
    assert(!var->isConstant());
    assert(var->toString() == "x");
    
    // 测试常量操作数
    auto constant = OperandUtils::createConstant("42", IRDataType::INT);
    assert(constant->isConstant());
    assert(!constant->isVariable());
    assert(constant->toString() == "42");
    
    // 测试临时变量操作数
    auto temp = OperandUtils::createTemporary("t1", IRDataType::INT);
    assert(temp->isTemporary());
    assert(temp->toString() == "t1");
    
    // 测试标签操作数
    auto label = OperandUtils::createLabel("L1");
    assert(label->isLabel());
    assert(label->toString() == "L1");
    
    std::cout << "✓ Operand tests passed!" << std::endl;
}

void testThreeAddressCode() {
    std::cout << "Testing ThreeAddressCode..." << std::endl;
    
    // 测试二元运算指令
    auto addInstr = InstructionUtils::createBinaryOp(
        OpType::ADD,
        OperandUtils::createTemporary("t1", IRDataType::INT),
        OperandUtils::createVariable("x", IRDataType::INT),
        OperandUtils::createConstant("5", IRDataType::INT),
        10
    );
    
    assert(addInstr->op == OpType::ADD);
    assert(addInstr->hasResult());
    assert(!addInstr->isJump());
    assert(!addInstr->isLabel());
    
    std::string instrStr = addInstr->toString();
    assert(instrStr.find("t1 = x + 5") != std::string::npos);
    
    // 测试跳转指令
    auto gotoInstr = InstructionUtils::createGoto(
        OperandUtils::createLabel("L1"), 15);
    
    assert(gotoInstr->op == OpType::GOTO);
    assert(!gotoInstr->hasResult());
    assert(gotoInstr->isJump());
    
    // 测试标签指令
    auto labelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel("L1"), 20);
    
    assert(labelInstr->op == OpType::LABEL);
    assert(labelInstr->isLabel());
    
    std::cout << "✓ ThreeAddressCode tests passed!" << std::endl;
}

void testTempManager() {
    std::cout << "Testing TempManager..." << std::endl;
    
    TempManager manager;
    
    // 测试临时变量生成
    std::string temp1 = manager.newTemp();
    std::string temp2 = manager.newTemp();
    assert(temp1 == "t0");
    assert(temp2 == "t1");
    assert(manager.getTempCount() == 2);
    
    // 测试标签生成
    std::string label1 = manager.newLabel();
    std::string label2 = manager.newLabel();
    assert(label1 == "L0");
    assert(label2 == "L1");
    assert(manager.getLabelCount() == 2);
    
    // 测试重置
    manager.reset();
    assert(manager.getTempCount() == 0);
    assert(manager.getLabelCount() == 0);
    
    std::string newTemp = manager.newTemp();
    assert(newTemp == "t0");
    
    std::cout << "✓ TempManager tests passed!" << std::endl;
}

void testIntermediateCode() {
    std::cout << "Testing IntermediateCode..." << std::endl;
    
    IntermediateCode ir;
    
    // 添加一些指令
    ir.addInstruction(InstructionUtils::createAssign(
        OperandUtils::createVariable("x", IRDataType::INT),
        OperandUtils::createConstant("10", IRDataType::INT)
    ));
    
    ir.addInstruction(InstructionUtils::createBinaryOp(
        OpType::ADD,
        OperandUtils::createTemporary("t1", IRDataType::INT),
        OperandUtils::createVariable("x", IRDataType::INT),
        OperandUtils::createConstant("5", IRDataType::INT)
    ));
    
    ir.addInstruction(InstructionUtils::createAssign(
        OperandUtils::createVariable("y", IRDataType::INT),
        OperandUtils::createTemporary("t1", IRDataType::INT)
    ));
    
    // 测试统计信息
    auto stats = ir.getStatistics();
    assert(stats.instructionCount == 3);
    
    // 测试输出
    std::ostringstream oss;
    ir.print(oss);
    std::string output = oss.str();
    assert(output.find("x = 10") != std::string::npos);
    assert(output.find("t1 = x + 5") != std::string::npos);
    assert(output.find("y = t1") != std::string::npos);
    
    std::cout << "✓ IntermediateCode tests passed!" << std::endl;
}

void testConstantFolding() {
    std::cout << "Testing Constant Folding..." << std::endl;
    
    IntermediateCode ir;
    
    // 添加可以常量折叠的指令
    ir.addInstruction(InstructionUtils::createBinaryOp(
        OpType::ADD,
        OperandUtils::createTemporary("t1", IRDataType::INT),
        OperandUtils::createConstant("3", IRDataType::INT),
        OperandUtils::createConstant("5", IRDataType::INT)
    ));
    
    ir.addInstruction(InstructionUtils::createBinaryOp(
        OpType::MUL,
        OperandUtils::createTemporary("t2", IRDataType::INT),
        OperandUtils::createConstant("4", IRDataType::INT),
        OperandUtils::createConstant("6", IRDataType::INT)
    ));
    
    // 执行常量折叠
    ir.constantFolding();
    
    // 验证结果
    const auto& instructions = ir.getInstructions();
    assert(instructions.size() == 2);
    
    // 第一个指令应该变成 t1 = 8
    assert(instructions[0]->op == OpType::ASSIGN);
    assert(instructions[0]->arg1->toString() == "8");
    
    // 第二个指令应该变成 t2 = 24
    assert(instructions[1]->op == OpType::ASSIGN);
    assert(instructions[1]->arg1->toString() == "24");
    
    std::cout << "✓ Constant Folding tests passed!" << std::endl;
}

void testCodeGenerator() {
    std::cout << "Testing CodeGenerator..." << std::endl;
    
    auto generator = CodeGeneratorFactory::createStandard();
    
    // 创建一个简单的程序用于测试
    auto program = std::make_unique<ProgramNode>();
    
    // 生成代码
    CodeGenResult result = generator->generate(program.get());
    
    // 检查结果
    assert(result.success);
    assert(result.errors.empty());
    assert(result.intermediateCode != nullptr);
    
    // 测试配置
    CodeGenConfig config;
    config.enableOptimization = false;
    config.generateComments = true;
    generator->setConfig(config);
    
    std::cout << "✓ CodeGenerator basic tests passed!" << std::endl;
}

void testCodeGeneratorFactory() {
    std::cout << "Testing CodeGeneratorFactory..." << std::endl;
    
    // 测试不同类型的代码生成器创建
    auto standard = CodeGeneratorFactory::createStandard();
    auto optimized = CodeGeneratorFactory::createOptimized();
    auto debug = CodeGeneratorFactory::createDebug();
    
    assert(standard != nullptr);
    assert(optimized != nullptr);
    assert(debug != nullptr);
    
    std::cout << "✓ CodeGeneratorFactory tests passed!" << std::endl;
}

void codegenPerformanceTest() {
    std::cout << "Performing Code Generation Performance Test..." << std::endl;
    
    auto generator = CodeGeneratorFactory::createOptimized();
    
    // 创建空程序进行性能测试
    auto program = std::make_unique<ProgramNode>();
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        CodeGenResult result = generator->generate(program.get());
        (void)result; // 避免未使用变量警告
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Performance: 1000 generations in " << duration.count() << " μs" << std::endl;
    std::cout << "Average: " << duration.count() / 1000.0 << " μs per generation" << std::endl;
    
    std::cout << "✓ Performance test completed!" << std::endl;
}

void demonstrateCodeGeneration() {
    std::cout << "\n=== Code Generation Demonstration ===" << std::endl;
    
    IntermediateCode ir;
    
    // 模拟生成一个简单的程序：
    // int main() {
    //     int x = 10;
    //     int y = x + 5;
    //     return y;
    // }
    
    // 函数开始
    ir.addInstruction(InstructionUtils::createLabel(
        OperandUtils::createLabel("main")));
    
    // x = 10
    ir.addInstruction(InstructionUtils::createAssign(
        OperandUtils::createVariable("x", IRDataType::INT),
        OperandUtils::createConstant("10", IRDataType::INT)));
    
    // t1 = x + 5
    ir.addInstruction(InstructionUtils::createBinaryOp(
        OpType::ADD,
        OperandUtils::createTemporary("t1", IRDataType::INT),
        OperandUtils::createVariable("x", IRDataType::INT),
        OperandUtils::createConstant("5", IRDataType::INT)));
    
    // y = t1
    ir.addInstruction(InstructionUtils::createAssign(
        OperandUtils::createVariable("y", IRDataType::INT),
        OperandUtils::createTemporary("t1", IRDataType::INT)));
    
    // return y
    ir.addInstruction(InstructionUtils::createReturn(
        OperandUtils::createVariable("y", IRDataType::INT)));
    
    std::cout << "\nGenerated Intermediate Code:" << std::endl;
    ir.print();
    
    std::cout << "\nAfter Constant Folding:" << std::endl;
    ir.constantFolding();
    ir.print();
    
    std::cout << "\nStatistics:" << std::endl;
    auto stats = ir.getStatistics();
    std::cout << "  Instructions: " << stats.instructionCount << std::endl;
    std::cout << "  Temporaries: " << stats.temporaryCount << std::endl;
    std::cout << "  Labels: " << stats.labelCount << std::endl;
}

int runCodegenTests() {
    std::cout << "=== Code Generation Module Test Suite ===" << std::endl;
    
    try {
        testOperand();
        testThreeAddressCode();
        testTempManager();
        testIntermediateCode();
        testConstantFolding();
        testCodeGenerator();
        testCodeGeneratorFactory();
        codegenPerformanceTest();
        
        std::cout << "\n🎉 All code generation tests passed successfully!" << std::endl;
        std::cout << "✅ Operand management" << std::endl;
        std::cout << "✅ Three-address code instructions" << std::endl;
        std::cout << "✅ Temporary variable management" << std::endl;
        std::cout << "✅ Intermediate code representation" << std::endl;
        std::cout << "✅ Constant folding optimization" << std::endl;
        std::cout << "✅ Code generator functionality" << std::endl;
        std::cout << "✅ Factory patterns" << std::endl;
        std::cout << "✅ Performance characteristics" << std::endl;
        
        demonstrateCodeGeneration();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
} 