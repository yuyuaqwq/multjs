#pragma once

#include <exception>
#include <memory>
#include <vector>

#include <mjs/noncopyable.h>

#include "lexer.h"
#include "stat.h"
#include "exp.h"

namespace mjs {
namespace compiler {

class ParserException : public std::exception {
public:
	using Base = std::exception;
	using Base::Base;
};

class Parser : public noncopyable {
public:
	Parser(Lexer* t_lexer);

	std::unique_ptr<Identifier> ParseIdentifier();
	std::unique_ptr<Expression> ParseLiteral();
	std::unique_ptr<ArrayExpression> ParseArrayExpression();
	std::unique_ptr<ObjectExpression> ParseObjectExpression();
	std::unique_ptr<FunctionExpression> ParseFunctionExpression();
	std::unique_ptr<MemberExpression> ParseMemberExpression(std::unique_ptr<Expression> object);


	void ParseProgram();
	std::unique_ptr<BlockStatement> ParseBlockStatement();
	std::unique_ptr<Statement> ParseStatement();

	std::unique_ptr<ExpressionStatement> ParseExpressionStatement();

	std::vector<std::string> ParseParNameList();
	std::unique_ptr<IfStatement> ParseIfStatement();

	std::unique_ptr<ForStatement> ParseForStatement();
	std::unique_ptr<WhileStatement> ParseWhileStatement();
	std::unique_ptr<ContinueStatement> ParseContinueStatement();
	std::unique_ptr<BreakStatement> ParseBreakStatement();
	std::unique_ptr<ReturnStatement> ParseReturnStatement();
	std::unique_ptr<TryStatement> ParseTryStatement();
	std::unique_ptr<CatchClause> ParseCatchClause();
	std::unique_ptr<FinallyClause> ParseFinallyClause();
	std::unique_ptr<ThrowStatement> ParseThrowStatement();

	std::unique_ptr<LabeledStatement> ParseLabeledStatement();
	std::unique_ptr<VariableDeclaration> ParseVariableDeclaration(TokenType kind);

	std::unique_ptr<Statement> ParseImportStatement(TokenType type);
	std::unique_ptr<ExportDeclaration> ParseExportDeclaration(TokenType type);

	std::unique_ptr<Expression> ParseExpression();
	std::unique_ptr<Expression> ParseExpression20();
	std::unique_ptr<Expression> ParseExpression19();
	std::unique_ptr<Expression> ParseExpression18();
	std::unique_ptr<Expression> ParseExpression17();
	std::unique_ptr<Expression> ParseExpression16();
	std::unique_ptr<Expression> ParseExpression15();
	std::unique_ptr<Expression> ParseExpression14();
	std::unique_ptr<Expression> ParseExpression13();
	std::unique_ptr<Expression> ParseExpression12();
	std::unique_ptr<Expression> ParseExpression11();
	std::unique_ptr<Expression> ParseExpression10();
	std::unique_ptr<Expression> ParseExpression9();
	std::unique_ptr<Expression> ParseExpression8();
	std::unique_ptr<Expression> ParseExpression7();
	std::unique_ptr<Expression> ParseExpression6();
	std::unique_ptr<Expression> ParseExpression5();
	std::unique_ptr<Expression> ParseExpression4();

	std::unique_ptr<Expression> ParseNewExpression();
	std::unique_ptr<Expression> ParseExpression3();
	
	std::unique_ptr<Expression> ParseExpression2(bool match_lparen);

	std::unique_ptr<Expression> ParseExpression1();


	std::vector<std::unique_ptr<Expression>> ParseExpressionList(TokenType begin, TokenType end, bool allow_comma_end);

	const auto& src_statements() const { return src_statements_; }
	const auto& import_declarations() const { return import_declarations_; }

private:
	Lexer* lexer_;

	std::vector<std::unique_ptr<Statement>> src_statements_;
	std::vector<std::unique_ptr<ImportDeclaration>> import_declarations_;
};

} // namespace compiler
} // namespace mjs