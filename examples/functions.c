// 函数定义和调用示例

// 简单函数
int add(int a, int b) {
    return a + b;
}

// 递归函数
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

// 斐波那契数列
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// 求最大值
int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

// 判断素数
bool isPrime(int num) {
    if (num <= 1) {
        return false;
    }
    for (int i = 2; i * i <= num; i += 1) {
        if (num % i == 0) {
            return false;
        }
    }
    return true;
}

// 数组处理函数（模拟）
int sumArray(int arr, int size) {
    int total = 0;
    for (int i = 0; i < size; i += 1) {
        total += arr;  // 简化的数组访问
    }
    return total;
}

// 主函数示例
int main() {
    // 函数调用示例
    int result1 = add(15, 25);
    int result2 = factorial(5);
    int result3 = fibonacci(8);
    int result4 = max(result1, result2);
    
    bool is_prime = isPrime(17);
    
    // 循环中调用函数
    int sum = 0;
    for (int i = 1; i <= 10; i += 1) {
        if (isPrime(i)) {
            sum += i;
        }
    }
    
    return 0;
} 