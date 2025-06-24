#ifndef INTERMEDIATE_CODE_H
#define INTERMEDIATE_CODE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>

/**
 * @brief 三地址码操作类型
 */
enum class OpType {
    // 算术运算
    ADD,        // result = arg1 + arg2
    SUB,        // result = arg1 - arg2
    MUL,        // result = arg1 * arg2
    DIV,        // result = arg1 / arg2
    MOD,        // result = arg1 % arg2
    NEG,        // result = -arg1
    
    // 逻辑运算
    AND,        // result = arg1 && arg2
    OR,         // result = arg1 || arg2
    NOT,        // result = !arg1
    
    // 比较运算
    EQ,         // result = arg1 == arg2
    NE,         // result = arg1 != arg2
    LT,         // result = arg1 < arg2
    LE,         // result = arg1 <= arg2
    GT,         // result = arg1 > arg2
    GE,         // result = arg1 >= arg2
    
    // 赋值和移动
    ASSIGN,     // result = arg1
    LOAD,       // result = *arg1 (从内存加载)
    STORE,      // *result = arg1 (存储到内存)
    
    // 控制流
    GOTO,       // goto label
    IF_FALSE,   // if (!arg1) goto label
    IF_TRUE,    // if (arg1) goto label
    LABEL,      // label:
    
    // 函数相关
    CALL,       // result = call function(args...)
    PARAM,      // param arg1
    RETURN,     // return arg1
    
    // 数组相关
    ARRAY_REF,  // result = arg1[arg2]
    ARRAY_SET,  // arg1[arg2] = result
    
    // 类型转换
    CAST,       // result = (type)arg1
    
    // 特殊
    NOP         // 空操作
};

/**
 * @brief 操作数类型
 */
enum class OperandType {
    VARIABLE,   // 变量
    CONSTANT,   // 常量
    TEMPORARY,  // 临时变量
    LABEL,      // 标签
    FUNCTION    // 函数名
};

/**
 * @brief 数据类型
 */
enum class IRDataType {
    VOID,
    INT,
    FLOAT,
    BOOL,
    CHAR,
    STRING,
    POINTER,
    UNKNOWN
};

/**
 * @brief 操作数
 */
struct Operand {
    OperandType type;
    IRDataType dataType;
    std::string name;
    std::string value;  // 对于常量
    
    Operand() : type(OperandType::VARIABLE), dataType(IRDataType::UNKNOWN) {}
    
    Operand(OperandType t, const std::string& n, IRDataType dt = IRDataType::UNKNOWN)
        : type(t), dataType(dt), name(n) {}
    
    Operand(OperandType t, const std::string& n, const std::string& v, IRDataType dt)
        : type(t), dataType(dt), name(n), value(v) {}
    
    bool isConstant() const { return type == OperandType::CONSTANT; }
    bool isTemporary() const { return type == OperandType::TEMPORARY; }
    bool isVariable() const { return type == OperandType::VARIABLE; }
    bool isLabel() const { return type == OperandType::LABEL; }
    
    std::string toString() const;
};

/**
 * @brief 三地址码指令
 */
struct ThreeAddressCode {
    OpType op;                  // 操作类型
    std::unique_ptr<Operand> result;  // 结果操作数
    std::unique_ptr<Operand> arg1;    // 第一个参数
    std::unique_ptr<Operand> arg2;    // 第二个参数（可选）
    std::string comment;        // 注释
    int lineNumber;            // 对应源代码行号
    
    ThreeAddressCode(OpType operation, int line = 0)
        : op(operation), lineNumber(line) {}
    
    // 复制构造函数
    ThreeAddressCode(const ThreeAddressCode& other)
        : op(other.op), comment(other.comment), lineNumber(other.lineNumber) {
        if (other.result) {
            result = std::make_unique<Operand>(*other.result);
        }
        if (other.arg1) {
            arg1 = std::make_unique<Operand>(*other.arg1);
        }
        if (other.arg2) {
            arg2 = std::make_unique<Operand>(*other.arg2);
        }
    }
    
    // 拷贝赋值运算符
    ThreeAddressCode& operator=(const ThreeAddressCode& other) {
        if (this != &other) {
            op = other.op;
            comment = other.comment;
            lineNumber = other.lineNumber;
            
            result.reset();
            arg1.reset();
            arg2.reset();
            
            if (other.result) {
                result = std::make_unique<Operand>(*other.result);
            }
            if (other.arg1) {
                arg1 = std::make_unique<Operand>(*other.arg1);
            }
            if (other.arg2) {
                arg2 = std::make_unique<Operand>(*other.arg2);
            }
        }
        return *this;
    }
    
    // 移动构造函数
    ThreeAddressCode(ThreeAddressCode&& other) noexcept
        : op(other.op), result(std::move(other.result)), 
          arg1(std::move(other.arg1)), arg2(std::move(other.arg2)),
          comment(std::move(other.comment)), lineNumber(other.lineNumber) {}
    
    // 移动赋值运算符
    ThreeAddressCode& operator=(ThreeAddressCode&& other) noexcept {
        if (this != &other) {
            op = other.op;
            result = std::move(other.result);
            arg1 = std::move(other.arg1);
            arg2 = std::move(other.arg2);
            comment = std::move(other.comment);
            lineNumber = other.lineNumber;
        }
        return *this;
    }
    
    std::string toString() const;
    std::string getOpString() const;
    bool isJump() const;
    bool isLabel() const;
    bool hasResult() const;
};

/**
 * @brief 基本块
 */
class BasicBlock {
public:
    std::string label;                              // 基本块标签
    std::vector<std::unique_ptr<ThreeAddressCode>> instructions;  // 指令序列
    std::vector<BasicBlock*> predecessors;          // 前驱基本块
    std::vector<BasicBlock*> successors;            // 后继基本块
    bool isEntry;                                   // 是否为入口块
    bool isExit;                                    // 是否为出口块
    
    BasicBlock(const std::string& lbl) : label(lbl), isEntry(false), isExit(false) {}
    
    void addInstruction(std::unique_ptr<ThreeAddressCode> instr);
    void addSuccessor(BasicBlock* successor);
    void addPredecessor(BasicBlock* predecessor);
    std::string toString() const;
};

/**
 * @brief 临时变量管理器
 */
class TempManager {
private:
    int tempCount;
    int labelCount;
    
public:
    TempManager() : tempCount(0), labelCount(0) {}
    
    /**
     * @brief 生成新的临时变量
     */
    std::string newTemp();
    
    /**
     * @brief 生成新的标签
     */
    std::string newLabel();
    
    /**
     * @brief 重置计数器
     */
    void reset();
    
    /**
     * @brief 获取当前临时变量计数
     */
    int getTempCount() const { return tempCount; }
    
    /**
     * @brief 获取当前标签计数
     */
    int getLabelCount() const { return labelCount; }
};

/**
 * @brief 中间代码表示
 */
class IntermediateCode {
private:
    std::vector<std::unique_ptr<ThreeAddressCode>> instructions;  // 指令序列
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks;         // 基本块序列
    std::unordered_map<std::string, BasicBlock*> labelToBlock;    // 标签到基本块的映射
    TempManager tempManager;                                      // 临时变量管理器
    
public:
    IntermediateCode() = default;
    ~IntermediateCode() = default;
    
    /**
     * @brief 添加指令
     */
    void addInstruction(std::unique_ptr<ThreeAddressCode> instr);
    
    /**
     * @brief 生成新的临时变量
     */
    std::string newTemp() { return tempManager.newTemp(); }
    
    /**
     * @brief 生成新的标签
     */
    std::string newLabel() { return tempManager.newLabel(); }
    
    /**
     * @brief 构建基本块
     */
    void buildBasicBlocks();
    
    /**
     * @brief 构建控制流图
     */
    void buildControlFlowGraph();
    
    /**
     * @brief 获取指令序列
     */
    const std::vector<std::unique_ptr<ThreeAddressCode>>& getInstructions() const {
        return instructions;
    }
    
    /**
     * @brief 获取基本块序列
     */
    const std::vector<std::unique_ptr<BasicBlock>>& getBasicBlocks() const {
        return basicBlocks;
    }
    
    /**
     * @brief 优化：常量折叠
     */
    void constantFolding();
    
    /**
     * @brief 优化：死代码消除
     */
    void deadCodeElimination();
    
    /**
     * @brief 输出中间代码
     */
    void print(std::ostream& os = std::cout) const;
    
    /**
     * @brief 输出基本块
     */
    void printBasicBlocks(std::ostream& os = std::cout) const;
    
    /**
     * @brief 清空中间代码
     */
    void clear();
    
    /**
     * @brief 获取统计信息
     */
    struct Statistics {
        size_t instructionCount;
        size_t basicBlockCount;
        size_t temporaryCount;
        size_t labelCount;
    };
    
    Statistics getStatistics() const;
};

/**
 * @brief 工具函数：创建操作数
 */
namespace OperandUtils {
    std::unique_ptr<Operand> createVariable(const std::string& name, IRDataType type = IRDataType::UNKNOWN);
    std::unique_ptr<Operand> createConstant(const std::string& value, IRDataType type);
    std::unique_ptr<Operand> createTemporary(const std::string& name, IRDataType type = IRDataType::UNKNOWN);
    std::unique_ptr<Operand> createLabel(const std::string& name);
    std::unique_ptr<Operand> createFunction(const std::string& name);
}

/**
 * @brief 工具函数：创建指令
 */
namespace InstructionUtils {
    std::unique_ptr<ThreeAddressCode> createBinaryOp(OpType op, 
                                                      std::unique_ptr<Operand> result,
                                                      std::unique_ptr<Operand> arg1,
                                                      std::unique_ptr<Operand> arg2,
                                                      int line = 0);
    
    std::unique_ptr<ThreeAddressCode> createUnaryOp(OpType op,
                                                     std::unique_ptr<Operand> result,
                                                     std::unique_ptr<Operand> arg1,
                                                     int line = 0);
    
    std::unique_ptr<ThreeAddressCode> createAssign(std::unique_ptr<Operand> result,
                                                    std::unique_ptr<Operand> arg1,
                                                    int line = 0);
    
    std::unique_ptr<ThreeAddressCode> createGoto(std::unique_ptr<Operand> label, int line = 0);
    
    std::unique_ptr<ThreeAddressCode> createConditionalJump(OpType op,
                                                             std::unique_ptr<Operand> condition,
                                                             std::unique_ptr<Operand> label,
                                                             int line = 0);
    
    std::unique_ptr<ThreeAddressCode> createLabel(std::unique_ptr<Operand> label, int line = 0);
    
    std::unique_ptr<ThreeAddressCode> createReturn(std::unique_ptr<Operand> value = nullptr, int line = 0);
    
    std::unique_ptr<ThreeAddressCode> createFunctionCall(std::unique_ptr<Operand> result,
                                                          std::unique_ptr<Operand> function,
                                                          int line = 0);
}

#endif // INTERMEDIATE_CODE_H 