// 复合赋值运算符示例
int counter = 0;
int value = 100;
int multiplier = 2;
int divisor = 4;

// 递增和递减
counter += 1;        // counter = counter + 1
counter += 5;        // counter = counter + 5

// 减法赋值
value -= 10;         // value = value - 10
value -= counter;    // value = value - counter

// 乘法赋值  
multiplier *= 3;     // multiplier = multiplier * 3
value *= 2;          // value = value * 2

// 除法赋值
value /= divisor;    // value = value / divisor
divisor /= 2;        // divisor = divisor / 2

// 取模赋值
int remainder = 17;
remainder %= 5;      // remainder = remainder % 5

// 复合运算的实际应用
int score = 85;
int bonus = 10;
score += bonus;      // 加分
score *= 2;          // 双倍积分
score -= 50;         // 扣除惩罚分 