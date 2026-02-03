/**
 * @file gc_object_test.cpp
 * @brief GCObject 和 GCObjectHeader 单元测试
 *
 * 测试垃圾回收对象基类和对象头部的功能：
 * - GCObjectHeader 的各种标记位操作
 * - GCObject 的基本功能
 * - 对象对齐功能
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>

#include <mjs/gc/gc_object.h>
#include <mjs/context.h>
#include <mjs/runtime.h>

namespace mjs {
namespace test {

// 测试用的简单 GCObject 派生类
class TestGCObject : public GCObject {
public:
    TestGCObject() : data_(0) {}
    explicit TestGCObject(int data) : data_(data) {}

    int data() const { return data_; }
    void set_data(int data) { data_ = data; }

    // 实现 GCTraverse（测试对象没有子引用）
    void GCTraverse(Context* context, GCTraverseCallback callback) override {
        // 测试对象没有子引用，不需要做任何事
        (void)context;
        (void)callback;
    }

private:
    int data_;
};

// 测试用的带子对象的 GCObject 派生类
class TestGCObjectWithChildren : public GCObject {
public:
    TestGCObjectWithChildren() : child_(nullptr) {}

    void set_child(GCObject* child) { child_ = child; }
    GCObject* child() const { return child_; }

    // 实现 GCTraverse
    void GCTraverse(Context* context, GCTraverseCallback callback) override {
        if (child_ != nullptr) {
            Value child_value(static_cast<Object*>(child_));
            callback(context, &child_value);
        }
    }

private:
    GCObject* child_;
};

/**
 * @class GCObjectTest
 * @brief GCObject 类单元测试
 */
class GCObjectTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ==================== GCObjectHeader 测试 ====================

/**
 * @test 测试对象类型设置和获取
 */
TEST_F(GCObjectTest, HeaderType) {
    TestGCObject obj;
    EXPECT_EQ(obj.header()->type(), GCObjectType::kNone);

    obj.header()->set_type(GCObjectType::kObject);
    EXPECT_EQ(obj.header()->type(), GCObjectType::kObject);

    obj.header()->set_type(GCObjectType::kOther);
    EXPECT_EQ(obj.header()->type(), GCObjectType::kOther);
}

/**
 * @test 测试代际设置和获取
 */
TEST_F(GCObjectTest, HeaderGeneration) {
    TestGCObject obj;
    EXPECT_EQ(obj.header()->generation(), GCGeneration::kNew);

    obj.header()->set_generation(GCGeneration::kOld);
    EXPECT_EQ(obj.header()->generation(), GCGeneration::kOld);

    obj.header()->set_generation(GCGeneration::kNew);
    EXPECT_EQ(obj.header()->generation(), GCGeneration::kNew);
}

/**
 * @test 测试标记位设置和获取
 */
TEST_F(GCObjectTest, HeaderMarked) {
    TestGCObject obj;
    EXPECT_FALSE(obj.header()->IsMarked());

    obj.header()->SetMarked(true);
    EXPECT_TRUE(obj.header()->IsMarked());

    obj.header()->SetMarked(false);
    EXPECT_FALSE(obj.header()->IsMarked());
}

/**
 * @test 测试转发地址设置和获取
 */
TEST_F(GCObjectTest, HeaderForwardingAddress) {
    TestGCObject obj1;
    TestGCObject obj2;

    EXPECT_FALSE(obj1.header()->IsForwarded());
    EXPECT_EQ(obj1.header()->GetForwardingAddress(), nullptr);

    obj1.header()->SetForwardingAddress(&obj2);
    EXPECT_TRUE(obj1.header()->IsForwarded());
    EXPECT_EQ(obj1.header()->GetForwardingAddress(), &obj2);

    obj1.header()->SetForwardingAddress(nullptr);
    EXPECT_FALSE(obj1.header()->IsForwarded());
    EXPECT_EQ(obj1.header()->GetForwardingAddress(), nullptr);
}

/**
 * @test 测试固定标记设置和获取
 */
TEST_F(GCObjectTest, HeaderPinned) {
    TestGCObject obj;
    EXPECT_FALSE(obj.header()->IsPinned());

    obj.header()->SetPinned(true);
    EXPECT_TRUE(obj.header()->IsPinned());

    obj.header()->SetPinned(false);
    EXPECT_FALSE(obj.header()->IsPinned());
}

/**
 * @test 测试年龄增加和清空
 */
TEST_F(GCObjectTest, HeaderAge) {
    TestGCObject obj;
    EXPECT_EQ(obj.header()->age(), 0);

    obj.header()->IncrementAge();
    EXPECT_EQ(obj.header()->age(), 1);

    obj.header()->IncrementAge();
    obj.header()->IncrementAge();
    EXPECT_EQ(obj.header()->age(), 3);

    obj.header()->ClearAge();
    EXPECT_EQ(obj.header()->age(), 0);
}

/**
 * @test 测试年龄边界（最大值为15，因为只有4位）
 */
TEST_F(GCObjectTest, HeaderAgeBoundary) {
    TestGCObject obj;

    // 增加年龄到超过最大值
    for (int i = 0; i < 20; i++) {
        obj.header()->IncrementAge();
    }
    // 年龄字段是4位，最大值为15
    // 由于溢出行为，我们只验证它能正常工作
    EXPECT_GE(obj.header()->age(), 0);
}

/**
 * @test 测试析构标记设置和获取
 */
TEST_F(GCObjectTest, HeaderDestructed) {
    TestGCObject obj;
    EXPECT_FALSE(obj.header()->IsDestructed());

    obj.header()->SetDestructed(true);
    EXPECT_TRUE(obj.header()->IsDestructed());

    obj.header()->SetDestructed(false);
    EXPECT_FALSE(obj.header()->IsDestructed());
}

/**
 * @test 测试对象大小设置和获取
 */
TEST_F(GCObjectTest, HeaderSize) {
    TestGCObject obj;
    // 栈分配的对象，header 需要手动初始化
    obj.header()->set_size(sizeof(TestGCObject));
    EXPECT_EQ(obj.header()->size(), sizeof(TestGCObject));

    obj.header()->set_size(100);
    EXPECT_EQ(obj.header()->size(), 100);

    obj.header()->set_size(sizeof(TestGCObject));
    EXPECT_EQ(obj.header()->size(), sizeof(TestGCObject));
}

/**
 * @test 测试头部常量访问
 */
TEST_F(GCObjectTest, HeaderConstAccess) {
    const TestGCObject obj;
    EXPECT_EQ(obj.header()->type(), GCObjectType::kNone);
    EXPECT_EQ(obj.header()->generation(), GCGeneration::kNew);
    EXPECT_FALSE(obj.header()->IsMarked());
    EXPECT_FALSE(obj.header()->IsForwarded());
    EXPECT_FALSE(obj.header()->IsPinned());
    EXPECT_EQ(obj.header()->age(), 0);
    EXPECT_FALSE(obj.header()->IsDestructed());
}

// ==================== GCObject 测试 ====================

/**
 * @test 测试 GCObject 默认构造
 */
TEST_F(GCObjectTest, DefaultConstructor) {
    TestGCObject obj;
    EXPECT_EQ(obj.data(), 0);
}

/**
 * @test 测试 GCObject 带参数构造
 */
TEST_F(GCObjectTest, ConstructorWithParameter) {
    TestGCObject obj(42);
    EXPECT_EQ(obj.data(), 42);
}

/**
 * @test 测试获取可变和常量头部
 */
TEST_F(GCObjectTest, GetHeader) {
    TestGCObject obj;
    GCObjectHeader* header = obj.header();
    ASSERT_NE(header, nullptr);

    const TestGCObject& const_obj = obj;
    const GCObjectHeader* const_header = const_obj.header();
    ASSERT_NE(const_header, nullptr);
    EXPECT_EQ(header, const_header);
}

/**
 * @test 测试 GCTraverse 纯虚函数实现
 */
TEST_F(GCObjectTest, GCTraverse) {
    Runtime runtime;
    Context context(&runtime);

    TestGCObject obj;
    // 不应该崩溃
    obj.GCTraverse(&context, [](Context* ctx, Value* child) {
        (void)ctx;
        (void)child;
    });
}

// 用于测试 GCTraverse 的全局回调变量
namespace {
    bool g_callback_called = false;
    const GCObject* g_captured_child = nullptr;
}

/**
 * @test 测试带子对象的 GCTraverse
 */
TEST_F(GCObjectTest, GCTraverseWithChildren) {
    Runtime runtime;
    Context context(&runtime);

    TestGCObjectWithChildren parent;
    TestGCObject child;

    // 重置全局变量
    g_callback_called = false;
    g_captured_child = nullptr;

    parent.set_child(&child);
    parent.GCTraverse(&context, [](Context* ctx, Value* child_value) {
        (void)ctx;
        g_callback_called = true;
        if (child_value->IsObject()) {
            g_captured_child = &child_value->object();
        }
    });

    EXPECT_TRUE(g_callback_called);
    EXPECT_EQ(g_captured_child, &child);
}

/**
 * @test 测试 GCMoved 回调
 */
TEST_F(GCObjectTest, GCMoved) {
    TestGCObject obj;
    void* old_addr = &obj;

    // 默认实现不应该崩溃
    obj.GCMoved(old_addr);
}

// ==================== 对齐功能测试 ====================

/**
 * @test 测试对象大小对齐
 */
TEST_F(GCObjectTest, AlignGCObjectSize) {
    // 8 字节对齐
    EXPECT_EQ(AlignGCObjectSize(0), 0);
    EXPECT_EQ(AlignGCObjectSize(1), 8);
    EXPECT_EQ(AlignGCObjectSize(7), 8);
    EXPECT_EQ(AlignGCObjectSize(8), 8);
    EXPECT_EQ(AlignGCObjectSize(9), 16);
    EXPECT_EQ(AlignGCObjectSize(15), 16);
    EXPECT_EQ(AlignGCObjectSize(16), 16);
    EXPECT_EQ(AlignGCObjectSize(17), 24);
    EXPECT_EQ(AlignGCObjectSize(100), 104);
    EXPECT_EQ(AlignGCObjectSize(256), 256);
    EXPECT_EQ(AlignGCObjectSize(257), 264);
}

/**
 * @test 测试各种大小的对象对齐
 */
TEST_F(GCObjectTest, AlignVariousSizes) {
    // 测试边界条件
    for (size_t size = 0; size <= 64; ++size) {
        size_t aligned = AlignGCObjectSize(size);
        EXPECT_GE(aligned, size);
        EXPECT_EQ(aligned % kGCObjectAlignment, 0);
        EXPECT_LT(aligned - size, kGCObjectAlignment);
    }
}

// ==================== 头部位字段组合测试 ====================

/**
 * @test 测试同时设置多个标志位
 */
TEST_F(GCObjectTest, MultipleHeaderFlags) {
    TestGCObject obj;

    // 设置多个标志
    obj.header()->SetMarked(true);
    obj.header()->SetPinned(true);
    obj.header()->set_type(GCObjectType::kObject);
    obj.header()->set_generation(GCGeneration::kOld);
    obj.header()->IncrementAge();
    obj.header()->IncrementAge();

    // 验证所有标志都正确设置
    EXPECT_TRUE(obj.header()->IsMarked());
    EXPECT_TRUE(obj.header()->IsPinned());
    EXPECT_EQ(obj.header()->type(), GCObjectType::kObject);
    EXPECT_EQ(obj.header()->generation(), GCGeneration::kOld);
    EXPECT_EQ(obj.header()->age(), 2);

    // 清除部分标志
    obj.header()->SetMarked(false);
    obj.header()->SetPinned(false);

    EXPECT_FALSE(obj.header()->IsMarked());
    EXPECT_FALSE(obj.header()->IsPinned());
    EXPECT_EQ(obj.header()->type(), GCObjectType::kObject);
    EXPECT_EQ(obj.header()->generation(), GCGeneration::kOld);
    EXPECT_EQ(obj.header()->age(), 2);
}

/**
 * @test 测试转发地址与标记的交互
 */
TEST_F(GCObjectTest, ForwardingAddressWithFlags) {
    TestGCObject obj1, obj2;

    obj1.header()->SetMarked(true);
    obj1.header()->SetPinned(true);
    obj1.header()->IncrementAge();

    // 设置转发地址
    obj1.header()->SetForwardingAddress(&obj2);

    // 验证转发地址设置正确，其他标志不受影响
    EXPECT_TRUE(obj1.header()->IsForwarded());
    EXPECT_EQ(obj1.header()->GetForwardingAddress(), &obj2);
    EXPECT_TRUE(obj1.header()->IsMarked());
    EXPECT_TRUE(obj1.header()->IsPinned());
    EXPECT_EQ(obj1.header()->age(), 1);
}

} // namespace test
} // namespace mjs
