// 错误识别测试代码 - 包含多种错误类型
int a = 5;
int x = "hello";           // 错误1: 类型不匹配
b = a + 3;                 // 错误2: 未定义变量b
int c = a / 0;             // 错误3: 除零错误
int result = myFunction(a); // 错误4: 未定义函数myFunction
int str_calc = "text" + 5;  // 错误5: 字符串算术运算

int main() {
    return "error";         // 错误6: 返回类型不匹配
}