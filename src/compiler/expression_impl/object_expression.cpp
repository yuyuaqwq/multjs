#include "src/compiler/expression_impl/object_expression.h"

#include <mjs/class_def/object_class_def.h>
#include <mjs/error.h>

#include "src/compiler/statement.h"
#include "src/compiler/code_generator.h"

#include "src/compiler/expression_impl/yield_expression.h"
#include "src/compiler/expression_impl/identifier.h"
#include "src/compiler/expression_impl/function_expression.h"
#include "src/compiler/statement_impl/block_statement.h"

namespace mjs {
namespace compiler {

void ObjectExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 检查是否有 getter/setter
    bool has_getter_setter = false;
    for (auto& prop : properties()) {
        if (prop.kind == PropertyKind::kGetter || prop.kind == PropertyKind::kSetter) {
            has_getter_setter = true;
            break;
        }
    }

    if (!has_getter_setter) {
        // 没有 getter/setter，使用快速路径
        for (auto& prop : properties()) {
            auto key_const_index = code_generator->AllocateConst(Value(String::New(prop.key)));
            function_def_base->bytecode_table().EmitConstLoad(key_const_index);
            prop.value->GenerateCode(code_generator, function_def_base);
        }
        auto const_idx = code_generator->AllocateConst(Value(static_cast<uint32_t>(properties().size() * 2)));
        function_def_base->bytecode_table().EmitConstLoad(const_idx);

        auto literal_new = code_generator->AllocateConst(Value(ObjectClassDef::LiteralNew));
        function_def_base->bytecode_table().EmitConstLoad(literal_new);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kFunctionCall);
    } else {
        // 有 getter/setter，使用 Object.defineProperty 的内部形式
        // 先创建空对象
        auto const_idx_obj = code_generator->AllocateConst(Value(0u));
        function_def_base->bytecode_table().EmitConstLoad(const_idx_obj);

        auto literal_new = code_generator->AllocateConst(Value(ObjectClassDef::LiteralNew));
        function_def_base->bytecode_table().EmitConstLoad(literal_new);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kFunctionCall);

        // 对每个属性使用 Object.defineProperty
        for (auto& prop : properties()) {
            if (prop.kind == PropertyKind::kNormal) {
                // 普通属性：直接设置
                auto key_const_index = code_generator->AllocateConst(Value(String::New(prop.key)));
                function_def_base->bytecode_table().EmitConstLoad(key_const_index);
                prop.value->GenerateCode(code_generator, function_def_base);

                auto param_count = code_generator->AllocateConst(Value(3u));
                function_def_base->bytecode_table().EmitConstLoad(param_count);
				auto set_property = code_generator->AllocateConst(Value(ObjectClassDef::SetProperty));
				function_def_base->bytecode_table().EmitConstLoad(set_property);
				function_def_base->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
                function_def_base->bytecode_table().EmitOpcode(OpcodeType::kFunctionCall);
            } else {
                // getter/setter：使用 Object.defineProperty(obj, key, accessor, kind)



                auto key_const_index = code_generator->AllocateConst(Value(String::New(prop.key)));
                function_def_base->bytecode_table().EmitConstLoad(key_const_index);
                prop.value->GenerateCode(code_generator, function_def_base);

                // kind: 1 for getter, 2 for setter
                int kind = (prop.kind == PropertyKind::kGetter) ? 1 : 2;
                auto kind_const_index = code_generator->AllocateConst(Value(static_cast<int64_t>(kind)));
                function_def_base->bytecode_table().EmitConstLoad(kind_const_index);

				auto param_count = code_generator->AllocateConst(Value(4u));
				function_def_base->bytecode_table().EmitConstLoad(param_count);
                auto define_property = code_generator->AllocateConst(Value(ObjectClassDef::DefineProperty));
                function_def_base->bytecode_table().EmitConstLoad(define_property);
                function_def_base->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
                function_def_base->bytecode_table().EmitOpcode(OpcodeType::kFunctionCall);
            }
        }
    }
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
 * - getter: get propName() { ... }
 * - setter: set propName(value) { ... }
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
		PropertyKind kind = PropertyKind::kNormal;

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
		} else if (token.is(TokenType::kKwGet)) {
			// getter: get propName() { ... }
			kind = PropertyKind::kGetter;
			lexer->NextToken(); // 消耗 get

			// 解析属性名
			auto name_token = lexer->PeekToken();
			if (name_token.is(TokenType::kIdentifier)) {
				key = name_token.value();
				lexer->NextToken();
			} else if (name_token.is(TokenType::kSepLBrack)) {
				// 计算属性名暂不支持
				throw SyntaxError("Computed property names for getters are not yet supported: 'get [expr]()'");
			} else {
				throw SyntaxError("Expected property name after 'get'");
			}

			// 检查是否有左括号
			if (!lexer->PeekToken().is(TokenType::kSepLParen)) {
				throw SyntaxError("Expected '(' after getter name");
			}

			// 解析参数列表
			auto method_start = lexer->GetSourcePosition();
			auto params_res = Expression::TryParseParameters(lexer);
			if (!params_res) {
				throw SyntaxError("Expected parameter list for getter");
			}
			auto params = *params_res;

			// 解析方法体
			auto body = BlockStatement::ParseBlockStatement(lexer);
			auto method_end = lexer->GetRawSourcePosition();

			// 创建函数表达式
			std::string method_id;
			value = std::make_unique<FunctionExpression>(
				method_start, method_end,
				std::move(method_id), std::move(params),
				std::move(body),
				false, false, false
			);
		} else if (token.is(TokenType::kKwSet)) {
			// setter: set propName(value) { ... }
			kind = PropertyKind::kSetter;
			lexer->NextToken(); // 消耗 set

			// 解析属性名
			auto name_token = lexer->PeekToken();
			if (name_token.is(TokenType::kIdentifier)) {
				key = name_token.value();
				lexer->NextToken();
			} else if (name_token.is(TokenType::kSepLBrack)) {
				// 计算属性名暂不支持
				throw SyntaxError("Computed property names for setters are not yet supported: 'set [expr](value)'");
			} else {
				throw SyntaxError("Expected property name after 'set'");
			}

			// 检查是否有左括号
			if (!lexer->PeekToken().is(TokenType::kSepLParen)) {
				throw SyntaxError("Expected '(' after setter name");
			}

			// 解析参数列表
			auto method_start = lexer->GetSourcePosition();
			auto params_res = Expression::TryParseParameters(lexer);
			if (!params_res) {
				throw SyntaxError("Expected parameter list for setter");
			}
			auto params = *params_res;

			// 解析方法体
			auto body = BlockStatement::ParseBlockStatement(lexer);
			auto method_end = lexer->GetRawSourcePosition();

			// 创建函数表达式
			std::string method_id;
			value = std::make_unique<FunctionExpression>(
				method_start, method_end,
				std::move(method_id), std::move(params),
				std::move(body),
				false, false, false
			);
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
		properties.push_back({key, std::move(value), shorthand, computed, kind});

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