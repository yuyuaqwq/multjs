#include "boolean_literal.h"

#include "../code_generator.h"
#include "../statement.h"

namespace mjs {
namespace compiler {

void BooleanLiteral::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 布尔字面量作为常量加载
    auto const_idx = code_generator->AllocateConst(Value(value_));
    function_def_base->bytecode_table().EmitConstLoad(const_idx);
}

} // namespace compiler
} // namespace mjs