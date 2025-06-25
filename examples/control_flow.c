// 控制流语句示例

// if-else 条件语句
int temperature = 25;
if (temperature > 30) {
    int status = 1;  // 热
} else if (temperature < 10) {
    int status = -1; // 冷
} else {
    int status = 0;  // 适中
}

// while 循环
int count = 0;
while (count < 10) {
    count += 1;
}

// for 循环
int sum = 0;
for (int i = 1; i <= 100; i += 1) {
    sum += i;
}

// do-while 循环  
int number = 5;
int factorial = 1;
do {
    factorial *= number;
    number -= 1;
} while (number > 0);

// 嵌套循环和break/continue
int matrix_sum = 0;
for (int row = 0; row < 5; row += 1) {
    for (int col = 0; col < 5; col += 1) {
        if (row == col) {
            continue;  // 跳过对角线元素
        }
        if (row + col > 6) {
            break;     // 提前退出内层循环
        }
        matrix_sum += row * col;
    }
}

// 复杂条件判断
int x = 15;
int y = 25;
bool is_valid = false;
if ((x > 10 && y < 30) || (x == 15 && y == 25)) {
    if (!(x == y)) {
        is_valid = true;
    }
} 