/**
 * @file exception_integration_test.cpp
 * @brief 异常处理集成测试
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include "test_helper.h"
#include <gtest/gtest.h>

namespace mjs::test {

/**
 * @class ExceptionIntegrationTest
 * @brief 异常处理集成测试
 */
class ExceptionIntegrationTest : public IntegrationTestHelper {
};

// ==================== Try/Catch基础 ====================

TEST_F(ExceptionIntegrationTest, BasicTryCatch) {
    // 测试基本的try-catch
    AssertEq(R"(
        try {
            throw 'error';
        } catch (e) {
            return 'caught: ' + e;
        }
    )", Value("caught: error"));
}

TEST_F(ExceptionIntegrationTest, CatchErrorObject) {
    // 测试捕获Error对象
    AssertEq(R"(
        try {
            throw new Error('Something went wrong');
        } catch (e) {
            return e.message;
        }
    )", Value("Something went wrong"));
}

TEST_F(ExceptionIntegrationTest, TryCatchFinally) {
    // 测试try-catch-finally
    AssertEq(R"(
        let result = '';
        try {
            result += 'try ';
            throw 'error';
        } catch (e) {
            result += 'catch ';
        } finally {
            result += 'finally';
        }
        result;
    )", Value("try catch finally"));
}

TEST_F(ExceptionIntegrationTest, FinallyWithoutCatch) {
    // 测试只有try-finally
    AssertEq(R"(
        let result = '';
        try {
            result += 'try ';
        } finally {
            result += 'finally';
        }
        result;
    )", Value("try finally"));
}

TEST_F(ExceptionIntegrationTest, FinallyAfterNoError) {
    // 测试没有异常时的finally
    AssertEq(R"(
        let result = '';
        try {
            result = 'success';
        } catch (e) {
            result = 'error';
        } finally {
            result += ' finally';
        }
        result;
    )", Value("success finally"));
}

// ==================== 抛出异常 ====================

TEST_F(ExceptionIntegrationTest, ThrowString) {
    // 测试抛出字符串
    AssertThrows(R"(
        throw 'error message';
    )");
}

TEST_F(ExceptionIntegrationTest, ThrowNumber) {
    // 测试抛出数字
    AssertThrows(R"(
        throw 404;
    )");
}

TEST_F(ExceptionIntegrationTest, ThrowObject) {
    // 测试抛出对象
    AssertThrows(R"(
        throw { code: 500, message: 'Server Error' };
    )");
}

TEST_F(ExceptionIntegrationTest, ThrowError) {
    // 测试抛出Error对象
    AssertThrows(R"(
        throw new Error('Custom error');
    )");
}

TEST_F(ExceptionIntegrationTest, ThrowTypeError) {
    // 测试抛出TypeError
    AssertThrows(R"(
        throw new TypeError('Type mismatch');
    )");
}

TEST_F(ExceptionIntegrationTest, ThrowReferenceError) {
    // 测试抛出ReferenceError
    AssertThrows(R"(
        throw new ReferenceError('Variable not defined');
    )");
}

// ==================== 嵌套异常处理 ====================

TEST_F(ExceptionIntegrationTest, NestedTryCatch) {
    // 测试嵌套try-catch
    AssertEq(R"(
        let result = '';
        try {
            try {
                throw 'inner error';
            } catch (innerError) {
                result += 'inner: ' + innerError + ' ';
                throw 'outer error';
            }
        } catch (outerError) {
            result += 'outer: ' + outerError;
        }
        result;
    )", Value("inner: inner error outer: outer error"));
}

TEST_F(ExceptionIntegrationTest, NestedFinally) {
    // 测试嵌套finally
    AssertEq(R"(
        let result = '';
        try {
            try {
                result += 'inner try ';
            } finally {
                result += 'inner finally ';
            }
        } finally {
            result += 'outer finally';
        }
        result;
    )", Value("inner try inner finally outer finally"));
}

TEST_F(ExceptionIntegrationTest, CatchRethrow) {
    // 测试捕获后重新抛出
    AssertEq(R"(
        let result = '';
        try {
            try {
                throw 'initial error';
            } catch (e) {
                result += 'caught ';
                throw e;
            }
        } catch (e) {
            result += 'recaught: ' + e;
        }
        result;
    )", Value("caught recaught: initial error"));
}

// ==================== 异常传播 ====================

TEST_F(ExceptionIntegrationTest, ExceptionPropagation) {
    // 测试异常传播
    AssertEq(R"(
        function inner() {
            throw 'error from inner';
        }

        function middle() {
            inner();
        }

        function outer() {
            try {
                middle();
            } catch (e) {
                return 'caught in outer: ' + e;
            }
        }

        outer();
    )", Value("caught in outer: error from inner"));
}

TEST_F(ExceptionIntegrationTest, UncaughtException) {
    // 测试未捕获的异常
    AssertThrows(R"(
        function throwError() {
            throw 'uncaught';
        }

        throwError();
    )");
}

// ==================== 函数中的异常 ====================

TEST_F(ExceptionIntegrationTest, ErrorInFunction) {
    // 测试函数中的错误
    AssertEq(R"(
        function divide(a, b) {
            if (b === 0) {
                throw new Error('Division by zero');
            }
            return a / b;
        }

        try {
            divide(10, 0);
        } catch (e) {
            return e.message;
        }
    )", Value("Division by zero"));
}

TEST_F(ExceptionIntegrationTest, ErrorInCallback) {
    // 测试回调中的错误
    AssertEq(R"(
        function process(callback) {
            try {
                callback();
            } catch (e) {
                return 'callback error: ' + e;
            }
        }

        process(function() {
            throw 'callback failed';
        });
    )", Value("callback error: callback failed"));
}

// ==================== 复杂场景 ====================

TEST_F(ExceptionIntegrationTest, ResourceCleanup) {
    // 测试资源清理
    AssertEq(R"(
        let resourceOpen = false;

        function openResource() {
            resourceOpen = true;
            return 'resource';
        }

        function closeResource() {
            resourceOpen = false;
        }

        function processResource() {
            const resource = openResource();
            try {
                // 处理资源
                throw 'processing error';
            } catch (e) {
                throw 'rethrown: ' + e;
            } finally {
                closeResource();
            }
        }

        try {
            processResource();
        } catch (e) {
            e + ', resource closed: ' + !resourceOpen;
        }
    )", Value("rethrown: processing error, resource closed: true"));
}

TEST_F(ExceptionIntegrationTest, RetryPattern) {
    // 测试重试模式
    AssertEq(R"(
        let attempts = 0;

        function unreliableOperation() {
            attempts += 1;
            if (attempts < 3) {
                throw new Error('Attempt ' + attempts + ' failed');
            }
            return 'success';
        }

        function retry(operation, maxAttempts) {
            for (let i = 0; i < maxAttempts; i += 1) {
                try {
                    return operation();
                } catch (e) {
                    if (i === maxAttempts - 1) {
                        return 'gave up after ' + maxAttempts + ' attempts';
                    }
                }
            }
        }

        retry(unreliableOperation, 5);
    )", Value("success"));
}

TEST_F(ExceptionIntegrationTest, ErrorHandlingPipeline) {
    // 测试错误处理管道
    AssertEq(R"(
        function validateInput(input) {
            if (typeof input !== 'number') {
                throw new TypeError('Input must be a number');
            }
            if (input < 0) {
                throw new RangeError('Input must be positive');
            }
            return input;
        }

        function process(input) {
            try {
                const validated = validateInput(input);
                return 'processed: ' + validated;
            } catch (e) {
                if (e instanceof TypeError) {
                    return 'type error: ' + e.message;
                }
                if (e instanceof RangeError) {
                    return 'range error: ' + e.message;
                }
                return 'unknown error: ' + e.message;
            }
        }

        process(-5);
    )", Value("range error: Input must be positive"));
}

TEST_F(ExceptionIntegrationTest, ConditionalErrorHandling) {
    // 测试条件错误处理
    AssertEq(R"(
        function safeDivide(a, b) {
            try {
                if (b === 0) {
                    return { success: false, error: 'Division by zero' };
                }
                return { success: true, result: a / b };
            } catch (e) {
                return { success: false, error: e.message };
            }
        }

        const result1 = safeDivide(10, 2);
        const result2 = safeDivide(10, 0);

        result1.success + ', ' + result2.success;
    )", Value("true, false"));
}

TEST_F(ExceptionIntegrationTest, CustomErrorClass) {
    // 测试自定义错误类
    AssertEq(R"(
        class CustomError extends Error {
            constructor(message, code) {
                super(message);
                this.code = code;
            }
        }

        try {
            throw new CustomError('Custom error occurred', 500);
        } catch (e) {
            e.message + ' (code: ' + e.code + ')';
        }
    )", Value("Custom error occurred (code: 500)"));
}

// ==================== 边界情况 ====================

TEST_F(ExceptionIntegrationTest, EmptyTryCatch) {
    // 测试空的try-catch
    AssertEq(R"(
        try {
        } catch (e) {
        }
        'no error';
    )", Value("no error"));
}

TEST_F(ExceptionIntegrationTest, FinallyReturnValue) {
    // 测试finally的返回值
    AssertEq(R"(
        try {
            return 'try return';
        } catch (e) {
            return 'catch return';
        } finally {
            return 'finally return';
        }
    )", Value("finally return"));
}

TEST_F(ExceptionIntegrationTest, ThrowInFinally) {
    // 测试在finally中抛出异常
    AssertThrows(R"(
        try {
            throw 'first error';
        } catch (e) {
        } finally {
            throw 'second error';
        }
    )");
}

TEST_F(ExceptionIntegrationTest, MultipleCatchBlocks) {
    // 测试多个catch块（模拟）
    AssertEq(R"(
        try {
            throw new TypeError('Type error');
        } catch (e) {
            if (e instanceof TypeError) {
                return 'TypeError: ' + e.message;
            }
            if (e instanceof ReferenceError) {
                return 'ReferenceError: ' + e.message;
            }
            return 'Error: ' + e.message;
        }
    )", Value("TypeError: Type error"));
}

} // namespace mjs::test
