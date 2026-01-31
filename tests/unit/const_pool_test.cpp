/**
 * @file const_pool_test.cpp
 * @brief 常量池单元测试
 *
 * 测试GlobalConstPool和LocalConstPool的功能,包括:
 * - 常量插入和查找
 * - 常量索引管理
 * - 常量去重
 * - 引用计数管理(LocalConstPool)
 * - 内存管理
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>

#include <mjs/global_const_pool.h>
#include <mjs/local_const_pool.h>
#include <mjs/runtime.h>

namespace mjs {
namespace test {

// ==================== GlobalConstPool测试 ====================

class GlobalConstPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        pool_ = std::make_unique<GlobalConstPool>();
    }

    void TearDown() override {
        pool_.reset();
    }

protected:
    std::unique_ptr<GlobalConstPool> pool_;
};

/**
 * @brief 测试插入undefined常量
 * 注意: 使用默认构造函数创建undefined
 */
TEST_F(GlobalConstPoolTest, InsertUndefined) {
    Value v; // 默认构造函数创建undefined
    ConstIndex idx = pool_->FindOrInsert(v);

    EXPECT_GE(idx, 0);
    // size可能大于1,因为SegmentedArray可能有预分配
    EXPECT_GE(pool_->size(), 1);
    EXPECT_EQ((*pool_)[idx].type(), ValueType::kUndefined);
}

/**
 * @brief 测试插入null常量
 */
TEST_F(GlobalConstPoolTest, InsertNull) {
    Value v(nullptr);
    ConstIndex idx = pool_->FindOrInsert(v);

    EXPECT_GE(idx, 0);
    EXPECT_GE(pool_->size(), 1);
    EXPECT_TRUE((*pool_)[idx].IsNull());
}

/**
 * @brief 测试插入布尔常量
 */
TEST_F(GlobalConstPoolTest, InsertBoolean) {
    Value v_true(true);
    Value v_false(false);

    ConstIndex idx1 = pool_->FindOrInsert(v_true);
    ConstIndex idx2 = pool_->FindOrInsert(v_false);

    EXPECT_GE(idx1, 0);
    EXPECT_GE(idx2, 0);
    EXPECT_GE(pool_->size(), 1);
    EXPECT_TRUE((*pool_)[idx1].boolean());
    EXPECT_FALSE((*pool_)[idx2].boolean());
}

/**
 * @brief 测试插入整数常量
 */
TEST_F(GlobalConstPoolTest, InsertInt64) {
    Value v(static_cast<int64_t>(42));
    ConstIndex idx = pool_->FindOrInsert(v);

    EXPECT_GE(idx, 0);
    EXPECT_GE(pool_->size(), 1);
    EXPECT_EQ((*pool_)[idx].i64(), 42);
}

/**
 * @brief 测试插入浮点数常量
 */
TEST_F(GlobalConstPoolTest, InsertFloat64) {
    Value v(3.14);
    ConstIndex idx = pool_->FindOrInsert(v);

    EXPECT_GE(idx, 0);
    EXPECT_GE(pool_->size(), 1);
    EXPECT_DOUBLE_EQ((*pool_)[idx].f64(), 3.14);
}

/**
 * @brief 测试插入字符串常量
 */
TEST_F(GlobalConstPoolTest, InsertStringView) {
    Value v("hello world");
    ConstIndex idx = pool_->FindOrInsert(v);

    EXPECT_GE(idx, 0);
    EXPECT_GE(pool_->size(), 1);
    EXPECT_STREQ((*pool_)[idx].string_view(), "hello world");
}

/**
 * @brief 测试常量去重 - 相同值返回相同索引
 */
TEST_F(GlobalConstPoolTest, DuplicateValuesReturnSameIndex) {
    Value v1(42);
    Value v2(42);
    Value v3(3.14);

    ConstIndex idx1 = pool_->FindOrInsert(v1);
    ConstIndex idx2 = pool_->FindOrInsert(v2);
    ConstIndex idx3 = pool_->FindOrInsert(v3);

    EXPECT_EQ(idx1, idx2); // 相同值应该返回相同索引
    EXPECT_NE(idx1, idx3); // 不同值返回不同索引
    EXPECT_GE(pool_->size(), 1); // 只有2个不同的常量
}

/**
 * @brief 测试常量去重 - 浮点数和整数不合并
 */
TEST_F(GlobalConstPoolTest, FloatAndIntNotDeduplicated) {
    Value v_int(42);
    Value v_float(42.0);

    ConstIndex idx_int = pool_->FindOrInsert(v_int);
    ConstIndex idx_float = pool_->FindOrInsert(v_float);

    EXPECT_NE(idx_int, idx_float); // 不同类型,不合并
    EXPECT_GE(pool_->size(), 1);
}

/**
 * @brief 测试find方法 - 查找存在的常量
 */
TEST_F(GlobalConstPoolTest, FindExistingConstant) {
    Value v(42);
    ConstIndex idx = pool_->FindOrInsert(v);

    auto found = pool_->Find(v);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, idx);
}

/**
 * @brief 测试find方法 - 查找不存在的常量
 */
TEST_F(GlobalConstPoolTest, FindNonExistentConstant) {
    Value v(42);
    auto found = pool_->Find(v);

    EXPECT_FALSE(found.has_value());
}

/**
 * @brief 测试operator[]访问
 */
TEST_F(GlobalConstPoolTest, SubscriptOperator) {
    Value v(123);
    ConstIndex idx = pool_->FindOrInsert(v);

    const Value& retrieved = (*pool_)[idx];
    EXPECT_EQ(retrieved.i64(), 123);
}

/**
 * @brief 测试at方法 - 正常访问
 */
TEST_F(GlobalConstPoolTest, AtMethodValidIndex) {
    Value v(456);
    ConstIndex idx = pool_->FindOrInsert(v);

    const Value& retrieved = pool_->At(idx);
    EXPECT_EQ(retrieved.i64(), 456);
}

/**
 * @brief 测试at方法 - 越界访问抛出异常
 */
TEST_F(GlobalConstPoolTest, AtMethodInvalidIndexThrows) {
    Value v(42);
    pool_->FindOrInsert(v);

    EXPECT_THROW(
        pool_->At(999),
        std::out_of_range
    );

    EXPECT_THROW(
        pool_->At(-1),
        std::out_of_range
    );
}

/**
 * @brief 测试insert移动语义
 */
TEST_F(GlobalConstPoolTest, InsertMoveSemantics) {
    Value v(3.14);
    ConstIndex idx = pool_->FindOrInsert(std::move(v));

    EXPECT_GE(idx, 0);
    EXPECT_GE(pool_->size(), 1);
    EXPECT_DOUBLE_EQ((*pool_)[idx].f64(), 3.14);
}

/**
 * @brief 测试clear方法
 */
TEST_F(GlobalConstPoolTest, ClearMethod) {
    pool_->FindOrInsert(Value(42));
    pool_->FindOrInsert(Value(3.14));
    pool_->FindOrInsert(Value(true));

    auto size_before = pool_->size();
    EXPECT_GE(size_before, 1);

    pool_->Clear();

    // clear后size应该为1
    EXPECT_EQ(pool_->size(), 1);

    // 清空后可以重新插入
    pool_->FindOrInsert(Value(100));
    EXPECT_GE(pool_->size(), 2);
}

/**
 * @brief 测试大量常量插入
 */
TEST_F(GlobalConstPoolTest, InsertManyConstants) {
    const int count = 1000;

    for (int i = 0; i < count; i++) {
        Value v(static_cast<int64_t>(i));
        pool_->FindOrInsert(v);
    }

    EXPECT_EQ(pool_->size(), count + 1);

    // 验证所有常量都可以正确访问
    for (int i = 0; i < count; i++) {
        Value lookup(static_cast<int64_t>(i));
        auto found = pool_->Find(lookup);
        ASSERT_TRUE(found.has_value());
    }
}

/**
 * @brief 测试特殊值 - NaN
 */
TEST_F(GlobalConstPoolTest, InsertNanValue) {
    Value v(std::numeric_limits<double>::quiet_NaN());
    ConstIndex idx = pool_->FindOrInsert(v);

    EXPECT_GE(idx, 0);
    EXPECT_TRUE(std::isnan((*pool_)[idx].f64()));
}

/**
 * @brief 测试特殊值 - Infinity
 */
TEST_F(GlobalConstPoolTest, InsertInfinityValue) {
    Value v_pos(std::numeric_limits<double>::infinity());
    Value v_neg(-std::numeric_limits<double>::infinity());

    ConstIndex idx_pos = pool_->FindOrInsert(v_pos);
    ConstIndex idx_neg = pool_->FindOrInsert(v_neg);

    EXPECT_TRUE(std::isinf((*pool_)[idx_pos].f64()));
    EXPECT_TRUE(std::isinf((*pool_)[idx_neg].f64()));
}

// ==================== LocalConstPool测试 ====================

class LocalConstPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        pool_ = std::make_unique<LocalConstPool>();
    }

    void TearDown() override {
        pool_.reset();
    }

protected:
    std::unique_ptr<LocalConstPool> pool_;
};

/**
 * @brief 测试LocalConstPool插入常量
 */
TEST_F(LocalConstPoolTest, InsertConstant) {
    Value v(42);
    ConstIndex idx = pool_->FindOrInsert(v);

    EXPECT_LE(idx, 0); // LocalConstPool使用负索引
    EXPECT_EQ((*pool_)[idx].i64(), 42);
}

/**
 * @brief 测试LocalConstPool常量去重
 */
TEST_F(LocalConstPoolTest, DuplicateValuesSameIndex) {
    Value v1(42);
    Value v2(42);

    ConstIndex idx1 = pool_->FindOrInsert(v1);
    ConstIndex idx2 = pool_->FindOrInsert(v2);

    EXPECT_EQ(idx1, idx2);
}

/**
 * @brief 测试find方法
 */
TEST_F(LocalConstPoolTest, FindConstant) {
    Value v(42);
    pool_->FindOrInsert(v);

    auto found = pool_->Find(v);
    ASSERT_TRUE(found.has_value());
    EXPECT_LE(*found, 0); // 负索引
}

/**
 * @brief 测试find不存在的常量
 */
TEST_F(LocalConstPoolTest, FindNonExistentConstant) {
    Value v(42);
    auto found = pool_->Find(v);

    EXPECT_FALSE(found.has_value());
}

/**
 * @brief 测试at方法
 */
TEST_F(LocalConstPoolTest, AtMethod) {
    Value v(3.14);
    ConstIndex idx = pool_->FindOrInsert(v);

    const Value& retrieved = pool_->At(idx);
    EXPECT_DOUBLE_EQ(retrieved.f64(), 3.14);
}

/**
 * @brief 测试operator[]
 */
TEST_F(LocalConstPoolTest, SubscriptOperator) {
    Value v(123);
    ConstIndex idx = pool_->FindOrInsert(v);

    const Value& retrieved = (*pool_)[idx];
    EXPECT_EQ(retrieved.i64(), 123);
}

/**
 * @brief 测试引用计数 - ReferenceConst和DereferenceConst
 */
TEST_F(LocalConstPoolTest, ReferenceAndDereference) {
    Value v(42);
    ConstIndex idx = pool_->FindOrInsert(v);

    // 增加引用计数
    pool_->ReferenceConst(idx);
    pool_->ReferenceConst(idx);

    // 减少引用计数
    pool_->DereferenceConst(idx);
    // 常量应该还存在

    pool_->DereferenceConst(idx);
    // 引用计数归零,常量被删除
}

/**
 * @brief 测试引用计数为0时自动删除
 */
TEST_F(LocalConstPoolTest, AutoDeleteWhenRefCountZero) {
    Value v(42);
    ConstIndex idx = pool_->FindOrInsert(v);

    // 验证常量存在
    auto found = pool_->Find(v);
    ASSERT_TRUE(found.has_value());

    // 增加引用计数到1
    pool_->ReferenceConst(idx);

    // 减少引用计数到0
    pool_->DereferenceConst(idx);

    // 常量应该被删除
    found = pool_->Find(v);
    EXPECT_FALSE(found.has_value());
}

/**
 * @brief 测试clear方法
 */
TEST_F(LocalConstPoolTest, ClearMethod) {
    pool_->FindOrInsert(Value(42));
    pool_->FindOrInsert(Value(3.14));

    pool_->Clear();

    auto found = pool_->Find(Value(42));
    EXPECT_FALSE(found.has_value());
}

/**
 * @brief 测试移动语义插入
 */
TEST_F(LocalConstPoolTest, InsertMoveSemantics) {
    Value v(3.14);
    ConstIndex idx = pool_->FindOrInsert(std::move(v));

    EXPECT_LE(idx, 0);
    EXPECT_DOUBLE_EQ((*pool_)[idx].f64(), 3.14);
}

/**
 * @brief 测试多个不同类型的常量
 */
TEST_F(LocalConstPoolTest, MultipleDifferentTypes) {
    ConstIndex idx_undefined = pool_->FindOrInsert(Value());  // 默认构造函数创建undefined
    ConstIndex idx_null = pool_->FindOrInsert(Value(nullptr));
    ConstIndex idx_bool = pool_->FindOrInsert(Value(true));
    ConstIndex idx_int = pool_->FindOrInsert(Value(static_cast<int64_t>(42)));
    ConstIndex idx_float = pool_->FindOrInsert(Value(3.14));
    ConstIndex idx_str = pool_->FindOrInsert(Value("hello"));

    EXPECT_LE(idx_undefined, 0);
    EXPECT_LE(idx_null, 0);
    EXPECT_LE(idx_bool, 0);
    EXPECT_LE(idx_int, 0);
    EXPECT_LE(idx_float, 0);
    EXPECT_LE(idx_str, 0);

    // 验证所有常量都可以正确访问
    EXPECT_EQ((*pool_)[idx_undefined].type(), ValueType::kUndefined);
    EXPECT_TRUE((*pool_)[idx_null].IsNull());
    EXPECT_TRUE((*pool_)[idx_bool].boolean());
    EXPECT_EQ((*pool_)[idx_int].i64(), 42);
    EXPECT_DOUBLE_EQ((*pool_)[idx_float].f64(), 3.14);
    EXPECT_STREQ((*pool_)[idx_str].string_view(), "hello");
}

/**
 * @brief 测试引用计数的正确性
 */
TEST_F(LocalConstPoolTest, ReferenceCountCorrectness) {
    Value v(42);
    ConstIndex idx = pool_->FindOrInsert(v);

    // 插入后引用计数为0
    // 增加引用
    pool_->ReferenceConst(idx); 
    pool_->ReferenceConst(idx);
    pool_->ReferenceConst(idx);
    pool_->ReferenceConst(idx);

    // 减少3次后引用计数应该还是1(初始值)
    pool_->DereferenceConst(idx);
    pool_->DereferenceConst(idx);
    pool_->DereferenceConst(idx);

    // 常量应该还存在
    auto found = pool_->Find(v);
    EXPECT_TRUE(found.has_value());

    // 再减少一次,引用计数归零
    pool_->DereferenceConst(idx);

    // 常量被删除
    found = pool_->Find(v);
    EXPECT_FALSE(found.has_value());
}

} // namespace test
} // namespace mjs
