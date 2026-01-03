#include "src/compiler/expression_impl/import_expression.h"

#include <mjs/function_def.h>
#include "src/compiler/lexer.h"
#include "src/compiler/parser.h"

namespace mjs {
namespace compiler {

void ImportExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // import 表达式代码生成
    auto& import_exp = const_cast<ImportExpression&>(*this);
    import_exp.source()->GenerateCode(code_generator, function_def_base);
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGetModuleAsync);
}

/**
 * @brief 解析import表达式
 *
 * import表达式的形式为：import(module_specifier)
 *
 * @return 解析后的import表达式
 */
std::unique_ptr<ImportExpression> ImportExpression::ParseImportExpression(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();

	lexer->MatchToken(TokenType::kKwImport);

	// 解析模块说明符
	lexer->MatchToken(TokenType::kSepLParen);
	auto source = ParseExpression(lexer);
	lexer->MatchToken(TokenType::kSepRParen);

	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<ImportExpression>(start, end, std::move(source));
}

} // namespace compiler
} // namespace mjs