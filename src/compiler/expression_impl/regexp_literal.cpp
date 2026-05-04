/**
 * @file regexp_literal.cpp
 * @brief 正则表达式字面量实现
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include "regexp_literal.h"

#include "src/compiler/code_generator.h"
#include <mjs/value/function_def.h>
#include <mjs/value/string.h>

namespace mjs {
namespace compiler {

void RegExpLiteral::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // TODO: 需要创建RegExp对象，但目前CodeGenerator没有公开context访问方法
    // 暂时使用字符串表示正则表达式
    auto str = "/" + pattern_ + "/" + flags_;
    auto const_idx = code_generator->AllocateConst(Value(String::New(str)));
    function_def_base->bytecode_table().EmitConstLoad(const_idx);
}

} // namespace compiler
} // namespace mjs
