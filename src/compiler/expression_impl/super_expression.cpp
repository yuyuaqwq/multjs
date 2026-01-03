#include "src/compiler/expression_impl/super_expression.h"

#include "src/compiler/code_generator.h"

namespace mjs {
namespace compiler {

void SuperExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // super 表达式生成 super 指令
    // super 的行为取决于上下文：
    // 1. 在构造函数中作为函数调用: super(...) -> 调用父类构造函数
    // 2. 在成员表达式中: super.prop 或 super.method() -> 访问父类原型

    // 标记函数使用了 super
    function_def_base->set_has_this(true);

    // 生成获取 super 的指令
    // super 本质上是当前函数的原型的原型 (当前类的父类原型)
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGetSuper);
}

} // namespace compiler
} // namespace mjs