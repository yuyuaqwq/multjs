#include "try_statement.h"

#include "mjs/error.h"

#include "../code_generator.h"

namespace mjs {
namespace compiler {

/**
 * @brief 解析try语句
 *
 * try语句的形式为：try { block } [catch (error) { block }] [finally { block }]
 *
 * @return 解析后的try语句
 */
std::unique_ptr<TryStatement> TryStatement::ParseTryStatement(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kKwTry);

	auto block = BlockStatement::ParseBlockStatement(lexer);

	auto token = lexer->PeekToken();

	std::unique_ptr<CatchClause> catch_stat;
	if (token.is(TokenType::kKwCatch)) {
		catch_stat = CatchClause::ParseCatchClause(lexer);
		token = lexer->PeekToken();
	}

	std::unique_ptr<FinallyClause> finally_stat;
	if (token.is(TokenType::kKwFinally)) {
		finally_stat = FinallyClause::ParseFinallyClause(lexer);
	}

	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<TryStatement>(start, end, std::move(block), std::move(catch_stat), std::move(finally_stat));
}

void TryStatement::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    auto has_finally = bool(finalizer_);

    auto try_start_pc = function_def_base->bytecode_table().Size();

    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kTryBegin);

    code_generator->EnterScope(function_def_base, nullptr, has_finally ? ScopeType::kTryFinally : ScopeType::kTry);
    block_->GenerateCode(code_generator, function_def_base);
    code_generator->ExitScope();

    auto try_end_pc = function_def_base->bytecode_table().Size();

    // 这里需要生成跳向finally的指令
    auto repair_end_pc = try_end_pc;
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    function_def_base->bytecode_table().EmitPcOffset(0);

    auto catch_start_pc = kInvalidPc;
    auto catch_end_pc = kInvalidPc;
    auto catch_err_var_idx = kVarInvaildIndex;

    if (handler_) {
        catch_start_pc = function_def_base->bytecode_table().Size();
        code_generator->EnterScope(function_def_base, nullptr, has_finally ? ScopeType::kCatchFinally : ScopeType::kCatch);

        // 加载error参数到变量
        catch_err_var_idx = code_generator->AllocateVar(handler_->param()->name()).var_idx;

        handler_->body()->GenerateCode(code_generator, function_def_base);

        code_generator->ExitScope();
        catch_end_pc = function_def_base->bytecode_table().Size();
    }
    else {
        catch_end_pc = try_end_pc;
    }

    // 修复pc
    function_def_base->bytecode_table().RepairPc(repair_end_pc, function_def_base->bytecode_table().Size());

    // finally是必定会执行的
    auto finally_start_pc = kInvalidPc;
    auto finally_end_pc = kInvalidPc;
    if (finalizer_) {
        finally_start_pc = function_def_base->bytecode_table().Size();
        code_generator->EnterScope(function_def_base, nullptr, ScopeType::kFinally);
        finalizer_->body()->GenerateCode(code_generator, function_def_base);
        code_generator->ExitScope();
        finally_end_pc = function_def_base->bytecode_table().Size();
    }

    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kTryEnd);

    if (!handler_ && !finalizer_) {
        throw SyntaxError("There cannot be a statement with only try.");
    }

    // 添加到异常表
    auto& exception_table = function_def_base->exception_table();
    auto exception_idx = exception_table.AddEntry({});
    auto& entry = exception_table.GetEntry(exception_idx);
    entry.try_start_pc = try_start_pc;
    entry.try_end_pc = try_end_pc;
    entry.catch_start_pc = catch_start_pc;
    entry.catch_end_pc = catch_end_pc;
    entry.catch_err_var_idx = catch_err_var_idx;
    entry.finally_start_pc = finally_start_pc;
    entry.finally_end_pc = finally_end_pc;
}

} // namespace compiler
} // namespace mjs