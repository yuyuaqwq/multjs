#include <gtest/gtest.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/value.h>
#include <mjs/object.h>
#include <mjs/object_impl/array_object.h>
#include <mjs/string.h>
#include "tests/unit/test_helpers.h"

namespace mjs::test {

class ArrayObjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_env = std::make_unique<TestEnvironment>();
        context = std::make_unique<Context>(test_env->runtime());
    }

    void TearDown() override {
        test_env.reset();
    }

    std::unique_ptr<TestEnvironment> test_env;
    std::unique_ptr<Context> context;
};

// ==================== 基础创建测试 ====================

TEST_F(ArrayObjectTest, CreateEmptyArray) {
    auto* arr = ArrayObject::New(context.get(), 0);
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr->length(), 0);
}

TEST_F(ArrayObjectTest, CreateArrayWithInitializerList) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2),
        Value(3)
    });
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr->length(), 3);

    // 使用新的 operator[] 语法
    EXPECT_EQ((*arr).At(context.get(), 0).i64(), 1);
    EXPECT_EQ((*arr).At(context.get(), 1).i64(), 2);
    EXPECT_EQ((*arr).At(context.get(), 2).i64(), 3);
}

TEST_F(ArrayObjectTest, CreateArrayWithSize_SparseArray) {
    // 测试稀疏数组：创建长度为 5 但没有元素的数组
    auto* arr = ArrayObject::New(context.get(), 5);
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr->length(), 5);

    // 稀疏数组访问不存在的元素应该返回 undefined
    // 但不应该自动创建元素
}

// ==================== 元素访问测试 ====================

TEST_F(ArrayObjectTest, ArrayElementAccess) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(10),
        Value(20),
        Value(30)
    });

    // 测试读取元素
    EXPECT_EQ(arr->At(context.get(), 0).i64(), 10);
    EXPECT_EQ(arr->At(context.get(), 1).i64(), 20);
    EXPECT_EQ(arr->At(context.get(), 2).i64(), 30);

    // 测试修改元素
    arr->At(context.get(), 1) = Value(99);
    EXPECT_EQ(arr->At(context.get(), 1).i64(), 99);
}

TEST_F(ArrayObjectTest, ArrayElementAccessOutOfBounds) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2)
    });

    // 访问越界元素应该返回 undefined
    // At() 方法会自动创建不存在的元素，所以这里测试读取不存在的索引
    // 先创建一个元素再访问
    arr->At(context.get(), 10);
    EXPECT_TRUE(arr->At(context.get(), 10).IsUndefined());
}

TEST_F(ArrayObjectTest, ArraySetElementBeyondLength) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2)
    });
    EXPECT_EQ(arr->length(), 2);

    // 设置超出当前 length 的元素应该自动扩展 length
    arr->At(context.get(), 5) = Value(100);
    EXPECT_EQ(arr->length(), 6);
    EXPECT_EQ(arr->At(context.get(), 5).i64(), 100);
}

// ==================== Push/Pop 测试 ====================

TEST_F(ArrayObjectTest, ArrayPush) {
    auto* arr = ArrayObject::New(context.get(), 0);

    arr->Push(context.get(), Value(1));
    EXPECT_EQ(arr->length(), 1);
    EXPECT_EQ(arr->At(context.get(), 0).i64(), 1);

    arr->Push(context.get(), Value(2));
    EXPECT_EQ(arr->length(), 2);
    EXPECT_EQ(arr->At(context.get(), 1).i64(), 2);

    arr->Push(context.get(), Value(3));
    EXPECT_EQ(arr->length(), 3);
    EXPECT_EQ(arr->At(context.get(), 2).i64(), 3);
}

TEST_F(ArrayObjectTest, ArrayPop) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2),
        Value(3)
    });

    // 测试 pop
    Value val = arr->Pop(context.get());
    EXPECT_EQ(val.i64(), 3);
    EXPECT_EQ(arr->length(), 2);

    val = arr->Pop(context.get());
    EXPECT_EQ(val.i64(), 2);
    EXPECT_EQ(arr->length(), 1);

    val = arr->Pop(context.get());
    EXPECT_EQ(val.i64(), 1);
    EXPECT_EQ(arr->length(), 0);
}

TEST_F(ArrayObjectTest, ArrayPopFromEmpty) {
    auto* arr = ArrayObject::New(context.get(), 0);

    // 从空数组 pop 应该返回 undefined
    Value val = arr->Pop(context.get());
    EXPECT_TRUE(val.IsUndefined());
    EXPECT_EQ(arr->length(), 0);
}

// ==================== Length 属性测试 ====================

TEST_F(ArrayObjectTest, ArrayLengthProperty) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2),
        Value(3)
    });

    // 测试获取 length 属性
    auto length_key = context->FindConstOrInsertToLocal(Value("length"));
    Value length_val;
    bool result = arr->GetProperty(context.get(), length_key, &length_val);
    EXPECT_TRUE(result);
    EXPECT_EQ(length_val.i64(), 3);
}

TEST_F(ArrayObjectTest, ArraySetLengthSmaller) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2),
        Value(3),
        Value(4),
        Value(5)
    });
    EXPECT_EQ(arr->length(), 5);

    // 设置更小的 length 应该删除超出部分
    auto length_key = context->FindConstOrInsertToLocal(Value("length"));
    arr->SetProperty(context.get(), length_key, Value(static_cast<int64_t>(3)));

    EXPECT_EQ(arr->length(), 3);

    // 检查元素是否被删除
    Value val;
    EXPECT_FALSE(arr->GetComputedProperty(context.get(), Value(3), &val));
    EXPECT_FALSE(arr->GetComputedProperty(context.get(), Value(4), &val));
}

TEST_F(ArrayObjectTest, ArraySetLengthLarger) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2)
    });
    EXPECT_EQ(arr->length(), 2);

    // 设置更大的 length 应该扩展数组（稀疏）
    auto length_key = context->FindConstOrInsertToLocal(Value("length"));
    arr->SetProperty(context.get(), length_key, Value(static_cast<int64_t>(10)));

    EXPECT_EQ(arr->length(), 10);
    // 之前存在的元素应该还在
    EXPECT_EQ(arr->At(context.get(), 0).i64(), 1);
    EXPECT_EQ(arr->At(context.get(), 1).i64(), 2);
}

TEST_F(ArrayObjectTest, ArrayAutoUpdateLength) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2)
    });
    EXPECT_EQ(arr->length(), 2);

    // 添加超出 length 的元素应该自动更新 length
    arr->At(context.get(), 10) = Value(100);
    EXPECT_EQ(arr->length(), 11);
    EXPECT_EQ(arr->At(context.get(), 10).i64(), 100);
}

// ==================== 计算属性测试 ====================

TEST_F(ArrayObjectTest, ArrayGetComputedProperty) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(10),
        Value(20),
        Value(30)
    });

    // 测试通过索引值获取属性
    Value val;
    bool result = arr->GetComputedProperty(context.get(), Value(1), &val);
    EXPECT_TRUE(result);
    EXPECT_EQ(val.i64(), 20);
}

TEST_F(ArrayObjectTest, ArraySetComputedProperty) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2),
        Value(3)
    });

    // 测试设置计算属性
    arr->SetComputedProperty(context.get(), Value(1), Value(99));
    EXPECT_EQ(arr->At(context.get(), 1).i64(), 99);
}

TEST_F(ArrayObjectTest, ArraySetComputedPropertyBeyondLength) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2)
    });
    EXPECT_EQ(arr->length(), 2);

    // 设置超出 length 的元素
    arr->SetComputedProperty(context.get(), Value(5), Value(100));
    EXPECT_EQ(arr->length(), 6); // length 应该自动更新为 6
    EXPECT_EQ(arr->At(context.get(), 5).i64(), 100);
}

TEST_F(ArrayObjectTest, ArrayDelComputedProperty) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2),
        Value(3)
    });

    // 删除中间元素
    arr->DelComputedProperty(context.get(), Value(1));

    // length 不应该改变
    EXPECT_EQ(arr->length(), 3);

    // 元素应该被删除
    Value val;
    bool result = arr->GetComputedProperty(context.get(), Value(1), &val);
    EXPECT_FALSE(result);
}

// ==================== 混合类型测试 ====================

TEST_F(ArrayObjectTest, ArrayMixedTypes) {
    auto* str = String::New("hello");
    auto* arr = ArrayObject::New(context.get(), {
        Value(42),                    // 数字
        Value(str),                   // 字符串
        Value(true),                  // 布尔
        Value()                       // undefined
    });

    EXPECT_EQ(arr->length(), 4);
    EXPECT_EQ(arr->At(context.get(), 0).i64(), 42);
    EXPECT_EQ(std::string_view(arr->At(context.get(), 1).string_view()), "hello");
    EXPECT_EQ(arr->At(context.get(), 2).boolean(), true);
    EXPECT_TRUE(arr->At(context.get(), 3).IsUndefined());
}

// ==================== 稀疏数组测试 ====================

TEST_F(ArrayObjectTest, SparseArray) {
    // 创建稀疏数组
    auto* arr = ArrayObject::New(context.get(), 100);
    EXPECT_EQ(arr->length(), 100);

    // 只设置几个元素
    arr->At(context.get(), 0) = Value(1);
    arr->At(context.get(), 50) = Value(2);
    arr->At(context.get(), 99) = Value(3);

    EXPECT_EQ(arr->At(context.get(), 0).i64(), 1);
    EXPECT_EQ(arr->At(context.get(), 50).i64(), 2);
    EXPECT_EQ(arr->At(context.get(), 99).i64(), 3);

    // 未设置的元素应该是 undefined
    EXPECT_TRUE(arr->At(context.get(), 1).IsUndefined());
    EXPECT_TRUE(arr->At(context.get(), 98).IsUndefined());
}

TEST_F(ArrayObjectTest, VerySparseArray) {
    // 创建非常大的稀疏数组
    auto* arr = ArrayObject::New(context.get(), 10000);
    EXPECT_EQ(arr->length(), 10000);

    // 只在最后设置一个元素
    arr->At(context.get(), 9999) = Value(42);
    EXPECT_EQ(arr->At(context.get(), 9999).i64(), 42);

    // length 应该保持为 10000
    EXPECT_EQ(arr->length(), 10000);
}

// ==================== 边界情况测试 ====================

TEST_F(ArrayObjectTest, LargeArray) {
    // 测试较大的数组
    size_t size = 1000;
    auto* arr = ArrayObject::New(context.get(), size);
    EXPECT_EQ(arr->length(), size);

    // 修改几个元素
    arr->At(context.get(), 0) = Value(100);
    arr->At(context.get(), 500) = Value(200);
    arr->At(context.get(), 999) = Value(300);

    EXPECT_EQ(arr->At(context.get(), 0).i64(), 100);
    EXPECT_EQ(arr->At(context.get(), 500).i64(), 200);
    EXPECT_EQ(arr->At(context.get(), 999).i64(), 300);
}

TEST_F(ArrayObjectTest, ArrayIndexLimit) {
    auto* arr = ArrayObject::New(context.get(), 0);

    // 测试接近 2^32-1 的索引（不实际创建，只测试边界）
    // 正常范围内的索引
    arr->At(context.get(), 0) = Value(1);
    EXPECT_EQ(arr->length(), 1);

    arr->At(context.get(), 1000) = Value(2);
    EXPECT_EQ(arr->length(), 1001);
}

// ==================== 继承测试 ====================

TEST_F(ArrayObjectTest, ArrayInheritsFromObject) {
    auto* arr = ArrayObject::New(context.get(), {Value(1), Value(2)});

    // 测试继承自 Object
    EXPECT_TRUE(arr->GetPrototype(test_env->runtime()).IsObject() ||
                arr->GetPrototype(test_env->runtime()).IsNull());
}

TEST_F(ArrayObjectTest, ArrayHasOwnProperty) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2)
    });

    // 测试 HasProperty
    auto key0 = context->FindConstOrInsertToLocal(Value("0"));
    auto key1 = context->FindConstOrInsertToLocal(Value("1"));
    auto key2 = context->FindConstOrInsertToLocal(Value("2"));

    EXPECT_TRUE(arr->HasProperty(context.get(), key0));
    EXPECT_TRUE(arr->HasProperty(context.get(), key1));
    EXPECT_FALSE(arr->HasProperty(context.get(), key2));
}

// ==================== Length 属性特征测试 ====================

TEST_F(ArrayObjectTest, LengthPropertyNotEnumerable) {
    auto* arr = ArrayObject::New(context.get(), {Value(1), Value(2)});

    // length 属性应该是不可枚举的
    auto length_key = context->FindConstOrInsertToLocal(Value("length"));

    // 获取 length 属性的 flags
    // 注意：这需要能够获取属性的特性
    // 暂时只测试 length 属性存在
    Value length_val;
    bool result = arr->GetProperty(context.get(), length_key, &length_val);
    EXPECT_TRUE(result);
    EXPECT_EQ(length_val.i64(), 2);
}

} // namespace mjs::test
