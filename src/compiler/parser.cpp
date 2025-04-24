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

void Parser::ParseProgram() {
	while (!lexer_->PeekToken().is(TokenType::kEof)) {
		src_statements_.emplace_back(ParseStatement());
	}
}


std::unique_ptr<Expression> Parser::ParseExpression() {
	return ParseCommaExpression();
}

std::unique_ptr<Expression> Parser::ParseCommaExpression() {
	auto start = lexer_->pos();
	auto exp = ParseYieldFunctionOrAssignment();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kSepComma) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseYieldFunctionOrAssignment());

	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseYieldFunctionOrAssignment() {
	auto start = lexer_->pos();
	auto type = lexer_->PeekToken().type();
	if (type == TokenType::kKwYield) {
		return ParseYieldExpression();
	}
	else if (type == TokenType::kKwFunction || type == TokenType::kKwAsync) {
		return ParseFunctionOrGeneratorExpression();
	}
	return ParseAssignmentExpression();
}

std::unique_ptr<YieldExpression> Parser::ParseYieldExpression() {
	auto start = lexer_->pos();
	lexer_->NextToken();
	std::unique_ptr<Expression> yielded_value = ParseAssignmentExpression();
	auto end = lexer_->pos();
	return std::make_unique<YieldExpression>(start, end, std::move(yielded_value));
}

std::unique_ptr<FunctionExpression> Parser::ParseFunctionOrGeneratorExpression() {
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
		// ����������
		lexer_->NextToken();
		is_generator = true;
	}
	std::string id;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		id = lexer_->NextToken().str();
	}
	auto params = ParseParNameList();

	ParseTypeAnnotation();

	auto block = ParseBlockStatement();

	auto end = lexer_->pos();
	return std::make_unique<FunctionExpression>(start, end, id, std::move(params), std::move(block), is_generator, is_async, false);
}

std::unique_ptr<Expression> Parser::ParseAssignmentExpression() {
	auto start = lexer_->pos();
	// ��ֵ���ҽ��
	auto exp = ParseTernaryExpression();
	auto op = lexer_->PeekToken().type();
	if (op != TokenType::kOpAssign) {
		return exp;
	}
	lexer_->NextToken();
	auto end = lexer_->pos();
	exp = std::make_unique<AssignmentExpression>(start, end, op, std::move(exp), ParseAssignmentExpression());
	return exp;
}

std::unique_ptr<Expression> Parser::ParseTernaryExpression() {
	auto start = lexer_->pos();
	// ��Ԫ���ҽ��
	auto test = ParseLogicalOrExpression();
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kSepQuestion) {
		return test;
	}
	lexer_->NextToken();
	auto consequent = ParseTernaryExpression();
	lexer_->MatchToken(TokenType::kSepColon);
	auto alternate = ParseTernaryExpression();
	auto end = lexer_->pos();
	return std::make_unique<ConditionalExpression>(start, end, 
		std::move(test), std::move(consequent), std::move(alternate));
}

std::unique_ptr<Expression> Parser::ParseLogicalOrExpression() {
	auto start = lexer_->pos();
	auto exp = ParseLogicalAndExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpOr
			&& op != TokenType::kOpNullishCoalescing) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseLogicalAndExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseLogicalAndExpression() {
	auto start = lexer_->pos();
	auto exp = ParseBitwiseOrExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpAnd) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseBitwiseOrExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseBitwiseOrExpression() {
	auto start = lexer_->pos();
	auto exp = ParseBitwiseXorExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpBitOr) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp),  ParseBitwiseXorExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseBitwiseXorExpression() {
	auto start = lexer_->pos();
	auto exp = ParseBitwiseAndExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpBitXor) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseBitwiseAndExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseBitwiseAndExpression() {
	auto start = lexer_->pos();
	auto exp = ParseEqualityExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpBitAnd) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp),ParseEqualityExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseEqualityExpression() {
	auto start = lexer_->pos();
	auto exp = ParseRelationalExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpNe
			&& op != TokenType::kOpEq
			&& op != TokenType::kOpStrictEq
			&& op != TokenType::kOpStrictNe) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp),  ParseRelationalExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseRelationalExpression() {
	auto start = lexer_->pos();
	auto exp = ParseShiftExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpLt
			&& op != TokenType::kOpLe
			&& op != TokenType::kOpGt
			&& op != TokenType::kOpGe
			&& op != TokenType::kKwIn
			&& op != TokenType::kKwInstanceof) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseShiftExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseShiftExpression() {
	auto start = lexer_->pos();
	auto exp = ParseAdditiveExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpShiftLeft
			&& op != TokenType::kOpShiftRight
			&& op != TokenType::kOpUnsignedShiftRight) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseAdditiveExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseAdditiveExpression() {
	auto start = lexer_->pos();
	auto exp = ParseMultiplicativeExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpAdd
			&& op != TokenType::kOpSub) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp),  ParseMultiplicativeExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseMultiplicativeExpression() {
	auto start = lexer_->pos();
	auto exp = ParseExponentiationExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpMul
			&& op != TokenType::kOpDiv
			&& op != TokenType::kOpMod) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExponentiationExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExponentiationExpression() {
	// .. ** ..���ҽ��
	auto start = lexer_->pos();
	auto exp = ParseUnaryExpression();
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kOpPower) {
		return exp;
	}
	lexer_->NextToken();
	auto end = lexer_->pos();
	exp = std::make_unique<UnaryExpression>(start, end, type, ParseExponentiationExpression());
	return exp;
}

std::unique_ptr<Expression> Parser::ParseUnaryExpression() {
	auto start = lexer_->pos();
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
		exp = std::make_unique<UnaryExpression>(start, lexer_->pos(), token.type(), ParseUnaryExpression());
		break;
	}
	case TokenType::kOpInc: {
		lexer_->NextToken();
		exp = ParseUnaryExpression();
		if (exp->value_category() != ValueCategory::kLValue) {
			throw ParserException("Only use auto inc on lvalue.");
		}
		exp = std::make_unique<UnaryExpression>(start, lexer_->pos(), TokenType::kOpPrefixInc, std::move(exp));
		break;
	}
	case TokenType::kOpDec: {
		lexer_->NextToken();
		exp = ParseUnaryExpression();
		if (exp->value_category() != ValueCategory::kLValue) {
			throw ParserException("Only use auto dec on lvalue.");
		}
		exp = std::make_unique<UnaryExpression>(start, lexer_->pos(), TokenType::kOpPrefixDec, std::move(exp));
		break;
	}
	default:
		exp = ParsePostfixExpression();
	}
	return exp;
}

std::unique_ptr<Expression> Parser::ParsePostfixExpression() {
	auto start = lexer_->pos();
	auto exp = ParseNewImportOrMemberExpression();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpInc
			&& type != TokenType::kOpDec) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->pos();
		exp = std::make_unique<UnaryExpression>(start, end, TokenType::kOpSuffixInc, std::move(exp));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseNewImportOrMemberExpression() {
	auto type = lexer_->PeekToken().type();
	if (type == TokenType::kKwNew) {
		return ParseNewExpression();
	}
	else if (type == TokenType::kKwImport) {
		return ParseImportExpression();
	}
	return ParseMemberOrCallExpression(true);
}

std::unique_ptr<NewExpression> Parser::ParseNewExpression() {
	auto start = lexer_->pos();
	lexer_->NextToken();
	std::unique_ptr<Expression> callee;

	auto type = lexer_->PeekToken().type();
	if (type == TokenType::kKwNew) {
		callee = ParseNewExpression();
	}
	else {
		callee = ParseMemberOrCallExpression(false);
	}

	std::vector<std::unique_ptr<Expression>> arguments;
	if (lexer_->PeekToken().is(TokenType::kSepLParen)) {
		arguments = ParseExpressionList(TokenType::kSepLParen, TokenType::kSepRParen, false);
	}
	auto end = lexer_->pos();
	auto exp = std::make_unique<NewExpression>(start, end, std::move(callee), std::move(arguments));
	return exp;
}

std::unique_ptr<Expression> Parser::ParseMemberOrCallExpression(bool match_lparen) {
	auto exp = ParsePrimaryExpression();
	do {
		auto token = lexer_->PeekToken();
		if (token.is(TokenType::kSepDot) || token.is(TokenType::kSepLBrack)) {
			exp = ParseMemberExpression(std::move(exp));
		}
		else if (match_lparen && token.is(TokenType::kSepLParen)) {
			exp = ParseCallExpression(std::move(exp));
		}
		else {
			break;
		}
	} while (true);
	return exp;
}

std::unique_ptr<MemberExpression> Parser::ParseMemberExpression(std::unique_ptr<Expression> object) {
	auto start = lexer_->pos();

	std::unique_ptr<Expression> member;

	bool computed = false;
	auto token = lexer_->NextToken();
	if (token.is(TokenType::kSepDot)) {
		member = ParseIdentifier();
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

	return std::make_unique<MemberExpression>(start, end,
		std::move(object), std::move(member), is_method_call, computed, false);
}

std::unique_ptr<CallExpression> Parser::ParseCallExpression(std::unique_ptr<Expression> callee) {
	auto start = lexer_->pos();

	auto arguments = ParseExpressionList(TokenType::kSepLParen, TokenType::kSepRParen, false);

	auto end = lexer_->pos();
	return std::make_unique<CallExpression>(start, end, std::move(callee), std::move(arguments));
}

std::unique_ptr<ImportExpression> Parser::ParseImportExpression() {
	auto start = lexer_->pos();
	lexer_->MatchToken(TokenType::kKwImport);
	lexer_->MatchToken(TokenType::kSepLParen);
	auto source = lexer_->MatchToken(TokenType::kString).str();
	lexer_->MatchToken(TokenType::kSepRParen);
	auto end = lexer_->pos();
	return std::make_unique<ImportExpression>(start, end, std::move(source));
}

std::unique_ptr<Expression> Parser::ParsePrimaryExpression() {
	auto type = lexer_->PeekToken().type();
	switch (type) {
	case TokenType::kSepLBrack: {
		return ParseArrayExpression();
	}
	case TokenType::kSepLCurly: {
		return ParseObjectExpression();
	}
	case TokenType::kSepLParen: {
		lexer_->NextToken();
		auto exp = ParseExpression();
		lexer_->MatchToken(TokenType::kSepRParen);
		return exp;
	}
	case TokenType::kIdentifier: {
		return ParseIdentifier();
	}
	case TokenType::kKwThis: {
		return ParseThis();
	}
	default: {
		return ParseLiteral();
	}
	}
}

std::unique_ptr<ArrayExpression> Parser::ParseArrayExpression() {
	auto start = lexer_->pos();
	auto arr_literal = ParseExpressionList(TokenType::kSepLBrack, TokenType::kSepRBrack, true);
	auto end = lexer_->pos();
	return std::make_unique<ArrayExpression>(start, end, std::move(arr_literal));
}

std::unique_ptr<ObjectExpression> Parser::ParseObjectExpression() {
	auto start = lexer_->pos();
	lexer_->MatchToken(TokenType::kSepLCurly);

	std::vector<ObjectExpression::Property> properties;
	if (!lexer_->PeekToken().is(TokenType::kSepRCurly)) {
		do {
			auto ident = lexer_->MatchToken(TokenType::kIdentifier);
			lexer_->MatchToken(TokenType::kSepColon);
			// �������kSepComma
			properties.emplace_back(ObjectExpression::Property{
				.key = ident.str(),
				.value = ParseYieldFunctionOrAssignment()
			});
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
	return std::make_unique<ObjectExpression>(start, end, std::move(properties));
}

std::unique_ptr<ThisExpression> Parser::ParseThis() {
	auto start = lexer_->pos();
	lexer_->MatchToken(TokenType::kKwThis);
	auto end = lexer_->pos();
	return std::make_unique<ThisExpression>(start, end);
}

std::unique_ptr<Expression> Parser::ParseLiteral() {
	auto start = lexer_->pos();
	auto token = lexer_->PeekToken();
	std::unique_ptr<Expression> exp;
	switch (token.type()) {
	case TokenType::kUndefined: {
		lexer_->NextToken();
		exp = std::make_unique<UndefinedLiteral>(start, lexer_->pos());
		break;
	}
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
		exp = std::make_unique<StringLiteral>(start, lexer_->pos(), std::string(token.str()));
		break;
	}
	}

	if (!exp) {
		throw ParserException("Unable to parse expression.");
	}
	return exp;
}

std::unique_ptr<Identifier> Parser::ParseIdentifier() {
	auto start = lexer_->pos();
	auto token = lexer_->MatchToken(TokenType::kIdentifier);
	auto end = lexer_->pos();
	auto exp = std::make_unique<Identifier>(start, end, std::string(token.str()));
	exp->set_value_category(ValueCategory::kLValue);
	return exp;
}


std::unique_ptr<Statement> Parser::ParseStatement() {
	auto token = lexer_->PeekToken();
	switch (token.type()) {
	case TokenType::kKwImport: {
		return ParseImportStatement(token.type());
	}
	case TokenType::kKwExport: {
		return ParseExportDeclaration(token.type());
	}

	case TokenType::kKwLet:
	case TokenType::kKwConst: {
		return ParseVariableDeclaration(token.type());
	}

	case TokenType::kKwIf: {
		return ParseIfStatement();
	}
	case TokenType::kIdentifier: {
		if (lexer_->PeekTokenN(2).is(TokenType::kSepColon)) {
			return ParseLabeledStatement();
		}
		return ParseExpressionStatement();
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

	case TokenType::kKwAsync:
	case TokenType::kKwFunction: {
		// �����ֱ�Ӷ��壬�Ͳ���Ҫ��ӷֺ�
		auto start = lexer_->pos();
		auto exp = ParseFunctionOrGeneratorExpression();
		auto end = lexer_->pos();
		return std::make_unique<ExpressionStatement>(start, end, std::move(exp));
	}
	case TokenType::kKwReturn: {
		return ParseReturnStatement();
	}

	case TokenType::kKwThrow: {
		return ParseThrowStatement();
	}
	case TokenType::kKwTry: {
		return ParseTryStatement();
	}


	case TokenType::kSepLCurly: {
		return ParseBlockStatement();
	}
	default: {
		return ParseExpressionStatement();
	}
	}
}


std::unique_ptr<Statement> Parser::ParseImportStatement(TokenType type) {
	auto start = lexer_->pos();
	auto token = lexer_->PeekTokenN(2);
	if (token.is(TokenType::kOpMul)) {
		lexer_->MatchToken(type);
		auto token = lexer_->NextToken();
		lexer_->MatchToken(TokenType::kKwAs);
		auto module_name = lexer_->MatchToken(TokenType::kIdentifier).str();
		lexer_->MatchToken(TokenType::kKwFrom);

		auto source = lexer_->MatchToken(TokenType::kString).str();

		// ��̬import�ᱻ��������������
		auto end = lexer_->pos();
		auto import_stat = std::make_unique<ImportDeclaration>(start, end, std::move(source), std::move(module_name));
		import_declarations_.emplace_back(std::move(import_stat));

		// ������һ����䷵��
		return ParseStatement();
	}
	else if (token.is(TokenType::kSepLParen)) {
		// ��̬import
		return ParseExpressionStatement();
	}
	else {
		throw ParserException("Unsupported module parsing.");
	}
}

std::unique_ptr<ExportDeclaration> Parser::ParseExportDeclaration(TokenType type) {
	auto start = lexer_->pos();
	lexer_->MatchToken(type);
	auto stat = ParseStatement();
	if (stat->is(StatementType::kExpression)) {
		auto& exp = stat->as<ExpressionStatement>().expression();
		if (exp->is(ExpressionType::kFunctionExpression)) {
			auto& func_decl_exp = exp->as<FunctionExpression>();
			func_decl_exp.set_is_export(true);
			auto end = lexer_->pos();
			return std::make_unique<ExportDeclaration>(start, end, std::move(stat));
		}
	}
	if (stat->is(StatementType::kVariableDeclaration)) {
		auto& var_decl = stat->as<VariableDeclaration>();
		var_decl.set_is_export(true);
	}
	else {
		throw ParserException("Statement that cannot be exported.");
	}
	auto end = lexer_->pos();
	return std::make_unique<ExportDeclaration>(start, end, std::move(stat));
}


std::unique_ptr<VariableDeclaration> Parser::ParseVariableDeclaration(TokenType kind) {
	auto start = lexer_->pos();
	lexer_->MatchToken(kind);
	auto name = lexer_->MatchToken(TokenType::kIdentifier).str();

	ParseTypeAnnotation();

	lexer_->MatchToken(TokenType::kOpAssign);
	auto init = ParseExpression();
	lexer_->MatchToken(TokenType::kSepSemi);
	auto end = lexer_->pos();
	return std::make_unique<VariableDeclaration>(start, end, std::move(name), std::move(init), kind);
}


std::unique_ptr<IfStatement> Parser::ParseIfStatement() {
	auto start = lexer_->pos();
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
	auto end = lexer_->pos();
	return std::make_unique<IfStatement>(start, end, std::move(test), std::move(consequent), std::move(alternate));
}

std::unique_ptr<LabeledStatement> Parser::ParseLabeledStatement() {
	auto start = lexer_->pos();
	auto label_name = lexer_->MatchToken(TokenType::kIdentifier).str();
	lexer_->MatchToken(TokenType::kSepColon);
	auto stat = ParseStatement();
	auto end = lexer_->pos();
	return std::make_unique<LabeledStatement>(start, end, std::move(label_name), std::move(stat));
}


std::unique_ptr<ReturnStatement> Parser::ParseReturnStatement() {
	auto start = lexer_->pos();
	lexer_->MatchToken(TokenType::kKwReturn);
	std::unique_ptr<Expression> exp;
	if (!lexer_->PeekToken().is(TokenType::kSepSemi)) {
		exp = ParseExpression();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	auto end = lexer_->pos();
	return std::make_unique<ReturnStatement>(start, end, std::move(exp));
}


std::unique_ptr<ForStatement> Parser::ParseForStatement() {
	auto start = lexer_->pos();
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

	auto end = lexer_->pos();
	return std::make_unique<ForStatement>(start, end, 
		std::move(initialization), std::move(condition),
		std::move(final_expression), std::move(block));
}

std::unique_ptr<WhileStatement> Parser::ParseWhileStatement() {
	auto start = lexer_->pos();
	lexer_->MatchToken(TokenType::kKwWhile);
	lexer_->MatchToken(TokenType::kSepLParen);
	auto exp = ParseExpression();
	lexer_->MatchToken(TokenType::kSepRParen);
	auto block = ParseBlockStatement();
	auto end = lexer_->pos();
	return std::make_unique<WhileStatement>(start, end, std::move(exp), std::move(block));
}

std::unique_ptr<ContinueStatement> Parser::ParseContinueStatement() {
	auto start = lexer_->pos();
	lexer_->MatchToken(TokenType::kKwContinue);
	std::optional<std::string> label_name;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		label_name = lexer_->NextToken().str();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	auto end = lexer_->pos();
	return std::make_unique<ContinueStatement>(start, end, std::move(label_name));
}

std::unique_ptr<BreakStatement> Parser::ParseBreakStatement() {
	auto start = lexer_->pos();
	lexer_->MatchToken(TokenType::kKwBreak);
	std::optional<std::string> label_name;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		label_name = lexer_->NextToken().str();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	auto end = lexer_->pos();
	return std::make_unique<BreakStatement>(start, end, std::move(label_name));
}


std::unique_ptr<TryStatement> Parser::ParseTryStatement() {
	auto start = lexer_->pos();
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

	auto end = lexer_->pos();
	return std::make_unique<TryStatement>(start, end, std::move(block), std::move(catch_stat), std::move(finally_stat));
}

std::unique_ptr<CatchClause> Parser::ParseCatchClause() {
	auto start = lexer_->pos();
	lexer_->MatchToken(TokenType::kKwCatch);

	lexer_->MatchToken(TokenType::kSepLParen);
	auto exp = ParseIdentifier();
	lexer_->MatchToken(TokenType::kSepRParen);

	auto block = ParseBlockStatement();

	auto end = lexer_->pos();
	return std::make_unique<CatchClause>(start, end, std::move(exp), std::move(block));
}

std::unique_ptr<FinallyClause> Parser::ParseFinallyClause() {
	auto start = lexer_->pos();
	lexer_->MatchToken(TokenType::kKwFinally);
	auto block = ParseBlockStatement();
	auto end = lexer_->pos();
	return std::make_unique<FinallyClause>(start, end, std::move(block));
}

std::unique_ptr<ThrowStatement> Parser::ParseThrowStatement() {
	auto start = lexer_->pos();
	lexer_->MatchToken(TokenType::kKwThrow);
	auto exp = ParseExpression();
	auto end = lexer_->pos();
	return std::make_unique<ThrowStatement>(start, end, std::move(exp));
}


std::unique_ptr<BlockStatement> Parser::ParseBlockStatement() {
	auto start = lexer_->pos();
	lexer_->MatchToken(TokenType::kSepLCurly);

	std::vector<std::unique_ptr<Statement>> stat_list;

	while (!lexer_->PeekToken().is(TokenType::kSepRCurly)) {
		stat_list.push_back(ParseStatement());
	}

	lexer_->MatchToken(TokenType::kSepRCurly);

	auto end = lexer_->pos();
	return std::make_unique<BlockStatement>(start, end, std::move(stat_list));
}

std::unique_ptr<ExpressionStatement> Parser::ParseExpressionStatement() {
	auto start = lexer_->pos();
	if (lexer_->PeekToken().is(TokenType::kSepSemi)) {
		lexer_->NextToken();
		auto end = lexer_->pos();
		return std::make_unique<ExpressionStatement>(start, end, nullptr);
	}
	auto exp = ParseExpression();
	lexer_->MatchToken(TokenType::kSepSemi);
	auto end = lexer_->pos();
	return std::make_unique<ExpressionStatement>(start, end, std::move(exp));
}


std::vector<std::string> Parser::ParseParNameList() {
	lexer_->MatchToken(TokenType::kSepLParen);
	std::vector<std::string> parList;
	if (!lexer_->PeekToken().is(TokenType::kSepRParen)) {
		do {
			parList.push_back(lexer_->MatchToken(TokenType::kIdentifier).str());

			ParseTypeAnnotation();

			if (!lexer_->PeekToken().is(TokenType::kSepComma)) {
				break;
			}
			lexer_->NextToken();
		} while (true);
	}
	lexer_->MatchToken(TokenType::kSepRParen);
	return parList;
}

std::vector<std::unique_ptr<Expression>> Parser::ParseExpressionList(TokenType begin, TokenType end, bool allow_comma_end) {
	lexer_->MatchToken(begin);
	std::vector<std::unique_ptr<Expression>> par_list;
	if (!lexer_->PeekToken().is(end)) {
		do {
			// �������kSepComma
			par_list.emplace_back(ParseYieldFunctionOrAssignment());
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

void Parser::ParseTypeAnnotation() {
	if (lexer_->PeekToken().is(TokenType::kSepColon)) {
		lexer_->NextToken();
		lexer_->MatchToken(TokenType::kIdentifier);
	}
}

} // namespace compiler
} // namespace parser