// 函数和控制流测试
// 测试函数定义、调用和控制语句

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

void print_numbers(int count) {
    for (int i = 1; i <= count; i++) {
        // 简单的打印模拟
        int result = factorial(i);
    }
}

int main() {
    int num = 5;
    
    while (num > 0) {
        print_numbers(num);
        num = num - 1;
    }
    
    return 0;
}
