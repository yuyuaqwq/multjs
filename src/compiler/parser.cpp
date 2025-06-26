#include "parser.h"

#include <unordered_set>
#include <stdexcept>

#include <mjs/error.h>

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
		auto statement = ParseStatement();
		if (statement->is(StatementType::kImport)) {
			auto import_declaration = std::unique_ptr<ImportDeclaration>(&statement.release()->as<ImportDeclaration>());
			import_declarations_.emplace_back(std::move(import_declaration));
		}
		else {
			statements_.emplace_back(std::move(statement));
		}
	}
}

std::unique_ptr<Expression> Parser::ParseExpression() {
	return ParseCommaExpression();
}

std::unique_ptr<Expression> Parser::ParseCommaExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseAssignmentOrFunction();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kSepComma) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseAssignmentOrFunction());

	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseAssignmentOrFunction() {
	auto start = lexer_->GetSourcePosition();
	auto type = lexer_->PeekToken().type();
	
	// 处理yield表达式
	if (type == TokenType::kKwYield) {
		return ParseYieldExpression();
	}

	// 处理函数表达式
	switch (type) {
	case TokenType::kIdentifier:
	case TokenType::kSepLParen:
	case TokenType::kKwAsync:
	case TokenType::kKwFunction:
		return ParseFunctionExpression();
	default:
		break;
	}
	
	// 处理赋值表达式
	return ParseAssignmentExpression();
}

std::unique_ptr<YieldExpression> Parser::ParseYieldExpression() {
	auto start = lexer_->GetSourcePosition();
	lexer_->NextToken(); // 消耗yield关键字
	
	std::unique_ptr<Expression> yielded_value = ParseAssignmentExpression();
	auto end = lexer_->GetRawSourcePosition();
	
	return std::make_unique<YieldExpression>(start, end, std::move(yielded_value));
}

std::unique_ptr<Expression> Parser::ParseFunctionExpression() {
	auto start = lexer_->GetSourcePosition();

	// 处理 async 关键字
	bool is_async = false;
	if (lexer_->PeekToken().is(TokenType::kKwAsync)) {
		lexer_->NextToken();

		// 检查 async 后面是否是 function 关键字（普通异步函数）
		if (lexer_->PeekToken().is(TokenType::kKwFunction)) {
			is_async = true;
			return ParseTraditionalFunction(start, is_async, false);
		}

		// 否则可能是异步箭头函数，继续向下处理
		is_async = true;
	}

	// 处理 function 关键字（传统函数）
	if (lexer_->PeekToken().is(TokenType::kKwFunction)) {
		return ParseTraditionalFunction(start, is_async, false);
	}

	// 检查是否是箭头函数 (参数列表或单个参数)
	if (lexer_->PeekToken().is(TokenType::kSepLParen) || 
		lexer_->PeekToken().is(TokenType::kIdentifier)) {
		// 可能是箭头函数或其他表达式
		return TryParseArrowFunction(start, is_async);
	}

	// 不是函数表达式，回退到普通表达式解析
	return ParseAssignmentExpression();
}

std::unique_ptr<Expression> Parser::TryParseArrowFunction(SourcePos start, bool is_async) {
	// 保存当前状态以便回退
	auto checkpoint = lexer_->CreateCheckpoint();

	std::vector<std::string> params;

	// 解析参数
	if (lexer_->PeekToken().is(TokenType::kSepLParen)) {
		params = ParseParameters();
	} else {
		// 单个参数
		params.push_back(lexer_->NextToken().value());
	}

	// 检查是否有箭头 =>
	if (!lexer_->PeekToken().is(TokenType::kSepArrow)) {
		// 不是箭头函数，回退
		lexer_->RewindToCheckpoint(checkpoint);
		return ParseAssignmentExpression();
	}

	// 确认是箭头函数
	lexer_->NextToken(); // 跳过 =>

	// 解析函数体
	std::unique_ptr<Statement> body;
	if (lexer_->PeekToken().is(TokenType::kSepLCurly)) {
		body = ParseBlockStatement();
	} else {
		auto exp_start = lexer_->GetSourcePosition();
		// 避免解析kSepComma
		auto exp = ParseAssignmentOrFunction();
		auto exp_end = lexer_->GetRawSourcePosition();
		body = std::make_unique<ExpressionStatement>(exp_start, exp_end, std::move(exp));
	}

	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ArrowFunctionExpression>(
		start, end, std::move(params), std::move(body), is_async
	);
}

std::unique_ptr<Expression> Parser::ParseTraditionalFunction(SourcePos start, bool is_async, bool is_generator) {
	// 处理传统函数声明 function [name](params) { ... }
	lexer_->MatchToken(TokenType::kKwFunction);

	// 处理生成器函数
	if (lexer_->PeekToken().is(TokenType::kOpMul)) {
		if (is_async) {
			throw SyntaxError("Async generator functions are not supported");
		}
		lexer_->NextToken();
		is_generator = true;
	}

	// 函数名（可选）
	std::string id;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		id = lexer_->NextToken().value();
	}

	// 参数列表
	auto params = ParseParameters();

	// 类型注解
	TryParseTypeAnnotation();

	// 函数体
	auto block = ParseBlockStatement();

	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<FunctionExpression>(
		start, end, id, std::move(params), std::move(block), 
		is_generator, is_async, false
	);
}

std::unique_ptr<Expression> Parser::ParseAssignmentExpression() {
	auto start = lexer_->GetSourcePosition();
	// 赋值，右结合
	auto exp = ParseTernaryExpression();
	auto op = lexer_->PeekToken().type();
	if (op != TokenType::kOpAssign) {
		return exp;
	}
	lexer_->NextToken();
	auto end = lexer_->GetRawSourcePosition();
	exp = std::make_unique<AssignmentExpression>(start, end, op, std::move(exp), ParseAssignmentExpression());
	return exp;
}

std::unique_ptr<Expression> Parser::ParseTernaryExpression() {
	auto start = lexer_->GetSourcePosition();
	// 三元，右结合
	auto test = ParseLogicalOrExpression();
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kSepQuestion) {
		return test;
	}
	lexer_->NextToken();
	auto consequent = ParseTernaryExpression();
	lexer_->MatchToken(TokenType::kSepColon);
	auto alternate = ParseTernaryExpression();
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ConditionalExpression>(start, end,
		std::move(test), std::move(consequent), std::move(alternate));
}

std::unique_ptr<Expression> Parser::ParseLogicalOrExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseLogicalAndExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpOr
			&& op != TokenType::kOpNullishCoalescing) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseLogicalAndExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseLogicalAndExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseBitwiseOrExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpAnd) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseBitwiseOrExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseBitwiseOrExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseBitwiseXorExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpBitOr) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseBitwiseXorExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseBitwiseXorExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseBitwiseAndExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpBitXor) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseBitwiseAndExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseBitwiseAndExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseEqualityExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpBitAnd) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseEqualityExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseEqualityExpression() {
	auto start = lexer_->GetSourcePosition();
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
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseRelationalExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseRelationalExpression() {
	auto start = lexer_->GetSourcePosition();
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
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseShiftExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseShiftExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseAdditiveExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpShiftLeft
			&& op != TokenType::kOpShiftRight
			&& op != TokenType::kOpUnsignedShiftRight) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseAdditiveExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseAdditiveExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseMultiplicativeExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpAdd
			&& op != TokenType::kOpSub) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseMultiplicativeExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseMultiplicativeExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseExponentiationExpression();
	do {
		auto op = lexer_->PeekToken().type();
		if (op != TokenType::kOpMul
			&& op != TokenType::kOpDiv
			&& op != TokenType::kOpMod) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExponentiationExpression());
	} while (true);
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExponentiationExpression() {
	// .. ** ..，右结合
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseUnaryExpression();
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kOpPower) {
		return exp;
	}
	lexer_->NextToken();
	auto end = lexer_->GetRawSourcePosition();
	exp = std::make_unique<UnaryExpression>(start, end, type, ParseExponentiationExpression());
	return exp;
}

/**
 * @brief 解析一元表达式
 * 
 * 一元表达式包括：
 * - 前缀自增/自减: ++x, --x
 * - 一元运算符: +x, -x, !x, ~x, typeof x, void x, delete x
 * - 后缀表达式
 * 
 * @return 解析后的表达式
 */
std::unique_ptr<Expression> Parser::ParseUnaryExpression() {
	auto start = lexer_->GetSourcePosition();
	auto token = lexer_->PeekToken();
	
	// 处理前缀一元运算符
	switch (token.type()) {
	case TokenType::kKwAwait: {    // await
		lexer_->NextToken();
		auto argument = ParseUnaryExpression();
		auto end = lexer_->GetRawSourcePosition();
		return std::make_unique<AwaitExpression>(start, end, std::move(argument));
	}
	case TokenType::kOpAdd:      // +
	case TokenType::kOpSub:      // -
	case TokenType::kOpNot:      // !
	case TokenType::kOpBitNot:   // ~
	case TokenType::kKwTypeof:   // typeof
	case TokenType::kKwVoid:     // void
	case TokenType::kKwDelete: {  // delete
		lexer_->NextToken();
		auto argument = ParseUnaryExpression();
		auto end = lexer_->GetRawSourcePosition();
		return std::make_unique<UnaryExpression>(start, end, token.type(), std::move(argument));
	}
	case TokenType::kOpInc:      // ++
	case TokenType::kOpDec: {     // --
		lexer_->NextToken();
		auto argument = ParseUnaryExpression();

		auto end = lexer_->GetRawSourcePosition();
			
		// 使用特殊的前缀自增/自减标记
		TokenType prefix_op = (token.type() == TokenType::kOpInc) ?
			TokenType::kOpPrefixInc : TokenType::kOpPrefixDec;
				
		return std::make_unique<UnaryExpression>(
			start, end, prefix_op, std::move(argument), true
		);
	}
	default:
		// 如果不是一元运算符，则解析后缀表达式
		return ParsePostfixExpression();
	}
}

/**
 * @brief 解析后缀表达式
 * 
 * 后缀表达式包括：
 * - 后缀自增/自减: x++, x--
 * - 成员访问、函数调用等左值表达式
 * 
 * @return 解析后的表达式
 */
std::unique_ptr<Expression> Parser::ParsePostfixExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseNewImportOrMemberExpression();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpInc
			&& type != TokenType::kOpDec) {
			break;
		}
		lexer_->NextToken();
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<UnaryExpression>(start, end, TokenType::kOpSuffixInc, std::move(exp));
	} while (true);
	return exp;
}

/**
 * @brief 解析new表达式、import表达式或成员表达式
 * 
 * 这个函数处理以下几种表达式：
 * - new表达式：new Constructor(...)
 * - import表达式：import(...)
 * - 成员表达式：object.property, object[property]
 * 
 * @return 解析后的表达式
 */
std::unique_ptr<Expression> Parser::ParseNewImportOrMemberExpression() {
	auto start = lexer_->GetSourcePosition();
	
	// 处理new表达式
	if (lexer_->PeekToken().is(TokenType::kKwNew)) {
		return ParseNewExpression();
	}
	
	// 处理import表达式
	if (lexer_->PeekToken().is(TokenType::kKwImport)) {
		return ParseImportExpression();
	}
	
	return ParseMemberOrCallExpression(nullptr, true);
}

/**
 * @brief 解析new表达式
 * 
 * new表达式的形式为：new Constructor(args)
 * 
 * @return 解析后的new表达式
 */
std::unique_ptr<Expression> Parser::ParseNewExpression() {
	// new new ... 右结合
	auto start = lexer_->GetSourcePosition();
	lexer_->NextToken();
	std::unique_ptr<Expression> callee;

	auto type = lexer_->PeekToken().type();
	if (type == TokenType::kKwNew) {
		callee = ParseNewExpression();
	}
	else {
		callee = ParseMemberOrCallExpression(nullptr, false);
	}

	std::vector<std::unique_ptr<Expression>> arguments;
	if (lexer_->PeekToken().is(TokenType::kSepLParen)) {
		arguments = ParseExpressions(TokenType::kSepLParen, TokenType::kSepRParen, false);
	}

	auto end = lexer_->GetRawSourcePosition();
	auto exp = std::make_unique<NewExpression>(start, end, std::move(callee), std::move(arguments));

	// 后面可能还会跟函数调用之类的
	return ParseMemberOrCallExpression(std::move(exp), true);
}

/**
 * @brief 解析成员表达式或调用表达式
 * 
 * 这个函数处理以下几种表达式：
 * - 成员访问：object.property
 * - 计算属性：object[property]
 * - 函数调用：function(args)
 * - 可选链：object?.property, object?.[property], function?.()
 * 
 * @param left 左侧表达式
 * @param match_lparen 是否需要匹配左括号（用于嵌套调用）
 * @return 解析后的表达式
 */
std::unique_ptr<Expression> Parser::ParseMemberOrCallExpression(
	std::unique_ptr<Expression> right, bool match_lparen) {
	if (!right) {
		right = ParsePrimaryExpression();
	}

	do {
		auto token = lexer_->PeekToken();
		if (token.is(TokenType::kSepDot) || token.is(TokenType::kSepLBrack)) {
			right = ParseMemberExpression(std::move(right));
		}
		else if (match_lparen && token.is(TokenType::kSepLParen)) {
			right = ParseCallExpression(std::move(right));
		}
		else {
			break;
		}
	} while (true);
	return right;
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
std::unique_ptr<MemberExpression> Parser::ParseMemberExpression(std::unique_ptr<Expression> object) {
	auto start = lexer_->GetSourcePosition();

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
		throw SyntaxError("Incorrect member expression.");
	}

	bool is_method_call = false;
	if (lexer_->PeekToken().type() == TokenType::kSepLParen) {
		is_method_call = true;
	}

	auto end = lexer_->GetRawSourcePosition();

	return std::make_unique<MemberExpression>(start, end,
		std::move(object), std::move(member), is_method_call, computed, false);
}

/**
 * @brief 解析函数调用表达式
 * 
 * 函数调用表达式的形式为：callee(arg1, arg2, ...)
 * 
 * @param callee 被调用的函数表达式
 * @return 解析后的调用表达式
 */
std::unique_ptr<CallExpression> Parser::ParseCallExpression(std::unique_ptr<Expression> callee) {
	auto start = lexer_->GetSourcePosition();

	auto arguments = ParseExpressions(TokenType::kSepLParen, TokenType::kSepRParen, false);

	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<CallExpression>(start, end, std::move(callee), std::move(arguments));
}

/**
 * @brief 解析import表达式
 * 
 * import表达式的形式为：import(module_specifier)
 * 
 * @return 解析后的import表达式
 */
std::unique_ptr<ImportExpression> Parser::ParseImportExpression() {
	auto start = lexer_->GetSourcePosition();

	lexer_->MatchToken(TokenType::kKwImport);

	// 解析模块说明符
	lexer_->MatchToken(TokenType::kSepLParen);
	auto source = ParseExpression();
	lexer_->MatchToken(TokenType::kSepRParen);
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ImportExpression>(start, end, std::move(source));
}

/**
 * @brief 解析基本表达式
 * 
 * 基本表达式包括：
 * - 字面量：undefined, null, true, false, 数字, 字符串
 * - 标识符：变量名, 函数名等
 * - this表达式
 * - 括号表达式：(expression)
 * - 数组字面量：[item1, item2, ...]
 * - 对象字面量：{key1: value1, key2: value2, ...}
 * - 模板字符串：`template ${expression} string`
 * 
 * @return 解析后的表达式
 */
std::unique_ptr<Expression> Parser::ParsePrimaryExpression() {
	auto start = lexer_->GetSourcePosition();
	auto token = lexer_->PeekToken();
	
	switch (token.type()) {
	case TokenType::kUndefined: {
		lexer_->NextToken();
		return std::make_unique<UndefinedLiteral>(start, lexer_->GetRawSourcePosition());
	}
	case TokenType::kNull: {
		lexer_->NextToken();
		return std::make_unique<NullLiteral>(start, lexer_->GetRawSourcePosition());
	}
	case TokenType::kTrue: {
		lexer_->NextToken();
		return std::make_unique<BooleanLiteral>(start, lexer_->GetRawSourcePosition(), true);
	}
	case TokenType::kFalse: {
		lexer_->NextToken();
		return std::make_unique<BooleanLiteral>(start, lexer_->GetRawSourcePosition(), false);
	}
	case TokenType::kInteger: {
		lexer_->NextToken();
		int64_t value = std::stoll(token.value(), nullptr, 0);
		return std::make_unique<IntegerLiteral>(start, lexer_->GetRawSourcePosition(), value);
	}
	case TokenType::kFloat: {
		lexer_->NextToken();
		double value = std::stod(token.value());
		return std::make_unique<FloatLiteral>(start, lexer_->GetRawSourcePosition(), value);
	}
	case TokenType::kString: {
		lexer_->NextToken();
		return std::make_unique<StringLiteral>(start, lexer_->GetRawSourcePosition(), 
											  std::string(token.value()));
	}
	case TokenType::kIdentifier: {
		return ParseIdentifier();
	}
	case TokenType::kKwThis: {
		lexer_->NextToken();
		return std::make_unique<ThisExpression>(start, lexer_->GetRawSourcePosition());
	}
	case TokenType::kSepLParen: {
		// 括号表达式
		lexer_->NextToken(); // 消耗左括号
		auto exp = ParseExpression();
		lexer_->MatchToken(TokenType::kSepRParen);
		return exp;
	}
	case TokenType::kSepLBrack: {
		// 数组字面量
		return ParseArrayExpression();
	}
	case TokenType::kSepLCurly: {
		// 对象字面量
		return ParseObjectExpression();
	}
	case TokenType::kBacktick: {
		// 模板字符串
		return ParseTemplateLiteral();
	}
	default: {
		throw SyntaxError("Unexpected token: '{}'", Token::TypeToString(token.type()));
	}
	}
}

/**
 * @brief 解析标识符
 * 
 * @return 解析后的标识符表达式
 */
std::unique_ptr<Identifier> Parser::ParseIdentifier() {
	auto start = lexer_->GetSourcePosition();
	auto token = lexer_->NextToken();
	
	if (!token.is(TokenType::kIdentifier)) {
		throw SyntaxError("Expected identifier, got: '{}'", Token::TypeToString(token.type()));
	}
	
	return std::make_unique<Identifier>(start, lexer_->GetRawSourcePosition(), 
									   std::string(token.value()));
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
		// 如果是直接定义，就不需要添加分号
		auto start = lexer_->GetSourcePosition();
		auto exp = ParseFunctionExpression();
		auto end = lexer_->GetRawSourcePosition();
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
	auto start = lexer_->GetSourcePosition();
	auto token = lexer_->PeekTokenN(2);
	if (token.is(TokenType::kOpMul)) {
		lexer_->MatchToken(type);
		auto token = lexer_->NextToken();
		lexer_->MatchToken(TokenType::kKwAs);
		auto module_name = lexer_->MatchToken(TokenType::kIdentifier).value();
		lexer_->MatchToken(TokenType::kKwFrom);

		auto source = lexer_->MatchToken(TokenType::kString).value();

		lexer_->MatchToken(TokenType::kSepSemi);

		// 静态import会被提升，单独保存
		auto end = lexer_->GetRawSourcePosition();

		auto import_declaration = std::make_unique<ImportDeclaration>(start, end, std::move(source), std::move(module_name));
		return import_declaration;
	}
	else if (token.is(TokenType::kSepLParen)) {
		// 动态import
		return ParseExpressionStatement();
	}
	else {
		throw SyntaxError("Unsupported module parsing.");
	}
}

std::unique_ptr<ExportDeclaration> Parser::ParseExportDeclaration(TokenType type) {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(type);
	auto stat = ParseStatement();
	if (stat->is(StatementType::kExpression)) {
		auto& exp = stat->as<ExpressionStatement>().expression();
		if (exp->is(ExpressionType::kFunctionExpression)) {
			auto& func_decl_exp = exp->as<FunctionExpression>();
			func_decl_exp.set_is_export(true);
			auto end = lexer_->GetRawSourcePosition();
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
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ExportDeclaration>(start, end, std::move(stat));
}

std::unique_ptr<VariableDeclaration> Parser::ParseVariableDeclaration(TokenType kind) {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(kind);
	auto name = lexer_->MatchToken(TokenType::kIdentifier).value();

	TryParseTypeAnnotation();

	lexer_->MatchToken(TokenType::kOpAssign);
	auto init = ParseExpression();
	lexer_->MatchToken(TokenType::kSepSemi);
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<VariableDeclaration>(start, end, std::move(name), std::move(init), kind);
}

std::unique_ptr<IfStatement> Parser::ParseIfStatement() {
	auto start = lexer_->GetSourcePosition();
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
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<IfStatement>(start, end, std::move(test), std::move(consequent), std::move(alternate));
}

std::unique_ptr<LabeledStatement> Parser::ParseLabeledStatement() {
	auto start = lexer_->GetSourcePosition();
	auto label_name = lexer_->MatchToken(TokenType::kIdentifier).value();
	lexer_->MatchToken(TokenType::kSepColon);
	auto stat = ParseStatement();
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<LabeledStatement>(start, end, std::move(label_name), std::move(stat));
}

std::unique_ptr<ForStatement> Parser::ParseForStatement() {
	auto start = lexer_->GetSourcePosition();
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

	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ForStatement>(start, end,
		std::move(initialization), std::move(condition),
		std::move(final_expression), std::move(block));
}

std::unique_ptr<WhileStatement> Parser::ParseWhileStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwWhile);
	lexer_->MatchToken(TokenType::kSepLParen);
	auto exp = ParseExpression();
	lexer_->MatchToken(TokenType::kSepRParen);
	auto block = ParseBlockStatement();
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<WhileStatement>(start, end, std::move(exp), std::move(block));
}

std::unique_ptr<ContinueStatement> Parser::ParseContinueStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwContinue);
	std::optional<std::string> label_name;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		label_name = lexer_->NextToken().value();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ContinueStatement>(start, end, std::move(label_name));
}

std::unique_ptr<BreakStatement> Parser::ParseBreakStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwBreak);
	std::optional<std::string> label_name;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		label_name = lexer_->NextToken().value();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<BreakStatement>(start, end, std::move(label_name));
}


std::unique_ptr<ReturnStatement> Parser::ParseReturnStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwReturn);
	std::unique_ptr<Expression> exp;
	if (!lexer_->PeekToken().is(TokenType::kSepSemi)) {
		exp = ParseExpression();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ReturnStatement>(start, end, std::move(exp));
}

std::unique_ptr<TryStatement> Parser::ParseTryStatement() {
	auto start = lexer_->GetSourcePosition();
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

	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<TryStatement>(start, end, std::move(block), std::move(catch_stat), std::move(finally_stat));
}


std::unique_ptr<CatchClause> Parser::ParseCatchClause() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwCatch);

	lexer_->MatchToken(TokenType::kSepLParen);
	auto exp = ParseIdentifier();
	lexer_->MatchToken(TokenType::kSepRParen);

	auto block = ParseBlockStatement();

	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<CatchClause>(start, end, std::move(exp), std::move(block));
}

std::unique_ptr<FinallyClause> Parser::ParseFinallyClause() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwFinally);
	auto block = ParseBlockStatement();
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<FinallyClause>(start, end, std::move(block));
}


std::unique_ptr<ThrowStatement> Parser::ParseThrowStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwThrow);
	auto exp = ParseExpression();
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ThrowStatement>(start, end, std::move(exp));
}


std::unique_ptr<BlockStatement> Parser::ParseBlockStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kSepLCurly);

	std::vector<std::unique_ptr<Statement>> stat_list;

	while (!lexer_->PeekToken().is(TokenType::kSepRCurly)) {
		stat_list.push_back(ParseStatement());
	}

	lexer_->MatchToken(TokenType::kSepRCurly);

	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<BlockStatement>(start, end, std::move(stat_list));
}

std::unique_ptr<ExpressionStatement> Parser::ParseExpressionStatement() {
	auto start = lexer_->GetSourcePosition();
	if (lexer_->PeekToken().is(TokenType::kSepSemi)) {
		lexer_->NextToken();
		auto end = lexer_->GetRawSourcePosition();
		return std::make_unique<ExpressionStatement>(start, end, nullptr);
	}
	auto exp = ParseExpression();
	lexer_->MatchToken(TokenType::kSepSemi);
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ExpressionStatement>(start, end, std::move(exp));
}


std::vector<std::string> Parser::ParseParameters() {
	lexer_->MatchToken(TokenType::kSepLParen);
	std::vector<std::string> parList;
	if (!lexer_->PeekToken().is(TokenType::kSepRParen)) {
		do {
			parList.push_back(lexer_->MatchToken(TokenType::kIdentifier).value());

			TryParseTypeAnnotation();

			if (!lexer_->PeekToken().is(TokenType::kSepComma)) {
				break;
			}
			lexer_->NextToken();
		} while (true);
	}
	lexer_->MatchToken(TokenType::kSepRParen);
	return parList;
}

std::vector<std::unique_ptr<Expression>> Parser::ParseExpressions(
	TokenType begin, TokenType end, bool allow_comma_end) {
	
	lexer_->MatchToken(begin);
	std::vector<std::unique_ptr<Expression>> par_list;
	if (!lexer_->PeekToken().is(end)) {
		do {
			// 避免解析kSepComma
			par_list.emplace_back(ParseAssignmentOrFunction());
			if (!lexer_->PeekToken().is(TokenType::kSepComma)) {
				break;
			}
			lexer_->NextToken();
			if (allow_comma_end && lexer_->PeekToken().is(end)) {
				break;
			}
		} while (true);
	}
	lexer_->MatchToken(end);
	return par_list;
}

std::unique_ptr<TypeAnnotation> Parser::TryParseTypeAnnotation() {
	if (!lexer_->PeekToken().is(TokenType::kSepColon)) {
		return nullptr;
	}
	
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kSepColon);
	
	// 解析类型
	std::unique_ptr<Type> type;
	
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		// 命名类型
		auto type_start = lexer_->GetSourcePosition();
		auto type_name = lexer_->NextToken().value();
		auto type_end = lexer_->GetRawSourcePosition();
		
		type = std::make_unique<NamedType>(type_start, type_end, std::move(type_name));
	} else if (lexer_->PeekToken().is(TokenType::kSepLParen)) {
		// 联合类型
		type = ParseUnionType();
	} else {
		throw SyntaxError("Invalid type annotation");
	}
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<TypeAnnotation>(start, end, std::move(type));
}

std::unique_ptr<UnionType> Parser::ParseUnionType() {
	auto start = lexer_->GetSourcePosition();
	
	// 解析联合类型的成员
	std::vector<std::unique_ptr<Type>> types;
	
	// 第一个类型
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		auto type_start = lexer_->GetSourcePosition();
		auto type_name = lexer_->NextToken().value();
		auto type_end = lexer_->GetRawSourcePosition();
		
		types.push_back(std::make_unique<NamedType>(type_start, type_end, std::move(type_name)));
	} else {
		throw SyntaxError("Expected type name");
	}
	
	// 后续类型
	while (lexer_->PeekToken().is(TokenType::kOpBitOr)) {
		lexer_->NextToken(); // 消耗 |
		
		if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
			auto type_start = lexer_->GetSourcePosition();
			auto type_name = lexer_->NextToken().value();
			auto type_end = lexer_->GetRawSourcePosition();
			
			types.push_back(std::make_unique<NamedType>(type_start, type_end, std::move(type_name)));
		} else {
			throw SyntaxError("Expected type name after |");
		}
	}
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<UnionType>(start, end, std::move(types));
}

/**
 * @brief 解析数组表达式
 * 
 * 数组表达式的形式为：[element1, element2, ...]
 * 支持稀疏数组（有空洞的数组）和展开运算符
 * 
 * @return 解析后的数组表达式
 */
std::unique_ptr<ArrayExpression> Parser::ParseArrayExpression() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kSepLBrack);
	
	std::vector<std::unique_ptr<Expression>> elements;
	
	// 解析数组元素
	while (!lexer_->PeekToken().is(TokenType::kSepRBrack)) {
		if (lexer_->PeekToken().is(TokenType::kSepComma)) {
			// 处理稀疏数组的空洞
			lexer_->NextToken(); // 消耗逗号
			elements.push_back(nullptr); // 空洞用nullptr表示
			continue;
		}
		
		if (lexer_->PeekToken().is(TokenType::kSepEllipsis)) {
			// 处理展开运算符 ...arr
			lexer_->NextToken(); // 消耗...
			auto arg = ParseAssignmentExpression();
			auto spread_end = lexer_->GetRawSourcePosition();
			auto spread = std::make_unique<UnaryExpression>(
				arg->start(), spread_end, TokenType::kSepEllipsis, std::move(arg), true
			);
			elements.push_back(std::move(spread));
		} else {
			elements.push_back(ParseAssignmentExpression());
		}
		
		// 检查是否有逗号分隔符
		if (lexer_->PeekToken().is(TokenType::kSepComma)) {
			lexer_->NextToken(); // 消耗逗号
		} else {
			break;
		}
	}
	
	lexer_->MatchToken(TokenType::kSepRBrack);
	auto end = lexer_->GetRawSourcePosition();
	
	return std::make_unique<ArrayExpression>(start, end, std::move(elements));
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
std::unique_ptr<ObjectExpression> Parser::ParseObjectExpression() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kSepLCurly);
	
	std::vector<ObjectExpression::Property> properties;
	
	// 解析对象属性
	while (!lexer_->PeekToken().is(TokenType::kSepRCurly)) {
		bool computed = false;
		std::string key;
		std::unique_ptr<Expression> value;
		bool shorthand = false;
		
		// 解析属性键
		if (lexer_->PeekToken().is(TokenType::kSepLBrack)) {
			// 计算属性名: [expr]
			computed = true;
			lexer_->NextToken(); // 消耗 [
			auto key_expr = ParseExpression();
			lexer_->MatchToken(TokenType::kSepRBrack);
			
			// 必须有冒号
			lexer_->MatchToken(TokenType::kSepColon);
			
			// 解析属性值
			value = ParseAssignmentExpression();
		} else if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
			// 标识符作为属性名
			key = lexer_->NextToken().value();
			
			if (lexer_->PeekToken().is(TokenType::kSepColon)) {
				// 普通属性: key: value
				lexer_->NextToken(); // 消耗 :
				value = ParseAssignmentExpression();
			} else {
				// 简写属性: key (等同于 key: key)
				shorthand = true;
				auto id_start = lexer_->GetSourcePosition() - key.length();
				auto id_end = lexer_->GetRawSourcePosition();
				value = std::make_unique<Identifier>(id_start, id_end, std::string(key));
			}
		} else if (lexer_->PeekToken().is(TokenType::kString)) {
			// 字符串作为属性名: "key": value
			key = lexer_->NextToken().value();
			
			// 必须有冒号
			lexer_->MatchToken(TokenType::kSepColon);
			
			// 解析属性值
			value = ParseAssignmentExpression();
		} else {
			throw SyntaxError("Invalid property name");
		}
		
		// 添加属性
		properties.push_back({key, std::move(value), shorthand, computed});
		
		// 如果有逗号，继续解析下一个属性
		if (lexer_->PeekToken().is(TokenType::kSepComma)) {
			lexer_->NextToken(); // 消耗 ,
		} else {
			break;
		}
	}
	
	lexer_->MatchToken(TokenType::kSepRCurly);
	auto end = lexer_->GetRawSourcePosition();
	
	return std::make_unique<ObjectExpression>(start, end, std::move(properties));
}

/**
 * @brief 解析模板字符串
 * 
 * 模板字符串的形式为：`template ${expr1} string ${expr2}`
 * 
 * @return 解析后的模板字符串表达式
 */
std::unique_ptr<TemplateLiteral> Parser::ParseTemplateLiteral() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kBacktick);
	
	std::vector<std::unique_ptr<Expression>> expressions;
	
	// 解析模板字符串内容
	while (!lexer_->PeekToken().is(TokenType::kBacktick)) {
		if (lexer_->PeekToken().is(TokenType::kTemplateElement)) {
			// 普通文本部分
			auto text = lexer_->NextToken().value();
			auto text_start = lexer_->GetSourcePosition() - text.length();
			auto text_end = lexer_->GetRawSourcePosition();
			
			expressions.push_back(
				std::make_unique<StringLiteral>(text_start, text_end, std::move(text))
			);
		} else if (lexer_->PeekToken().is(TokenType::kTemplateInterpolationStart)) {
			// 插值表达式部分 ${expr}
			lexer_->NextToken(); // 消耗 ${
			
			auto expr = ParseExpression();
			expressions.push_back(std::move(expr));
			
			lexer_->MatchToken(TokenType::kTemplateInterpolationEnd); // 匹配 }
		} else {
			throw SyntaxError("Invalid template literal");
		}
	}
	
	lexer_->MatchToken(TokenType::kBacktick); // 匹配结束的 `
	auto end = lexer_->GetRawSourcePosition();
	
	return std::make_unique<TemplateLiteral>(start, end, std::move(expressions));
}

} // namespace compiler
} // namespace mjs