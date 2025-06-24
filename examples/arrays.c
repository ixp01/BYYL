// 数组和复杂表达式测试
// 测试数组操作和复杂运算

int global_array[10];
float pi = 3.14159;

int sum_array(int arr[], int size) {
    int total = 0;
    for (int i = 0; i < size; i++) {
        total = total + arr[i];
    }
    return total;
}

bool is_prime(int n) {
    if (n <= 1) {
        return false;
    }
    
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

int main() {
    int numbers[5] = {2, 3, 5, 7, 11};
    int sum = sum_array(numbers, 5);
    
    // 复杂条件表达式
    bool result = (sum > 20) && is_prime(sum);
    
    // 三元运算符
    int final_value = result ? sum * 2 : sum / 2;
    
    return final_value;
}
