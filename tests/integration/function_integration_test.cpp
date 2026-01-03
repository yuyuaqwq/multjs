/**
 * @file function_integration_test.cpp
 * @brief 函数与闭包集成测试
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include "test_helper.h"
#include <gtest/gtest.h>

namespace mjs::test {

/**
 * @class FunctionIntegrationTest
 * @brief 函数与闭包集成测试
 */
class FunctionIntegrationTest : public IntegrationTestHelper {
};

// ==================== 函数基础 ====================

TEST_F(FunctionIntegrationTest, FunctionDeclaration) {
    // 测试函数声明
    AssertEq(R"(
        function add(a, b) {
            return a + b;
        }
        add(2, 3);
    )", Value(5));

    AssertEq(R"(
        function greet(name) {
            return 'hello ' + name;
        }
        greet('world');
    )", Value("hello world"));
}

TEST_F(FunctionIntegrationTest, FunctionExpression) {
    // 测试函数表达式
    AssertEq(R"(
        const multiply = function(a, b) {
            return a * b;
        };
        multiply(3, 4);
    )", Value(12));
}

TEST_F(FunctionIntegrationTest, ArrowFunction) {
    // 测试箭头函数
    AssertEq(R"(
        const add = (a, b) => a + b;
        add(2, 3);
    )", Value(5));

    AssertEq(R"(
        const square = x => x * x;
        square(5);
    )", Value(25));
}

TEST_F(FunctionIntegrationTest, DefaultParameters) {
    // 测试默认参数
    AssertEq(R"(
        function greet(name = 'world') {
            return 'hello ' + name;
        }
        greet();
    )", Value("hello world"));

    AssertEq(R"(
        function greet(name = 'world') {
            return 'hello ' + name;
        }
        greet('alice');
    )", Value("hello alice"));
}

TEST_F(FunctionIntegrationTest, MultipleReturns) {
    // 测试多条return语句
    AssertEq(R"(
        function abs(x) {
            if (x < 0) {
                return -x;
            }
            return x;
        }
        abs(-5);
    )", Value(5));

    AssertEq(R"(
        function abs(x) {
            if (x < 0) {
                return -x;
            }
            return x;
        }
        abs(10);
    )", Value(10));
}

TEST_F(FunctionIntegrationTest, RecursiveFunction) {
    // 测试递归函数
    AssertEq(R"(
        function factorial(n) {
            if (n <= 1) {
                return 1;
            }
            return n * factorial(n - 1);
        }
        factorial(5);
    )", Value(120)); // 5! = 120
}

TEST_F(FunctionIntegrationTest, FibonacciRecursive) {
    // 测试斐波那契递归
    AssertEq(R"(
        function fib(n) {
            if (n <= 1) {
                return n;
            }
            return fib(n - 1) + fib(n - 2);
        }
        fib(10);
    )", Value(55)); // 第10个斐波那契数
}

// ==================== 闭包 ====================

TEST_F(FunctionIntegrationTest, SimpleClosure) {
    // 测试简单闭包
    AssertEq(R"(
        function createCounter() {
            let count = 0;
            return function() {
                count += 1;
                return count;
            };
        }

        const counter = createCounter();
        counter();
    )", Value(1));
}

TEST_F(FunctionIntegrationTest, ClosureMaintainsState) {
    // 测试闭包保持状态
    AssertEq(R"(
        function createCounter() {
            let count = 0;
            return function() {
                count += 1;
                return count;
            };
        }

        const counter = createCounter();
        counter();
        counter();
        counter();
    )", Value(3));
}

TEST_F(FunctionIntegrationTest, ClosureWithParameters) {
    // 测试带参数的闭包
    AssertEq(R"(
        function makeAdder(x) {
            return function(y) {
                return x + y;
            };
        }

        const add5 = makeAdder(5);
        add5(10);
    )", Value(15));
}

TEST_F(FunctionIntegrationTest, MultipleClosures) {
    // 测试多个闭包独立工作
    AssertEq(R"(
        function createCounter() {
            let count = 0;
            return function() {
                count += 1;
                return count;
            };
        }

        const counter1 = createCounter();
        const counter2 = createCounter();

        counter1();
        counter1();

        counter2();

        counter1();
    )", Value(3)); // counter1 = 3, counter2 = 1
}

TEST_F(FunctionIntegrationTest, ClosureModifiesOuterVariable) {
    // 测试闭包修改外部变量
    AssertEq(R"(
        function createAccumulator() {
            let sum = 0;
            return {
                add: function(x) {
                    sum += x;
                },
                getSum: function() {
                    return sum;
                }
            };
        }

        const acc = createAccumulator();
        acc.add(5);
        acc.add(10);
        acc.add(15);
        acc.getSum();
    )", Value(30));
}

TEST_F(FunctionIntegrationTest, NestedClosures) {
    // 测试嵌套闭包
    AssertEq(R"(
        function outer() {
            let x = 10;

            function middle() {
                let y = 20;

                function inner() {
                    return x + y;
                }

                return inner;
            }

            return middle;
        }

        const fn = outer()();
        fn();
    )", Value(30));
}

TEST_F(FunctionIntegrationTest, ClosureInLoop) {
    // 测试循环中的闭包
    AssertEq(R"(
        function createFunctions() {
            const funcs = [];
            for (let i = 0; i < 3; i += 1) {
                funcs.push(function() {
                    return i;
                });
            }
            return funcs;
        }

        const funcs = createFunctions();
        funcs[0]() + funcs[1]() + funcs[2]();
    )", Value(3)); // 0 + 1 + 2 = 3
}

TEST_F(FunctionIntegrationTest, ClosureWithMultipleReferences) {
    // 测试多个闭包引用同一变量
    AssertTrue(R"(
        let shared = 10;

        const getter = function() {
            return shared;
        };

        const setter = function(val) {
            shared = val;
        };

        setter(20);
        getter() === 20;
    )");
}

// ==================== 高阶函数 ====================

TEST_F(FunctionIntegrationTest, FunctionAsParameter) {
    // 测试函数作为参数
    AssertEq(R"(
        function apply(fn, x, y) {
            return fn(x, y);
        }

        function add(a, b) {
            return a + b;
        }

        apply(add, 5, 3);
    )", Value(8));
}

TEST_F(FunctionIntegrationTest, FunctionReturningFunction) {
    // 测试函数返回函数
    AssertEq(R"(
        function multiplier(factor) {
            return function(number) {
                return number * factor;
            };
        }

        const double = multiplier(2);
        const triple = multiplier(3);

        double(5) + triple(5);
    )", Value(25)); // 10 + 15 = 25
}

TEST_F(FunctionIntegrationTest, MapLikeOperation) {
    // 测试类似map的操作
    AssertEq(R"(
        function map(arr, fn) {
            const result = [];
            for (let i = 0; i < arr.length; i += 1) {
                result.push(fn(arr[i]));
            }
            return result;
        }

        const arr = [1, 2, 3, 4];
        const doubled = map(arr, x => x * 2);
        doubled[0] + doubled[1] + doubled[2] + doubled[3];
    )", Value(20)); // 2 + 4 + 6 + 8 = 20
}

TEST_F(FunctionIntegrationTest, FilterLikeOperation) {
    // 测试类似filter的操作
    AssertEq(R"(
        function filter(arr, predicate) {
            const result = [];
            for (let i = 0; i < arr.length; i += 1) {
                if (predicate(arr[i])) {
                    result.push(arr[i]);
                }
            }
            return result;
        }

        const arr = [1, 2, 3, 4, 5, 6];
        const evens = filter(arr, x => x % 2 === 0);
        evens.length;
    )", Value(3)); // 3个偶数
}

TEST_F(FunctionIntegrationTest, ReduceLikeOperation) {
    // 测试类似reduce的操作
    AssertEq(R"(
        function reduce(arr, fn, initial) {
            let result = initial;
            for (let i = 0; i < arr.length; i += 1) {
                result = fn(result, arr[i]);
            }
            return result;
        }

        const arr = [1, 2, 3, 4, 5];
        reduce(arr, (sum, x) => sum + x, 0);
    )", Value(15)); // 1+2+3+4+5 = 15
}

// ==================== 复杂场景 ====================

TEST_F(FunctionIntegrationTest, MemoizationPattern) {
    // 测试记忆化模式
    AssertEq(R"(
        function memoize(fn) {
            const cache = {};

            return function(x) {
                if (x in cache) {
                    return cache[x];
                }
                const result = fn(x);
                cache[x] = result;
                return result;
            };
        }

        function expensive(x) {
            return x * x;
        }

        const memoExpensive = memoize(expensive);
        memoExpensive(10) + memoExpensive(10);
    )", Value(200)); // 100 + 100 (第二次从缓存读取)
}

TEST_F(FunctionIntegrationTest, CurryPattern) {
    // 测试柯里化
    AssertEq(R"(
        function curry(fn) {
            return function(a) {
                return function(b) {
                    return fn(a, b);
                };
            };
        }

        function add(a, b) {
            return a + b;
        }

        const curriedAdd = curry(add);
        const add5 = curriedAdd(5);
        add5(10);
    )", Value(15));
}

TEST_F(FunctionIntegrationTest, ModulePattern) {
    // 测试模块模式
    AssertEq(R"(
        function createModule() {
            let privateVar = 0;

            return {
                increment: function() {
                    privateVar += 1;
                },
                decrement: function() {
                    privateVar -= 1;
                },
                getValue: function() {
                    return privateVar;
                }
            };
        }

        const mod = createModule();
        mod.increment();
        mod.increment();
        mod.decrement();
        mod.getValue();
    )", Value(1)); // 0 + 1 + 1 - 1 = 1
}

TEST_F(FunctionIntegrationTest, LazyEvaluation) {
    // 测试惰性求值
    AssertEq(R"(
        function lazy(thunk) {
            let cached = false;
            let result;

            return function() {
                if (!cached) {
                    result = thunk();
                    cached = true;
                }
                return result;
            };
        }

        let callCount = 0;
        const expensive = lazy(function() {
            callCount += 1;
            return 42;
        });

        expensive();
        expensive();
        expensive();
        callCount;
    )", Value(1)); // 只调用了一次
}

} // namespace mjs::test
