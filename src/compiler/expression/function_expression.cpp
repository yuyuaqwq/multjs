#include "function_expression.h"

#include <mjs/error.h>

#include "../code_generator.h"
#include "../statement.h"
#include "../statement/block_statement.h"
#include "arrow_function_expression.h"
#include "assignment_expression.h"

namespace mjs {
namespace compiler {

FunctionExpression::FunctionExpression(SourcePosition start, SourcePosition end,
	std::string id, std::vector<std::string>&& params,
	std::unique_ptr<BlockStatement> body,
	bool is_generator, bool is_async, bool is_module)
	: Expression(start, end), id_(std::move(id)),
	params_(std::move(params)), body_(std::move(body)),
	is_export_(0), is_generator_(is_generator),
	is_async_(is_async), is_module_(is_module) {
}

void FunctionExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 函数表达式代码生成
	// 创建函数定义
	auto new_func_def = FunctionDef::New(&function_def_base->module_def(), id(), params().size());
	// 将函数定义添加到常量池
	auto const_idx = code_generator->AllocateConst(Value(new_func_def));

	// 设置函数属性
	new_func_def->set_is_normal();
	if (is_generator()) {
		new_func_def->set_is_generator();
	}
	else if (is_async()) {
		new_func_def->set_is_async();
	}

	auto load_pc = function_def_base->bytecode_table().Size();
	// 可能需要修复，统一用U32了
	function_def_base->bytecode_table().EmitOpcode(OpcodeType::kCLoadD);
	function_def_base->bytecode_table().EmitU32(const_idx);

	if (!id().empty()) {
		// 非匿名函数分配变量来装，这里其实有个没考虑的地方
		// 如果外层还有一层赋值，那么该函数的名字应该只在函数内作用域有效
		auto& var_info = code_generator->AllocateVar(id(), VarFlags::kConst);
		function_def_base->bytecode_table().EmitVarStore(var_info.var_idx);

		if (is_export()) {
			static_cast<ModuleDef*>(function_def_base)->export_var_def_table().AddExportVar(id(), var_info.var_idx);
		}
	}

	// 保存环境，以生成新指令流
	// auto savefunc = current_func_def_;

	// 切换环境
	code_generator->EnterScope(function_def_base, new_func_def, ScopeType::kFunction);

	// 参数正序分配
	for (size_t i = 0; i < new_func_def->param_count(); ++i) {
		code_generator->AllocateVar(params()[i]);
	}

	code_generator->GenerateFunctionBody(new_func_def, body_.get());

	bool need_repair = !new_func_def->closure_var_table().closure_var_defs().empty();

	// 恢复环境
	code_generator->ExitScope();
	new_func_def->debug_table().Sort();


	if (need_repair) {
		function_def_base->bytecode_table().RepairOpcode(load_pc, OpcodeType::kClosure);
	}
}

std::unique_ptr<Expression> FunctionExpression::ParseExpressionAtFunctionLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();

	// 处理 async 关键字
	bool is_async = false;
	if (lexer->PeekToken().is(TokenType::kKwAsync)) {
		lexer->NextToken();

		// 检查 async 后面是否是 function 关键字（普通异步函数）
		if (lexer->PeekToken().is(TokenType::kKwFunction)) {
			is_async = true;
			return ParseTraditionalFunctionExpression(lexer, start, is_async, false);
		}

		// 否则可能是异步箭头函数，继续向下处理
		is_async = true;
	}

	// 处理 function 关键字（传统函数）
	if (lexer->PeekToken().is(TokenType::kKwFunction)) {
		return ParseTraditionalFunctionExpression(lexer, start, is_async, false);
	}

	// 检查是否是箭头函数 (参数列表或单个参数)
	if (lexer->PeekToken().is(TokenType::kSepLParen) ||
		lexer->PeekToken().is(TokenType::kIdentifier)) {
		// 可能是箭头函数或其他表达式
		return ArrowFunctionExpression::TryParseArrowFunctionExpression(lexer, start, is_async);
	}

	// 不是函数表达式，回退到普通表达式解析
	return AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer);
}



std::unique_ptr<Expression> FunctionExpression::ParseTraditionalFunctionExpression(Lexer* lexer, SourcePosition start, bool is_async, bool is_generator) {
	// 处理传统函数声明 function [name](params) { ... }
	lexer->MatchToken(TokenType::kKwFunction);

	// 处理生成器函数
	if (lexer->PeekToken().is(TokenType::kOpMul)) {
		if (is_async) {
			throw SyntaxError("Async generator functions are not supported");
		}
		lexer->NextToken();
		is_generator = true;
	}

	// 函数名（可选）
	std::string id;
	if (lexer->PeekToken().is(TokenType::kIdentifier)) {
		id = lexer->NextToken().value();
	}

	// 参数列表
	auto params = Expression::ParseParameters(lexer);

	// 类型注解
	TryParseTypeAnnotation(lexer);

	// 函数体
	auto block = BlockStatement::ParseBlockStatement(lexer);

	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<FunctionExpression>(
		start, end, id, std::move(params), std::move(block),
		is_generator, is_async, false
	);
}

} // namespace compiler
} // namespace mjs