/**
 * @file async_integration_test.cpp
 * @brief 异步特性集成测试
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include "test_helper.h"
#include <gtest/gtest.h>

namespace mjs::test {

/**
 * @class AsyncIntegrationTest
 * @brief 异步特性集成测试
 */
class AsyncIntegrationTest : public IntegrationTestHelper {
};

// ==================== Promise基础 ====================

TEST_F(AsyncIntegrationTest, PromiseResolve) {
    // 测试Promise.resolve
    AssertEq(R"(
        const promise = new Promise(function(resolve, reject) {
            resolve(42);
        });
        return promise;
    )", Value(42)); // Promise对象本身
}

TEST_F(AsyncIntegrationTest, PromiseReject) {
    // 测试Promise.reject
    AssertThrows(R"(
        const promise = new Promise(function(resolve, reject) {
            reject(new Error('Failed'));
        });
        promise;
    )");
}

TEST_F(AsyncIntegrationTest, PromiseThen) {
    // 测试Promise.then
    AssertEq(R"(
        let result = 0;
        const promise = Promise.resolve(10);

        promise.then(function(value) {
            result = value * 2;
        });

        result; // 注意：这可能在then回调执行前返回
    )", Value(0)); // 初始值为0
}

// ==================== Async/Await ====================

TEST_F(AsyncIntegrationTest, AsyncFunctionDeclaration) {
    // 测试async函数声明
    AssertEq(R"(
        async function fetchData() {
            return 42;
        }

        fetchData();
    )", Value(42)); // async函数返回Promise
}

TEST_F(AsyncIntegrationTest, AwaitExpression) {
    // 测试await表达式
    AssertEq(R"(
        async function testAwait() {
            const promise = Promise.resolve(10);
            const result = await promise;
            return result * 2;
        }

        testAwait();
    )", Value(20));
}

TEST_F(AsyncIntegrationTest, AwaitWithRejectedPromise) {
    // 测试await处理rejected Promise
    AssertThrows(R"(
        async function testReject() {
            const promise = Promise.reject(new Error('Failed'));
            await promise;
        }

        testReject();
    )");
}

TEST_F(AsyncIntegrationTest, MultipleAwait) {
    // 测试多个await表达式
    AssertEq(R"(
        async function multipleAwait() {
            const value1 = await Promise.resolve(10);
            const value2 = await Promise.resolve(20);
            const value3 = await Promise.resolve(30);
            return value1 + value2 + value3;
        }

        multipleAwait();
    )", Value(60));
}

// ==================== 生成器函数 ====================

TEST_F(AsyncIntegrationTest, BasicGenerator) {
    // 测试基本的生成器函数
    AssertEq(R"(
        function* numberGenerator() {
            yield 1;
            yield 2;
            yield 3;
        }

        const gen = numberGenerator();
        gen.next().value + gen.next().value;
    )", Value(3)); // 1 + 2 = 3
}

TEST_F(AsyncIntegrationTest, GeneratorWithReturn) {
    // 测试带return的生成器
    AssertEq(R"(
        function* generator() {
            yield 1;
            yield 2;
            return 3;
        }

        const gen = generator();
        gen.next().value;
    )", Value(1));
}

TEST_F(AsyncIntegrationTest, GeneratorWithYieldStar) {
    // 测试yield*委托
    AssertEq(R"(
        function* innerGenerator() {
            yield 1;
            yield 2;
        }

        function* outerGenerator() {
            yield* innerGenerator();
            yield 3;
        }

        const gen = outerGenerator();
        gen.next().value + gen.next().value + gen.next().value;
    )", Value(6)); // 1 + 2 + 3 = 6
}

TEST_F(AsyncIntegrationTest, InfiniteGenerator) {
    // 测试无限生成器
    AssertEq(R"(
        function* counter() {
            let count = 0;
            while (true) {
                yield count;
                count += 1;
            }
        }

        const gen = counter();
        gen.next().value + gen.next().value + gen.next().value;
    )", Value(3)); // 0 + 1 + 2 = 3
}

// ==================== 复杂场景 ====================

TEST_F(AsyncIntegrationTest, SequentialAsyncOperations) {
    // 测试顺序执行异步操作
    AssertEq(R"(
        async function sequential() {
            const result1 = await Promise.resolve(10);
            const result2 = await Promise.resolve(result1 * 2);
            const result3 = await Promise.resolve(result2 + 5);
            return result3;
        }

        sequential();
    )", Value(25)); // ((10 * 2) + 5) = 25
}

TEST_F(AsyncIntegrationTest, ParallelAsyncOperations) {
    // 测试并行执行异步操作
    AssertEq(R"(
        async function parallel() {
            const promise1 = Promise.resolve(10);
            const promise2 = Promise.resolve(20);
            const promise3 = Promise.resolve(30);

            const [result1, result2, result3] = [await promise1, await promise2, await promise3];
            return result1 + result2 + result3;
        }

        parallel();
    )", Value(60)); // 10 + 20 + 30 = 60
}

TEST_F(AsyncIntegrationTest, AsyncErrorHandling) {
    // 测试async函数的异常处理
    AssertEq(R"(
        async function withErrorHandling() {
            try {
                await Promise.reject(new Error('Failed'));
                return 'success';
            } catch (e) {
                return 'caught';
            }
        }

        withErrorHandling();
    )", Value("caught"));
}

TEST_F(AsyncIntegrationTest, GeneratorFibonacci) {
    // 测试使用生成器实现斐波那契数列
    AssertEq(R"(
        function* fibonacci() {
            let [prev, curr] = [0, 1];
            while (true) {
                yield curr;
                [prev, curr] = [curr, prev + curr];
            }
        }

        const gen = fibonacci();
        gen.next().value; // 1
        gen.next().value; // 1
        gen.next().value; // 2
        gen.next().value; // 3
        gen.next().value; // 5
        gen.next().value;
    )", Value(8)); // 第6个斐波那契数
}

TEST_F(AsyncIntegrationTest, AsyncGenerator) {
    // 测试异步生成器
    AssertEq(R"(
        async function* asyncGen() {
            yield await Promise.resolve(1);
            yield await Promise.resolve(2);
            yield await Promise.resolve(3);
        }

        async function run() {
            let sum = 0;
            const gen = asyncGen();
            sum += (await gen.next()).value;
            sum += (await gen.next()).value;
            sum += (await gen.next()).value;
            return sum;
        }

        run();
    )", Value(6)); // 1 + 2 + 3 = 6
}

TEST_F(AsyncIntegrationTest, PromiseChain) {
    // 测试Promise链式调用
    AssertEq(R"(
        Promise.resolve(5)
            .then(function(x) {
                return x * 2;
            })
            .then(function(x) {
                return x + 3;
            })
            .then(function(x) {
                return x * 2;
            });
    )", Value(26)); // ((5 * 2) + 3) * 2 = 26
}

TEST_F(AsyncIntegrationTest, PromiseAll) {
    // 测试Promise.all
    AssertEq(R"(
        Promise.all([
            Promise.resolve(1),
            Promise.resolve(2),
            Promise.resolve(3)
        ]).then(function(results) {
            return results[0] + results[1] + results[2];
        });
    )", Value(6)); // 1 + 2 + 3 = 6
}

TEST_F(AsyncIntegrationTest, PromiseRace) {
    // 测试Promise.race
    AssertEq(R"(
        Promise.race([
            Promise.resolve(1),
            Promise.resolve(2),
            Promise.resolve(3)
        ]).then(function(result) {
            return result * 10;
        });
    )", Value(10)); // 第一个完成的结果 * 10
}

// ==================== 微任务队列测试 ====================

TEST_F(AsyncIntegrationTest, MicrotaskOrder) {
    // 测试微任务执行顺序
    AssertEq(R"(
        let order = [];

        Promise.resolve().then(function() {
            order.push(1);
        });

        Promise.resolve().then(function() {
            order.push(2);
        });

        Promise.resolve().then(function() {
            order.push(3);
        });

        order[0] + order[1] + order[2];
    )", Value(6)); // 1 + 2 + 3 = 6
}

TEST_F(AsyncIntegrationTest, MicrotaskWithSync) {
    // 测试微任务与同步代码的执行顺序
    AssertEq(R"(
        let result = 0;

        result += 1; // 同步

        Promise.resolve().then(function() {
            result += 10;
        });

        result += 1; // 同步

        result;
    )", Value(2)); // 只执行同步代码,微任务尚未执行
}

// ==================== 实际应用场景 ====================

TEST_F(AsyncIntegrationTest, FetchSimulation) {
    // 测试模拟fetch请求
    AssertEq(R"(
        function simulateFetch(url) {
            return new Promise(function(resolve) {
                resolve('Data from ' + url);
            });
        }

        async function fetchData() {
            const data = await simulateFetch('/api/data');
            return data;
        }

        fetchData();
    )", Value("Data from /api/data"));
}

TEST_F(AsyncIntegrationTest, RetryPattern) {
    // 测试重试模式
    AssertEq(R"(
        let attempts = 0;

        function unreliableOperation() {
            attempts += 1;
            if (attempts < 3) {
                return Promise.reject(new Error('Failed'));
            }
            return Promise.resolve('success');
        }

        async function retry(operation, maxRetries) {
            for (let i = 0; i < maxRetries; i += 1) {
                try {
                    return await operation();
                } catch (e) {
                    if (i === maxRetries - 1) {
                        throw e;
                    }
                }
            }
        }

        retry(unreliableOperation, 5);
    )", Value("success"));
}

TEST_F(AsyncIntegrationTest, TimeoutPattern) {
    // 测试超时模式
    AssertEq(R"(
        function withTimeout(promise, ms) {
            return Promise.race([
                promise,
                new Promise(function(_, reject) {
                    setTimeout(function() {
                        reject(new Error('Timeout'));
                    }, ms);
                })
            ]);
        }

        withTimeout(Promise.resolve('done'), 1000);
    )", Value("done"));
}

TEST_F(AsyncIntegrationTest, BatchingRequests) {
    // 测试批量请求
    AssertEq(R"(
        function fetchItem(id) {
            return Promise.resolve('Item ' + id);
        }

        async function fetchAllItems(ids) {
            const promises = [];
            for (let i = 0; i < ids.length; i += 1) {
                promises.push(fetchItem(ids[i]));
            }
            const results = await Promise.all(promises);
            return results.join(', ');
        }

        fetchAllItems([1, 2, 3]);
    )", Value("Item 1, Item 2, Item 3"));
}

} // namespace mjs::test
