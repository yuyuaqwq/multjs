#include "src/compiler/expression_impl/template_literal.h"

#include <mjs/error.h>

#include "src/compiler/statement.h"
#include "src/compiler/code_generator.h"
#include "src/compiler/expression_impl/string_literal.h"

namespace mjs {
namespace compiler {

void TemplateLiteral::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 生成字符串拼接
    if (expressions_.empty()) {
        auto const_idx = code_generator->AllocateConst(Value(""));
        function_def_base->bytecode_table().EmitConstLoad(const_idx);
    }
    size_t i = 0;
    for (auto& exp : expressions_) {
        exp->GenerateCode(code_generator, function_def_base);
        ++i;
        if (i == 1) {
            // 确保有一个字符串
            function_def_base->bytecode_table().EmitOpcode(OpcodeType::kToString);
            continue;
        }
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kAdd);
    }
}

/**
 * @brief 解析模板字符串
 *
 * 模板字符串的形式为：`template ${expr1} string ${expr2}`
 *
 * @return 解析后的模板字符串表达式
 */
std::unique_ptr<TemplateLiteral> TemplateLiteral::ParseTemplateLiteral(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kBacktick);

	std::vector<std::unique_ptr<Expression>> expressions;

	// 解析模板字符串内容
	while (!lexer->PeekToken().is(TokenType::kBacktick)) {
		if (lexer->PeekToken().is(TokenType::kTemplateElement)) {
			// 普通文本部分
			auto text = lexer->NextToken().value();
			auto text_start = lexer->GetSourcePosition() - text.length();
			auto text_end = lexer->GetRawSourcePosition();

			expressions.push_back(
				std::make_unique<StringLiteral>(text_start, text_end, std::move(text))
			);
		} else if (lexer->PeekToken().is(TokenType::kTemplateInterpolationStart)) {
			// 插值表达式部分 ${expr}
			lexer->NextToken(); // 消耗 ${

			auto expr = ParseExpression(lexer);
			expressions.push_back(std::move(expr));

			lexer->MatchToken(TokenType::kTemplateInterpolationEnd); // 匹配 }
		} else {
			throw SyntaxError("Invalid template literal");
		}
	}

	lexer->MatchToken(TokenType::kBacktick); // 匹配结束的 `
	auto end = lexer->GetRawSourcePosition();

	return std::make_unique<TemplateLiteral>(start, end, std::move(expressions));
}

} // namespace compiler
} // namespace mjs