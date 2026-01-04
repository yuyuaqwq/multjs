#include "src/compiler/expression_impl/object_expression.h"

#include <mjs/class_def_impl/object_class_def.h>
#include <mjs/error.h>

#include "src/compiler/statement.h"
#include "src/compiler/code_generator.h"

#include "src/compiler/expression_impl/yield_expression.h"
#include "src/compiler/expression_impl/identifier.h"

namespace mjs {
namespace compiler {

void ObjectExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 对象表达式代码生成
	for (auto& prop : properties()) {
		// 将key和value入栈
		auto key_const_index = code_generator->AllocateConst(Value(String::New(prop.key)));
		function_def_base->bytecode_table().EmitConstLoad(key_const_index);
		prop.value->GenerateCode(code_generator, function_def_base);
	}
	auto const_idx = code_generator->AllocateConst(Value(properties().size() * 2));
	function_def_base->bytecode_table().EmitConstLoad(const_idx);

	auto literal_new = code_generator->AllocateConst(Value(ObjectClassDef::LiteralNew));
	function_def_base->bytecode_table().EmitConstLoad(literal_new);
	function_def_base->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
	function_def_base->bytecode_table().EmitOpcode(OpcodeType::kFunctionCall);
}

/**
 * @brief 解析对象表达式
 *
 * 对象表达式的形式为：{key1: value1, key2: value2, ...}
 * 支持以下几种属性形式：
 * - 普通属性：key: value
 * - 简写属性：key（等同于key: key）
 * - 计算属性：[expr]: value
 * - 字符串键属性："key": value
 *
 * @return 解析后的对象表达式
 */
std::unique_ptr<ObjectExpression> ObjectExpression::ParseObjectExpression(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kSepLCurly);

	std::vector<ObjectExpression::Property> properties;

	// 解析对象属性
	while (!lexer->PeekToken().is(TokenType::kSepRCurly)) {
		bool computed = false;
		std::string key;
		std::unique_ptr<Expression> value;
		bool shorthand = false;

		// 解析属性键
		auto token = lexer->PeekToken();
		if (token.is(TokenType::kSepLBrack)) {
			// 计算属性名: [expr]
			computed = true;
			lexer->NextToken(); // 消耗 [
			auto key_expr = Expression::ParseExpression(lexer);
			lexer->MatchToken(TokenType::kSepRBrack);

			// 必须有冒号
			lexer->MatchToken(TokenType::kSepColon);

			// 解析属性值
			value = YieldExpression::ParseExpressionAtYieldLevel(lexer);
		} else if (token.is(TokenType::kIdentifier)) {
			// 标识符作为属性名
			key = lexer->NextToken().value();

			if (lexer->PeekToken().is(TokenType::kSepColon)) {
				// 普通属性: key: value
				lexer->NextToken(); // 消耗 :
				value = YieldExpression::ParseExpressionAtYieldLevel(lexer);
			} else {
				// 简写属性: key (等同于 key: key)
				shorthand = true;
				auto id_start = lexer->GetSourcePosition() - key.length();
				auto id_end = lexer->GetRawSourcePosition();
				value = std::make_unique<Identifier>(id_start, id_end, std::string(key));
			}
		} else if (token.is(TokenType::kString)) {
			// 字符串作为属性名: "key": value
			key = lexer->NextToken().value();

			// 必须有冒号
			lexer->MatchToken(TokenType::kSepColon);

			// 解析属性值
			value = YieldExpression::ParseExpressionAtYieldLevel(lexer);
		} else {
			throw SyntaxError("Invalid property name: {}", token.TypeToString(token.type()));
		}

		// 添加属性
		properties.push_back({key, std::move(value), shorthand, computed});

		// 如果有逗号，继续解析下一个属性
		if (lexer->PeekToken().is(TokenType::kSepComma)) {
			lexer->NextToken(); // 消耗 ,
		} else {
			break;
		}
	}

	lexer->MatchToken(TokenType::kSepRCurly);
	auto end = lexer->GetRawSourcePosition();

	return std::make_unique<ObjectExpression>(start, end, std::move(properties));
}

} // namespace compiler
} // namespace mjs