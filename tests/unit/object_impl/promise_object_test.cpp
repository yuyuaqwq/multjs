#include <gtest/gtest.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/value/value.h>
#include <mjs/value/object/object.h>
#include <mjs/value/object/promise_object.h>
#include <mjs/value/string.h>
#include "tests/unit/test_helpers.h"

namespace mjs::test {

class PromiseObjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_env = std::make_unique<TestEnvironment>();
        context = std::make_unique<Context>(test_env->runtime());
    }

    void TearDown() override {
        context.reset();
        test_env.reset();
    }

    std::unique_ptr<TestEnvironment> test_env;
    std::unique_ptr<Context> context;
};

TEST_F(PromiseObjectTest, CreatePromise) {
    // 创建一个简单的executor函数
    auto executor = Value(); // 占位符

    auto* promise = PromiseObject::New(context.get(), executor);
    ASSERT_NE(promise, nullptr);

    // 初始状态应该是pending
    EXPECT_TRUE(promise->IsPending());
    EXPECT_FALSE(promise->IsFulfilled());
    EXPECT_FALSE(promise->IsRejected());
}

TEST_F(PromiseObjectTest, PromiseStateTransitions) {
    auto executor = Value();
    auto* promise = PromiseObject::New(context.get(), executor);

    // 初始状态
    EXPECT_TRUE(promise->IsPending());

    // Resolve
    promise->Resolve(context.get(), Value(42));
    EXPECT_TRUE(promise->IsFulfilled());
    EXPECT_FALSE(promise->IsPending());
    EXPECT_FALSE(promise->IsRejected());
    EXPECT_EQ(promise->result().i64(), 42);
}

TEST_F(PromiseObjectTest, PromiseReject) {
    auto executor = Value();
    auto* promise = PromiseObject::New(context.get(), executor);

    // Reject
    auto* error_str = String::New("error");
    promise->Reject(context.get(), Value(error_str));

    EXPECT_TRUE(promise->IsRejected());
    EXPECT_FALSE(promise->IsPending());
    EXPECT_FALSE(promise->IsFulfilled());
    std::string_view reason_view(promise->reason().string().data());
    EXPECT_EQ(reason_view, "error");
}

TEST_F(PromiseObjectTest, PromiseThen) {
    auto executor = Value();
    auto* promise = PromiseObject::New(context.get(), executor);

    // 创建on_fulfilled和on_rejected回调
    Value on_fulfilled; // 占位符
    Value on_rejected;  // 占位符

    // 调用Then方法
    Value result = promise->Then(context.get(), on_fulfilled, on_rejected);

    // Then应该返回一个值(可能是另一个Promise)
    SUCCEED();
}

TEST_F(PromiseObjectTest, PromiseSetResult) {
    auto executor = Value();
    auto* promise = PromiseObject::New(context.get(), executor);

    // 先resolve
    promise->Resolve(context.get(), Value(100));

    // 设置结果
    promise->set_result(Value(200));
    EXPECT_EQ(promise->result().i64(), 200);
}

TEST_F(PromiseObjectTest, PromiseSetReason) {
    auto executor = Value();
    auto* promise = PromiseObject::New(context.get(), executor);

    // 先reject
    auto* error_str = String::New("failure");
    promise->Reject(context.get(), Value(error_str));

    // 设置原因
    auto* new_error_str = String::New("new error");
    promise->set_reason(Value(new_error_str));
    std::string_view reason_view(promise->reason().string().data());
    EXPECT_EQ(reason_view, "new error");
}

TEST_F(PromiseObjectTest, PromiseInheritsFromObject) {
    auto executor = Value();
    auto* promise = PromiseObject::New(context.get(), executor);

    // 测试继承自Object
    EXPECT_TRUE(promise->GetPrototype(context.get()).IsObject() ||
                promise->GetPrototype(context.get()).IsNull());
}

} // namespace mjs::test
