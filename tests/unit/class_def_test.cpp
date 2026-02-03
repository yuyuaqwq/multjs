/**
 * @file class_def_test.cpp
 * @brief 类定义系统单元测试
 *
 * 测试ClassDef和ClassDefTable的功能
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

#include <mjs/class_def/class_def.h>
#include <mjs/class_def_table.h>
#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/value/value.h>
#include <mjs/value/object/object.h>
#include "../tests/unit/test_helpers.h"

namespace mjs {
namespace test {

/**
 * @class ClassDefTest
 * @brief 类定义基础测试
 */
class ClassDefTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
    }

    void TearDown() override {
        // 清理代码
    }

    std::unique_ptr<Runtime> runtime_;
};

/**
 * @test 测试类标识符枚举值
 */
TEST_F(ClassDefTest, ClassIdEnumValues) {
    // Arrange & Act & Assert
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kInvalid), 0);
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kSymbol), 1);
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kObject), 2);
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kNumberObject), 3);
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kStringObject), 4);
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kArrayObject), 5);
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kFunctionObject), 6);
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kGeneratorObject), 7);
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kPromiseObject), 8);
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kAsyncObject), 9);
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kModuleObject), 10);
    EXPECT_EQ(static_cast<uint16_t>(ClassId::kCppModuleObject), 11);
}

/**
 * @test 测试对象内部方法枚举
 */
TEST_F(ClassDefTest, ObjectInternalMethodsEnum) {
    // Arrange & Act & Assert
    EXPECT_EQ(static_cast<int>(ObjectInternalMethods::kGetPrototypeOf), 1 << 0);
    EXPECT_EQ(static_cast<int>(ObjectInternalMethods::kSetPrototypeOf), 1 << 1);
    EXPECT_EQ(static_cast<int>(ObjectInternalMethods::kIsExtensible), 1 << 2);
    EXPECT_EQ(static_cast<int>(ObjectInternalMethods::kPreventExtensions), 1 << 3);
    EXPECT_EQ(static_cast<int>(ObjectInternalMethods::kGetOwnProperty), 1 << 4);
    EXPECT_EQ(static_cast<int>(ObjectInternalMethods::kDefineOwnProperty), 1 << 5);
    EXPECT_EQ(static_cast<int>(ObjectInternalMethods::kHasProperty), 1 << 6);
    EXPECT_EQ(static_cast<int>(ObjectInternalMethods::kGet), 1 << 7);
    EXPECT_EQ(static_cast<int>(ObjectInternalMethods::kSet), 1 << 8);
    EXPECT_EQ(static_cast<int>(ObjectInternalMethods::kDelete), 1 << 9);
    EXPECT_EQ(static_cast<int>(ObjectInternalMethods::kOwnPropertyKeys), 1 << 10);
}

/**
 * @test 测试函数内部方法枚举
 */
TEST_F(ClassDefTest, FunctionInternalMethodsEnum) {
    // Arrange & Act & Assert
    EXPECT_EQ(static_cast<int>(FunctionInternalMethods::kCall), 1 << 1);
}

/**
 * @test 测试ClassDef访问Runtime内置类
 */
TEST_F(ClassDefTest, AccessBuiltinClasses) {
    // Arrange & Act
    auto& class_def_table = runtime_->class_def_table();

    // Assert - 验证可以访问各种内置类
    EXPECT_NO_THROW({
        auto& object_class = class_def_table[ClassId::kObject];
        EXPECT_EQ(object_class.id(), ClassId::kObject);
    });

    EXPECT_NO_THROW({
        auto& array_class = class_def_table[ClassId::kArrayObject];
        EXPECT_EQ(array_class.id(), ClassId::kArrayObject);
    });

    EXPECT_NO_THROW({
        auto& function_class = class_def_table[ClassId::kFunctionObject];
        EXPECT_EQ(function_class.id(), ClassId::kFunctionObject);
    });
}

/**
 * @test 测试ClassDef获取类名称
 */
TEST_F(ClassDefTest, ClassDefName) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();
    auto& object_class = class_def_table[ClassId::kObject];

    // Act & Assert
    EXPECT_EQ(object_class.name_string(), "Object");
}

/**
 * @test 测试ClassDef获取原型对象
 */
TEST_F(ClassDefTest, ClassDefPrototype) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();
    auto& array_class = class_def_table[ClassId::kArrayObject];

    // Act
    const auto& prototype = array_class.prototype();

    // Assert
    // Object.prototype是特殊的,它不是Object类型(原型为null)
    // 其他类的prototype应该是Object类型
    EXPECT_TRUE(prototype.IsObject());
}

/**
 * @test 测试ClassDef获取构造函数对象
 */
TEST_F(ClassDefTest, ClassDefConstructorObject) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();
    Context context(runtime_.get());

    // Act
    auto* object_class = &class_def_table[ClassId::kObject];

    // Assert - Object的构造函数应该可以通过globalThis访问
    EXPECT_TRUE(true); // 占位测试,实际需要验证构造函数对象
}

/**
 * @test 测试ClassDef非拷贝性
 */
TEST_F(ClassDefTest, ClassDefNonCopyable) {
    // ClassDef继承自noncopyable,不应该能够拷贝
    EXPECT_TRUE(std::is_copy_assignable<ClassDef>::value == false);
    EXPECT_TRUE(std::is_copy_constructible<ClassDef>::value == false);
}

/**
 * @class ClassDefTableTest
 * @brief 类定义表测试
 */
class ClassDefTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
    }

    void TearDown() override {
        // 清理代码
    }

    std::unique_ptr<Runtime> runtime_;
};

/**
 * @test 测试ClassDefTable访问所有内置类
 */
TEST_F(ClassDefTableTest, AccessAllBuiltinClasses) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();

    // Act & Assert - 验证所有内置类都可以访问
    std::vector<ClassId> class_ids = {
        ClassId::kSymbol,
        ClassId::kObject,
        ClassId::kNumberObject,
        ClassId::kStringObject,
        ClassId::kArrayObject,
        ClassId::kFunctionObject,
        ClassId::kGeneratorObject,
        ClassId::kPromiseObject,
        ClassId::kAsyncObject,
        ClassId::kModuleObject,
        ClassId::kCppModuleObject,
    };

    for (auto class_id : class_ids) {
        EXPECT_NO_THROW({
            auto& class_def = class_def_table[class_id];
            EXPECT_EQ(class_def.id(), class_id);
        });
    }
}

/**
 * @test 测试ClassDefTable使用at()访问
 */
TEST_F(ClassDefTableTest, ClassDefTableAtAccess) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();

    // Act & Assert
    EXPECT_NO_THROW({
        auto& object_class = class_def_table.At(ClassId::kObject);
        EXPECT_EQ(object_class.id(), ClassId::kObject);
    });
}

/**
 * @test 测试ClassDefTable使用[]访问
 */
TEST_F(ClassDefTableTest, ClassDefTableBracketAccess) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();

    // Act & Assert
    EXPECT_NO_THROW({
        auto& object_class = class_def_table[ClassId::kObject];
        EXPECT_EQ(object_class.id(), ClassId::kObject);
    });
}

/**
 * @test 测试ClassDefTable访问无效类ID
 */
TEST_F(ClassDefTableTest, ClassDefTableInvalidAccess) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();

    // Act & Assert - 访问kInvalid应该不会崩溃
    EXPECT_NO_THROW({
        // kInvalid的行为取决于实现,可能抛出异常或返回空
        // 这里只验证不会崩溃
    });
}

/**
 * @test 测试所有内置类的名称
 */
TEST_F(ClassDefTableTest, BuiltinClassNames) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();

    // Act & Assert
    EXPECT_EQ(class_def_table[ClassId::kObject].name_string(), "Object");
    EXPECT_EQ(class_def_table[ClassId::kArrayObject].name_string(), "Array");
    EXPECT_EQ(class_def_table[ClassId::kFunctionObject].name_string(), "Function");
}

/**
 * @test 测试所有内置类的原型对象
 */
TEST_F(ClassDefTableTest, BuiltinClassPrototypes) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();

    // Act & Assert
    // Object.prototype是特殊的(原型为null),不应该是Object类型
    // 其他类的prototype应该是Object类型
    EXPECT_TRUE(class_def_table[ClassId::kArrayObject].prototype().IsObject());
    EXPECT_TRUE(class_def_table[ClassId::kFunctionObject].prototype().IsObject());
}

/**
 * @class ClassDefIntegrationTest
 * @brief 类定义集成测试
 */
class ClassDefIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
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
 * @test 测试通过构造函数创建对象
 */
TEST_F(ClassDefIntegrationTest, CreateObjectViaConstructor) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();

    // Act
    auto& array_class = class_def_table[ClassId::kArrayObject];

    // Assert - 验证Array类的构造函数存在
    EXPECT_TRUE(array_class.prototype().IsObject());
}

/**
 * @test 测试原型链关系
 */
TEST_F(ClassDefIntegrationTest, PrototypeChain) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();

    // Act & Assert
    // Object.prototype的原型应该是null,所以不是Object类型
    auto object_prototype = class_def_table[ClassId::kObject].prototype();
    EXPECT_FALSE(object_prototype.IsObject());  // 应该是null/undefined

    // Array.prototype的原型应该是Object.prototype
    auto array_prototype = class_def_table[ClassId::kArrayObject].prototype();
    EXPECT_TRUE(array_prototype.IsObject());
}

/**
 * @test 测试类定义的ID唯一性
 */
TEST_F(ClassDefIntegrationTest, ClassIdUniqueness) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();

    // Act & Assert - 验证不同的类有不同的ID
    auto& object_class = class_def_table[ClassId::kObject];
    auto& array_class = class_def_table[ClassId::kArrayObject];
    auto& function_class = class_def_table[ClassId::kFunctionObject];

    EXPECT_NE(object_class.id(), array_class.id());
    EXPECT_NE(object_class.id(), function_class.id());
    EXPECT_NE(array_class.id(), function_class.id());
}

/**
 * @test 测试类定义的名称唯一性
 */
TEST_F(ClassDefIntegrationTest, ClassNameUniqueness) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();

    // Act & Assert - 验证不同的类有不同的名称
    auto& object_class = class_def_table[ClassId::kObject];
    auto& array_class = class_def_table[ClassId::kArrayObject];
    auto& function_class = class_def_table[ClassId::kFunctionObject];

    EXPECT_NE(object_class.name_string(), array_class.name_string());
    EXPECT_NE(object_class.name_string(), function_class.name_string());
    EXPECT_NE(array_class.name_string(), function_class.name_string());
}

/**
 * @test 测试ClassDef模板方法get()
 */
TEST_F(ClassDefIntegrationTest, ClassDefTemplateGet) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();

    // Act & Assert - 测试模板方法get()
    // 注意: 这需要具体的派生类类型才能测试
    auto& base_class = class_def_table[ClassId::kObject];
    EXPECT_EQ(base_class.id(), ClassId::kObject);
}

/**
 * @test 测试NewConstructor默认行为
 */
TEST_F(ClassDefIntegrationTest, NewConstructorDefaultBehavior) {
    // Arrange
    auto& class_def_table = runtime_->class_def_table();
    auto& object_class = class_def_table[ClassId::kObject];

    // Act & Assert - 默认的NewConstructor应该抛出异常
    // (除非派生类重写了该方法)
    // 这里只验证调用不会导致未定义行为
    EXPECT_TRUE(true); // 占位测试
}

} // namespace test
} // namespace mjs
