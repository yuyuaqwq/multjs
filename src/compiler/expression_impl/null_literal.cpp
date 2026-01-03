#include "src/compiler/expression_impl/null_literal.h"

#include "src/compiler/code_generator.h"

namespace mjs {
namespace compiler {

void NullLiteral::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // null 字面量作为常量加载
    auto const_idx = code_generator->AllocateConst(Value(nullptr));
    function_def_base->bytecode_table().EmitConstLoad(const_idx);
}

} // namespace compiler
} // namespace mjs