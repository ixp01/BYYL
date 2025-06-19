#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <map>

/**
 * @brief Token类型枚举
 */
enum class TokenType {
    // 基本类型
    IDENTIFIER,     // 标识符 
    NUMBER,         // 数字常量
    REAL,           // 浮点数
    STRING,         // 字符串字面量
    
    // 关键字
    IF,             // if
    ELSE,           // else
    WHILE,          // while
    FOR,            // for
    DO,             // do
    BREAK,          // break
    CONTINUE,       // continue
    RETURN,         // return
    INT,            // int
    FLOAT,          // float
    BOOL,           // bool
    TRUE,           // true
    FALSE,          // false
    
    // 运算符
    ASSIGN,         // =
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    MODULO,         // %
    
    // 比较运算符
    EQ,             // ==
    NE,             // !=
    LT,             // <
    LE,             // <=
    GT,             // >
    GE,             // >=
    
    // 逻辑运算符
    AND,            // &&
    OR,             // ||
    NOT,            // !
    
    // 分隔符
    SEMICOLON,      // ;
    COMMA,          // ,
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]
    
    // 特殊
    EOF_TOKEN,      // 文件结束
    ERROR_TOKEN,    // 错误token
    NEWLINE,        // 换行符
    
    // AST专用（非词法单元）
    INDEX,          // 数组索引
    TEMP            // 临时变量
};

/**
 * @brief Token类，表示词法单元
 */
class Token {
public:
    TokenType type;         // Token类型
    std::string value;      // Token值
    int line;               // 行号
    int column;             // 列号
    
    /**
     * @brief 构造函数
     */
    Token(TokenType t = TokenType::ERROR_TOKEN, 
          const std::string& v = "", 
          int l = 1, 
          int c = 1);
    
    /**
     * @brief 判断是否为关键字
     */
    bool isKeyword() const;
    
    /**
     * @brief 判断是否为运算符
     */
    bool isOperator() const;
    
    /**
     * @brief 获取Token类型的字符串表示
     */
    std::string getTypeString() const;
    
    /**
     * @brief 转换为字符串（用于调试）
     */
    std::string toString() const;
    
    /**
     * @brief 获取关键字映射表
     */
    static const std::map<std::string, TokenType>& getKeywordMap();
    
private:
    static std::map<std::string, TokenType> keywordMap;
    static std::map<TokenType, std::string> typeStringMap;
};

#endif // TOKEN_H 