/**
 * @file function_example.js
 * @brief 函数声明和调用示例
 */

// 简单函数
function add(a, b) {
    return a + b;
}

// 带类型推断的函数
function calculateDamage(base, multiplier) {
    return base * multiplier;
}

// 调用函数
let result = add(10, 20);
let damage = calculateDamage(100, 1.5);

// 多个返回值
function getStatus(health) {
    if (health > 50) {
        return "healthy";
    } else {
        return "wounded";
    }
}

let status = getStatus(75);
