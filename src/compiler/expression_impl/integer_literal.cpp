#include "src/compiler/expression_impl/integer_literal.h"

#include "src/compiler/code_generator.h"

namespace mjs {
namespace compiler {

void IntegerLiteral::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 整数字面量作为常量加载
    auto const_idx = code_generator->AllocateConst(Value(value_));
    function_def_base->bytecode_table().EmitConstLoad(const_idx);
}

} // namespace compiler
} // namespace mjs