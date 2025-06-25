#include "intermediate_code.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

std::string Operand::toString() const {
    switch (type) {
        case OperandType::VARIABLE:
            return name;
        case OperandType::CONSTANT:
            return value.empty() ? name : value;
        case OperandType::TEMPORARY:
            return name;
        case OperandType::LABEL:
            return name;
        case OperandType::FUNCTION:
            return name;
        default:
            return "unknown";
    }
}

std::string ThreeAddressCode::toString() const {
    std::ostringstream oss;
    
    switch (op) {
        case OpType::LABEL:
            if (arg1) oss << arg1->toString() << ":";
            break;
        case OpType::GOTO:
            if (arg1) oss << "goto " << arg1->toString();
            break;
        case OpType::RETURN:
            oss << "return";
            if (arg1) oss << " " << arg1->toString();
            break;
        default:
            if (result) {
                oss << result->toString() << " = ";
                if (arg2) {
                    oss << arg1->toString() << " " << getOpString() << " " << arg2->toString();
                } else if (arg1) {
                    if (op == OpType::ASSIGN) {
                        oss << arg1->toString();
                    } else {
                        oss << getOpString() << arg1->toString();
                    }
                }
            }
            break;
    }
    
    if (!comment.empty()) {
        oss << " // " << comment;
    }
    
    return oss.str();
}

std::string ThreeAddressCode::getOpString() const {
    switch (op) {
        case OpType::ADD: return "+";
        case OpType::SUB: return "-";
        case OpType::MUL: return "*";
        case OpType::DIV: return "/";
        case OpType::MOD: return "%";
        case OpType::ASSIGN: return "=";
        default: return "?";
    }
}

bool ThreeAddressCode::isJump() const {
    return op == OpType::GOTO || op == OpType::IF_FALSE || op == OpType::IF_TRUE;
}

bool ThreeAddressCode::isLabel() const {
    return op == OpType::LABEL;
}

bool ThreeAddressCode::hasResult() const {
    return result != nullptr;
}

void BasicBlock::addInstruction(std::unique_ptr<ThreeAddressCode> instr) {
    instructions.push_back(std::move(instr));
}

void BasicBlock::addSuccessor(BasicBlock* successor) {
    if (successor && std::find(successors.begin(), successors.end(), successor) == successors.end()) {
        successors.push_back(successor);
        successor->addPredecessor(this);
    }
}

void BasicBlock::addPredecessor(BasicBlock* predecessor) {
    if (predecessor && std::find(predecessors.begin(), predecessors.end(), predecessor) == predecessors.end()) {
        predecessors.push_back(predecessor);
    }
}

std::string BasicBlock::toString() const {
    std::ostringstream oss;
    oss << "Basic Block: " << label << "\n";
    for (const auto& instr : instructions) {
        oss << "  " << instr->toString() << "\n";
    }
    return oss.str();
}

std::string TempManager::newTemp() {
    return "t" + std::to_string(tempCount++);
}

std::string TempManager::newLabel() {
    return "L" + std::to_string(labelCount++);
}

void TempManager::reset() {
    tempCount = 0;
    labelCount = 0;
}

void IntermediateCode::addInstruction(std::unique_ptr<ThreeAddressCode> instr) {
    instructions.push_back(std::move(instr));
}

void IntermediateCode::buildBasicBlocks() {
    // 简化实现
}

void IntermediateCode::buildControlFlowGraph() {
    // 简化实现
}

void IntermediateCode::constantFolding() {
    for (auto& instr : instructions) {
        if (!instr->arg1 || !instr->arg2) {
            continue;
        }
        
        if (!instr->arg1->isConstant() || !instr->arg2->isConstant()) {
            continue;
        }
        
        try {
            int val1 = std::stoi(instr->arg1->value);
            int val2 = std::stoi(instr->arg2->value);
            int result = 0;
            bool canFold = true;
            
            switch (instr->op) {
                case OpType::ADD: result = val1 + val2; break;
                case OpType::SUB: result = val1 - val2; break;
                case OpType::MUL: result = val1 * val2; break;
                case OpType::DIV: 
                    if (val2 != 0) {
                        result = val1 / val2; 
                    } else {
                        canFold = false; // 避免除零
                    }
                    break;
                case OpType::MOD: 
                    if (val2 != 0) {
                        result = val1 % val2; 
                    } else {
                        canFold = false; // 避免除零
                    }
                    break;
                default: canFold = false; break;
            }
            
            if (canFold) {
                instr->op = OpType::ASSIGN;
                instr->arg1 = OperandUtils::createConstant(std::to_string(result), IRDataType::INT);
                instr->arg2.reset();
                instr->comment = "constant folding";
            }
        } catch (...) {
            // 忽略转换错误
        }
    }
}

void IntermediateCode::deadCodeElimination() {
    // 简化实现
}

void IntermediateCode::print(std::ostream& os) const {
    os << "=== Intermediate Code ===" << std::endl;
    for (const auto& instr : instructions) {
        os << instr->toString() << std::endl;
    }
    os << "========================" << std::endl;
}

void IntermediateCode::printBasicBlocks(std::ostream& os) const {
    os << "=== Basic Blocks ===" << std::endl;
    for (const auto& block : basicBlocks) {
        os << block->toString() << std::endl;
    }
    os << "===================" << std::endl;
}

void IntermediateCode::clear() {
    instructions.clear();
    basicBlocks.clear();
    labelToBlock.clear();
    tempManager.reset();
}

IntermediateCode::Statistics IntermediateCode::getStatistics() const {
    Statistics stats;
    stats.instructionCount = instructions.size();
    stats.basicBlockCount = basicBlocks.size();
    stats.temporaryCount = tempManager.getTempCount();
    stats.labelCount = tempManager.getLabelCount();
    return stats;
}

namespace OperandUtils {
    std::unique_ptr<Operand> createVariable(const std::string& name, IRDataType type) {
        return std::make_unique<Operand>(OperandType::VARIABLE, name, type);
    }
    
    std::unique_ptr<Operand> createConstant(const std::string& value, IRDataType type) {
        return std::make_unique<Operand>(OperandType::CONSTANT, value, value, type);
    }
    
    std::unique_ptr<Operand> createTemporary(const std::string& name, IRDataType type) {
        return std::make_unique<Operand>(OperandType::TEMPORARY, name, type);
    }
    
    std::unique_ptr<Operand> createLabel(const std::string& name) {
        return std::make_unique<Operand>(OperandType::LABEL, name);
    }
    
    std::unique_ptr<Operand> createFunction(const std::string& name) {
        return std::make_unique<Operand>(OperandType::FUNCTION, name);
    }
}

namespace InstructionUtils {
    std::unique_ptr<ThreeAddressCode> createBinaryOp(OpType op, 
                                                      std::unique_ptr<Operand> result,
                                                      std::unique_ptr<Operand> arg1,
                                                      std::unique_ptr<Operand> arg2,
                                                      int line) {
        auto instr = std::make_unique<ThreeAddressCode>(op, line);
        instr->result = std::move(result);
        instr->arg1 = std::move(arg1);
        instr->arg2 = std::move(arg2);
        return instr;
    }
    
    std::unique_ptr<ThreeAddressCode> createUnaryOp(OpType op,
                                                     std::unique_ptr<Operand> result,
                                                     std::unique_ptr<Operand> arg1,
                                                     int line) {
        auto instr = std::make_unique<ThreeAddressCode>(op, line);
        instr->result = std::move(result);
        instr->arg1 = std::move(arg1);
        return instr;
    }
    
    std::unique_ptr<ThreeAddressCode> createAssign(std::unique_ptr<Operand> result,
                                                    std::unique_ptr<Operand> arg1,
                                                    int line) {
        return createUnaryOp(OpType::ASSIGN, std::move(result), std::move(arg1), line);
    }
    
    std::unique_ptr<ThreeAddressCode> createGoto(std::unique_ptr<Operand> label, int line) {
        auto instr = std::make_unique<ThreeAddressCode>(OpType::GOTO, line);
        instr->arg1 = std::move(label);
        return instr;
    }
    
    std::unique_ptr<ThreeAddressCode> createConditionalJump(OpType op,
                                                             std::unique_ptr<Operand> condition,
                                                             std::unique_ptr<Operand> label,
                                                             int line) {
        auto instr = std::make_unique<ThreeAddressCode>(op, line);
        instr->arg1 = std::move(condition);
        instr->arg2 = std::move(label);
        return instr;
    }
    
    std::unique_ptr<ThreeAddressCode> createLabel(std::unique_ptr<Operand> label, int line) {
        auto instr = std::make_unique<ThreeAddressCode>(OpType::LABEL, line);
        instr->arg1 = std::move(label);
        return instr;
    }
    
    std::unique_ptr<ThreeAddressCode> createReturn(std::unique_ptr<Operand> value, int line) {
        auto instr = std::make_unique<ThreeAddressCode>(OpType::RETURN, line);
        if (value) {
            instr->arg1 = std::move(value);
        }
        return instr;
    }
    
    std::unique_ptr<ThreeAddressCode> createFunctionCall(std::unique_ptr<Operand> result,
                                                          std::unique_ptr<Operand> function,
                                                          int line) {
        auto instr = std::make_unique<ThreeAddressCode>(OpType::CALL, line);
        instr->result = std::move(result);
        instr->arg1 = std::move(function);
        return instr;
    }
}
