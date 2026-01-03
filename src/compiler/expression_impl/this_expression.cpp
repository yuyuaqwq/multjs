#include "src/compiler/expression_impl/this_expression.h"

#include "src/compiler/code_generator.h"

namespace mjs {
namespace compiler {

void ThisExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // this 表达式生成 this 指令
    function_def_base->set_has_this(true);
    if (code_generator->scope_manager().IsInTypeScope({ ScopeType::kFunction }, { ScopeType::kArrowFunction })) {
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGetThis);
    }
    else {
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGetOuterThis);
    }
}

} // namespace compiler
} // namespace mjs