#include "undefined_literal.h"

#include "../code_generator.h"

namespace mjs {
namespace compiler {

void UndefinedLiteral::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // undefined 字面量生成 undefined 指令
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
}

} // namespace compiler
} // namespace mjs