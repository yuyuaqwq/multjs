/**
 * @file getter_setter_integration_test.cpp
 * @brief Getter/Setter 属性集成测试
 *
 * 测试对象的 getter/setter 属性语义，包括:
 * - 基础 getter 语义（自动调用）
 * - 基础 setter 语义（自动调用）
 * - getter/setter 组合
 * - this 绑定
 * - 与普通属性混合使用
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include "test_helper.h"
#include <gtest/gtest.h>

namespace mjs::test {

/**
 * @class GetterSetterIntegrationTest
 * @brief Getter/Setter 集成测试
 */
class GetterSetterIntegrationTest : public IntegrationTestHelper {
};

// ==================== 基础 Getter 语义 ====================

TEST_F(GetterSetterIntegrationTest, BasicGetter) {
    // 测试基础 getter：访问属性时自动调用 getter 函数
    AssertEq(R"(
        const obj = {
            get value() {
                return 42;
            }
        };
        obj.value;
    )", Value(42));
}

TEST_F(GetterSetterIntegrationTest, GetterWithThis) {
    // 测试 getter 中的 this 绑定
    AssertEq(R"(
        const obj = {
            _x: 10,
            get x() {
                return this._x;
            }
        };
        obj.x;
    )", Value(10));
}

TEST_F(GetterSetterIntegrationTest, GetterAccessMultipleTimes) {
    // 测试多次访问 getter
    AssertEq(R"(
        let count = 0;
        const obj = {
            get value() {
                count++;
                return 100;
            }
        };
        const a = obj.value;
        const b = obj.value;
        const c = obj.value;
        count;
    )", Value(3));
}

// ==================== 基础 Setter 语义 ====================

TEST_F(GetterSetterIntegrationTest, BasicSetter) {
    // 测试基础 setter：设置属性时自动调用 setter 函数
    AssertEq(R"(
        let capturedValue = 0;
        const obj = {
            set value(x) {
                capturedValue = x;
            }
        };
        obj.value = 42;
        capturedValue;
    )", Value(42));
}

TEST_F(GetterSetterIntegrationTest, SetterWithThis) {
    // 测试 setter 中的 this 绑定
    AssertEq(R"(
        const obj = {
            _x: 0,
            set x(value) {
                this._x = value;
            }
        };
        obj.x = 99;
        obj._x;
    )", Value(99));
}

TEST_F(GetterSetterIntegrationTest, SetterAccessMultipleTimes) {
    // 测试多次调用 setter
    AssertEq(R"(
        let sum = 0;
        const obj = {
            set value(x) {
                sum += x;
            }
        };
        obj.value = 10;
        obj.value = 20;
        obj.value = 30;
        sum;
    )", Value(60));
}

// ==================== Getter/Setter 组合 ====================

TEST_F(GetterSetterIntegrationTest, GetterSetterPair) {
    // 测试 getter/setter 对
    AssertEq(R"(
        const obj = {
            _value: 0,
            get value() {
                return this._value;
            },
            set value(x) {
                this._value = x;
            }
        };
        obj.value = 100;
        obj.value;
    )", Value(100));
}

TEST_F(GetterSetterIntegrationTest, GetterSetterPairMultipleAccess) {
    // 测试 getter/setter 多次读写
    AssertEq(R"(
        const obj = {
            _value: 0,
            get value() {
                return this._value * 2;
            },
            set value(x) {
                this._value = x;
            }
        };
        obj.value = 10;
        const a = obj.value;
        obj.value = 20;
        const b = obj.value;
        a + b;
    )", Value(60));  // (10*2) + (20*2) = 20 + 40 = 60
}

// ==================== 混合普通属性和 Accessor ====================

TEST_F(GetterSetterIntegrationTest, MixNormalAndAccessor) {
    // 测试普通属性和 getter/setter 混合使用
    AssertEq(R"(
        const obj = {
            normal: 1,
            get computed() {
                return this.normal * 2;
            },
            set computed(x) {
                this.normal = x / 2;
            }
        };
        const a = obj.computed;
        obj.computed = 10;
        const b = obj.normal;
        a + b;
    )", Value(7));  // a=2, b=5, 2+5=7
}

TEST_F(GetterSetterIntegrationTest, ObjectWithMultipleProperties) {
    // 测试包含多个属性的对象
    AssertEq(R"(
        const obj = {
            x: 1,
            y: 2,
            get sum() {
                return this.x + this.y;
            },
            get product() {
                return this.x * this.y;
            }
        };
        obj.sum + obj.product;
    )", Value(5));  // (1+2) + (1*2) = 3 + 2 = 5
}

// ==================== 复杂场景 ====================

TEST_F(GetterSetterIntegrationTest, NestedObjectWithGetter) {
    // 测试嵌套对象中的 getter
    AssertEq(R"(
        const obj = {
            inner: {
                _value: 5,
                get value() {
                    return this._value;
                }
            }
        };
        obj.inner.value;
    )", Value(5));
}

TEST_F(GetterSetterIntegrationTest, GetterWithComputation) {
    // 测试 getter 进行动态计算
    AssertEq(R"(
        const obj = {
            base: 10,
            multiplier: 3,
            get result() {
                return this.base * this.multiplier;
            }
        };
        obj.base = 20;
        obj.result;
    )", Value(60));  // 20 * 3 = 60
}

TEST_F(GetterSetterIntegrationTest, SetterWithValidation) {
    // 测试 setter 中的验证逻辑
    AssertEq(R"(
        const obj = {
            _age: 0,
            set age(value) {
                if (value < 0) {
                    this._age = 0;
                } else if (value > 150) {
                    this._age = 150;
                } else {
                    this._age = value;
                }
            },
            get age() {
                return this._age;
            }
        };
        obj.age = -5;
        const a = obj.age;
        obj.age = 200;
        const b = obj.age;
        obj.age = 25;
        const c = obj.age;
        a + b + c;
    )", Value(175));  // 0 + 150 + 25 = 175
}

// ==================== 边界情况 ====================

TEST_F(GetterSetterIntegrationTest, EmptyGetter) {
    // 测试 getter 没有 return（返回 undefined）
    AssertTrue(R"(
        const obj = {
            get value() {
                // 没有 return
            }
        };
        const v = obj.value;
        v === undefined;
    )");
}

TEST_F(GetterSetterIntegrationTest, GetterReturningObject) {
    // 测试 getter 返回对象
    AssertEq(R"(
        const obj = {
            get data() {
                return { x: 10, y: 20 };
            }
        };
        obj.data.x + obj.data.y;
    )", Value(30));
}

TEST_F(GetterSetterIntegrationTest, SetterWithoutUsingValue) {
    // 测试 setter 不使用参数
    AssertEq(R"(
        let called = false;
        const obj = {
            set value(x) {
                called = true;
            }
        };
        obj.value = 999;
        called;
    )", Value(true));
}

} // namespace mjs::test
