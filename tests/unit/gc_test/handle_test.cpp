/**
 * @file handle_test.cpp
 * @brief GCHandleScope 和 GCHandle 单元测试
 *
 * 测试句柄机制的功能：
 * - 句柄作用域的创建和销毁
 * - 句柄的创建和使用
 * - 对象分配和句柄保护
 * - 嵌套作用域
 * - Close方法
 * - 边界条件
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>

#include <mjs/gc/handle.h>
#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/value/object/object.h>
#include <mjs/value/object/array_object.h>

namespace mjs {
namespace test {

// 测试用的简单对象
class TestHandleObject : public GCObject {
public:
    TestHandleObject() : data_(0) {}
    explicit TestHandleObject(int data) : data_(data) {}
    TestHandleObject(Context* context, int data) : GCObject(), data_(data) { (void)context; }

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
 * @class GCHandleScopeTest
 * @brief GCHandleScope 类单元测试
 */
class GCHandleScopeTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = std::make_unique<Runtime>();
        context_ = std::make_unique<Context>(runtime_.get());
    }

    void TearDown() override {
        context_.reset();
        runtime_.reset();
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
};

// ==================== 基本功能测试 ====================

/**
 * @test 测试句柄作用域构造和析构
 */
TEST_F(GCHandleScopeTest, ConstructAndDestruct) {
    {
        GCHandleScope<4> scope(context_.get());
        EXPECT_EQ(scope.size(), 0);
    }
    // 作用域结束后应该自动弹出
}

/**
 * @test 测试句柄作用域容量
 */
TEST_F(GCHandleScopeTest, Capacity) {
    GCHandleScope<8> scope8(context_.get());
    EXPECT_EQ(scope8.capacity(), 8);

    GCHandleScope<16> scope16(context_.get());
    EXPECT_EQ(scope16.capacity(), 16);

    GCHandleScope<1> scope1(context_.get());
    EXPECT_EQ(scope1.capacity(), 1);
}

/**
 * @test 测试获取句柄数组
 */
TEST_F(GCHandleScopeTest, GetHandles) {
    GCHandleScope<4> scope(context_.get());
    const auto& handles = scope.handles();

    EXPECT_EQ(handles.size(), 4);
}

// ==================== New方法测试 ====================

/**
 * @test 测试使用New分配对象
 */
TEST_F(GCHandleScopeTest, NewObject) {
    GCHandleScope<4> scope(context_.get());

    auto obj = scope.New<TestHandleObject>(42);
    ASSERT_NE(obj.operator->(), nullptr);
    EXPECT_EQ(obj->data(), 42);
    EXPECT_EQ(scope.size(), 1);
}

/**
 * @test 测试分配多个对象
 */
TEST_F(GCHandleScopeTest, NewMultipleObjects) {
    GCHandleScope<10> scope(context_.get());

    auto obj1 = scope.New<TestHandleObject>(1);
    auto obj2 = scope.New<TestHandleObject>(2);
    auto obj3 = scope.New<TestHandleObject>(3);

    EXPECT_EQ(obj1->data(), 1);
    EXPECT_EQ(obj2->data(), 2);
    EXPECT_EQ(obj3->data(), 3);
    EXPECT_EQ(scope.size(), 3);
}

/**
 * @test 测试分配Object类型
 */
TEST_F(GCHandleScopeTest, NewObjectObject) {
    GCHandleScope<4> scope(context_.get());

    auto obj = scope.New<Object>();
    ASSERT_NE(obj.operator->(), nullptr);
    EXPECT_EQ(scope.size(), 1);
}

/**
 * @test 测试分配ArrayObject类型
 */
TEST_F(GCHandleScopeTest, NewArrayObject) {
    GCHandleScope<4> scope(context_.get());

    auto arr = scope.New<ArrayObject>(10);
    ASSERT_NE(arr.operator->(), nullptr);
    EXPECT_EQ(scope.size(), 1);
}

// ==================== Create方法测试 ====================

/**
 * @test 测试使用Create创建句柄
 */
TEST_F(GCHandleScopeTest, CreateHandle) {
    GCHandleScope<4> scope(context_.get());

    // 先分配一个对象
    auto obj1 = scope.New<TestHandleObject>(42);

    // 从已有对象创建句柄
    auto obj2 = scope.Create<TestHandleObject>(obj1.operator->());

    EXPECT_EQ(obj2->data(), 42);
    EXPECT_EQ(scope.size(), 2);
}

/**
 * @test 测试从外部对象创建句柄
 */
TEST_F(GCHandleScopeTest, CreateFromExternalObject) {
    GCHandleScope<4> scope(context_.get());
    GCHandleScope<4> temp_scope(context_.get());

    // 在临时作用域中分配对象
    auto temp_obj = temp_scope.New<TestHandleObject>(100);

    // 在主作用域中创建句柄
    auto obj = scope.Create<TestHandleObject>(temp_obj.operator->());

    EXPECT_EQ(obj->data(), 100);
}

// ==================== 句柄访问测试 ====================

/**
 * @test 测试句柄箭头操作符
 */
TEST_F(GCHandleScopeTest, HandleArrowOperator) {
    GCHandleScope<4> scope(context_.get());

    auto obj = scope.New<TestHandleObject>(42);
    EXPECT_EQ(obj->data(), 42);

    obj->set_data(100);
    EXPECT_EQ(obj->data(), 100);
}

/**
 * @test 测试句柄解引用操作符
 */
TEST_F(GCHandleScopeTest, HandleDereferenceOperator) {
    GCHandleScope<4> scope(context_.get());

    auto obj = scope.New<TestHandleObject>(42);
    TestHandleObject& ref = *obj;
    EXPECT_EQ(ref.data(), 42);

    ref.set_data(200);
    EXPECT_EQ(obj->data(), 200);
}

/**
 * @test 测试句柄转Value
 */
TEST_F(GCHandleScopeTest, HandleToValue) {
    GCHandleScope<4> scope(context_.get());

    auto obj = scope.New<Object>();
    Value val = obj.ToValue();

    EXPECT_TRUE(val.IsObject());
}

/**
 * @test 测试空句柄检查
 */
TEST_F(GCHandleScopeTest, HandleIsEmpty) {
    GCHandleScope<4> scope(context_.get());

    // 默认构造的句柄是空的
    GCHandle<TestHandleObject> empty_handle;
    EXPECT_TRUE(empty_handle.IsEmpty());

    // 分配对象后的句柄不为空
    auto obj = scope.New<TestHandleObject>(42);
    EXPECT_FALSE(obj.IsEmpty());
}

/**
 * @test 测试获取GCObject指针
 */
TEST_F(GCHandleScopeTest, HandleGCObject) {
    GCHandleScope<4> scope(context_.get());

    auto obj = scope.New<TestHandleObject>(42);
    GCObject* gc_obj = obj.gc_obj();

    ASSERT_NE(gc_obj, nullptr);
}

// ==================== Close方法测试 ====================

/**
 * @test 测试Close方法
 */
TEST_F(GCHandleScopeTest, CloseHandleScope) {
    GCHandleScope<4> scope(context_.get());

    auto obj = scope.New<Object>();
    Value result = scope.Close(obj);

    EXPECT_TRUE(result.IsObject());
}

/**
 * @test 测试Close后作用域分离
 */
TEST_F(GCHandleScopeTest, ScopeDetachedAfterClose) {
    GCHandleScope<4> scope(context_.get());

    auto obj = scope.New<Object>();
    scope.Close(obj);

    // Close后作用域应该被标记为分离
    // 析构时不会再次弹出
}

// ==================== 嵌套作用域测试 ====================

/**
 * @test 测试嵌套句柄作用域
 */
TEST_F(GCHandleScopeTest, NestedHandleScope) {
    {
        GCHandleScope<4> outer_scope(context_.get());

        auto obj1 = outer_scope.New<TestHandleObject>(1);
        auto obj2 = outer_scope.New<TestHandleObject>(2);

        EXPECT_EQ(outer_scope.size(), 2);

        {
            GCHandleScope<4> inner_scope(context_.get());

            auto obj3 = inner_scope.New<TestHandleObject>(3);
            auto obj4 = inner_scope.New<TestHandleObject>(4);

            EXPECT_EQ(inner_scope.size(), 2);

            // 内部作用域的对象仍然有效
            EXPECT_EQ(obj3->data(), 3);
            EXPECT_EQ(obj4->data(), 4);
        }

        // 内部作用域结束后，外部对象仍然有效
        EXPECT_EQ(obj1->data(), 1);
        EXPECT_EQ(obj2->data(), 2);
    }
}

/**
 * @test 测试多层嵌套作用域
 */
TEST_F(GCHandleScopeTest, MultipleNestedScope) {
    {
        GCHandleScope<2> scope1(context_.get());
        auto obj1 = scope1.New<TestHandleObject>(1);

        {
            GCHandleScope<2> scope2(context_.get());
            auto obj2 = scope2.New<TestHandleObject>(2);

            {
                GCHandleScope<2> scope3(context_.get());
                auto obj3 = scope3.New<TestHandleObject>(3);

                EXPECT_EQ(obj3->data(), 3);
            }

            EXPECT_EQ(obj2->data(), 2);
        }

        EXPECT_EQ(obj1->data(), 1);
    }
}

/**
 * @test 测试Close后创建新作用域
 */
TEST_F(GCHandleScopeTest, NewScopeAfterClose) {
    {
        GCHandleScope<4> scope1(context_.get());
        auto obj1 = scope1.New<Object>();
        Value result = scope1.Close(obj1);
    }

    // Close后可以创建新的作用域
    GCHandleScope<4> scope2(context_.get());
    auto obj2 = scope2.New<TestHandleObject>(42);
    EXPECT_EQ(obj2->data(), 42);
}

// ==================== GC保护测试 ====================

/**
 * @test 测试句柄保护对象不被GC回收
 */
TEST_F(GCHandleScopeTest, HandleProtectsFromGC) {
    GCHandleScope<10> scope(context_.get());

    // 分配一些对象
    auto obj1 = scope.New<TestHandleObject>(1);
    auto obj2 = scope.New<TestHandleObject>(2);
    auto obj3 = scope.New<TestHandleObject>(3);

    // 触发GC
    context_->gc_manager().CollectGarbage(false);

    // 对象应该仍然有效
    EXPECT_EQ(obj1->data(), 1);
    EXPECT_EQ(obj2->data(), 2);
    EXPECT_EQ(obj3->data(), 3);
}

/**
 * @test 测试作用域结束后对象可被回收
 */
TEST_F(GCHandleScopeTest, ObjectsCollectedAfterScopeExit) {
    // 分配对象并在作用域结束后触发GC
    {
        GCHandleScope<10> scope(context_.get());
        for (int i = 0; i < 10; i++) {
            scope.New<TestHandleObject>(i);
        }
    }

    // 作用域结束后，对象应该可以被回收
    context_->gc_manager().CollectGarbage(false);

    // 验证没有崩溃
}

// ==================== 边界条件测试 ====================

/**
 * @test 测试容量为1的作用域
 */
TEST_F(GCHandleScopeTest, MinimalCapacityScope) {
    GCHandleScope<1> scope(context_.get());

    auto obj = scope.New<TestHandleObject>(42);
    EXPECT_EQ(scope.size(), 1);
    EXPECT_EQ(obj->data(), 42);
}

/**
 * @test 测试容量为0的编译时错误（通过static_assert）
 */
TEST_F(GCHandleScopeTest, ZeroCapacityScope) {
    // 这个测试应该是编译时错误，所以被禁用
    // GCHandleScope<0> scope(context_.get());  // 编译错误
}

/**
 * @test 测试大容量作用域
 */
TEST_F(GCHandleScopeTest, LargeCapacityScope) {
    GCHandleScope<100> scope(context_.get());

    const int kNumObjects = 50;
    for (int i = 0; i < kNumObjects; i++) {
        auto obj = scope.New<TestHandleObject>(i);
        EXPECT_EQ(obj->data(), i);
    }

    EXPECT_EQ(scope.size(), kNumObjects);
}

/**
 * @test 测试填满作用域
 */
TEST_F(GCHandleScopeTest, FillScopeCapacity) {
    GCHandleScope<5> scope(context_.get());

    auto obj1 = scope.New<TestHandleObject>(1);
    auto obj2 = scope.New<TestHandleObject>(2);
    auto obj3 = scope.New<TestHandleObject>(3);
    auto obj4 = scope.New<TestHandleObject>(4);
    auto obj5 = scope.New<TestHandleObject>(5);

    EXPECT_EQ(scope.size(), 5);
}

// ==================== 作用域链测试 ====================

/**
 * @test 测试获取上一个作用域
 */
TEST_F(GCHandleScopeTest, GetPreviousScope) {
    GCHandleScope<4> outer_scope(context_.get());

    {
        GCHandleScope<4> inner_scope(context_.get());

        // 内部作用域的上一个作用域应该是外部作用域
        EXPECT_EQ(inner_scope.prev(), &outer_scope);
    }

    // 外部作用域的上一个作用域应该是nullptr
    EXPECT_EQ(outer_scope.prev(), nullptr);
}

/**
 * @test 测试作用域链中的prev指针
 */
TEST_F(GCHandleScopeTest, ScopeChainPrevPointers) {
    GCHandleScope<2> scope1(context_.get());
    EXPECT_EQ(scope1.prev(), nullptr);

    GCHandleScope<2> scope2(context_.get());
    EXPECT_EQ(scope2.prev(), &scope1);

    GCHandleScope<2> scope3(context_.get());
    EXPECT_EQ(scope3.prev(), &scope2);
}

// ==================== 类型转换测试 ====================

/**
 * @test 测试句柄的类型转换
 */
TEST_F(GCHandleScopeTest, HandleTypeConversion) {
    GCHandleScope<4> scope(context_.get());

    auto arr = scope.New<ArrayObject>(10);
    GCHandle<GCObject> base_handle = GCHandle<GCObject>(arr.gc_obj());

    EXPECT_EQ(base_handle.gc_obj(), arr.gc_obj());
}

// ==================== 默认容量别名测试 ====================

/**
 * @test 测试默认容量常量
 */
TEST_F(GCHandleScopeTest, DefaultCapacityConstant) {
    EXPECT_EQ(kDefaultHandleScopeCapacity, 8);
}

/**
 * @test 测试默认句柄作用域
 */
TEST_F(GCHandleScopeTest, DefaultHandleScope) {
    DefaultHandleScope scope(context_.get());

    auto obj1 = scope.New<Object>();
    auto obj2 = scope.New<Object>();

    EXPECT_EQ(scope.capacity(), 8);
    EXPECT_EQ(scope.size(), 2);
}

// ==================== 分离状态测试 ====================

/**
 * @test 测试作用域分离状态
 */
TEST_F(GCHandleScopeTest, ScopeDetachedState) {
    GCHandleScope<4> scope(context_.get());

    // 初始状态未分离
    EXPECT_FALSE(scope.is_detached());

    auto obj = scope.New<Object>();
    scope.Close(obj);

    // Close后应该被标记为分离
    EXPECT_TRUE(scope.is_detached());
}

// ==================== data方法测试 ====================

/**
 * @test 测试获取句柄数据
 */
TEST_F(GCHandleScopeTest, GetHandleData) {
    GCHandleScope<4> scope(context_.get());

    auto obj1 = scope.New<TestHandleObject>(1);
    auto obj2 = scope.New<TestHandleObject>(2);

    const GCObject* const* data = scope.data();
    ASSERT_NE(data, nullptr);

    // 验证数据不为空
    EXPECT_NE(data[0], nullptr);
    EXPECT_NE(data[1], nullptr);
}

/**
 * @test 测试空作用域的data
 */
TEST_F(GCHandleScopeTest, EmptyScopeData) {
    GCHandleScope<4> scope(context_.get());

    const GCObject* const* data = scope.data();
    ASSERT_NE(data, nullptr);

    // 空作用域的数据指针应该有效
    EXPECT_EQ(scope.size(), 0);
}

} // namespace test
} // namespace mjs
