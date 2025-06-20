#include "symbol_table.h"
#include <iostream>
#include <algorithm>
#include <functional>

// ============ Scope类实现 ============

SymbolInfo* Scope::findLocal(const std::string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    return nullptr;
}

SymbolInfo* Scope::findSymbol(const std::string& name) {
    // 先在当前作用域查找
    SymbolInfo* symbol = findLocal(name);
    if (symbol) {
        return symbol;
    }
    
    // 在父作用域中查找
    if (parent) {
        return parent->findSymbol(name);
    }
    
    return nullptr;
}

bool Scope::addSymbol(const SymbolInfo& symbol) {
    // 检查符号是否已在当前作用域中定义
    if (isDefined(symbol.name)) {
        return false;
    }
    
    // 添加符号
    symbols[symbol.name] = symbol;
    symbols[symbol.name].scopeLevel = level;
    return true;
}

bool Scope::isDefined(const std::string& name) const {
    return symbols.find(name) != symbols.end();
}

std::vector<SymbolInfo*> Scope::getAllSymbols() {
    std::vector<SymbolInfo*> result;
    for (auto& pair : symbols) {
        result.push_back(&pair.second);
    }
    return result;
}

void Scope::print(int indent) const {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
    std::cout << "Scope Level " << level << " (" << symbols.size() << " symbols):\n";
    
    for (const auto& pair : symbols) {
        const SymbolInfo& symbol = pair.second;
        for (int i = 0; i <= indent; ++i) {
            std::cout << "  ";
        }
        std::cout << "- " << symbol.name 
                  << " (" << TypeUtils::symbolTypeToString(symbol.symbolType) 
                  << ", " << TypeUtils::dataTypeToString(symbol.dataType) << ")";
        if (!symbol.isInitialized) std::cout << " [未初始化]";
        if (!symbol.isUsed) std::cout << " [未使用]";
        std::cout << "\n";
    }
    
    // 打印子作用域
    for (const auto& child : children) {
        child->print(indent + 1);
    }
}

// ============ SymbolTable类实现 ============

SymbolTable::SymbolTable() 
    : globalScope(std::make_unique<Scope>(0)), 
      currentScope(globalScope.get()), 
      nextScopeLevel(1) {
    scopeStack.push(currentScope);
}

void SymbolTable::enterScope() {
    // 创建新的子作用域
    auto newScope = std::make_unique<Scope>(nextScopeLevel++, currentScope);
    Scope* newScopePtr = newScope.get();
    
    // 添加到当前作用域的子作用域列表
    currentScope->children.push_back(std::move(newScope));
    
    // 进入新作用域
    scopeStack.push(newScopePtr);
    currentScope = newScopePtr;
}

void SymbolTable::exitScope() {
    if (scopeStack.size() <= 1) {
        // 不能退出全局作用域
        return;
    }
    
    scopeStack.pop();
    currentScope = scopeStack.top();
}

bool SymbolTable::addSymbol(const SymbolInfo& symbol) {
    return currentScope->addSymbol(symbol);
}

SymbolInfo* SymbolTable::findSymbol(const std::string& name) {
    return currentScope->findSymbol(name);
}

SymbolInfo* SymbolTable::findLocalSymbol(const std::string& name) {
    return currentScope->findLocal(name);
}

bool SymbolTable::isDefined(const std::string& name) {
    return findSymbol(name) != nullptr;
}

bool SymbolTable::isLocalDefined(const std::string& name) {
    return currentScope->isDefined(name);
}

void SymbolTable::markSymbolUsed(const std::string& name) {
    SymbolInfo* symbol = findSymbol(name);
    if (symbol) {
        symbol->isUsed = true;
    }
}

void SymbolTable::markSymbolInitialized(const std::string& name) {
    SymbolInfo* symbol = findSymbol(name);
    if (symbol) {
        symbol->isInitialized = true;
    }
}

int SymbolTable::getCurrentScopeLevel() const {
    return currentScope->level;
}

Scope* SymbolTable::getGlobalScope() const {
    return globalScope.get();
}

Scope* SymbolTable::getCurrentScope() const {
    return currentScope;
}

std::vector<SymbolInfo*> SymbolTable::getUnusedVariables() {
    std::vector<SymbolInfo*> unused;
    
    std::function<void(Scope*)> collectUnused = [&](Scope* scope) {
        for (auto& pair : scope->symbols) {
            SymbolInfo& symbol = pair.second;
            if (!symbol.isUsed && symbol.symbolType == SymbolType::VARIABLE) {
                unused.push_back(&symbol);
            }
        }
        
        for (const auto& child : scope->children) {
            collectUnused(child.get());
        }
    };
    
    collectUnused(globalScope.get());
    return unused;
}

std::vector<SymbolInfo*> SymbolTable::getUninitializedVariables() {
    std::vector<SymbolInfo*> uninitialized;
    
    std::function<void(Scope*)> collectUninitialized = [&](Scope* scope) {
        for (auto& pair : scope->symbols) {
            SymbolInfo& symbol = pair.second;
            if (!symbol.isInitialized && symbol.symbolType == SymbolType::VARIABLE) {
                uninitialized.push_back(&symbol);
            }
        }
        
        for (const auto& child : scope->children) {
            collectUninitialized(child.get());
        }
    };
    
    collectUninitialized(globalScope.get());
    return uninitialized;
}

void SymbolTable::print() const {
    std::cout << "=== Symbol Table ===\n";
    globalScope->print(0);
    std::cout << "==================\n";
}

void SymbolTable::clear() {
    // 清空作用域栈
    while (!scopeStack.empty()) {
        scopeStack.pop();
    }
    
    // 重新创建全局作用域
    globalScope = std::make_unique<Scope>(0);
    currentScope = globalScope.get();
    nextScopeLevel = 1;
    scopeStack.push(currentScope);
}

// ============ TypeUtils类实现 ============

DataType TypeUtils::tokenTypeToDataType(TokenType tokenType) {
    switch (tokenType) {
        case TokenType::INT: return DataType::INT;
        case TokenType::FLOAT: return DataType::FLOAT;
        case TokenType::BOOL: return DataType::BOOL;
        case TokenType::NUMBER: return DataType::INT;
        case TokenType::REAL: return DataType::FLOAT;
        case TokenType::STRING: return DataType::STRING;
        case TokenType::TRUE:
        case TokenType::FALSE: return DataType::BOOL;
        default: return DataType::UNKNOWN;
    }
}

std::string TypeUtils::dataTypeToString(DataType dataType) {
    switch (dataType) {
        case DataType::VOID: return "void";
        case DataType::INT: return "int";
        case DataType::FLOAT: return "float";
        case DataType::DOUBLE: return "double";
        case DataType::CHAR: return "char";
        case DataType::STRING: return "string";
        case DataType::BOOL: return "bool";
        case DataType::ARRAY: return "array";
        case DataType::POINTER: return "pointer";
        case DataType::FUNCTION_TYPE: return "function";
        case DataType::UNKNOWN: return "unknown";
        default: return "unknown";
    }
}

std::string TypeUtils::symbolTypeToString(SymbolType symbolType) {
    switch (symbolType) {
        case SymbolType::VARIABLE: return "variable";
        case SymbolType::FUNCTION: return "function";
        case SymbolType::PARAMETER: return "parameter";
        case SymbolType::CONSTANT: return "constant";
        case SymbolType::TYPE_NAME: return "type";
        case SymbolType::LABEL: return "label";
        default: return "unknown";
    }
}

bool TypeUtils::areTypesCompatible(DataType type1, DataType type2) {
    if (type1 == type2) {
        return true;
    }
    
    // 数值类型之间可以兼容
    if (isNumericType(type1) && isNumericType(type2)) {
        return true;
    }
    
    // 字符可以与整型兼容
    if ((type1 == DataType::CHAR && type2 == DataType::INT) ||
        (type1 == DataType::INT && type2 == DataType::CHAR)) {
        return true;
    }
    
    return false;
}

bool TypeUtils::canImplicitlyConvert(DataType from, DataType to) {
    if (from == to) {
        return true;
    }
    
    // 整型可以隐式转换为浮点型
    if (isIntegerType(from) && isFloatingType(to)) {
        return true;
    }
    
    // 小精度浮点型可以转换为大精度浮点型
    if (from == DataType::FLOAT && to == DataType::DOUBLE) {
        return true;
    }
    
    // 字符可以转换为整型
    if (from == DataType::CHAR && to == DataType::INT) {
        return true;
    }
    
    return false;
}

DataType TypeUtils::getBinaryOperationResultType(DataType left, DataType right, TokenType op) {
    // 逻辑运算符返回布尔类型
    if (op == TokenType::AND || op == TokenType::OR || 
        op == TokenType::EQ || op == TokenType::NE ||
        op == TokenType::LT || op == TokenType::LE ||
        op == TokenType::GT || op == TokenType::GE) {
        return DataType::BOOL;
    }
    
    // 如果类型相同，返回该类型
    if (left == right) {
        return left;
    }
    
    // 数值运算的类型提升
    if (isNumericType(left) && isNumericType(right)) {
        // 双精度浮点型优先级最高
        if (left == DataType::DOUBLE || right == DataType::DOUBLE) {
            return DataType::DOUBLE;
        }
        
        // 浮点型次之
        if (left == DataType::FLOAT || right == DataType::FLOAT) {
            return DataType::FLOAT;
        }
        
        // 整型
        return DataType::INT;
    }
    
    return DataType::UNKNOWN;
}

DataType TypeUtils::getUnaryOperationResultType(DataType operand, TokenType op) {
    switch (op) {
        case TokenType::NOT:
            return DataType::BOOL;
        case TokenType::MINUS:
        case TokenType::PLUS:
            if (isNumericType(operand)) {
                return operand;
            }
            break;
        default:
            break;
    }
    
    return DataType::UNKNOWN;
}

bool TypeUtils::isNumericType(DataType type) {
    return type == DataType::INT || 
           type == DataType::FLOAT || 
           type == DataType::DOUBLE ||
           type == DataType::CHAR;
}

bool TypeUtils::isIntegerType(DataType type) {
    return type == DataType::INT || type == DataType::CHAR;
}

bool TypeUtils::isFloatingType(DataType type) {
    return type == DataType::FLOAT || type == DataType::DOUBLE;
}

int TypeUtils::getTypeSize(DataType type) {
    switch (type) {
        case DataType::VOID: return 0;
        case DataType::CHAR: return 1;
        case DataType::BOOL: return 1;
        case DataType::INT: return 4;
        case DataType::FLOAT: return 4;
        case DataType::DOUBLE: return 8;
        case DataType::POINTER: return 8; // 64位系统
        case DataType::STRING: return 8;  // 字符串指针
        default: return 0;
    }
} 