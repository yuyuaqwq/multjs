/**
 * @file class_declaration.cpp
 * @brief 类声明实现
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include "src/compiler/statement_impl/class_declaration.h"

#include <mjs/error.h>
#include <mjs/value.h>
#include <mjs/function_def.h>
#include <mjs/string.h>
#include <mjs/opcode.h>

#include "src/compiler/code_generator.h"
#include "src/compiler/lexer.h"
#include "src/compiler/expression.h"
#include "src/compiler/expression_impl/identifier.h"
#include "src/compiler/expression_impl/class_expression.h"
#include "src/compiler/expression_impl/function_expression.h"
#include "src/compiler/statement_impl/block_statement.h"

namespace mjs {
namespace compiler {

void ClassDeclaration::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    expression_->GenerateCode(code_generator, function_def_base);

    // 弹出栈顶的类引用(因为声明语句不产生值)
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPop);
}

std::unique_ptr<ClassDeclaration> ClassDeclaration::ParseClassDeclaration(Lexer* lexer) {
    auto start = lexer->GetSourcePosition();
    
    auto expression = ClassExpression::ParseClassExpression(lexer, true);

    auto end = lexer->GetRawSourcePosition();
    return std::make_unique<ClassDeclaration>(start, end, std::move(expression));
}

} // namespace compiler
} // namespace mjs
