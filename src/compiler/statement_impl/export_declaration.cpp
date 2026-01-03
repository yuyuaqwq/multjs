#include "src/compiler/statement_impl/export_declaration.h"

#include <mjs/error.h>

#include "src/compiler/code_generator.h"
#include "src/compiler/statement_impl/expression_statement.h"
#include "src/compiler/statement_impl/variable_declaration.h"
#include "src/compiler/expression_impl/function_expression.h"

namespace mjs {
namespace compiler {

void ExportDeclaration::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    if (!function_def_base->is_module()) {
        throw SyntaxError("Only modules can export.");
    }

    auto& decl = declaration_;
    code_generator->GenerateStatement(function_def_base, decl.get());
}

std::unique_ptr<ExportDeclaration> ExportDeclaration::ParseExportDeclaration(Lexer* lexer, TokenType type) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(type);
	auto stat = Statement::ParseStatement(lexer);
	if (stat->is(StatementType::kExpression)) {
		auto& exp = stat->as<ExpressionStatement>().expression();
		if (auto* func_exp = dynamic_cast<FunctionExpression*>(exp.get())) {
			func_exp->set_is_export(true);
			auto end = lexer->GetRawSourcePosition();
			return std::make_unique<ExportDeclaration>(start, end, std::move(stat));
		}
	}
	if (stat->is(StatementType::kVariableDeclaration)) {
		auto& var_decl = stat->as<VariableDeclaration>();
		var_decl.set_is_export(true);
	}
	else {
		throw SyntaxError("Statement that cannot be exported.");
	}
	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<ExportDeclaration>(start, end, std::move(stat));
}

} // namespace compiler
} // namespace mjs