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

TEST_F(ArrayObjectTest, CreateEmptyArray) {
    auto* arr = ArrayObject::New(context.get(), 0);
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr->length(), 0);
    EXPECT_EQ(arr->class_id(), ClassId::kArrayObject);
}

TEST_F(ArrayObjectTest, CreateArrayWithInitializerList) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2),
        Value(3)
    });
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr->length(), 3);
    EXPECT_EQ((*arr)[0].i64(), 1);
    EXPECT_EQ((*arr)[1].i64(), 2);
    EXPECT_EQ((*arr)[2].i64(), 3);
}

TEST_F(ArrayObjectTest, CreateArrayWithSize) {
    auto* arr = ArrayObject::New(context.get(), 5);
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr->length(), 5);
}

TEST_F(ArrayObjectTest, ArrayElementAccess) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(10),
        Value(20),
        Value(30)
    });

    // 测试读取元素
    EXPECT_EQ((*arr)[0].i64(), 10);
    EXPECT_EQ((*arr)[1].i64(), 20);
    EXPECT_EQ((*arr)[2].i64(), 30);

    // 测试修改元素
    (*arr)[1] = Value(99);
    EXPECT_EQ((*arr)[1].i64(), 99);
}

TEST_F(ArrayObjectTest, ArrayPush) {
    auto* arr = ArrayObject::New(context.get(), 0);

    arr->Push(context.get(), Value(1));
    EXPECT_EQ(arr->length(), 1);
    EXPECT_EQ((*arr)[0].i64(), 1);

    arr->Push(context.get(), Value(2));
    EXPECT_EQ(arr->length(), 2);
    EXPECT_EQ((*arr)[1].i64(), 2);

    arr->Push(context.get(), Value(3));
    EXPECT_EQ(arr->length(), 3);
    EXPECT_EQ((*arr)[2].i64(), 3);
}

TEST_F(ArrayObjectTest, ArrayPop) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2),
        Value(3)
    });

    // 测试pop
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

TEST_F(ArrayObjectTest, ArrayMixedTypes) {
    auto* str = String::New("hello");
    auto* arr = ArrayObject::New(context.get(), {
        Value(42),                    // 数字
        Value(str),                   // 字符串
        Value(true),                  // 布尔
        Value()                       // undefined
    });

    EXPECT_EQ(arr->length(), 4);
    EXPECT_EQ((*arr)[0].i64(), 42);
    EXPECT_EQ(std::string_view((*arr)[1].string_view()), "hello");
    EXPECT_EQ((*arr)[2].boolean(), true);
    EXPECT_TRUE((*arr)[3].IsUndefined());
}

TEST_F(ArrayObjectTest, ArrayGetProperty) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2),
        Value(3)
    });

    // 测试通过常量索引获取属性
    Value val = arr->operator[](0);
    EXPECT_EQ(val.i64(), 1);
}

TEST_F(ArrayObjectTest, ArrayGetComputedProperty) {
    auto* arr = ArrayObject::New(context.get(), {
        Value(10),
        Value(20),
        Value(30)
    });

    // 测试通过值获取计算属性
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
    EXPECT_EQ((*arr)[1].i64(), 99);
}

TEST_F(ArrayObjectTest, LargeArray) {
    // 测试大数组
    size_t size = 1000;
    auto* arr = ArrayObject::New(context.get(), size);
    EXPECT_EQ(arr->length(), size);

    // 修改几个元素
    (*arr)[0] = Value(100);
    (*arr)[500] = Value(200);
    (*arr)[999] = Value(300);

    EXPECT_EQ((*arr)[0].i64(), 100);
    EXPECT_EQ((*arr)[500].i64(), 200);
    EXPECT_EQ((*arr)[999].i64(), 300);
}

TEST_F(ArrayObjectTest, ArrayInheritsFromObject) {
    auto* arr = ArrayObject::New(context.get(), {Value(1), Value(2)});

    // 测试继承自Object
    EXPECT_TRUE(arr->GetPrototype(test_env->runtime()).IsObject() ||
                arr->GetPrototype(test_env->runtime()).IsNull());
    EXPECT_EQ(arr->class_id(), ClassId::kArrayObject);
}

} // namespace mjs::test
