#pragma once

#include <exception>
#include <memory>
#include <vector>

#include <mjs/noncopyable.h>

#include "lexer.h"
#include "statement.h"
#include "expression.h"

namespace mjs {
namespace compiler {

class Parser : public noncopyable {
public:
	Parser(Lexer* t_lexer);

	void ParseProgram();

	std::unique_ptr<Expression> ParseExpression();
	std::unique_ptr<Expression> ParseCommaExpression();
	std::unique_ptr<Expression> ParseAssignmentOrFunction();
	std::unique_ptr<YieldExpression> ParseYieldExpression();
	std::unique_ptr<Expression> ParseFunctionExpression();
	std::unique_ptr<Expression> TryParseArrowFunction(SourcePos start, bool is_async);
	std::unique_ptr<Expression> ParseTraditionalFunction(SourcePos start, bool is_async, bool is_generator);
	std::unique_ptr<Expression> ParseAssignmentExpression();
	std::unique_ptr<Expression> ParseTernaryExpression();
	std::unique_ptr<Expression> ParseLogicalOrExpression();
	std::unique_ptr<Expression> ParseLogicalAndExpression();
	std::unique_ptr<Expression> ParseBitwiseOrExpression();
	std::unique_ptr<Expression> ParseBitwiseXorExpression();
	std::unique_ptr<Expression> ParseBitwiseAndExpression();
	std::unique_ptr<Expression> ParseEqualityExpression();
	std::unique_ptr<Expression> ParseRelationalExpression();
	std::unique_ptr<Expression> ParseShiftExpression();
	std::unique_ptr<Expression> ParseAdditiveExpression();
	std::unique_ptr<Expression> ParseMultiplicativeExpression();
	std::unique_ptr<Expression> ParseExponentiationExpression();
	std::unique_ptr<Expression> ParseUnaryExpression();
	std::unique_ptr<Expression> ParsePostfixExpression();
	std::unique_ptr<Expression> ParseNewImportOrMemberExpression();
	std::unique_ptr<Expression> ParseNewExpression();
	std::unique_ptr<Expression> ParseMemberOrCallExpression(std::unique_ptr<Expression> right, bool match_lparen);
	std::unique_ptr<MemberExpression> ParseMemberExpression(std::unique_ptr<Expression> object);
	std::unique_ptr<CallExpression> ParseCallExpression(std::unique_ptr<Expression> callee);

	std::unique_ptr<ImportExpression> ParseImportExpression();
	std::unique_ptr<Expression> ParsePrimaryExpression();
	std::unique_ptr<ArrayExpression> ParseArrayExpression();
	std::unique_ptr<ObjectExpression> ParseObjectExpression();
	std::unique_ptr<ThisExpression> ParseThis();
	std::unique_ptr<TemplateLiteral> ParseTemplateLiteral();
	std::unique_ptr<Expression> ParseLiteral();
	std::unique_ptr<Expression> TryParseLiteral();
	std::unique_ptr<Identifier> ParseIdentifier();
	
	std::unique_ptr<Statement> ParseStatement();

	std::unique_ptr<Statement> ParseImportStatement(TokenType type);
	std::unique_ptr<ExportDeclaration> ParseExportDeclaration(TokenType type);

	std::unique_ptr<VariableDeclaration> ParseVariableDeclaration(TokenType kind);

	std::unique_ptr<IfStatement> ParseIfStatement();
	std::unique_ptr<LabeledStatement> ParseLabeledStatement();

	std::unique_ptr<ForStatement> ParseForStatement();
	std::unique_ptr<WhileStatement> ParseWhileStatement();
	std::unique_ptr<ContinueStatement> ParseContinueStatement();
	std::unique_ptr<BreakStatement> ParseBreakStatement();
	
	std::unique_ptr<ReturnStatement> ParseReturnStatement();

	std::unique_ptr<TryStatement> ParseTryStatement();
	std::unique_ptr<CatchClause> ParseCatchClause();
	std::unique_ptr<FinallyClause> ParseFinallyClause();
	std::unique_ptr<ThrowStatement> ParseThrowStatement();

	std::unique_ptr<BlockStatement> ParseBlockStatement();
	std::unique_ptr<ExpressionStatement> ParseExpressionStatement();

	std::vector<std::string> ParseParameters();
	std::vector<std::unique_ptr<Expression>> ParseExpressions(TokenType begin, TokenType end, bool allow_comma_end);

	std::unique_ptr<TypeAnnotation> ParseTypeAnnotation();
	std::unique_ptr<UnionType> ParseUnionType();


	const auto& src_statements() const { return src_statements_; }
	const auto& import_declarations() const { return import_declarations_; }

private:
	Lexer* lexer_;

	std::vector<std::unique_ptr<Statement>> src_statements_;
	std::vector<std::unique_ptr<ImportDeclaration>> import_declarations_;
};

} // namespace compiler
} // namespace mjs