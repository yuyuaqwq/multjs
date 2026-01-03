/**
 * @file performance_integration_test.cpp
 * @brief 性能与边界测试
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include "test_helper.h"
#include <gtest/gtest.h>

namespace mjs::test {

/**
 * @class PerformanceIntegrationTest
 * @brief 性能与边界集成测试
 */
class PerformanceIntegrationTest : public IntegrationTestHelper {
};

// ==================== 大规模操作 ====================

TEST_F(PerformanceIntegrationTest, LargeArrayOperations) {
    // 测试大数组操作
    AssertEq(R"(
        const size = 1000;
        const arr = [];
        for (let i = 0; i < size; i += 1) {
            arr.push(i);
        }

        let sum = 0;
        for (let i = 0; i < arr.length; i += 1) {
            sum += arr[i];
        }
        sum;
    )", Value(499500)); // 0 + 1 + ... + 999 = 499500
}

TEST_F(PerformanceIntegrationTest, ManyObjectCreations) {
    // 测试大量对象创建
    AssertEq(R"(
        class Point {
            constructor(x, y) {
                this.x = x;
                this.y = y;
            }
        }

        const points = [];
        for (let i = 0; i < 100; i += 1) {
            points.push(new Point(i, i * 2));
        }

        points.length;
    )", Value(100));
}

TEST_F(PerformanceIntegrationTest, DeepRecursion) {
    // 测试深度递归
    AssertEq(R"(
        function factorial(n) {
            if (n <= 1) {
                return 1;
            }
            return n * factorial(n - 1);
        }

        factorial(10);
    )", Value(3628800)); // 10! = 3628800
}

TEST_F(PerformanceIntegrationTest, ManyClosureCreations) {
    // 测试大量闭包创建
    AssertEq(R"(
        const closures = [];
        for (let i = 0; i < 100; i += 1) {
            closures.push(function() {
                return i;
            });
        }

        let sum = 0;
        for (let i = 0; i < 10; i += 1) {
            sum += closures[i]();
        }
        sum;
    )", Value(45)); // 0 + 1 + ... + 9 = 45
}

// ==================== 字符串操作 ====================

TEST_F(PerformanceIntegrationTest, LargeStringConcatenation) {
    // 测试大字符串拼接
    AssertEq(R"(
        let result = '';
        for (let i = 0; i < 100; i += 1) {
            result += 'a';
        }
        result.length;
    )", Value(100));
}

TEST_F(PerformanceIntegrationTest, StringOperations) {
    // 测试字符串操作
    AssertEq(R"(
        const str = 'hello world';
        str.length;
    )", Value(11));
}

// ==================== 内存管理 ====================

TEST_F(PerformanceIntegrationTest, ObjectLifecycle) {
    // 测试对象生命周期
    AssertEq(R"(
        function createObjects() {
            const objs = [];
            for (let i = 0; i < 10; i += 1) {
                objs.push({ value: i });
            }
            return objs.length;
        }

        createObjects();
    )", Value(10));
}

TEST_F(PerformanceIntegrationTest, CircularReference) {
    // 测试循环引用
    AssertEq(R"(
        const obj1 = {};
        const obj2 = {};
        obj1.ref = obj2;
        obj2.ref = obj1;

        obj1.ref.ref === obj1;
    )", Value(true));
}

// ==================== 数值边界 ====================

TEST_F(PerformanceIntegrationTest, LargeNumbers) {
    // 测试大数运算
    AssertEq(R"(
        1e10 + 1e10;
    )", Value(2e10));
}

TEST_F(PerformanceIntegrationTest, FloatingPointPrecision) {
    // 测试浮点数精度
    AssertEq(R"(
        0.1 + 0.2;
    )", Value(0.1 + 0.2)); // JavaScript中的0.1 + 0.2 !== 0.3
}

TEST_F(PerformanceIntegrationTest, NumericOperations) {
    // 测试数值运算
    AssertEq(R"(
        let result = 0;
        for (let i = 0; i < 100; i += 1) {
            result += i;
            result *= 2;
            result /= 2;
        }
        result;
    )", Value(4950)); // 0 + 1 + ... + 99 = 4950
}

// ==================== 算法性能 ====================

TEST_F(PerformanceIntegrationTest, Sorting) {
    // 测试排序
    AssertEq(R"(
        const arr = [5, 2, 8, 1, 9, 3, 7, 4, 6];

        // 简单的冒泡排序
        for (let i = 0; i < arr.length; i += 1) {
            for (let j = 0; j < arr.length - i - 1; j += 1) {
                if (arr[j] > arr[j + 1]) {
                    const temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            }
        }

        arr[0] + arr[arr.length - 1];
    )", Value(10)); // 1 + 9 = 10
}

TEST_F(PerformanceIntegrationTest, SearchAlgorithm) {
    // 测试搜索算法
    AssertEq(R"(
        const arr = [1, 3, 5, 7, 9, 11, 13, 15];

        function binarySearch(arr, target) {
            let left = 0;
            let right = arr.length - 1;

            while (left <= right) {
                const mid = Math.floor((left + right) / 2);
                if (arr[mid] === target) {
                    return mid;
                }
                if (arr[mid] < target) {
                    left = mid + 1;
                } else {
                    right = mid - 1;
                }
            }
            return -1;
        }

        binarySearch(arr, 7);
    )", Value(3)); // 索引为3
}

TEST_F(PerformanceIntegrationTest, FibonacciPerformance) {
    // 测试斐波那契性能
    AssertEq(R"(
        function fib(n) {
            if (n <= 1) {
                return n;
            }
            let prev = 0;
            let curr = 1;
            for (let i = 2; i <= n; i += 1) {
                const temp = prev + curr;
                prev = curr;
                curr = temp;
            }
            return curr;
        }

        fib(50);
    )", Value(12586269025)); // 第50个斐波那契数
}

// ==================== 复杂数据结构 ====================

TEST_F(PerformanceIntegrationTest, StackImplementation) {
    // 测试栈实现
    AssertEq(R"(
        class Stack {
            constructor() {
                this.items = [];
            }

            push(item) {
                this.items.push(item);
            }

            pop() {
                return this.items.pop();
            }

            peek() {
                return this.items[this.items.length - 1];
            }

            size() {
                return this.items.length;
            }
        }

        const stack = new Stack();
        for (let i = 0; i < 10; i += 1) {
            stack.push(i);
        }

        stack.size();
    )", Value(10));
}

TEST_F(PerformanceIntegrationTest, QueueImplementation) {
    // 测试队列实现
    AssertEq(R"(
        class Queue {
            constructor() {
                this.items = [];
            }

            enqueue(item) {
                this.items.push(item);
            }

            dequeue() {
                return this.items.shift();
            }

            size() {
                return this.items.length;
            }
        }

        const queue = new Queue();
        for (let i = 0; i < 10; i += 1) {
            queue.enqueue(i);
        }

        queue.dequeue();
        queue.size();
    )", Value(9));
}

// ==================== 边界条件 ====================

TEST_F(PerformanceIntegrationTest, EmptyArrayOperations) {
    // 测试空数组操作
    AssertEq(R"(
        const arr = [];
        arr.push(1);
        arr.push(2);
        arr.length;
    )", Value(2));
}

TEST_F(PerformanceIntegrationTest, SingleElementArray) {
    // 测试单元素数组
    AssertEq(R"(
        const arr = [42];
        arr[0];
    )", Value(42));
}

TEST_F(PerformanceIntegrationTest, ZeroIterations) {
    // 测试零次迭代
    AssertEq(R"(
        let sum = 0;
        for (let i = 0; i < 0; i += 1) {
            sum += i;
        }
        sum;
    )", Value(0));
}

TEST_F(PerformanceIntegrationTest, ConditionalBranches) {
    // 测试条件分支
    AssertEq(R"(
        let result = 0;
        for (let i = 0; i < 100; i += 1) {
            if (i % 2 === 0) {
                result += 1;
            } else {
                result -= 1;
            }
        }
        result;
    )", Value(0)); // 50次+1和50次-1
}

// ==================== 实际应用场景 ====================

TEST_F(PerformanceIntegrationTest, MatrixOperations) {
    // 测试矩阵操作
    AssertEq(R"(
        function createMatrix(rows, cols) {
            const matrix = [];
            for (let i = 0; i < rows; i += 1) {
                const row = [];
                for (let j = 0; j < cols; j += 1) {
                    row.push(i * cols + j);
                }
                matrix.push(row);
            }
            return matrix;
        }

        function matrixSum(matrix) {
            let sum = 0;
            for (let i = 0; i < matrix.length; i += 1) {
                for (let j = 0; j < matrix[i].length; j += 1) {
                    sum += matrix[i][j];
                }
            }
            return sum;
        }

        const matrix = createMatrix(10, 10);
        matrixSum(matrix);
    )", Value(4950)); // 0+1+...+99 = 4950
}

TEST_F(PerformanceIntegrationTest, DataProcessingPipeline) {
    // 测试数据处理管道
    AssertEq(R"(
        // 生成数据
        const data = [];
        for (let i = 0; i < 100; i += 1) {
            data.push(i);
        }

        // 过滤
        const filtered = [];
        for (let i = 0; i < data.length; i += 1) {
            if (data[i] % 2 === 0) {
                filtered.push(data[i]);
            }
        }

        // 转换
        const mapped = [];
        for (let i = 0; i < filtered.length; i += 1) {
            mapped.push(filtered[i] * 2);
        }

        // 归约
        let sum = 0;
        for (let i = 0; i < mapped.length; i += 1) {
            sum += mapped[i];
        }

        sum;
    )", Value(9800)); // 偶数*2的和
}

TEST_F(PerformanceIntegrationTest, TreeTraversal) {
    // 测试树遍历
    AssertEq(R"(
        function createNode(value) {
            return { value: value, left: null, right: null };
        }

        function insert(root, value) {
            if (root === null) {
                return createNode(value);
            }
            if (value < root.value) {
                root.left = insert(root.left, value);
            } else {
                root.right = insert(root.right, value);
            }
            return root;
        }

        function inorderSum(node) {
            if (node === null) {
                return 0;
            }
            return inorderSum(node.left) + node.value + inorderSum(node.right);
        }

        let root = null;
        for (let i = 0; i < 10; i += 1) {
            root = insert(root, i);
        }

        inorderSum(root);
    )", Value(45)); // 0+1+...+9 = 45
}

// ==================== 压力测试 ====================

TEST_F(PerformanceIntegrationTest, NestedLoops) {
    // 测试嵌套循环
    AssertEq(R"(
        let count = 0;
        for (let i = 0; i < 10; i += 1) {
            for (let j = 0; j < 10; j += 1) {
                for (let k = 0; k < 10; k += 1) {
                    count += 1;
                }
            }
        }
        count;
    )", Value(1000)); // 10*10*10 = 1000
}

TEST_F(PerformanceIntegrationTest, ManyFunctionCalls) {
    // 测试大量函数调用
    AssertEq(R"(
        function add(a, b) {
            return a + b;
        }

        let result = 0;
        for (let i = 0; i < 100; i += 1) {
            result = add(result, i);
        }
        result;
    )", Value(4950)); // 0+1+...+99 = 4950
}

} // namespace mjs::test
