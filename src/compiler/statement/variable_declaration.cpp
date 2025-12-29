#include "variable_declaration.h"

#include "../code_generator.h"
#include "../expression/yield_expression.h"

namespace mjs {
namespace compiler {

void VariableDeclaration::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 确定变量标志
    VarFlags flags = VarFlags::kNone;
    if (kind_ == TokenType::kKwConst) {
        flags = VarFlags::kConst;
    }

    // 分配变量
    auto& var_info = code_generator->AllocateVar(name_, flags);

    // 如果有初始值，生成初始值代码
    if (init_) {
        code_generator->GenerateExpression(function_def_base, init_.get());
        function_def_base->bytecode_table().EmitVarStore(var_info.var_idx);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPop);
    }

    // 如果是导出变量，添加到模块导出
    if (is_export_) {
        static_cast<ModuleDef*>(function_def_base)->export_var_def_table().AddExportVar(name_, var_info.var_idx);
    }
}

std::unique_ptr<VariableDeclaration> VariableDeclaration::ParseVariableDeclaration(Lexer* lexer, TokenType kind) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(kind);
	auto name = lexer->MatchToken(TokenType::kIdentifier).value();

	// 尝试解析类型注解
	if (lexer->PeekToken().is(TokenType::kSepColon)) {
		lexer->MatchToken(TokenType::kSepColon);
		// 跳过类型注解
		lexer->MatchToken(TokenType::kIdentifier);
	}

	lexer->MatchToken(TokenType::kOpAssign);
	auto init = YieldExpression::ParseExpressionAtYieldLevel(lexer);
	lexer->MatchToken(TokenType::kSepSemi);
	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<VariableDeclaration>(start, end, std::move(name), std::move(init), kind);
}

} // namespace compiler
} // namespace mjs