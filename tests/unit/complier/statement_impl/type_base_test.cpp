/**
 * @file type_base_test.cpp
 * @brief 类型基类单元测试
 *
 * 测试TypeBase等类型基类组件
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "src/compiler/statement_impl/type_base.h"
#include "src/compiler/statement_impl/predefined_type.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class TypeBaseTest
 * @brief 类型基类测试
 */
class TypeBaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        start_ = 0;
        end_ = 0;
    }

    SourceBytePosition start_;
    SourceBytePosition end_;
};

} // namespace test
} // namespace compiler
} // namespace mjs
