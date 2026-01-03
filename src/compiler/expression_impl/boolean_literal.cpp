#include "src/compiler/expression_impl/boolean_literal.h"

#include "src/compiler/code_generator.h"
#include "src/compiler/statement.h"

namespace mjs {
namespace compiler {

void BooleanLiteral::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 布尔字面量作为常量加载
    auto const_idx = code_generator->AllocateConst(Value(value_));
    function_def_base->bytecode_table().EmitConstLoad(const_idx);
}

} // namespace compiler
} // namespace mjs