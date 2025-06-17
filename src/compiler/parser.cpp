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
	: lexer_(lexer) {
	if (!lexer_) {
		throw std::invalid_argument("Lexer pointer cannot be null");
	}
}

void Parser::ParseProgram() {
	while (!lexer_->PeekToken().is(TokenType::kEof)) {
		statements_.emplace_back(ParseStatement());
	}
}

std::unique_ptr<Expression> Parser::ParseExpression() {
	return ParseCommaExpression();
}

std::unique_ptr<Expression> Parser::ParseCommaExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseAssignmentOrFunction();
	
	while (lexer_->PeekToken().is(TokenType::kSepComma)) {
		lexer_->NextToken(); // 消耗逗号
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(
			start, end, TokenType::kSepComma, 
			std::move(exp), ParseAssignmentOrFunction()
		);
	}
	
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

	// 类型注解 (如果有)
	if (lexer_->PeekToken().is(TokenType::kSepColon)) {
		ParseTypeAnnotation();
	}

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
	
	// 检查是否是赋值运算符
	switch (op) {
	case TokenType::kOpAssign:      // =
	case TokenType::kOpAdd:         // +=
	case TokenType::kOpSub:         // -=
	case TokenType::kOpMul:         // *=
	case TokenType::kOpDiv:         // /=
	case TokenType::kOpMod:         // %=
	case TokenType::kOpBitAnd:      // &=
	case TokenType::kOpBitOr:       // |=
	case TokenType::kOpBitXor:      // ^=
	case TokenType::kOpShiftLeft:   // <<=
	case TokenType::kOpShiftRight: {// >>=
		// 确保是复合赋值运算符
		if (op != TokenType::kOpAssign &&
			!lexer_->PeekTokenN(2).is(TokenType::kOpAssign)) {
			break;
		}

		lexer_->NextToken(); // 消耗运算符

		// 如果是复合赋值，再消耗一个等号
		if (op != TokenType::kOpAssign) {
			lexer_->NextToken(); // 消耗 =
		}

		auto end = lexer_->GetRawSourcePosition();
		return std::make_unique<AssignmentExpression>(
			start, end, op, std::move(exp), ParseAssignmentExpression()
		);
	}
	default:
		break;
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseTernaryExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseLogicalOrExpression();
	
	// 检查是否是三元运算符 ?:
	if (lexer_->PeekToken().is(TokenType::kSepQuestion)) {
		lexer_->NextToken(); // 消耗 ?
		
		// 解析条件为真时的表达式
		auto consequent = ParseAssignmentExpression();
		
		// 必须有冒号
		lexer_->MatchToken(TokenType::kSepColon);
		
		// 解析条件为假时的表达式
		auto alternate = ParseAssignmentExpression();
		
		auto end = lexer_->GetRawSourcePosition();
		exp = std::make_unique<ConditionalExpression>(
			start, end, std::move(exp), std::move(consequent), std::move(alternate)
		);
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseLogicalOrExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseLogicalAndExpression();
	
	// 处理逻辑或运算符 ||
	while (lexer_->PeekToken().is(TokenType::kOpOr)) {
		auto op = lexer_->NextToken().type();
		auto right = ParseLogicalAndExpression();
		auto end = lexer_->GetRawSourcePosition();
		
		exp = std::make_unique<BinaryExpression>(
			start, end, op, std::move(exp), std::move(right)
		);
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseLogicalAndExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseBitwiseOrExpression();
	
	// 处理逻辑与运算符 &&
	while (lexer_->PeekToken().is(TokenType::kOpAnd)) {
		auto op = lexer_->NextToken().type();
		auto right = ParseBitwiseOrExpression();
		auto end = lexer_->GetRawSourcePosition();
		
		exp = std::make_unique<BinaryExpression>(
			start, end, op, std::move(exp), std::move(right)
		);
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseBitwiseOrExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseBitwiseXorExpression();
	
	// 处理按位或运算符 |
	while (lexer_->PeekToken().is(TokenType::kOpBitOr)) {
		auto op = lexer_->NextToken().type();
		auto right = ParseBitwiseXorExpression();
		auto end = lexer_->GetRawSourcePosition();
		
		exp = std::make_unique<BinaryExpression>(
			start, end, op, std::move(exp), std::move(right)
		);
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseBitwiseXorExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseBitwiseAndExpression();
	
	// 处理按位异或运算符 ^
	while (lexer_->PeekToken().is(TokenType::kOpBitXor)) {
		auto op = lexer_->NextToken().type();
		auto right = ParseBitwiseAndExpression();
		auto end = lexer_->GetRawSourcePosition();
		
		exp = std::make_unique<BinaryExpression>(
			start, end, op, std::move(exp), std::move(right)
		);
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseBitwiseAndExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseEqualityExpression();
	
	// 处理按位与运算符 &
	while (lexer_->PeekToken().is(TokenType::kOpBitAnd)) {
		auto op = lexer_->NextToken().type();
		auto right = ParseEqualityExpression();
		auto end = lexer_->GetRawSourcePosition();
		
		exp = std::make_unique<BinaryExpression>(
			start, end, op, std::move(exp), std::move(right)
		);
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseEqualityExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseRelationalExpression();
	
	// 处理相等性运算符 ==, !=, ===, !==
	while (lexer_->PeekToken().is(TokenType::kOpEq) || 
		   lexer_->PeekToken().is(TokenType::kOpNe) ||
		   lexer_->PeekToken().is(TokenType::kOpStrictEq) ||
		   lexer_->PeekToken().is(TokenType::kOpStrictNe)) {
		auto op = lexer_->NextToken().type();
		auto right = ParseRelationalExpression();
		auto end = lexer_->GetRawSourcePosition();
		
		exp = std::make_unique<BinaryExpression>(
			start, end, op, std::move(exp), std::move(right)
		);
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseRelationalExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseShiftExpression();
	
	// 处理关系运算符 <, >, <=, >=, instanceof, in
	while (lexer_->PeekToken().is(TokenType::kOpLt) || 
		   lexer_->PeekToken().is(TokenType::kOpGt) ||
		   lexer_->PeekToken().is(TokenType::kOpLe) ||
		   lexer_->PeekToken().is(TokenType::kOpGe) ||
		   lexer_->PeekToken().is(TokenType::kKwInstanceof) ||
		   lexer_->PeekToken().is(TokenType::kKwIn)) {
		auto op = lexer_->NextToken().type();
		auto right = ParseShiftExpression();
		auto end = lexer_->GetRawSourcePosition();
		
		exp = std::make_unique<BinaryExpression>(
			start, end, op, std::move(exp), std::move(right)
		);
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseShiftExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseAdditiveExpression();
	
	// 处理移位运算符 <<, >>, >>>
	while (lexer_->PeekToken().is(TokenType::kOpShiftLeft) || 
		   lexer_->PeekToken().is(TokenType::kOpShiftRight) ||
		   lexer_->PeekToken().is(TokenType::kOpUnsignedShiftRight)) {
		auto op = lexer_->NextToken().type();
		auto right = ParseAdditiveExpression();
		auto end = lexer_->GetRawSourcePosition();
		
		exp = std::make_unique<BinaryExpression>(
			start, end, op, std::move(exp), std::move(right)
		);
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseAdditiveExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseMultiplicativeExpression();
	
	// 处理加法和减法运算符 +, -
	while (lexer_->PeekToken().is(TokenType::kOpAdd) || 
		   lexer_->PeekToken().is(TokenType::kOpSub)) {
		auto op = lexer_->NextToken().type();
		auto right = ParseMultiplicativeExpression();
		auto end = lexer_->GetRawSourcePosition();
		
		exp = std::make_unique<BinaryExpression>(
			start, end, op, std::move(exp), std::move(right)
		);
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseMultiplicativeExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseExponentiationExpression();
	
	// 处理乘法、除法和取模运算符 *, /, %
	while (lexer_->PeekToken().is(TokenType::kOpMul) || 
		   lexer_->PeekToken().is(TokenType::kOpDiv) ||
		   lexer_->PeekToken().is(TokenType::kOpMod)) {
		auto op = lexer_->NextToken().type();
		auto right = ParseExponentiationExpression();
		auto end = lexer_->GetRawSourcePosition();
		
		exp = std::make_unique<BinaryExpression>(
			start, end, op, std::move(exp), std::move(right)
		);
	}
	
	return exp;
}

std::unique_ptr<Expression> Parser::ParseExponentiationExpression() {
	auto start = lexer_->GetSourcePosition();
	auto exp = ParseUnaryExpression();
	
	// 处理指数运算符 **（右结合）
	if (lexer_->PeekToken().is(TokenType::kOpPower)) {
		lexer_->NextToken(); // 消耗 **
		
		// 递归处理右侧表达式，因为指数运算符是右结合的
		auto right = ParseExponentiationExpression();
		auto end = lexer_->GetRawSourcePosition();
		
		exp = std::make_unique<BinaryExpression>(
			start, end, TokenType::kOpPower, std::move(exp), std::move(right)
		);
	}
	
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
	case TokenType::kOpAdd:      // +
	case TokenType::kOpSub:      // -
	case TokenType::kOpNot:      // !
	case TokenType::kOpBitNot:   // ~
	case TokenType::kKwTypeof:   // typeof
	case TokenType::kKwVoid:     // void
	case TokenType::kKwDelete:   // delete
	case TokenType::kKwAwait:    // await
		{
			auto op = lexer_->NextToken().type();
			auto argument = ParseUnaryExpression();
			auto end = lexer_->GetRawSourcePosition();
			
			// 特殊处理await表达式
			if (op == TokenType::kKwAwait) {
				return std::make_unique<AwaitExpression>(start, end, std::move(argument));
			}
			
			return std::make_unique<UnaryExpression>(
				start, end, op, std::move(argument), true
			);
		}
	
	case TokenType::kOpInc:      // ++
	case TokenType::kOpDec:      // --
		{
			auto op = lexer_->NextToken().type();
			auto argument = ParseUnaryExpression();
			auto end = lexer_->GetRawSourcePosition();
			
			// 使用特殊的前缀自增/自减标记
			TokenType prefix_op = (op == TokenType::kOpInc) ? 
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
	
	// 检查是否有后缀自增/自减运算符
	if (lexer_->PeekToken().is(TokenType::kOpInc) || 
		lexer_->PeekToken().is(TokenType::kOpDec)) {
		auto op = lexer_->NextToken().type();
		auto end = lexer_->GetRawSourcePosition();
		
		// 使用特殊的后缀自增/自减标记
		TokenType suffix_op = (op == TokenType::kOpInc) ? 
			TokenType::kOpSuffixInc : TokenType::kOpSuffixDec;
			
		return std::make_unique<UnaryExpression>(
			start, end, suffix_op, std::move(exp), false
		);
	}
	
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
		lexer_->NextToken(); // 消耗import关键字
		
		// import必须后跟左括号
		if (!lexer_->PeekToken().is(TokenType::kSepLParen)) {
			throw SyntaxError("Expected '(' after import");
		}
		
		return ParseImportExpression();
	}
	
	// 处理基本表达式（可能是成员表达式的起始部分）
	auto exp = ParsePrimaryExpression();
	
	// 检查是否有成员访问或函数调用
	if (lexer_->PeekToken().is(TokenType::kSepDot) || 
		lexer_->PeekToken().is(TokenType::kSepLBrack) ||
		lexer_->PeekToken().is(TokenType::kSepLParen) ||
		lexer_->PeekToken().is(TokenType::kOpOptionalChain)) {
		return ParseMemberOrCallExpression(std::move(exp), false);
	}
	
	return exp;
}

/**
 * @brief 解析new表达式
 * 
 * new表达式的形式为：new Constructor(args)
 * 
 * @return 解析后的new表达式
 */
std::unique_ptr<Expression> Parser::ParseNewExpression() {
	auto start = lexer_->GetSourcePosition();
	lexer_->NextToken(); // 消耗new关键字
	
	// 解析构造函数表达式
	auto callee = ParseNewImportOrMemberExpression();
	
	// 解析可选的参数列表
	std::vector<std::unique_ptr<Expression>> arguments;
	if (lexer_->PeekToken().is(TokenType::kSepLParen)) {
		lexer_->NextToken(); // 消耗左括号
		
		// 解析参数列表
		if (!lexer_->PeekToken().is(TokenType::kSepRParen)) {
			do {
				if (lexer_->PeekToken().is(TokenType::kSepEllipsis)) {
					// 处理展开运算符 ...arg
					lexer_->NextToken(); // 消耗...
					auto arg = ParseAssignmentExpression();
					auto spread_end = lexer_->GetRawSourcePosition();
					auto spread = std::make_unique<UnaryExpression>(
						arg->start(), spread_end, TokenType::kSepEllipsis, std::move(arg), true
					);
					arguments.push_back(std::move(spread));
				} else {
					arguments.push_back(ParseAssignmentExpression());
				}
			} while (lexer_->PeekToken().is(TokenType::kSepComma) && 
					(lexer_->NextToken(), true)); // 消耗逗号并继续
		}
		
		lexer_->MatchToken(TokenType::kSepRParen); // 确保有右括号
	}
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<NewExpression>(
		start, end, std::move(callee), std::move(arguments)
	);
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
	std::unique_ptr<Expression> left, bool match_lparen) {
	
	// 如果需要匹配左括号，则先消耗它
	if (match_lparen) {
		lexer_->MatchToken(TokenType::kSepLParen);
	}
	
	auto token = lexer_->PeekToken();
	
	// 处理成员访问 obj.prop 或可选链 obj?.prop
	if (token.is(TokenType::kSepDot) || token.is(TokenType::kOpOptionalChain)) {
		// 创建成员表达式
		left = ParseMemberExpression(std::move(left));
		
		// 检查是否有后续的成员访问或函数调用
		token = lexer_->PeekToken();
		if (token.is(TokenType::kSepDot) || 
			token.is(TokenType::kSepLBrack) ||
			token.is(TokenType::kSepLParen) ||
			token.is(TokenType::kOpOptionalChain)) {
			return ParseMemberOrCallExpression(std::move(left), false);
		}
		
		return left;
	}
	
	// 处理计算属性 obj[prop]
	else if (token.is(TokenType::kSepLBrack)) {
		// 创建成员表达式
		left = ParseMemberExpression(std::move(left));
		
		// 检查是否有后续的成员访问或函数调用
		token = lexer_->PeekToken();
		if (token.is(TokenType::kSepDot) || 
			token.is(TokenType::kSepLBrack) ||
			token.is(TokenType::kSepLParen) ||
			token.is(TokenType::kOpOptionalChain)) {
			return ParseMemberOrCallExpression(std::move(left), false);
		}
		
		return left;
	}
	
	// 处理函数调用 func(args)
	else if (token.is(TokenType::kSepLParen)) {
		// 创建调用表达式
		left = ParseCallExpression(std::move(left));
		
		// 检查是否有后续的成员访问或函数调用
		token = lexer_->PeekToken();
		if (token.is(TokenType::kSepDot) || 
			token.is(TokenType::kSepLBrack) ||
			token.is(TokenType::kSepLParen) ||
			token.is(TokenType::kOpOptionalChain)) {
			return ParseMemberOrCallExpression(std::move(left), false);
		}
		
		return left;
	}
	
	// 如果不是成员访问或函数调用，则直接返回左侧表达式
	return left;
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
	auto start = object->start();
	auto token = lexer_->NextToken(); // 消耗. 或 [ 或 ?.
	bool optional = token.is(TokenType::kOpOptionalChain);
	
	if (token.is(TokenType::kSepDot) || optional) {
		// 点访问：object.property
		if (!lexer_->PeekToken().is(TokenType::kIdentifier)) {
			throw SyntaxError("Expected identifier after '.' or '?.'");
		}
		
		// 解析属性名（标识符）
		auto property = ParseIdentifier();
		auto end = lexer_->GetRawSourcePosition();
		
		return std::make_unique<MemberExpression>(
			start, end, std::move(object), std::move(property), 
			false, false, optional
		);
	} else if (token.is(TokenType::kSepLBrack)) {
		// 计算属性：object[property]
		auto property = ParseExpression();
		lexer_->MatchToken(TokenType::kSepRBrack);
		auto end = lexer_->GetRawSourcePosition();
		
		return std::make_unique<MemberExpression>(
			start, end, std::move(object), std::move(property), 
			false, true, false
		);
	} else {
		throw SyntaxError("Expected '.' or '[' in member expression");
	}
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
	auto start = callee->start();
	lexer_->MatchToken(TokenType::kSepLParen);
	
	// 解析参数列表
	std::vector<std::unique_ptr<Expression>> arguments;
	
	if (!lexer_->PeekToken().is(TokenType::kSepRParen)) {
		do {
			if (lexer_->PeekToken().is(TokenType::kSepEllipsis)) {
				// 处理展开运算符 ...arg
				lexer_->NextToken(); // 消耗...
				auto arg = ParseAssignmentExpression();
				auto spread_end = lexer_->GetRawSourcePosition();
				auto spread = std::make_unique<UnaryExpression>(
					arg->start(), spread_end, TokenType::kSepEllipsis, std::move(arg), true
				);
				arguments.push_back(std::move(spread));
			} else {
				arguments.push_back(ParseAssignmentExpression());
			}
		} while (lexer_->PeekToken().is(TokenType::kSepComma) && 
				(lexer_->NextToken(), true)); // 消耗逗号并继续
	}
	
	lexer_->MatchToken(TokenType::kSepRParen);
	auto end = lexer_->GetRawSourcePosition();
	
	return std::make_unique<CallExpression>(
		start, end, std::move(callee), std::move(arguments)
	);
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
	case TokenType::kUndefined:
		lexer_->NextToken();
		return std::make_unique<UndefinedLiteral>(start, lexer_->GetRawSourcePosition());
		
	case TokenType::kNull:
		lexer_->NextToken();
		return std::make_unique<NullLiteral>(start, lexer_->GetRawSourcePosition());
		
	case TokenType::kTrue:
		lexer_->NextToken();
		return std::make_unique<BooleanLiteral>(start, lexer_->GetRawSourcePosition(), true);
		
	case TokenType::kFalse:
		lexer_->NextToken();
		return std::make_unique<BooleanLiteral>(start, lexer_->GetRawSourcePosition(), false);
		
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
		
	case TokenType::kIdentifier:
		return ParseIdentifier();
		
	case TokenType::kKwThis:
		lexer_->NextToken();
		return std::make_unique<ThisExpression>(start, lexer_->GetRawSourcePosition());
		
	case TokenType::kSepLParen: {
		// 括号表达式
		lexer_->NextToken(); // 消耗左括号
		auto exp = ParseExpression();
		lexer_->MatchToken(TokenType::kSepRParen);
		return exp;
	}
		
	case TokenType::kSepLBrack:
		// 数组字面量
		return ParseArrayExpression();
		
	case TokenType::kSepLCurly:
		// 对象字面量
		return ParseObjectExpression();
		
	case TokenType::kBacktick:
		// 模板字符串
		return ParseTemplateLiteral();
		
	default:
		throw SyntaxError("Unexpected token: '{}'", Token::TypeToString(token.type()));
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
	auto start = lexer_->GetSourcePosition();
	auto type = lexer_->PeekToken().type();
	
	switch (type) {
	case TokenType::kKwImport:
		return ParseImportStatement(type);
		
	case TokenType::kKwExport:
		return ParseExportDeclaration(type);
		
	case TokenType::kKwLet:
	case TokenType::kKwConst:
		return ParseVariableDeclaration(type);
		
	case TokenType::kKwIf:
		return ParseIfStatement();
		
	case TokenType::kKwFor:
		return ParseForStatement();
		
	case TokenType::kKwWhile:
		return ParseWhileStatement();
		
	case TokenType::kKwContinue:
		return ParseContinueStatement();
		
	case TokenType::kKwBreak:
		return ParseBreakStatement();
		
	case TokenType::kKwReturn:
		return ParseReturnStatement();
		
	case TokenType::kKwTry:
		return ParseTryStatement();
		
	case TokenType::kKwThrow:
		return ParseThrowStatement();
		
	case TokenType::kSepLCurly:
		return ParseBlockStatement();
		
	case TokenType::kIdentifier:
		// 检查是否是标签语句
		if (lexer_->PeekTokenN(2).is(TokenType::kSepColon)) {
			return ParseLabeledStatement();
		}
		// 否则作为表达式语句处理
		return ParseExpressionStatement();
		
	default:
		return ParseExpressionStatement();
	}
}

std::unique_ptr<Statement> Parser::ParseImportStatement(TokenType type) {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwImport);
	
	// 解析模块名
	std::string source;
	if (lexer_->PeekToken().is(TokenType::kString)) {
		source = lexer_->NextToken().value();
	} else {
		throw SyntaxError("Expected module name string");
	}
	
	// 解析导入名称
	std::string name;
	if (lexer_->PeekToken().is(TokenType::kKwAs)) {
		lexer_->NextToken(); // 消耗 as
		
		if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
			name = lexer_->NextToken().value();
		} else {
			throw SyntaxError("Expected identifier after 'as'");
		}
	}
	
	// 必须以分号结束
	lexer_->MatchToken(TokenType::kSepSemi);
	
	auto end = lexer_->GetRawSourcePosition();
	auto import_decl = std::make_unique<ImportDeclaration>(start, end, std::move(source), std::move(name));
	
	// 保存导入声明
	import_declarations_.push_back(std::unique_ptr<ImportDeclaration>(
		static_cast<ImportDeclaration*>(import_decl->clone())));
	
	return import_decl;
}

std::unique_ptr<ExportDeclaration> Parser::ParseExportDeclaration(TokenType type) {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwExport);
	
	// 解析导出的声明
	std::unique_ptr<Statement> declaration;
	
	if (lexer_->PeekToken().is(TokenType::kKwLet) || 
		lexer_->PeekToken().is(TokenType::kKwConst)) {
		// 导出变量声明
		declaration = ParseVariableDeclaration(lexer_->PeekToken().type());
		auto& var_decl = static_cast<VariableDeclaration&>(*declaration);
		var_decl.set_is_export(true);
	} else if (lexer_->PeekToken().is(TokenType::kKwFunction)) {
		// 导出函数声明
		auto func_start = lexer_->GetSourcePosition();
		lexer_->NextToken(); // 消耗 function
		
		// 函数名
		std::string name;
		if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
			name = lexer_->NextToken().value();
		} else {
			throw SyntaxError("Expected function name");
		}
		
		// 参数列表
		auto params = ParseParameters();
		
		// 函数体
		auto body = ParseBlockStatement();
		
		auto func_end = lexer_->GetRawSourcePosition();
		auto func_expr = std::make_unique<FunctionExpression>(
			func_start, func_end, name, std::move(params), 
			std::move(body), false, false, false
		);
		
		// 设置为导出
		func_expr->set_is_export(true);
		
		// 创建表达式语句
		declaration = std::make_unique<ExpressionStatement>(func_start, func_end, std::move(func_expr));
	} else {
		throw SyntaxError("Unsupported export declaration");
	}
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ExportDeclaration>(start, end, std::move(declaration));
}

std::unique_ptr<VariableDeclaration> Parser::ParseVariableDeclaration(TokenType kind) {
	auto start = lexer_->GetSourcePosition();
	lexer_->NextToken(); // 消耗 let 或 const
	
	// 变量名
	std::string name;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		name = lexer_->NextToken().value();
	} else {
		throw SyntaxError("Expected variable name");
	}
	
	// 类型注解（可选）
	if (lexer_->PeekToken().is(TokenType::kSepColon)) {
		ParseTypeAnnotation();
	}
	
	// 初始化表达式（可选）
	std::unique_ptr<Expression> init;
	if (lexer_->PeekToken().is(TokenType::kOpAssign)) {
		lexer_->NextToken(); // 消耗 =
		init = ParseExpression();
	} else if (kind == TokenType::kKwConst) {
		throw SyntaxError("Const declarations must be initialized");
	}
	
	// 分号
	lexer_->MatchToken(TokenType::kSepSemi);
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<VariableDeclaration>(start, end, std::move(name), std::move(init), kind);
}

std::unique_ptr<IfStatement> Parser::ParseIfStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwIf);
	
	// 条件表达式
	lexer_->MatchToken(TokenType::kSepLParen);
	auto test = ParseExpression();
	lexer_->MatchToken(TokenType::kSepRParen);
	
	// if 分支
	auto consequent = ParseBlockStatement();
	
	// else 分支（可选）
	std::unique_ptr<Statement> alternate;
	if (lexer_->PeekToken().is(TokenType::kKwElse)) {
		lexer_->NextToken(); // 消耗 else
		
		if (lexer_->PeekToken().is(TokenType::kKwIf)) {
			// else if 分支
			alternate = ParseIfStatement();
		} else {
			// else 分支
			alternate = ParseBlockStatement();
		}
	}
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<IfStatement>(start, end, std::move(test), std::move(consequent), std::move(alternate));
}

std::unique_ptr<LabeledStatement> Parser::ParseLabeledStatement() {
	auto start = lexer_->GetSourcePosition();
	
	// 标签名
	std::string label = lexer_->NextToken().value();
	
	// 冒号
	lexer_->MatchToken(TokenType::kSepColon);
	
	// 标签语句
	auto body = ParseStatement();
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<LabeledStatement>(start, end, std::move(label), std::move(body));
}

std::unique_ptr<ForStatement> Parser::ParseForStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwFor);
	
	// 左括号
	lexer_->MatchToken(TokenType::kSepLParen);
	
	// 初始化语句
	std::unique_ptr<Statement> init;
	if (!lexer_->PeekToken().is(TokenType::kSepSemi)) {
		if (lexer_->PeekToken().is(TokenType::kKwLet) || 
			lexer_->PeekToken().is(TokenType::kKwConst)) {
			init = ParseVariableDeclaration(lexer_->PeekToken().type());
		} else {
			auto init_start = lexer_->GetSourcePosition();
			auto init_expr = ParseExpression();
			auto init_end = lexer_->GetRawSourcePosition();
			init = std::make_unique<ExpressionStatement>(init_start, init_end, std::move(init_expr));
			lexer_->MatchToken(TokenType::kSepSemi);
		}
	} else {
		lexer_->NextToken(); // 消耗 ;
	}
	
	// 条件表达式
	std::unique_ptr<Expression> test;
	if (!lexer_->PeekToken().is(TokenType::kSepSemi)) {
		test = ParseExpression();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	
	// 更新表达式
	std::unique_ptr<Expression> update;
	if (!lexer_->PeekToken().is(TokenType::kSepRParen)) {
		update = ParseExpression();
	}
	
	// 右括号
	lexer_->MatchToken(TokenType::kSepRParen);
	
	// 循环体
	auto body = ParseBlockStatement();
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ForStatement>(start, end, std::move(init), std::move(test), std::move(update), std::move(body));
}

std::unique_ptr<WhileStatement> Parser::ParseWhileStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwWhile);
	
	// 条件表达式
	lexer_->MatchToken(TokenType::kSepLParen);
	auto test = ParseExpression();
	lexer_->MatchToken(TokenType::kSepRParen);
	
	// 循环体
	auto body = ParseBlockStatement();
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<WhileStatement>(start, end, std::move(test), std::move(body));
}

std::unique_ptr<ContinueStatement> Parser::ParseContinueStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwContinue);
	
	// 标签（可选）
	std::optional<std::string> label;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		label = lexer_->NextToken().value();
	}
	
	// 分号
	lexer_->MatchToken(TokenType::kSepSemi);
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ContinueStatement>(start, end, std::move(label));
}

std::unique_ptr<BreakStatement> Parser::ParseBreakStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwBreak);
	
	// 标签（可选）
	std::optional<std::string> label;
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		label = lexer_->NextToken().value();
	}
	
	// 分号
	lexer_->MatchToken(TokenType::kSepSemi);
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<BreakStatement>(start, end, std::move(label));
}

std::unique_ptr<ReturnStatement> Parser::ParseReturnStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwReturn);
	
	// 返回值（可选）
	std::unique_ptr<Expression> argument;
	if (!lexer_->PeekToken().is(TokenType::kSepSemi)) {
		argument = ParseExpression();
	}
	
	// 分号
	lexer_->MatchToken(TokenType::kSepSemi);
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ReturnStatement>(start, end, std::move(argument));
}

std::unique_ptr<TryStatement> Parser::ParseTryStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwTry);
	
	// try块
	auto block = ParseBlockStatement();
	
	// catch子句（可选）
	std::unique_ptr<CatchClause> handler;
	if (lexer_->PeekToken().is(TokenType::kKwCatch)) {
		handler = ParseCatchClause();
	}
	
	// finally子句（可选）
	std::unique_ptr<FinallyClause> finalizer;
	if (lexer_->PeekToken().is(TokenType::kKwFinally)) {
		finalizer = ParseFinallyClause();
	}
	
	// try语句必须有catch或finally子句
	if (!handler && !finalizer) {
		throw SyntaxError("Try statement must have either catch or finally clause");
	}
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<TryStatement>(start, end, std::move(block), std::move(handler), std::move(finalizer));
}

std::unique_ptr<CatchClause> Parser::ParseCatchClause() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwCatch);
	
	// 捕获参数
	std::unique_ptr<Identifier> param;
	if (lexer_->PeekToken().is(TokenType::kSepLParen)) {
		lexer_->NextToken(); // 消耗 (
		
		if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
			auto param_start = lexer_->GetSourcePosition();
			auto param_name = lexer_->NextToken().value();
			auto param_end = lexer_->GetRawSourcePosition();
			
			param = std::make_unique<Identifier>(param_start, param_end, std::move(param_name));
		} else {
			throw SyntaxError("Expected identifier in catch clause");
		}
		
		lexer_->MatchToken(TokenType::kSepRParen);
	}
	
	// catch块
	auto body = ParseBlockStatement();
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<CatchClause>(start, end, std::move(param), std::move(body));
}

std::unique_ptr<FinallyClause> Parser::ParseFinallyClause() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwFinally);
	
	// finally块
	auto body = ParseBlockStatement();
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<FinallyClause>(start, end, std::move(body));
}

std::unique_ptr<ThrowStatement> Parser::ParseThrowStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kKwThrow);
	
	// 异常表达式
	auto argument = ParseExpression();
	
	// 分号
	lexer_->MatchToken(TokenType::kSepSemi);
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ThrowStatement>(start, end, std::move(argument));
}

std::unique_ptr<BlockStatement> Parser::ParseBlockStatement() {
	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kSepLCurly);
	
	// 解析块内的语句
	std::vector<std::unique_ptr<Statement>> statements;
	while (!lexer_->PeekToken().is(TokenType::kSepRCurly)) {
		statements.push_back(ParseStatement());
	}
	
	lexer_->MatchToken(TokenType::kSepRCurly);
	auto end = lexer_->GetRawSourcePosition();
	
	return std::make_unique<BlockStatement>(start, end, std::move(statements));
}

std::unique_ptr<ExpressionStatement> Parser::ParseExpressionStatement() {
	auto start = lexer_->GetSourcePosition();
	
	// 解析表达式
	auto expression = ParseExpression();
	
	if (expression->is(compiler::ExpressionType::kFunctionExpression)) {
		if (lexer_->PeekToken().is(TokenType::kSepSemi)) {
			lexer_->NextToken();
		}
	}
	else {
		// 分号
		lexer_->MatchToken(TokenType::kSepSemi);
	}
	
	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<ExpressionStatement>(start, end, std::move(expression));
}

std::vector<std::string> Parser::ParseParameters() {
	lexer_->MatchToken(TokenType::kSepLParen);
	
	std::vector<std::string> parameters;
	
	// 解析参数列表
	if (!lexer_->PeekToken().is(TokenType::kSepRParen)) {
		do {
			if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
				parameters.push_back(lexer_->NextToken().value());
			} else {
				throw SyntaxError("Expected parameter name");
			}
			
			// 如果有逗号，继续解析下一个参数
			if (lexer_->PeekToken().is(TokenType::kSepComma)) {
				lexer_->NextToken(); // 消耗 ,
			} else {
				break;
			}
		} while (true);
	}
	
	lexer_->MatchToken(TokenType::kSepRParen);
	return parameters;
}

std::vector<std::unique_ptr<Expression>> Parser::ParseExpressions(
	TokenType begin, TokenType end, bool allow_comma_end) {
	
	lexer_->MatchToken(begin);
	
	std::vector<std::unique_ptr<Expression>> expressions;
	
	// 解析表达式列表
	if (!lexer_->PeekToken().is(end)) {
		do {
			expressions.push_back(ParseAssignmentExpression());
			
			// 如果有逗号，继续解析下一个表达式
			if (lexer_->PeekToken().is(TokenType::kSepComma)) {
				lexer_->NextToken(); // 消耗 ,
				
				// 如果允许逗号结尾且下一个是结束标记，则退出
				if (allow_comma_end && lexer_->PeekToken().is(end)) {
					break;
				}
			} else {
				break;
			}
		} while (true);
	}
	
	lexer_->MatchToken(end);
	return expressions;
}

std::unique_ptr<TypeAnnotation> Parser::ParseTypeAnnotation() {
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