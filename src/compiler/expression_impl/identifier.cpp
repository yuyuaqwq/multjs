#include "identifier.h"

#include <mjs/error.h>
#include "../code_generator.h"

namespace mjs {
namespace compiler {

void Identifier::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 尝试查找到对应的变量索引
    const auto* var_info = code_generator->scope_manager().GetVarInfoByExpression(function_def_base, const_cast<Identifier*>(this));
    if (var_info) {
        // 从变量中获取
        function_def_base->bytecode_table().EmitVarLoad(var_info->var_idx);
    }
    else {
        // 尝试从全局对象获取
        auto const_idx = code_generator->AllocateConst(Value(String::New(name_)));
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGetGlobal);
        function_def_base->bytecode_table().EmitI32(const_idx);
    }
}

/**
 * @brief 解析标识符
 *
 * @return 解析后的标识符表达式
 */
std::unique_ptr<Identifier> Identifier::ParseIdentifier(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto token = lexer->NextToken();

	if (!token.is(TokenType::kIdentifier)) {
		throw SyntaxError("Expected identifier, got: '{}'", Token::TypeToString(token.type()));
	}

	return std::make_unique<Identifier>(start, lexer->GetRawSourcePosition(),
							   std::string(token.value()));
}

} // namespace compiler
} // namespace mjs