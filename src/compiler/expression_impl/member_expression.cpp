#include "member_expression.h"

#include <mjs/error.h>

#include "../statement.h"
#include "../code_generator.h"
#include "identifier.h"

namespace mjs {
namespace compiler {

void MemberExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 成员访问表达式代码生成
    auto& mem_exp = const_cast<MemberExpression&>(*this);

    // 被访问的表达式，入栈这个表达式
    mem_exp.object()->GenerateCode(code_generator, function_def_base);
    // 判断下是否调用函数，是则dump
    if (mem_exp.is_method_call()) {
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kDump);
    }

    if (mem_exp.computed()) {
        // 用于访问的下标的表达式，入栈这个表达式
        mem_exp.property()->GenerateCode(code_generator, function_def_base);

        // 生成索引访问的指令
        function_def_base->bytecode_table().EmitIndexedLoad();
    }
    else {
        // 成员访问表达式
        auto& prop_exp = mem_exp.property()->as<Identifier>();

        // 访问对象成员
        auto const_idx = code_generator->AllocateConst(Value(String::New(prop_exp.name())));
        function_def_base->bytecode_table().EmitPropertyLoad(const_idx);
    }
}

/**
 * @brief 解析成员表达式
 *
 * 成员表达式包括：
 * - 点访问：object.property
 * - 计算属性：object[property]
 *
 * @param object 对象表达式
 * @return 解析后的成员表达式
 */
std::unique_ptr<MemberExpression> MemberExpression::ParseMemberExpression(Lexer* lexer, std::unique_ptr<Expression> object) {
	auto start = lexer->GetSourcePosition();

	std::unique_ptr<Expression> member;

	bool computed = false;
	bool optional = false;
	auto token = lexer->NextToken();
	if (token.is(TokenType::kSepDot)) {
		member = Identifier::ParseIdentifier(lexer);
	}
	else if (token.is(TokenType::kOpOptionalChain)) {
		member = Identifier::ParseIdentifier(lexer);
		optional = true;
	}
	else if (token.is(TokenType::kSepLBrack)) {
		member = ParseExpression(lexer);
		lexer->MatchToken(TokenType::kSepRBrack);
		computed = true;
	}
	else {
		throw SyntaxError("Incorrect member expression.");
	}

	bool is_method_call = false;
	if (lexer->PeekToken().type() == TokenType::kSepLParen) {
		is_method_call = true;
	}

	auto end = lexer->GetRawSourcePosition();

	return std::make_unique<MemberExpression>(start, end,
		std::move(object), std::move(member), is_method_call, computed, optional);
}

} // namespace compiler
} // namespace mjs