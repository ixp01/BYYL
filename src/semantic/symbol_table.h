#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <stack>
#include "../lexer/token.h"

/**
 * @brief 符号类型枚举
 */
enum class SymbolType {
    VARIABLE,       // 变量
    FUNCTION,       // 函数
    PARAMETER,      // 参数
    CONSTANT,       // 常量
    TYPE_NAME,      // 类型名
    LABEL          // 标签
};

/**
 * @brief 数据类型枚举
 */
enum class DataType {
    VOID,           // void类型
    INT,            // 整型
    FLOAT,          // 浮点型
    DOUBLE,         // 双精度浮点型
    CHAR,           // 字符型
    STRING,         // 字符串类型
    BOOL,           // 布尔类型
    ARRAY,          // 数组类型
    POINTER,        // 指针类型
    FUNCTION_TYPE,  // 函数类型
    UNKNOWN         // 未知类型
};

/**
 * @brief 符号属性信息
 */
struct SymbolInfo {
    std::string name;           // 符号名称
    SymbolType symbolType;      // 符号类型
    DataType dataType;          // 数据类型
    int line;                   // 定义行号
    int column;                 // 定义列号
    int scopeLevel;             // 作用域层级
    bool isInitialized;         // 是否已初始化
    bool isUsed;               // 是否被使用
    
    // 函数特有属性
    std::vector<DataType> paramTypes;   // 参数类型列表
    DataType returnType;                // 返回类型
    
    // 数组特有属性
    int arraySize;              // 数组大小
    DataType elementType;       // 元素类型
    
    // 常量值
    std::string constantValue;  // 常量值
    
    // 默认构造函数
    SymbolInfo() : name(""), symbolType(SymbolType::VARIABLE), dataType(DataType::UNKNOWN),
                   line(0), column(0), scopeLevel(0), isInitialized(false), isUsed(false),
                   returnType(DataType::VOID), arraySize(0), elementType(DataType::VOID) {}
    
    SymbolInfo(const std::string& n, SymbolType st, DataType dt, 
               int l = 0, int c = 0, int scope = 0)
        : name(n), symbolType(st), dataType(dt), line(l), column(c), 
          scopeLevel(scope), isInitialized(false), isUsed(false),
          returnType(DataType::VOID), arraySize(0), elementType(DataType::VOID) {}
};

/**
 * @brief 作用域类
 */
class Scope {
public:
    int level;                                              // 作用域层级
    std::unordered_map<std::string, SymbolInfo> symbols;   // 符号表
    Scope* parent;                                          // 父作用域
    std::vector<std::unique_ptr<Scope>> children;          // 子作用域
    
    Scope(int l = 0, Scope* p = nullptr) : level(l), parent(p) {}
    
    /**
     * @brief 在当前作用域中查找符号
     */
    SymbolInfo* findLocal(const std::string& name);
    
    /**
     * @brief 在当前作用域及父作用域中查找符号
     */
    SymbolInfo* findSymbol(const std::string& name);
    
    /**
     * @brief 添加符号到当前作用域
     */
    bool addSymbol(const SymbolInfo& symbol);
    
    /**
     * @brief 检查符号是否在当前作用域中定义
     */
    bool isDefined(const std::string& name) const;
    
    /**
     * @brief 获取作用域中的所有符号
     */
    std::vector<SymbolInfo*> getAllSymbols();
    
    /**
     * @brief 打印作用域信息（调试用）
     */
    void print(int indent = 0) const;
};

/**
 * @brief 符号表管理器
 */
class SymbolTable {
private:
    std::unique_ptr<Scope> globalScope;    // 全局作用域
    Scope* currentScope;                   // 当前作用域
    std::stack<Scope*> scopeStack;        // 作用域栈
    int nextScopeLevel;                    // 下一个作用域层级
    
public:
    SymbolTable();
    ~SymbolTable() = default;
    
    // 复制构造函数和赋值运算符
    SymbolTable(const SymbolTable& other);
    SymbolTable& operator=(const SymbolTable& other);
    
    /**
     * @brief 进入新作用域
     */
    void enterScope();
    
    /**
     * @brief 退出当前作用域
     */
    void exitScope();
    
    /**
     * @brief 添加符号
     */
    bool addSymbol(const SymbolInfo& symbol);
    
    /**
     * @brief 查找符号
     */
    SymbolInfo* findSymbol(const std::string& name);
    
    /**
     * @brief 在当前作用域查找符号
     */
    SymbolInfo* findLocalSymbol(const std::string& name);
    
    /**
     * @brief 检查符号是否已定义
     */
    bool isDefined(const std::string& name);
    
    /**
     * @brief 检查符号是否在当前作用域已定义
     */
    bool isLocalDefined(const std::string& name);
    
    /**
     * @brief 标记符号为已使用
     */
    void markSymbolUsed(const std::string& name);
    
    /**
     * @brief 标记符号为已初始化
     */
    void markSymbolInitialized(const std::string& name);
    
    /**
     * @brief 获取当前作用域层级
     */
    int getCurrentScopeLevel() const;
    
    /**
     * @brief 获取全局作用域
     */
    Scope* getGlobalScope() const;
    
    /**
     * @brief 获取当前作用域
     */
    Scope* getCurrentScope() const;
    
    /**
     * @brief 检查未使用的变量
     */
    std::vector<SymbolInfo*> getUnusedVariables();
    
    /**
     * @brief 检查未初始化的变量
     */
    std::vector<SymbolInfo*> getUninitializedVariables();
    
    /**
     * @brief 打印符号表（调试用）
     */
    void print() const;
    
    /**
     * @brief 清空符号表
     */
    void clear();
};

/**
 * @brief 类型工具类
 */
class TypeUtils {
public:
    /**
     * @brief 将TokenType转换为DataType
     */
    static DataType tokenTypeToDataType(TokenType tokenType);
    
    /**
     * @brief 将DataType转换为字符串
     */
    static std::string dataTypeToString(DataType dataType);
    
    /**
     * @brief 将SymbolType转换为字符串
     */
    static std::string symbolTypeToString(SymbolType symbolType);
    
    /**
     * @brief 检查两个类型是否兼容
     */
    static bool areTypesCompatible(DataType type1, DataType type2);
    
    /**
     * @brief 检查类型是否可以隐式转换
     */
    static bool canImplicitlyConvert(DataType from, DataType to);
    
    /**
     * @brief 获取二元运算的结果类型
     */
    static DataType getBinaryOperationResultType(DataType left, DataType right, TokenType op);
    
    /**
     * @brief 获取一元运算的结果类型
     */
    static DataType getUnaryOperationResultType(DataType operand, TokenType op);
    
    /**
     * @brief 检查类型是否为数值类型
     */
    static bool isNumericType(DataType type);
    
    /**
     * @brief 检查类型是否为整型
     */
    static bool isIntegerType(DataType type);
    
    /**
     * @brief 检查类型是否为浮点型
     */
    static bool isFloatingType(DataType type);
    
    /**
     * @brief 获取类型的字节大小
     */
    static int getTypeSize(DataType type);
};

#endif // SYMBOL_TABLE_H 