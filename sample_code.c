// 编译原理课程设计 - 测试程序
// 包含多种语言特性用于测试各个分析阶段

#include <stdio.h>

// 全局变量
int global_var = 100;

// 函数声明
int add(int a, int b);
void print_result(int result);

int main() {
    // 局部变量声明
    int x = 10;
    int y = 20;
    float pi = 3.14;
    char letter = 'A';
    
    // 算术运算
    int sum = add(x, y);
    int product = x * y;
    
    // 条件语句
    if (sum > 25) {
        printf("Sum is greater than 25\n");
    } else {
        printf("Sum is less than or equal to 25\n");
    }
    
    // 循环语句
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration: %d\n", i);
    }
    
    // 函数调用
    print_result(sum);
    
    return 0;
}

// 函数定义
int add(int a, int b) {
    return a + b;
}

void print_result(int result) {
    printf("Final result: %d\n", result);
} 