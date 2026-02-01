/**
 * @file gc_test.cpp
 * @brief GC系统单元测试
 *
 * 测试垃圾回收系统的功能，包括：
 * - 新生代复制GC
 * - 老年代标记-压缩GC
 * - 对象晋升机制
 * - GCTraverse遍历
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/value/value.h>
#include <mjs/value/function_def.h>
#include <mjs/value/module_def.h>
#include <mjs/value/object/object.h>
#include <mjs/value/object/array_object.h>
#include <mjs/value/object/function_object.h>
#include <mjs/value/object/generator_object.h>
#include <mjs/value/object/promise_object.h>
#include <mjs/gc/gc_manager.h>
#include <mjs/gc/gc_heap.h>
#include "tests/unit/test_helpers.h"

namespace mjs::test {

/**
 * @class GCTest
 * @brief GC系统测试类
 */
class GCTest : public ::testing::Test {
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

// ==================== 基础功能测试 ====================

TEST_F(GCTest, GCManagerInitialization) {
    // 测试GCManager初始化
    auto& gc_manager = context->gc_manager();
    EXPECT_TRUE(gc_manager.heap() != nullptr);
}

TEST_F(GCTest, GCHeapStats) {
    // 测试获取GC堆统计信息
    auto& gc_manager = context->gc_manager();

    size_t new_used, new_capacity, old_used, old_capacity;
    gc_manager.GetHeapStats(new_used, new_capacity, old_used, old_capacity);

    // 初始状态下，容量应该大于0
    EXPECT_GT(new_capacity, 0);
    EXPECT_GT(old_capacity, 0);
}

TEST_F(GCTest, GCStats) {
    // 测试获取GC统计信息
    auto& gc_manager = context->gc_manager();

    size_t allocated, collected;
    uint32_t gc_count;
    gc_manager.GetGCStats(allocated, collected, gc_count);

    // 初始状态下应该为0
    EXPECT_EQ(allocated, 0);
    EXPECT_EQ(collected, 0);
    EXPECT_EQ(gc_count, 0);
}

// ==================== GCTraverse测试 ====================

TEST_F(GCTest, ObjectGCTraverse) {
    // 测试Object的GCTraverse方法
    auto* obj = Object::New(context.get());
    obj->SetProperty(context.get(),
        context->FindConstOrInsertToLocal(Value("test")),
        Value(42));

    bool called = false;
    obj->GCTraverse(context.get(), [&](Context* ctx, Value& value) {
        called = true;
        // 遍历属性时应该被调用
    });

    EXPECT_TRUE(called);
}

TEST_F(GCTest, ArrayObjectGCTraverse) {
    // 测试ArrayObject的GCTraverse方法
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(2),
        Value(3)
    });

    int call_count = 0;
    arr->GCTraverse(context.get(), [&](Context* ctx, Value& value) {
        call_count++;
    });

    // 应该遍历至少length属性
    EXPECT_GT(call_count, 0);
}

TEST_F(GCTest, FunctionObjectGCTraverse) {
    // 测试FunctionObject的GCTraverse方法
    auto* func_def = test_env->CreateFunctionDef("testFunc", 2);
    auto* func_obj = FunctionObject::New(context.get(), func_def);

    bool called = false;
    func_obj->GCTraverse(context.get(), [&](Context* ctx, Value& value) {
        called = true;
    });

    EXPECT_TRUE(called);
}

TEST_F(GCTest, GeneratorObjectGCTraverse) {
    // 测试GeneratorObject的GCTraverse方法
    auto* func_def = test_env->CreateFunctionDef("genFunc", 0);
    auto* gen = GeneratorObject::New(context.get(), Value(func_def));

    bool called = false;
    gen->GCTraverse(context.get(), [&](Context* ctx, Value& value) {
        called = true;
    });

    EXPECT_TRUE(called);
}

TEST_F(GCTest, PromiseObjectGCTraverse) {
    // 测试PromiseObject的GCTraverse方法
    auto executor = Value();
    auto* promise = PromiseObject::New(context.get(), executor);

    bool called = false;
    promise->GCTraverse(context.get(), [&](Context* ctx, Value& value) {
        called = true;
    });

    EXPECT_TRUE(called);
}

// ==================== GC元数据测试 ====================

TEST_F(GCTest, ObjectGCMetadata) {
    // 测试Object的GC元数据
    auto* obj = Object::New(context.get());

    // 初始状态应该是新生代
    EXPECT_EQ(obj->gc_generation(), 0);
    EXPECT_EQ(obj->gc_age(), 0);
    EXPECT_FALSE(obj->gc_forwarded());
    EXPECT_FALSE(obj->gc_pinned());
    EXPECT_EQ(obj->gc_forwarding_ptr(), nullptr);

    // 修改GC元数据
    obj->set_gc_generation(1);
    obj->gc_increment_age();
    obj->set_gc_forwarded(true);
    obj->set_gc_pinned(true);
    obj->set_gc_forwarding_ptr(obj);

    EXPECT_EQ(obj->gc_generation(), 1);
    EXPECT_EQ(obj->gc_age(), 1);
    EXPECT_TRUE(obj->gc_forwarded());
    EXPECT_TRUE(obj->gc_pinned());
    EXPECT_EQ(obj->gc_forwarding_ptr(), obj);

    // 清空年龄
    obj->gc_clear_age();
    EXPECT_EQ(obj->gc_age(), 0);
}

// ==================== GC触发测试 ====================

TEST_F(GCTest, TriggerGC) {
    // 测试手动触发GC
    auto& gc_manager = context->gc_manager();

    size_t allocated_before, collected_before;
    uint32_t gc_count_before;
    gc_manager.GetGCStats(allocated_before, collected_before, gc_count_before);

    // 手动触发GC
    gc_manager.CollectGarbage(false);

    size_t allocated_after, collected_after;
    uint32_t gc_count_after;
    gc_manager.GetGCStats(allocated_after, collected_after, gc_count_after);

    // GC次数应该增加
    EXPECT_EQ(gc_count_after, gc_count_before + 1);
}

TEST_F(GCTest, ForceFullGC) {
    // 测试强制完整GC
    auto& gc_manager = context->gc_manager();

    size_t allocated_before, collected_before;
    uint32_t gc_count_before;
    gc_manager.GetGCStats(allocated_before, collected_before, gc_count_before);

    // 强制完整GC
    gc_manager.ForceFullGC();

    size_t allocated_after, collected_after;
    uint32_t gc_count_after;
    gc_manager.GetGCStats(allocated_after, collected_after, gc_count_after);

    // GC次数应该增加
    EXPECT_GT(gc_count_after, gc_count_before);
}

// ==================== GC阈值测试 ====================

TEST_F(GCTest, SetGCThreshold) {
    // 测试设置GC阈值
    auto& gc_manager = context->gc_manager();

    // 设置不同的阈值
    gc_manager.SetGCThreshold(50);
    gc_manager.SetGCThreshold(90);
    gc_manager.SetGCThreshold(10);

    // 验证不会崩溃
    // 实际阈值效果需要更多的对象分配来测试
}

// ==================== 复杂场景测试 ====================

TEST_F(GCTest, CircularReferenceGCTraverse) {
    // 测试循环引用的GCTraverse
    auto* obj1 = Object::New(context.get());
    auto* obj2 = Object::New(context.get());

    // 创建循环引用
    obj1->SetProperty(context.get(),
        context->FindConstOrInsertToLocal(Value("ref")),
        Value(obj2));
    obj2->SetProperty(context.get(),
        context->FindConstOrInsertToLocal(Value("ref")),
        Value(obj1));

    // 遍历应该能处理循环引用
    int call_count = 0;
    obj1->GCTraverse(context.get(), [&](Context* ctx, Value& value) {
        call_count++;
    });

    EXPECT_GT(call_count, 0);
}

TEST_F(GCTest, NestedObjectsGCTraverse) {
    // 测试嵌套对象的GCTraverse
    auto* inner = Object::New(context.get());
    inner->SetProperty(context.get(),
        context->FindConstOrInsertToLocal(Value("value")),
        Value(123));

    auto* middle = Object::New(context.get());
    middle->SetProperty(context.get(),
        context->FindConstOrInsertToLocal(Value("inner")),
        Value(inner));

    auto* outer = Object::New(context.get());
    outer->SetProperty(context.get(),
        context->FindConstOrInsertToLocal(Value("middle")),
        Value(middle));

    // 遍历应该能处理所有嵌套层级
    int call_count = 0;
    outer->GCTraverse(context.get(), [&](Context* ctx, Value& value) {
        call_count++;
    });

    EXPECT_GT(call_count, 0);
}

TEST_F(GCTest, LargeArrayGCTraverse) {
    // 测试大数组的GCTraverse
    const size_t size = 1000;
    auto* arr = ArrayObject::New(context.get(), size);

    // 填充数组
    for (size_t i = 0; i < size; ++i) {
        arr->At(context.get(), i) = Value(static_cast<int64_t>(i));
    }

    // 遍历应该能处理大数组
    int call_count = 0;
    arr->GCTraverse(context.get(), [&](Context* ctx, Value& value) {
        call_count++;
    });

    EXPECT_GT(call_count, 0);
}

} // namespace mjs::test
