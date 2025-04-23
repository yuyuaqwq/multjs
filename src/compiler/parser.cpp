#include "parser.h"

/* EBNF
exp = exp3
exp3 = exp2 {oper3 exp2}
oper3 = '==' | '!='
exp2 = exp1 {oper2 exp1}
oper2 = '+' | '-'
exp1 = exp0 {oper1 exp0}
oper1 = '*' | '/'
exp0 = '(' exp ')' | exp0
exp0 = number
*/

namespace mjs {
namespace compiler {

Parser::Parser(Lexer* lexer)
	: lexer_(lexer) {}

std::unique_ptr<Identifier> Parser::ParseIdentifier() {
	auto start = lexer_->pos();
	auto token = lexer_->MatchToken(TokenType::kIdentifier);
	auto end = lexer_->pos();
	auto exp = std::make_unique<Identifier>(start, end, token.str());
	exp->set_value_category(ValueCategory::kLValue);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseLiteral() {
	auto start = lexer_->pos();
	auto token = lexer_->PeekToken();
	std::unique_ptr<Expression> exp;
	switch (token.type()) {
	//case TokenType::kKwAsync:
	//case TokenType::kKwFunction:
	//	exp = ParseFunctionExpression();
	//	break;
	case TokenType::kUndefined:
		lexer_->NextToken();
		exp = std::make_unique<UndefinedLiteral>(start, lexer_->pos());
		break;
	case TokenType::kNull: {
		lexer_->NextToken();
		exp = std::make_unique<NullLiteral>(start, lexer_->pos());
		break;
	}
	case TokenType::kTrue: {
		lexer_->NextToken();
		exp = std::make_unique<BooleanLiteral>(start, lexer_->pos(), true);
		break;
	}
	case TokenType::kFalse: {
		lexer_->NextToken();
		exp = std::make_unique<BooleanLiteral>(start, lexer_->pos(), false);
		break;
	}
	case TokenType::kFloatLiteral: {
		lexer_->NextToken();
		exp = std::make_unique<FloatLiteral>(start, lexer_->pos(), std::stod(token.str()));
		break;
	}
	case TokenType::kIntegerLiteral: {
		lexer_->NextToken();
		exp = std::make_unique<IntegerLiteral>(start, lexer_->pos(), std::stoll(token.str()));
		break;
	}
	case TokenType::kString: {
		lexer_->NextToken();
		exp = std::make_unique<StringLiteral>(start, lexer_->pos(), token.str());
		break;
	}
	//case TokenType::kSepLBrack: {
	//	exp = ParseArrayExpression();
	//	break;
	//}
	//case TokenType::kSepLCurly: {
	//	exp = ParseObjectExpression();
	//	break;
	//}
	//case TokenType::kIdentifier: {
	//	exp = ParseIdentifier();
	//	break;
	//}
	//case TokenType::kKwThis: {
	//	lexer_->NextToken();
	//	exp = std::make_unique<ThisExpression>();
	//	break;
	//}
	//case TokenType::kKwImport: {
	//	lexer_->NextToken();
	//	lexer_->MatchToken(TokenType::kSepLParen);
	//	auto path = lexer_->MatchToken(TokenType::kString).str();
	//	lexer_->MatchToken(TokenType::kSepRParen);
	//	exp = std::make_unique<ImportExpression>(std::move(path));
	//	break;
	//}
	}

	if (!exp) {
		throw ParserException("Unable to parse expression.");
	}
	return exp;
}

std::unique_ptr<ArrayExpression> Parser::ParseArrayExpression() {
	auto start = lexer_->pos();
	auto arr_literal = ParseExpressionList(TokenType::kSepLBrack, TokenType::kSepRBrack, true);
	auto end = lexer_->pos();
	return std::make_unique<ArrayExpression>(start, end, std::move(arr_literal));
}

std::unique_ptr<ObjectExpression> Parser::ParseObjectExpression() {
	auto start = lexer_->pos();
	lexer_->NextToken();
	std::unordered_map<std::string, std::unique_ptr<Expression>> obj_literal;
	if (!lexer_->PeekToken().is(TokenType::kSepRCurly)) {
		do {
			auto ident = lexer_->MatchToken(TokenType::kIdentifier);
			lexer_->MatchToken(TokenType::kSepColon);
			// ParseExpression20会解析kSepComma，避免
			obj_literal.emplace(ident.str(), ParseExpression19());
			if (!lexer_->PeekToken().is(TokenType::kSepComma)) {
				break;
			}
			if (lexer_->PeekToken().is(TokenType::kSepRCurly)) {
				break;
			}
			lexer_->NextToken();
		} while (true);
	}
	lexer_->MatchToken(TokenType::kSepRCurly);
	auto end = lexer_->pos();
	return std::make_unique<ObjectExpression>(start, end, std::move(obj_literal));
}

std::unique_ptr<FunctionExpression> Parser::ParseFunctionExpression() {
	auto start = lexer_->pos();
	bool is_async = false;
	bool is_generator = false;
	if (lexer_->PeekToken().is(TokenType::kKwAsync)) {
		lexer_->NextToken();
		is_async = true;
	}
	lexer_->MatchToken(TokenType::kKwFunction);
	if (lexer_->PeekToken().is(TokenType::kOpMul)) {
		if (is_async) {
			throw ParserException("Does not support asynchronous generator function.");
		}
		// 生成器函数
		lexer_->NextToken();
		is_generator = true;
	}
	std::string id;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		id = lexer_->NextToken().str();
	}
	auto params = ParseParNameList();
	auto block = ParseBlockStatement();

	auto end = lexer_->pos();
	return std::make_unique<FunctionExpression>(start, end, id, std::move(params), std::move(block), is_async, is_generator, false);
}

std::unique_ptr<MemberExpression> Parser::ParseMemberExpression(std::unique_ptr<Expression> object) {
	auto start = lexer_->pos();

	std::unique_ptr<Expression> member;

	bool computed = false;
	auto token = lexer_->NextToken();
	if (token.is(TokenType::kSepDot)) {
		member = ParseLiteral();
	}
	else if (token.is(TokenType::kSepLBrack)) {
		member = ParseExpression();
		lexer_->MatchToken(TokenType::kSepRBrack);
		computed = true;
	}
	else {
		throw;
	}

	bool is_method_call = false;
	if (lexer_->PeekToken().type() == TokenType::kSepLParen) {
		is_method_call = true;
	}

	auto end = lexer_->pos();

	return std::make_unique<MemberExpression>(start, end, std::move(object), std::move(member), computed, false);
}


void Parser::ParseProgram() {
	while (!lexer_->PeekToken().is(TokenType::kEof)) {
		src_statements_.emplace_back(ParseStatement());
	}
}

std::unique_ptr<BlockStatement> Parser::ParseBlockStatement() {
	lexer_->MatchToken(TokenType::kSepLCurly);

	std::vector<std::unique_ptr<Statement>> stat_list;

	while (!lexer_->PeekToken().is(TokenType::kSepRCurly)) {
		stat_list.push_back(ParseStatement());
	}

	lexer_->MatchToken(TokenType::kSepRCurly);

	return std::make_unique<BlockStatement>(std::move(stat_list));
}

std::unique_ptr<Statement> Parser::ParseStatement() {
	auto token = lexer_->PeekToken();
	switch (token.type()) {
		case TokenType::kIdentifier: {
			if (lexer_->PeekTokenN(2).is(TokenType::kSepColon)) {
				return ParseLabeledStatement();
			}
		}
		case TokenType::kKwLet:
		case TokenType::kKwConst: {
			return ParseVariableDeclaration(token.type());
		}
		case TokenType::kKwIf: {
			return ParseIfStatement();
		}
		case TokenType::kKwFor: {
			return ParseForStatement();
		}
		case TokenType::kKwWhile: {
			return ParseWhileStatement();
		}
		case TokenType::kKwContinue: {
			return ParseContinueStatement();
		}
		case TokenType::kKwBreak: {
			return ParseBreakStatement();
		}
		case TokenType::kKwReturn: {
			return ParseReturnStatement();
		}
		case TokenType::kKwTry: {
			return ParseTryStatement();
		}
		case TokenType::kKwThrow: {
			return ParseThrowStatement();
		}
		case TokenType::kKwAsync:
		case TokenType::kKwFunction: {
			// 如果是直接定义，就不需要添加分号
			return std::make_unique<ExpressionStatement>(ParseFunctionExpression());
		}
		case TokenType::kSepLCurly: {
			return ParseBlockStatement();
		}
		case TokenType::kKwImport: {
			return ParseImportStatement(token.type());
		}
		case TokenType::kKwExport: {
			return ParseExportDeclaration(token.type());
		}
		default: {
			return ParseExpressionStatement();
		}
	}
}

std::unique_ptr<ExpressionStatement> Parser::ParseExpressionStatement() {
	if (lexer_->PeekToken().is(TokenType::kSepSemi)) {
		lexer_->NextToken();
		return std::make_unique<ExpressionStatement>(nullptr);
	}
	auto exp = ParseExpression();
	lexer_->MatchToken(TokenType::kSepSemi);
	return std::make_unique<ExpressionStatement>(std::move(exp));
}

std::vector<std::string> Parser::ParseParNameList() {
	lexer_->MatchToken(TokenType::kSepLParen);
	std::vector<std::string> parList;
	if (!lexer_->PeekToken().is(TokenType::kSepRParen)) {
		do {
			parList.push_back(lexer_->MatchToken(TokenType::kIdentifier).str());
			if (!lexer_->PeekToken().is(TokenType::kSepComma)) {
				break;
			}
			lexer_->NextToken();
		} while (true);
	}
	lexer_->MatchToken(TokenType::kSepRParen);
	return parList;
}

std::unique_ptr<IfStatement> Parser::ParseIfStatement() {
	lexer_->NextToken();

	lexer_->MatchToken(TokenType::kSepLParen);
	auto test = ParseExpression();
	lexer_->MatchToken(TokenType::kSepRParen);

	auto consequent = ParseBlockStatement();

	std::unique_ptr<Statement> alternate;
	auto token = lexer_->PeekToken();
	if (token.is(TokenType::kKwElse)) {
		lexer_->NextToken();
		token = lexer_->PeekToken();

		if (token.is(TokenType::kKwIf)) {
			alternate = ParseIfStatement();
		}
		else {
			alternate = ParseBlockStatement();
		}
	}
	return std::make_unique<IfStatement>(std::move(test), std::move(consequent), std::move(alternate));
}

std::unique_ptr<ForStatement> Parser::ParseForStatement() {
	lexer_->MatchToken(TokenType::kKwFor);
	lexer_->MatchToken(TokenType::kSepLParen);

	std::unique_ptr<Statement> initialization;
	auto token = lexer_->PeekToken();
	if (token.is(TokenType::kSepSemi)) {
		lexer_->NextToken();
	}
	else {
		if (token.is(TokenType::kKwLet) || token.is(TokenType::kKwConst)) {
			initialization = ParseVariableDeclaration(token.type());
		}
		else {
			initialization = ParseExpressionStatement();
		}
	}

	token = lexer_->PeekToken();
	std::unique_ptr<Expression> condition;
	if (!token.is(TokenType::kSepSemi)) {
		condition = ParseExpression();
	}
	lexer_->MatchToken(TokenType::kSepSemi);

	token = lexer_->PeekToken();
	std::unique_ptr<Expression> final_expression;
	if (!token.is(TokenType::kSepRParen)) {
		final_expression = ParseExpression();
	}
	lexer_->MatchToken(TokenType::kSepRParen);

	auto block = ParseBlockStatement();

	return std::make_unique<ForStatement>(std::move(initialization), std::move(condition)
		, std::move(final_expression), std::move(block));
}

std::unique_ptr<WhileStatement> Parser::ParseWhileStatement() {
	lexer_->MatchToken(TokenType::kKwWhile);
	lexer_->MatchToken(TokenType::kSepLParen);
	auto exp = ParseExpression();
	lexer_->MatchToken(TokenType::kSepRParen);
	auto block = ParseBlockStatement();
	return std::make_unique<WhileStatement>(std::move(exp), std::move(block));
}

std::unique_ptr<ContinueStatement> Parser::ParseContinueStatement() {
	lexer_->MatchToken(TokenType::kKwContinue);
	std::optional<std::string> label_name;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		label_name = lexer_->NextToken().str();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	return std::make_unique<ContinueStatement>(std::move(label_name));
}

std::unique_ptr<BreakStatement> Parser::ParseBreakStatement() {
	lexer_->MatchToken(TokenType::kKwBreak);
	std::optional<std::string> label_name;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		label_name = lexer_->NextToken().str();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	return std::make_unique<BreakStatement>(std::move(label_name));
}

std::unique_ptr<ReturnStatement> Parser::ParseReturnStatement() {
	lexer_->MatchToken(TokenType::kKwReturn);
	std::unique_ptr<Expression> exp;
	if (!lexer_->PeekToken().is(TokenType::kSepSemi)) {
		exp = ParseExpression();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	return std::make_unique<ReturnStatement>(std::move(exp));
}

std::unique_ptr<TryStatement> Parser::ParseTryStatement() {
	lexer_->MatchToken(TokenType::kKwTry);

	auto block = ParseBlockStatement();

	auto token = lexer_->PeekToken();

	std::unique_ptr<CatchClause> catch_stat;
	if (token.is(TokenType::kKwCatch)) {
		catch_stat = ParseCatchClause();
		token = lexer_->PeekToken();
	}

	std::unique_ptr<FinallyClause> finally_stat;
	if (token.is(TokenType::kKwFinally)) {
		finally_stat = ParseFinallyClause();
	}

	return std::make_unique<TryStatement>(std::move(block), std::move(catch_stat), std::move(finally_stat));
}

std::unique_ptr<CatchClause> Parser::ParseCatchClause() {
	lexer_->MatchToken(TokenType::kKwCatch);

	lexer_->MatchToken(TokenType::kSepLParen);
	auto exp = ParseIdentifier();
	lexer_->MatchToken(TokenType::kSepRParen);

	auto block = ParseBlockStatement();

	return std::make_unique<CatchClause>(std::move(exp), std::move(block));
}

std::unique_ptr<FinallyClause> Parser::ParseFinallyClause() {
	lexer_->MatchToken(TokenType::kKwFinally);
	auto block = ParseBlockStatement();
	return std::make_unique<FinallyClause>(std::move(block));
}

std::unique_ptr<ThrowStatement> Parser::ParseThrowStatement() {
	lexer_->MatchToken(TokenType::kKwThrow);
	auto exp = ParseExpression();
	return std::make_unique<ThrowStatement>(std::move(exp));
}

std::unique_ptr<LabeledStatement> Parser::ParseLabeledStatement() {
	auto label_name = lexer_->MatchToken(TokenType::kIdentifier).str();
	lexer_->MatchToken(TokenType::kSepColon);
	auto stat = ParseStatement();
	return std::make_unique<LabeledStatement>(std::move(label_name), std::move(stat));
}

std::unique_ptr<VariableDeclaration> Parser::ParseVariableDeclaration(TokenType kind) {
	SourcePos start = lexer_->pos();
	lexer_->MatchToken(kind);
	auto name = lexer_->MatchToken(TokenType::kIdentifier).str();
	lexer_->MatchToken(TokenType::kOpAssign);
	auto init = ParseExpression();
	lexer_->MatchToken(TokenType::kSepSemi);
	SourcePos end = lexer_->pos();
	return std::make_unique<VariableDeclaration>(start, end, std::move(name), std::move(init), kind);
}

std::unique_ptr<Statement> Parser::ParseImportStatement(TokenType type) {
	auto token = lexer_->PeekTokenN(2);
	if (token.is(TokenType::kOpMul)) {
		lexer_->MatchToken(type);
		auto token = lexer_->NextToken();
		lexer_->MatchToken(TokenType::kKwAs);
		auto module_name = lexer_->MatchToken(TokenType::kIdentifier).str();
		lexer_->MatchToken(TokenType::kKwFrom);

		auto path = lexer_->MatchToken(TokenType::kString).str();

		// 静态import会被提升，单独保存
		auto import_stat = std::make_unique<ImportDeclaration>(std::move(path), std::move(module_name));
		import_declarations_.emplace_back(std::move(import_stat));

		// 解析下一个语句返回
		return ParseStatement();
	}
	else if (token.is(TokenType::kSepLParen)) {
		// 动态import
		return ParseExpressionStatement();
	}
	else {
		throw ParserException("Unsupported module parsing.");
	}
}

std::unique_ptr<ExportDeclaration> Parser::ParseExportDeclaration(TokenType type) {
	lexer_->MatchToken(type);
	auto stat = ParseStatement();
	if (stat->is(StatementType::kExpression)) {
		auto& exp = stat->as<ExpressionStatement>().expression();
		if (exp->is(ExpressionType::kFunctionExpression)) {
			auto& func_decl_exp = exp->as<FunctionExpression>();
			func_decl_exp.set_is_export(true);
			return std::make_unique<ExportDeclaration>(std::move(stat));
		}
	}
	if (stat->is(StatementType::kVariableDeclaration)) {
		auto& var_decl = stat->as<VariableDeclaration>();
		var_decl.set_is_export(true);
	}
	else {
		throw ParserException("Statement that cannot be exported.");
	}
	return std::make_unique<ExportDeclaration>(std::move(stat));
}



std::unique_ptr<Expression> Parser::ParseExpression() {
	return ParseExpression20();
}

std::unique_ptr<Expression> Parser::ParseExpression20() {
	auto exp = ParseExpression19();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kSepComma) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression19());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression19() {
	auto type = lexer_->PeekToken().type();
	if (type == TokenType::kKwYield) {
		lexer_->NextToken();

		std::unique_ptr<Expression> yielded_value = ParseExpression18();
		return std::make_unique<YieldExpression>(std::move(yielded_value));
	}

	return ParseExpression18();
}


std::unique_ptr<Expression> Parser::ParseExpression18() {
	// 赋值，右结合
	auto exp = ParseExpression17();
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kOpAssign) {
		return exp;
	}
	lexer_->NextToken();
	exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression18());
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression17() {
	// 三元，右结合
	auto exp = ParseExpression16();
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kSepQuestion) {
		return exp;
	}
	lexer_->NextToken();
	auto exp2 = ParseExpression17();
	lexer_->MatchToken(TokenType::kSepColon);
	auto exp3 = ParseExpression17();
	exp = std::make_unique<ConditionalExpression>(TokenType::kOpTernary, std::move(exp), std::move(exp2), std::move(exp3));
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression16() {
	auto exp = ParseExpression15();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpOr
			&& type != TokenType::kOpNullishCoalescing) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression15());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression15() {
	auto exp = ParseExpression14();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpAnd) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression14());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression14() {
	auto exp = ParseExpression13();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpBitOr) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression13());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression13() {
	auto exp = ParseExpression12();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpBitXor) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression12());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression12() {
	auto exp = ParseExpression11();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpBitAnd) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression11());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression11() {
	auto exp = ParseExpression10();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpNe
			&& type != TokenType::kOpEq
			&& type != TokenType::kOpStrictEq
			&& type != TokenType::kOpStrictNe) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression10());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression10() {
	auto exp = ParseExpression9();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpLt
			&& type != TokenType::kOpLe
			&& type != TokenType::kOpGt
			&& type != TokenType::kOpGe
			&& type != TokenType::kKwIn
			&& type != TokenType::kKwInstanceof) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression9());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression9() {
	auto exp = ParseExpression8();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpShiftLeft
			&& type != TokenType::kOpShiftRight
			&& type != TokenType::kOpUnsignedShiftRight) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression8());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression8() {
	auto exp = ParseExpression7();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpAdd
			&& type != TokenType::kOpSub) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression7());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression7() {
	auto exp = ParseExpression6();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpMul
			&& type != TokenType::kOpDiv
			&& type != TokenType::kOpMod) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryExpression>(std::move(exp), type, ParseExpression6());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression6() {
	// .. ** ..，右结合
	auto exp = ParseExpression5();
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kOpPower) {
		return exp;
	}
	lexer_->NextToken();
	exp = std::make_unique<UnaryExpression>(type, ParseExpression6());
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression5() {
	auto token = lexer_->PeekToken();
	std::unique_ptr<Expression> exp;
	switch (token.type()) {
	case TokenType::kOpNot:
	case TokenType::kOpBitNot:
	case TokenType::kOpAdd:
	case TokenType::kOpSub:
	case TokenType::kKwTypeof:
	case TokenType::kKwVoid:
	case TokenType::kKwDelete:
	case TokenType::kKwAwait: {
		lexer_->NextToken();
		exp = std::make_unique<UnaryExpression>(token.type(), ParseExpression5());
		break;
	}
	case TokenType::kOpInc: {
		lexer_->NextToken();
		exp = ParseExpression5();
		if (exp->value_category() != ValueCategory::kLValue) {
			throw ParserException("Only use auto inc on lvalue.");
		}
		exp = std::make_unique<UnaryExpression>(TokenType::kOpPrefixInc, std::move(exp));
		break;
	}
	case TokenType::kOpDec: {
		lexer_->NextToken();
		exp = ParseExpression5();
		if (exp->value_category() != ValueCategory::kLValue) {
			throw ParserException("Only use auto dec on lvalue.");
		}
		exp = std::make_unique<UnaryExpression>(TokenType::kOpPrefixDec, std::move(exp));
		break;
	}
	default:
		exp = ParseExpression4();
	}
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression4() {
	auto exp = ParseExpression3();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpInc
			&& type != TokenType::kOpDec) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<UnaryExpression>(TokenType::kOpSuffixInc, std::move(exp));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseNewExpression() {
	lexer_->NextToken();
	std::unique_ptr<Expression> callee;

	auto type = lexer_->PeekToken().type();
	if (type == TokenType::kKwNew) {
		callee = ParseNewExpression();
	}
	else {
		callee = ParseExpression2(false);
	}

	std::vector<std::unique_ptr<Expression>> args;
	if (lexer_->PeekToken().is(TokenType::kSepLParen)) {
		args = ParseExpressionList(TokenType::kSepLParen, TokenType::kSepRParen, false);
	}
	auto exp = std::make_unique<NewExpression>(std::move(callee), std::move(args));

	return exp;
}

std::unique_ptr<Expression> Parser::ParseExpression3() {
	auto type = lexer_->PeekToken().type();
	if (type == TokenType::kKwNew) {
		return ParseNewExpression();
	}
	return ParseExpression2(true);
}


std::unique_ptr<Expression> Parser::ParseExpression2(bool match_lparen) {
	auto exp = ParseExpression1();
	do {
		auto token = lexer_->PeekToken();
		if (token.is(TokenType::kSepDot) || token.is(TokenType::kSepLBrack)) {
			exp = ParseMemberExpression(std::move(exp));
		}
		else if (match_lparen && token.is(TokenType::kSepLParen)) {
			auto args = ParseExpressionList(TokenType::kSepLParen, TokenType::kSepRParen, false);
			exp = std::make_unique<CallExpression>(std::move(exp), std::move(args));
		}
		else {
			break;
		}
	} while (true);
	return exp;
}



std::unique_ptr<Expression> Parser::ParseExpression1() {
	auto type = lexer_->PeekToken().type();
	if (lexer_->PeekToken().is(TokenType::kSepLParen)) {
		lexer_->NextToken();
		auto exp = ParseExpression();
		lexer_->MatchToken(TokenType::kSepRParen);
		return exp;
	}
	else {
		return ParseLiteral(); // 解析基本表达式（标识符、字面量等）
	}
}



std::vector<std::unique_ptr<Expression>> Parser::ParseExpressionList(TokenType begin, TokenType end, bool allow_comma_end) {
	lexer_->MatchToken(begin);
	std::vector<std::unique_ptr<Expression>> par_list;
	if (!lexer_->PeekToken().is(end)) {
		do {
			// ParseExpression19会解析kSepComma，避免
			par_list.emplace_back(ParseExpression18());
			if (!lexer_->PeekToken().is(TokenType::kSepComma)) {
				break;
			}
			if (allow_comma_end && lexer_->PeekToken().is(end)) {
				break;
			}
			lexer_->NextToken();
		} while (true);
	}
	lexer_->MatchToken(end);
	return par_list;
}

} // namespace compiler
} // namespace parser