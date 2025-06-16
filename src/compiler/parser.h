#pragma once

#include <exception>
#include <memory>
#include <vector>
#include <optional>
#include <string_view>

#include <mjs/noncopyable.h>

#include "lexer.h"
#include "statement.h"
#include "expression.h"

namespace mjs {
namespace compiler {

// 前向声明测试类
namespace test {
class ParserTest;
}

/**
 * @class Parser
 * @brief 语法分析器，负责将词法标记序列转换为抽象语法树
 * 
 * 该类实现了一个递归下降的语法分析器，用于解析JavaScript语言的语法结构
 */
class Parser : public noncopyable {
public:
	/**
	 * @brief 构造函数
	 * @param lexer 词法分析器指针
	 */
	explicit Parser(Lexer* lexer);

	/**
	 * @brief 解析整个程序
	 * @throws SyntaxError 如果解析过程中遇到语法错误
	 */
	void ParseProgram();

	/**
	 * @brief 获取解析后的语句列表
	 * @return 语句列表的常量引用
	 */
	[[nodiscard]] const auto& statements() const noexcept { return statements_; }

	/**
	 * @brief 获取导入声明列表
	 * @return 导入声明列表的常量引用
	 */
	[[nodiscard]] const auto& import_declarations() const noexcept { return import_declarations_; }

	// 声明友元类，使其可以访问私有成员
	friend class test::ParserTest;

private:
	// 表达式解析方法
	/**
	 * @brief 解析表达式
	 * @return 解析后的表达式
	 */
	std::unique_ptr<Expression> ParseExpression();
	
	/**
	 * @brief 解析逗号表达式
	 * @return 解析后的表达式
	 */
	std::unique_ptr<Expression> ParseCommaExpression();
	
	/**
	 * @brief 解析赋值表达式或函数表达式
	 * @return 解析后的表达式
	 */
	std::unique_ptr<Expression> ParseAssignmentOrFunction();
	
	/**
	 * @brief 解析yield表达式
	 * @return 解析后的yield表达式
	 */
	std::unique_ptr<YieldExpression> ParseYieldExpression();
	
	/**
	 * @brief 解析函数表达式
	 * @return 解析后的函数表达式
	 */
	std::unique_ptr<Expression> ParseFunctionExpression();
	
	/**
	 * @brief 尝试解析箭头函数
	 * @param start 表达式起始位置
	 * @param is_async 是否是异步函数
	 * @return 解析后的箭头函数表达式，如果不是箭头函数则返回其他表达式
	 */
	std::unique_ptr<Expression> TryParseArrowFunction(SourcePos start, bool is_async);
	
	/**
	 * @brief 解析传统函数
	 * @param start 表达式起始位置
	 * @param is_async 是否是异步函数
	 * @param is_generator 是否是生成器函数
	 * @return 解析后的函数表达式
	 */
	std::unique_ptr<Expression> ParseTraditionalFunction(SourcePos start, bool is_async, bool is_generator);
	
	/**
	 * @brief 解析赋值表达式
	 * @return 解析后的赋值表达式
	 */
	std::unique_ptr<Expression> ParseAssignmentExpression();
	
	/**
	 * @brief 解析三元表达式
	 * @return 解析后的三元表达式
	 */
	std::unique_ptr<Expression> ParseTernaryExpression();
	
	/**
	 * @brief 解析逻辑或表达式
	 * @return 解析后的逻辑或表达式
	 */
	std::unique_ptr<Expression> ParseLogicalOrExpression();
	
	/**
	 * @brief 解析逻辑与表达式
	 * @return 解析后的逻辑与表达式
	 */
	std::unique_ptr<Expression> ParseLogicalAndExpression();
	
	/**
	 * @brief 解析按位或表达式
	 * @return 解析后的按位或表达式
	 */
	std::unique_ptr<Expression> ParseBitwiseOrExpression();
	
	/**
	 * @brief 解析按位异或表达式
	 * @return 解析后的按位异或表达式
	 */
	std::unique_ptr<Expression> ParseBitwiseXorExpression();
	
	/**
	 * @brief 解析按位与表达式
	 * @return 解析后的按位与表达式
	 */
	std::unique_ptr<Expression> ParseBitwiseAndExpression();
	
	/**
	 * @brief 解析相等性表达式
	 * @return 解析后的相等性表达式
	 */
	std::unique_ptr<Expression> ParseEqualityExpression();
	
	/**
	 * @brief 解析关系表达式
	 * @return 解析后的关系表达式
	 */
	std::unique_ptr<Expression> ParseRelationalExpression();
	
	/**
	 * @brief 解析移位表达式
	 * @return 解析后的移位表达式
	 */
	std::unique_ptr<Expression> ParseShiftExpression();
	
	/**
	 * @brief 解析加法表达式
	 * @return 解析后的加法表达式
	 */
	std::unique_ptr<Expression> ParseAdditiveExpression();
	
	/**
	 * @brief 解析乘法表达式
	 * @return 解析后的乘法表达式
	 */
	std::unique_ptr<Expression> ParseMultiplicativeExpression();
	
	/**
	 * @brief 解析指数表达式
	 * @return 解析后的指数表达式
	 */
	std::unique_ptr<Expression> ParseExponentiationExpression();
	
	/**
	 * @brief 解析一元表达式
	 * @return 解析后的一元表达式
	 */
	std::unique_ptr<Expression> ParseUnaryExpression();
	
	/**
	 * @brief 解析后缀表达式
	 * @return 解析后的后缀表达式
	 */
	std::unique_ptr<Expression> ParsePostfixExpression();
	
	/**
	 * @brief 解析new、import或成员表达式
	 * @return 解析后的表达式
	 */
	std::unique_ptr<Expression> ParseNewImportOrMemberExpression();
	
	/**
	 * @brief 解析new表达式
	 * @return 解析后的new表达式
	 */
	std::unique_ptr<Expression> ParseNewExpression();
	
	/**
	 * @brief 解析成员或调用表达式
	 * @param right 表达式右侧
	 * @param match_lparen 是否匹配左括号
	 * @return 解析后的表达式
	 */
	std::unique_ptr<Expression> ParseMemberOrCallExpression(std::unique_ptr<Expression> right, bool match_lparen);
	
	/**
	 * @brief 解析成员表达式
	 * @param object 对象表达式
	 * @return 解析后的成员表达式
	 */
	std::unique_ptr<MemberExpression> ParseMemberExpression(std::unique_ptr<Expression> object);
	
	/**
	 * @brief 解析函数调用表达式
	 * @param callee 被调用的函数表达式
	 * @return 解析后的调用表达式
	 */
	std::unique_ptr<CallExpression> ParseCallExpression(std::unique_ptr<Expression> callee);

	/**
	 * @brief 解析import表达式
	 * @return 解析后的import表达式
	 */
	std::unique_ptr<ImportExpression> ParseImportExpression();
	
	/**
	 * @brief 解析基本表达式
	 * @return 解析后的基本表达式
	 */
	std::unique_ptr<Expression> ParsePrimaryExpression();
	
	/**
	 * @brief 解析数组表达式
	 * @return 解析后的数组表达式
	 */
	std::unique_ptr<ArrayExpression> ParseArrayExpression();
	
	/**
	 * @brief 解析对象表达式
	 * @return 解析后的对象表达式
	 */
	std::unique_ptr<ObjectExpression> ParseObjectExpression();
	
	/**
	 * @brief 解析this表达式
	 * @return 解析后的this表达式
	 */
	std::unique_ptr<ThisExpression> ParseThis();
	
	/**
	 * @brief 解析模板字符串
	 * @return 解析后的模板字符串
	 */
	std::unique_ptr<TemplateLiteral> ParseTemplateLiteral();
	
	/**
	 * @brief 解析字面量
	 * @return 解析后的字面量表达式
	 */
	std::unique_ptr<Expression> ParseLiteral();
	
	/**
	 * @brief 尝试解析字面量
	 * @return 解析后的字面量表达式，如果不是字面量则返回nullptr
	 */
	std::unique_ptr<Expression> TryParseLiteral();
	
	/**
	 * @brief 解析标识符
	 * @return 解析后的标识符
	 */
	std::unique_ptr<Identifier> ParseIdentifier();
	
	// 语句解析方法
	/**
	 * @brief 解析语句
	 * @return 解析后的语句
	 */
	std::unique_ptr<Statement> ParseStatement();

	/**
	 * @brief 解析import语句
	 * @param type 标记类型
	 * @return 解析后的import语句
	 */
	std::unique_ptr<Statement> ParseImportStatement(TokenType type);
	
	/**
	 * @brief 解析export声明
	 * @param type 标记类型
	 * @return 解析后的export声明
	 */
	std::unique_ptr<ExportDeclaration> ParseExportDeclaration(TokenType type);

	/**
	 * @brief 解析变量声明
	 * @param kind 变量类型（let、const等）
	 * @return 解析后的变量声明
	 */
	std::unique_ptr<VariableDeclaration> ParseVariableDeclaration(TokenType kind);

	/**
	 * @brief 解析if语句
	 * @return 解析后的if语句
	 */
	std::unique_ptr<IfStatement> ParseIfStatement();
	
	/**
	 * @brief 解析带标签的语句
	 * @return 解析后的带标签语句
	 */
	std::unique_ptr<LabeledStatement> ParseLabeledStatement();

	/**
	 * @brief 解析for循环语句
	 * @return 解析后的for循环语句
	 */
	std::unique_ptr<ForStatement> ParseForStatement();
	
	/**
	 * @brief 解析while循环语句
	 * @return 解析后的while循环语句
	 */
	std::unique_ptr<WhileStatement> ParseWhileStatement();
	
	/**
	 * @brief 解析continue语句
	 * @return 解析后的continue语句
	 */
	std::unique_ptr<ContinueStatement> ParseContinueStatement();
	
	/**
	 * @brief 解析break语句
	 * @return 解析后的break语句
	 */
	std::unique_ptr<BreakStatement> ParseBreakStatement();
	
	/**
	 * @brief 解析return语句
	 * @return 解析后的return语句
	 */
	std::unique_ptr<ReturnStatement> ParseReturnStatement();

	/**
	 * @brief 解析try语句
	 * @return 解析后的try语句
	 */
	std::unique_ptr<TryStatement> ParseTryStatement();
	
	/**
	 * @brief 解析catch子句
	 * @return 解析后的catch子句
	 */
	std::unique_ptr<CatchClause> ParseCatchClause();
	
	/**
	 * @brief 解析finally子句
	 * @return 解析后的finally子句
	 */
	std::unique_ptr<FinallyClause> ParseFinallyClause();
	
	/**
	 * @brief 解析throw语句
	 * @return 解析后的throw语句
	 */
	std::unique_ptr<ThrowStatement> ParseThrowStatement();

	/**
	 * @brief 解析块语句
	 * @return 解析后的块语句
	 */
	std::unique_ptr<BlockStatement> ParseBlockStatement();
	
	/**
	 * @brief 解析表达式语句
	 * @return 解析后的表达式语句
	 */
	std::unique_ptr<ExpressionStatement> ParseExpressionStatement();

	/**
	 * @brief 解析函数参数列表
	 * @return 参数名称列表
	 */
	std::vector<std::string> ParseParameters();
	
	/**
	 * @brief 解析表达式列表
	 * @param begin 开始标记类型
	 * @param end 结束标记类型
	 * @param allow_comma_end 是否允许逗号结尾
	 * @return 表达式列表
	 */
	std::vector<std::unique_ptr<Expression>> ParseExpressions(TokenType begin, TokenType end, bool allow_comma_end);

	/**
	 * @brief 解析类型注解
	 * @return 解析后的类型注解
	 */
	std::unique_ptr<TypeAnnotation> ParseTypeAnnotation();
	
	/**
	 * @brief 解析联合类型
	 * @return 解析后的联合类型
	 */
	std::unique_ptr<UnionType> ParseUnionType();

private:
	Lexer* lexer_;  ///< 词法分析器指针

	std::vector<std::unique_ptr<Statement>> statements_;  ///< 解析后的语句列表
	std::vector<std::unique_ptr<ImportDeclaration>> import_declarations_;  ///< 导入声明列表
};

} // namespace compiler
} // namespace mjs