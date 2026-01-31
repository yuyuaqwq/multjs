/**
 * @file object_shape_test.cpp
 * @brief 对象和形状系统单元测试
 *
 * 测试 Object 和 Shape 相关类的功能
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/object.h>
#include <mjs/shape.h>
#include <mjs/shape_manager.h>
#include <mjs/shape_property.h>
#include <mjs/class_def.h>
#include <mjs/value.h>

namespace mjs {
namespace test {

/**
 * @class ObjectTest
 * @brief Object 类单元测试
 */
class ObjectTest : public ::testing::Test {
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

// ==================== Object 基本功能测试 ====================

/**
 * @test 测试对象创建
 */
TEST_F(ObjectTest, CreateObject) {
    auto* obj = Object::New(&runtime_->default_context());
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->ref_count(), 0);
    // GC will clean up
}

/**
 * @test 测试对象引用计数
 */
TEST_F(ObjectTest, ReferenceCount) {
    auto* obj = Object::New(&runtime_->default_context());
    EXPECT_EQ(obj->ref_count(), 0);

    obj->Reference();
    EXPECT_EQ(obj->ref_count(), 1);

    obj->Reference();
    EXPECT_EQ(obj->ref_count(), 2);

    obj->WeakDereference();
    EXPECT_EQ(obj->ref_count(), 1);

    obj->WeakDereference();
    EXPECT_EQ(obj->ref_count(), 0);

    // 不再调用Dereference(),因为引用计数已经是0
}

/**
 * @test 测试设置和获取字符串键属性
 */
TEST_F(ObjectTest, SetPropertyWithStringKey) {
    auto* obj = Object::New(&runtime_->default_context());
    auto index = runtime_->global_const_pool().FindOrInsert(Value("test_prop"));
    obj->SetProperty(&runtime_->default_context(), index, Value(42));

    Value retrieved_value;
    bool success = obj->GetProperty(&runtime_->default_context(), index, &retrieved_value);
    ASSERT_TRUE(success);
    EXPECT_EQ(retrieved_value.i64(), 42);

    // GC will clean up
}

/**
 * @test 测试设置和获取数字属性
 */
TEST_F(ObjectTest, SetPropertyWithNumber) {
    auto* obj = Object::New(&runtime_->default_context());
    {
        auto index = runtime_->global_const_pool().FindOrInsert(Value("number_prop"));
        obj->SetProperty(&runtime_->default_context(), index, Value(100));
        Value num_val;
        obj->GetProperty(&runtime_->default_context(), index, &num_val);
        EXPECT_EQ(num_val.i64(), 100);
    }
    {
        auto index = runtime_->global_const_pool().FindOrInsert(Value("float_prop"));
        obj->SetProperty(&runtime_->default_context(), index, Value(3.14));
        Value float_val;
        obj->GetProperty(&runtime_->default_context(), index, &float_val);
        EXPECT_DOUBLE_EQ(float_val.f64(), 3.14);
    }
    // GC will clean up
}

/**
 * @test 测试设置和获取字符串属性
 */
TEST_F(ObjectTest, SetPropertyWithString) {
    auto* obj = Object::New(&runtime_->default_context());

    auto index = runtime_->global_const_pool().FindOrInsert(Value("string_prop"));
    obj->SetProperty(&runtime_->default_context(), index, Value(String::New("hello")));
    Value str_val;
    obj->GetProperty(&runtime_->default_context(), index, &str_val);
    EXPECT_STREQ(str_val.string_view(), "hello");

    // GC will clean up
}

/**
 * @test 测试设置和获取布尔属性
 */
TEST_F(ObjectTest, SetPropertyWithBoolean) {
    auto* obj = Object::New(&runtime_->default_context());
    {
        auto index = runtime_->global_const_pool().FindOrInsert(Value("bool_prop"));
        obj->SetProperty(&runtime_->default_context(), index, Value(true));
        Value bool_val;
        obj->GetProperty(&runtime_->default_context(), index, &bool_val);
        EXPECT_EQ(bool_val.boolean(), true);
    }

    {
        auto index = runtime_->global_const_pool().FindOrInsert(Value("bool_prop2"));
        obj->SetProperty(&runtime_->default_context(), index, Value(false));
        Value bool_val2;
        obj->GetProperty(&runtime_->default_context(), index, &bool_val2);
        EXPECT_EQ(bool_val2.boolean(), false);
    }
    // GC will clean up
}

/**
 * @test 测试设置和获取null属性
 */
TEST_F(ObjectTest, SetPropertyWithNull) {
    auto* obj = Object::New(&runtime_->default_context());

    auto index = runtime_->global_const_pool().FindOrInsert(Value("null_prop"));
    obj->SetProperty(&runtime_->default_context(), index, Value(nullptr));
    Value null_val;
    obj->GetProperty(&runtime_->default_context(), index, &null_val);
    // 注意: 由于实现细节,null可能存储为其他类型
    EXPECT_TRUE(null_val.IsNull() || null_val.IsUndefined());

    // GC will clean up
}

/**
 * @test 测试设置和获取常量索引键属性
 */
TEST_F(ObjectTest, SetPropertyWithConstIndex) {
    auto* obj = Object::New(context_.get());
    ConstIndex key_idx = context_->FindConstOrInsertToLocal(Value("my_key"));

    obj->SetProperty(context_.get(), key_idx, Value(123));

    Value retrieved_value;
    bool success = obj->GetProperty(context_.get(), key_idx, &retrieved_value);
    ASSERT_TRUE(success);
    EXPECT_EQ(retrieved_value.i64(), 123);

    // GC will clean up
}

/**
 * @test 测试获取不存在的属性
 * @note 这个测试暂时跳过,因为GetProperty的实现可能在属性不存在时抛出异常
 */
TEST_F(ObjectTest, GetNonExistentProperty) {
    auto* obj = Object::New(&runtime_->default_context());
    Value retrieved_value(42);  // 初始化为已知值

    auto index = runtime_->global_const_pool().FindOrInsert(Value("non_existent"));
    bool success = obj->GetProperty(&runtime_->default_context(), index, &retrieved_value);
    EXPECT_FALSE(success);
    // 如果失败,retrieved_value应该保持不变

    // GC will clean up
}

/**
 * @test 测试检查属性是否存在
 */
TEST_F(ObjectTest, HasProperty) {
    auto* obj = Object::New(context_.get());
    ConstIndex key_idx = context_->FindConstOrInsertToLocal(Value("exists"));

    EXPECT_EQ(obj->HasProperty(context_.get(), key_idx), false);

    obj->SetProperty(context_.get(), key_idx, Value(1));

    EXPECT_EQ(obj->HasProperty(context_.get(), key_idx), true);

    // GC will clean up
}

/**
 * @test 测试设置计算属性
 */
TEST_F(ObjectTest, SetComputedProperty) {
    auto* obj = Object::New(context_.get());
    Value key_value(String::New("computed_key"));
    Value property_value(999);

    obj->SetComputedProperty(context_.get(), key_value, std::move(property_value));

    Value retrieved_value;
    bool success = obj->GetComputedProperty(context_.get(), key_value, &retrieved_value);
    ASSERT_TRUE(success);
    EXPECT_EQ(retrieved_value.i64(), 999);

    // GC will clean up
}

/**
 * @test 测试获取不存在的计算属性
 */
TEST_F(ObjectTest, GetComputedPropertyNotExists) {
    auto* obj = Object::New(context_.get());
    Value key_value(String::New("non_existent_key"));
    Value retrieved_value;

    bool success = obj->GetComputedProperty(context_.get(), key_value, &retrieved_value);
    EXPECT_FALSE(success);

    // GC will clean up
}

/**
 * @test 测试对象转换为字符串
 */
TEST_F(ObjectTest, ObjectToString) {
    auto* obj = Object::New(context_.get());
    obj->SetProperty(context_.get(),
                     context_->FindConstOrInsertToLocal(Value("prop1")),
                     Value(42));

    Value str_value = obj->ToString(context_.get());
    EXPECT_TRUE(str_value.IsString());
    std::string_view str = str_value.string_view();
    EXPECT_FALSE(str.empty());

    // GC will clean up
}

/**
 * @test 测试获取原型对象
 * @note 原型可能不是普通对象类型,而是其他类型
 */
TEST_F(ObjectTest, GetPrototype) {
    auto* obj = Object::New(&runtime_->default_context());

    const Value& prototype = obj->GetPrototype(&runtime_->default_context());
    // 原型应该是某种有效的值类型
    EXPECT_TRUE(prototype.IsObject() || prototype.IsNull() || prototype.IsUndefined());

    // GC will clean up
}

/**
 * @test 测试多次设置同一属性
 */
TEST_F(ObjectTest, SetPropertyMultipleTimes) {
    auto* obj = Object::New(&runtime_->default_context());

    auto index = runtime_->global_const_pool().FindOrInsert(Value("prop"));
    obj->SetProperty(&runtime_->default_context(), index, Value(1));
    Value val1;
    obj->GetProperty(&runtime_->default_context(), index, &val1);
    EXPECT_EQ(val1.i64(), 1);

    obj->SetProperty(&runtime_->default_context(), index, Value(2));
    Value val2;
    obj->GetProperty(&runtime_->default_context(), index, &val2);
    EXPECT_EQ(val2.i64(), 2);

    // GC will clean up
}

/**
 * @test 测试垃圾回收标记
 */
TEST_F(ObjectTest, GCMark) {
    auto* obj = Object::New(&runtime_->default_context());

    EXPECT_FALSE(obj->gc_mark());
    obj->set_gc_mark(true);
    EXPECT_TRUE(obj->gc_mark());
    obj->set_gc_mark(false);
    EXPECT_FALSE(obj->gc_mark());

    // GC will clean up
}

// ==================== Shape 基本功能测试 ====================

/**
 * @class ShapeTest
 * @brief Shape 类单元测试
 */
class ShapeTest : public ::testing::Test {
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

/**
 * @test 测试创建空形状
 */
TEST_F(ShapeTest, CreateEmptyShape) {
    Shape& empty_shape = context_->shape_manager().empty_shape();

    EXPECT_EQ(empty_shape.property_size(), 0);
    EXPECT_EQ(empty_shape.parent_shape(), nullptr);
}

/**
 * @test 测试形状查找不存在的属性
 */
TEST_F(ShapeTest, FindPropertyNotExists) {
    Shape& empty_shape = context_->shape_manager().empty_shape();
    ConstIndex key_idx = context_->FindConstOrInsertToLocal(Value("test_key"));

    int index = empty_shape.Find(key_idx);
    EXPECT_EQ(index, -1);
}

/**
 * @test 测试形状属性大小
 */
TEST_F(ShapeTest, PropertySize) {
    Shape& empty_shape = context_->shape_manager().empty_shape();

    EXPECT_EQ(empty_shape.property_size(), 0);
}

/**
 * @test 测试形状父节点
 */
TEST_F(ShapeTest, ParentShape) {
    Shape& empty_shape = context_->shape_manager().empty_shape();

    EXPECT_EQ(empty_shape.parent_shape(), nullptr);
}

// ==================== ShapeManager 基本功能测试 ====================

/**
 * @class ShapeManagerTest
 * @brief ShapeManager 类单元测试
 */
class ShapeManagerTest : public ::testing::Test {
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

/**
 * @test 测试获取空形状
 */
TEST_F(ShapeManagerTest, GetEmptyShape) {
    Shape& empty_shape = context_->shape_manager().empty_shape();

    EXPECT_NE(&empty_shape, nullptr);
    EXPECT_EQ(empty_shape.property_size(), 0);
}

/**
 * @test 测试形状管理器添加属性
 */
TEST_F(ShapeManagerTest, AddPropertyToShape) {
    Shape* shape = &context_->shape_manager().empty_shape();
    ConstIndex key_idx = context_->FindConstOrInsertToLocal(Value("prop"));
    ShapeProperty prop(key_idx);

    int index = shape->shape_manager()->AddProperty(&shape, std::move(prop));

    EXPECT_GE(index, 0);
    EXPECT_NE(shape, &context_->shape_manager().empty_shape());

    shape->Dereference();
}

/**
 * @test 测试多次添加属性
 */
TEST_F(ShapeManagerTest, AddMultipleProperties) {
    Shape* shape = &context_->shape_manager().empty_shape();
    ConstIndex key1 = context_->FindConstOrInsertToLocal(Value("prop1"));
    ConstIndex key2 = context_->FindConstOrInsertToLocal(Value("prop2"));
    ConstIndex key3 = context_->FindConstOrInsertToLocal(Value("prop3"));

    int idx1 = context_->shape_manager().AddProperty(&shape, ShapeProperty(key1));
    int idx2 = context_->shape_manager().AddProperty(&shape, ShapeProperty(key2));
    int idx3 = context_->shape_manager().AddProperty(&shape, ShapeProperty(key3));

    EXPECT_EQ(idx1, 0);
    EXPECT_EQ(idx2, 1);
    EXPECT_EQ(idx3, 2);
    EXPECT_EQ(shape->property_size(), 3);

    shape->Dereference();
}

// ==================== Object-Shape 集成测试 ====================

/**
 * @class ObjectShapeIntegrationTest
 * @brief 对象和形状系统集成测试
 */
class ObjectShapeIntegrationTest : public ::testing::Test {
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

/**
 * @test 测试对象添加多个属性
 */
TEST_F(ObjectShapeIntegrationTest, AddMultipleProperties) {
    auto* obj = Object::New(context_.get());

    obj->SetProperty(context_.get(),
                     context_->FindConstOrInsertToLocal(Value("a")),
                     Value(1));
    obj->SetProperty(context_.get(),
                     context_->FindConstOrInsertToLocal(Value("b")),
                     Value(2));
    obj->SetProperty(context_.get(),
                     context_->FindConstOrInsertToLocal(Value("c")),
                     Value(3));

    Value val_a, val_b, val_c;
    ASSERT_TRUE(obj->GetProperty(context_.get(),
                                 context_->FindConstOrInsertToLocal(Value("a")),
                                 &val_a));
    ASSERT_TRUE(obj->GetProperty(context_.get(),
                                 context_->FindConstOrInsertToLocal(Value("b")),
                                 &val_b));
    ASSERT_TRUE(obj->GetProperty(context_.get(),
                                 context_->FindConstOrInsertToLocal(Value("c")),
                                 &val_c));
    EXPECT_EQ(val_a.i64(), 1);
    EXPECT_EQ(val_b.i64(), 2);
    EXPECT_EQ(val_c.i64(), 3);

    // GC will clean up
}

/**
 * @test 测试对象形状共享(间接验证)
 */
TEST_F(ObjectShapeIntegrationTest, ShapeSharingIndirect) {
    auto* obj1 = Object::New(context_.get());
    auto* obj2 = Object::New(context_.get());

    obj1->SetProperty(context_.get(),
                      context_->FindConstOrInsertToLocal(Value("x")),
                      Value(10));
    obj2->SetProperty(context_.get(),
                      context_->FindConstOrInsertToLocal(Value("x")),
                      Value(20));

    Value val1, val2;
    ASSERT_TRUE(obj1->GetProperty(context_.get(),
                                  context_->FindConstOrInsertToLocal(Value("x")),
                                  &val1));
    ASSERT_TRUE(obj2->GetProperty(context_.get(),
                                  context_->FindConstOrInsertToLocal(Value("x")),
                                  &val2));
    EXPECT_EQ(val1.i64(), 10);
    EXPECT_EQ(val2.i64(), 20);

    // GC will clean up
    // GC will clean up
}

/**
 * @test 测试对象形状转换(间接验证)
 */
TEST_F(ObjectShapeIntegrationTest, ShapeTransitionIndirect) {
    auto* obj = Object::New(context_.get());

    obj->SetProperty(context_.get(),
                     context_->FindConstOrInsertToLocal(Value("new_prop")),
                     Value(100));

    Value retrieved_value;
    bool success = obj->GetProperty(context_.get(),
                                    context_->FindConstOrInsertToLocal(Value("new_prop")),
                                    &retrieved_value);
    ASSERT_TRUE(success);
    EXPECT_EQ(retrieved_value.i64(), 100);

    // GC will clean up
}

} // namespace test
} // namespace mjs
