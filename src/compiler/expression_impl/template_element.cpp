#include "template_element.h"

#include "../code_generator.h"

namespace mjs {
namespace compiler {

void TemplateElement::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 模板元素作为常量加载
    auto const_idx = code_generator->AllocateConst(Value(String::New(value_)));
    function_def_base->bytecode_table().EmitConstLoad(const_idx);
}

} // namespace compiler
} // namespace mjs