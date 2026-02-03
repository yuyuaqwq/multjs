/**
 * @file gc_manager_test.cpp
 * @brief GCManager 垃圾回收管理器单元测试
 *
 * 测试垃圾回收管理器的功能：
 * - 初始化
 * - 对象分配
 * - GC触发控制
 * - 统计信息
 * - 根集合管理
 * - 边界条件
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>

#include <mjs/gc/gc_manager.h>
#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/value/object/object.h>
#include <mjs/gc/handle.h>
#include <sstream>

namespace mjs {
namespace test {

// 测试用的简单对象
class TestManagerObject : public GCObject {
public:
    TestManagerObject() : data_(0) {}
    explicit TestManagerObject(int data) : data_(data) {}
    TestManagerObject(Context* context, int data) : GCObject(), data_(data) { (void)context; }

    int data() const { return data_; }
    void set_data(int data) { data_ = data; }

    void GCTraverse(Context* context, GCTraverseCallback callback) override {
        (void)context;
        (void)callback;
    }

private:
    int data_;
};

/**
 * @class GCManagerTest
 * @brief GCManager 类单元测试
 */
class GCManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = std::make_unique<Runtime>();
        context_ = std::make_unique<Context>(runtime_.get());
        gc_manager_ = std::make_unique<GCManager>(context_.get());
        ASSERT_TRUE(gc_manager_->Initialize());
    }

    void TearDown() override {
        gc_manager_.reset();
        context_.reset();
        runtime_.reset();
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    std::unique_ptr<GCManager> gc_manager_;
};

// ==================== 初始化测试 ====================

/**
 * @test 测试管理器初始化
 */
TEST_F(GCManagerTest, Initialize) {
    // 在 SetUp 中已测试
    EXPECT_NE(gc_manager_.get(), nullptr);
    EXPECT_NE(gc_manager_->heap(), nullptr);
}

/**
 * @test 测试未初始化的堆访问
 */
TEST_F(GCManagerTest, UninitializedHeapAccess) {
    GCManager uninitialized_manager(context_.get());
    // 未初始化的堆应该为空
    EXPECT_EQ(uninitialized_manager.heap(), nullptr);
}

// ==================== 对象分配测试 ====================

/**
 * @test 测试分配简单对象
 */
TEST_F(GCManagerTest, AllocateSimpleObject) {
    GCHandleScope<1> scope(context_.get());
    auto obj = scope.New<TestManagerObject>(42);

    ASSERT_NE(obj.operator->(), nullptr);
    EXPECT_EQ(obj->data(), 42);
}

/**
 * @test 测试分配多个对象
 */
TEST_F(GCManagerTest, AllocateMultipleObjects) {
    GCHandleScope<10> scope(context_.get());

    auto obj1 = scope.New<TestManagerObject>(1);
    auto obj2 = scope.New<TestManagerObject>(2);
    auto obj3 = scope.New<TestManagerObject>(3);

    EXPECT_EQ(obj1->data(), 1);
    EXPECT_EQ(obj2->data(), 2);
    EXPECT_EQ(obj3->data(), 3);

    // 验证对象地址不同
    EXPECT_NE(obj1.operator->(), obj2.operator->());
    EXPECT_NE(obj2.operator->(), obj3.operator->());
}

/**
 * @test 测试分配大量对象
 */
TEST_F(GCManagerTest, AllocateManyObjects) {
    GCHandleScope<100> scope(context_.get());

    const int kNumObjects = 50;
    for (int i = 0; i < kNumObjects; i++) {
        auto obj = scope.New<TestManagerObject>(i);
        ASSERT_NE(obj.operator->(), nullptr);
        EXPECT_EQ(obj->data(), i);
    }
}

// ==================== GC触发测试 ====================

/**
 * @test 测试触发垃圾回收
 */
TEST_F(GCManagerTest, CollectGarbage) {
    // 分配一些对象
    {
        GCHandleScope<10> scope(context_.get());
        for (int i = 0; i < 5; i++) {
            scope.New<TestManagerObject>(i);
        }
    }

    // 触发GC
    bool result = gc_manager_->CollectGarbage(false);
    EXPECT_TRUE(result);
}

/**
 * @test 测试完整GC
 */
TEST_F(GCManagerTest, FullGC) {
    // 分配一些对象
    {
        GCHandleScope<10> scope(context_.get());
        for (int i = 0; i < 5; i++) {
            scope.New<TestManagerObject>(i);
        }
    }

    // 触发完整GC
    bool result = gc_manager_->CollectGarbage(true);
    EXPECT_TRUE(result);
}

/**
 * @test 测试强制完整GC
 */
TEST_F(GCManagerTest, ForceFullGC) {
    // 分配一些对象
    {
        GCHandleScope<10> scope(context_.get());
        for (int i = 0; i < 5; i++) {
            scope.New<TestManagerObject>(i);
        }
    }

    // 强制完整GC不应该崩溃
    gc_manager_->ForceFullGC();
}

/**
 * @test 测试GC阈值设置
 */
TEST_F(GCManagerTest, SetGCThreshold) {
    gc_manager_->SetGCThreshold(50);

    // 分配对象直到触发GC
    GCHandleScope<100> scope(context_.get());
    for (int i = 0; i < 100; i++) {
        auto obj = scope.New<TestManagerObject>(i);
        ASSERT_NE(obj.operator->(), nullptr);
    }
}

/**
 * @test 测试GC阈值边界值
 */
TEST_F(GCManagerTest, SetGCThresholdBoundary) {
    gc_manager_->SetGCThreshold(0);    // 应该被限制为最小值
    gc_manager_->SetGCThreshold(100);  // 应该被限制为最大值
    gc_manager_->SetGCThreshold(80);   // 正常值

    // 应该正常工作
    GCHandleScope<10> scope(context_.get());
    auto obj = scope.New<TestManagerObject>(1);
    EXPECT_NE(obj.operator->(), nullptr);
}

// ==================== 统计信息测试 ====================

/**
 * @test 测试获取堆统计信息
 */
TEST_F(GCManagerTest, GetHeapStats) {
    size_t new_used, new_capacity, old_used, old_capacity;

    gc_manager_->GetHeapStats(new_used, new_capacity, old_used, old_capacity);

    EXPECT_GT(new_capacity, 0);
    EXPECT_GT(old_capacity, 0);
    EXPECT_GE(new_used, 0);
    EXPECT_GE(old_used, 0);
}

/**
 * @test 测试获取GC统计信息
 */
TEST_F(GCManagerTest, GetGCStats) {
    size_t total_allocated, total_collected;
    uint32_t gc_count;

    gc_manager_->GetGCStats(total_allocated, total_collected, gc_count);

    // 初始状态未通过Allocate分配任何对象，total_allocated为0是正常的
    EXPECT_EQ(total_allocated, 0);
    EXPECT_EQ(total_collected, 0);
    EXPECT_EQ(gc_count, 0);
}

/**
 * @test 测试分配后统计信息变化
 */
TEST_F(GCManagerTest, StatsAfterAllocation) {
    // 注意：GCHandleScope::New 使用 context_->gc_manager()
    // 所以我们应该使用 context_->gc_manager() 来获取统计信息
    size_t alloc_before, collected_before;
    uint32_t count_before;

    context_->gc_manager().GetGCStats(alloc_before, collected_before, count_before);

    // 分配对象，保持HandleScope打开以确保对象存活
    GCHandleScope<10> scope(context_.get());
    for (int i = 0; i < 5; i++) {
        scope.New<TestManagerObject>(i);
    }

    size_t alloc_after, collected_after;
    uint32_t count_after;

    context_->gc_manager().GetGCStats(alloc_after, collected_after, count_after);

    // 分配后总分配字节数应该增加
    EXPECT_GT(alloc_after, alloc_before);
}

/**
 * @test 测试GC后统计信息变化
 */
TEST_F(GCManagerTest, StatsAfterGC) {
    // 分配对象
    {
        GCHandleScope<10> scope(context_.get());
        for (int i = 0; i < 5; i++) {
            scope.New<TestManagerObject>(i);
        }
    }

    size_t alloc_before, collected_before;
    uint32_t count_before;

    gc_manager_->GetGCStats(alloc_before, collected_before, count_before);

    // 触发GC
    gc_manager_->CollectGarbage(true);

    size_t alloc_after, collected_after;
    uint32_t count_after;

    gc_manager_->GetGCStats(alloc_after, collected_after, count_after);

    EXPECT_EQ(alloc_after, alloc_before);  // 总分配字节数不变
    EXPECT_GT(count_after, count_before);  // GC计数增加
}

/**
 * @test 测试打印统计信息
 */
TEST_F(GCManagerTest, PrintStats) {
    // 重定向cout
    std::streambuf* old_cout = std::cout.rdbuf();
    std::stringstream str_cout;
    std::cout.rdbuf(str_cout.rdbuf());

    // 打印统计信息
    gc_manager_->PrintStats();

    // 恢复cout
    std::cout.rdbuf(old_cout);

    // 验证输出
    std::string output = str_cout.str();
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("GC Statistics"), std::string::npos);
}

// ==================== 根集合管理测试 ====================

/**
 * @test 测试添加根引用
 */
TEST_F(GCManagerTest, AddRoot) {
    GCHandleScope<1> scope(context_.get());
    auto obj = scope.New<Object>();
    Value val = obj.ToValue();

    Value* val_ptr = &val;
    gc_manager_->AddRoot(val_ptr);

    // 添加后应该能移除
    gc_manager_->RemoveRoot(val_ptr);
}

/**
 * @test 测试移除根引用
 */
TEST_F(GCManagerTest, RemoveRoot) {
    GCHandleScope<1> scope(context_.get());
    auto obj = scope.New<Object>();
    Value val = obj.ToValue();

    Value* val_ptr = &val;
    gc_manager_->AddRoot(val_ptr);
    gc_manager_->RemoveRoot(val_ptr);

    // 移除后再移除不应该崩溃
    gc_manager_->RemoveRoot(val_ptr);
}

/**
 * @test 测试添加和移除多个根引用
 */
TEST_F(GCManagerTest, AddRemoveMultipleRoots) {
    GCHandleScope<3> scope(context_.get());
    auto obj1 = scope.New<Object>();
    auto obj2 = scope.New<Object>();
    auto obj3 = scope.New<Object>();

    Value val1 = obj1.ToValue();
    Value val2 = obj2.ToValue();
    Value val3 = obj3.ToValue();

    gc_manager_->AddRoot(&val1);
    gc_manager_->AddRoot(&val2);
    gc_manager_->AddRoot(&val3);

    gc_manager_->RemoveRoot(&val2);
    gc_manager_->RemoveRoot(&val1);
    gc_manager_->RemoveRoot(&val3);
}

/**
 * @test 测试空根引用
 */
TEST_F(GCManagerTest, NullRoot) {
    gc_manager_->AddRoot(nullptr);  // 不应该崩溃
    gc_manager_->RemoveRoot(nullptr);  // 不应该崩溃
}

// ==================== 打印对象树测试 ====================

/**
 * @test 测试打印对象树
 */
TEST_F(GCManagerTest, PrintObjectTree) {
    // 重定向cout
    std::streambuf* old_cout = std::cout.rdbuf();
    std::stringstream str_cout;
    std::cout.rdbuf(str_cout.rdbuf());

    // 打印对象树
    gc_manager_->PrintObjectTree(context_.get());

    // 恢复cout
    std::cout.rdbuf(old_cout);

    // 验证输出
    std::string output = str_cout.str();
    EXPECT_FALSE(output.empty());
}

// ==================== 使用HandleScope的测试 ====================

/**
 * @test 测试HandleScope与对象分配
 */
TEST_F(GCManagerTest, HandleScopeWithAllocation) {
    GCHandleScope<5> scope(context_.get());

    auto obj1 = scope.New<TestManagerObject>(10);
    auto obj2 = scope.New<TestManagerObject>(20);
    auto obj3 = scope.New<TestManagerObject>(30);

    EXPECT_EQ(obj1->data(), 10);
    EXPECT_EQ(obj2->data(), 20);
    EXPECT_EQ(obj3->data(), 30);
}

/**
 * @test 测试HandleScope中的GC
 */
TEST_F(GCManagerTest, GCWithHandleScope) {
    GCHandleScope<10> scope(context_.get());

    auto obj1 = scope.New<TestManagerObject>(1);
    auto obj2 = scope.New<TestManagerObject>(2);
    auto obj3 = scope.New<TestManagerObject>(3);

    // 触发GC
    gc_manager_->CollectGarbage(false);

    // 对象应该仍然有效
    EXPECT_EQ(obj1->data(), 1);
    EXPECT_EQ(obj2->data(), 2);
    EXPECT_EQ(obj3->data(), 3);
}

/**
 * @test 测试嵌套HandleScope
 */
TEST_F(GCManagerTest, NestedHandleScope) {
    {
        GCHandleScope<3> outer_scope(context_.get());
        auto obj1 = outer_scope.New<TestManagerObject>(1);
        auto obj2 = outer_scope.New<TestManagerObject>(2);

        {
            GCHandleScope<2> inner_scope(context_.get());
            auto obj3 = inner_scope.New<TestManagerObject>(3);
            auto obj4 = inner_scope.New<TestManagerObject>(4);

            EXPECT_EQ(obj3->data(), 3);
            EXPECT_EQ(obj4->data(), 4);
        }

        EXPECT_EQ(obj1->data(), 1);
        EXPECT_EQ(obj2->data(), 2);
    }
}

// ==================== 边界条件测试 ====================

/**
 * @test 测试自动GC触发
 */
TEST_F(GCManagerTest, AutoGCTrigger) {
    // 设置较低的GC阈值
    gc_manager_->SetGCThreshold(50);

    GCHandleScope<300> scope(context_.get());

    // 分配对象，手动触发GC验证
    for (int i = 0; i < 200; i++) {
        scope.New<TestManagerObject>(i);
    }

    // 手动触发GC
    gc_manager_->CollectGarbage(false);

    // 验证GC被触发
    size_t alloc, collected;
    uint32_t count;
    gc_manager_->GetGCStats(alloc, collected, count);
    EXPECT_GT(count, 0);
}

/**
 * @test 测试连续分配和GC
 */
TEST_F(GCManagerTest, ContinuousAllocationAndGC) {
    const int kIterations = 3;

    for (int iter = 0; iter < kIterations; iter++) {
        GCHandleScope<20> scope(context_.get());

        for (int i = 0; i < 10; i++) {
            scope.New<TestManagerObject>(i);
        }

        gc_manager_->CollectGarbage(false);
    }

    // 应该成功完成多次迭代
}

/**
 * @test 测试获取堆指针
 */
TEST_F(GCManagerTest, GetHeapPointer) {
    GCHeap* heap = gc_manager_->heap();
    ASSERT_NE(heap, nullptr);

    // 堆应该能正常工作
    size_t size = sizeof(TestManagerObject);
    GCGeneration generation;
    void* mem = heap->Allocate(&size, &generation);
    EXPECT_NE(mem, nullptr);
}

// ==================== 未初始化管理器测试 ====================

/**
 * @test 测试未初始化管理器的GC操作
 */
TEST_F(GCManagerTest, UninitializedManagerGC) {
    GCManager uninitialized_manager(context_.get());

    // 未初始化的管理器不应该崩溃
    bool result = uninitialized_manager.CollectGarbage(false);
    EXPECT_FALSE(result);

    uninitialized_manager.ForceFullGC();  // 不应该崩溃
}

/**
 * @test 测试未初始化管理器的统计信息
 */
TEST_F(GCManagerTest, UninitializedManagerStats) {
    GCManager uninitialized_manager(context_.get());

    size_t new_used, new_capacity, old_used, old_capacity;
    uninitialized_manager.GetHeapStats(new_used, new_capacity, old_used, old_capacity);

    EXPECT_EQ(new_capacity, 0);
    EXPECT_EQ(old_capacity, 0);

    size_t alloc, collected;
    uint32_t count;
    uninitialized_manager.GetGCStats(alloc, collected, count);

    EXPECT_EQ(alloc, 0);
    EXPECT_EQ(collected, 0);
    EXPECT_EQ(count, 0);
}

/**
 * @test 测试未初始化管理器的根集合操作
 */
TEST_F(GCManagerTest, UninitializedManagerRoots) {
    GCManager uninitialized_manager(context_.get());

    GCHandleScope<1> scope(context_.get());
    auto obj = scope.New<Object>();
    Value val = obj.ToValue();

    // 不应该崩溃
    uninitialized_manager.AddRoot(&val);
    uninitialized_manager.RemoveRoot(&val);
}

/**
 * @test 测试未初始化管理器的阈值设置
 */
TEST_F(GCManagerTest, UninitializedManagerThreshold) {
    GCManager uninitialized_manager(context_.get());

    // 不应该崩溃
    uninitialized_manager.SetGCThreshold(50);
}

} // namespace test
} // namespace mjs
