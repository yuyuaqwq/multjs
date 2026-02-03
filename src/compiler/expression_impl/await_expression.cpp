#include "src/compiler/expression_impl/await_expression.h"

#include <mjs/value/function_def.h>

#include "src/compiler/statement.h"

namespace mjs {
namespace compiler {

void AwaitExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // await 表达式代码生成
    auto& await_exp = const_cast<AwaitExpression&>(*this);
    await_exp.argument()->GenerateCode(code_generator, function_def_base);
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kAwait);
}

} // namespace compiler
} // namespace mjs