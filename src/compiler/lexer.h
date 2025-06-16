#pragma once

#include <string>
#include <string_view>
#include <stdexcept>
#include <optional>
#include <stack>

#include <mjs/noncopyable.h>
#include <mjs/source.h>

#include "token.h"

namespace mjs {
namespace compiler {

/**
 * @class Lexer
 * @brief 词法分析器，负责将源代码转换为标记（Token）序列
 */
class Lexer : public noncopyable {
public:
	/**
	 * @brief 词法分析器的检查点，用于回溯
	 */
	struct Checkpoint {
		SourcePos position;
		SourcePos peek_position;
		Token current_token;
		Token peek_token;
		bool in_template;
		bool in_template_interpolation;
		std::stack<bool> template_stack;
	};

public:
	/**
	 * @brief 构造函数
	 * @param source 源代码字符串
	 */
	explicit Lexer(std::string_view source);
	
	/**
	 * @brief 析构函数
	 */
	~Lexer() noexcept = default;

	/**
	 * @brief 获取下一个标记但不消耗它
	 * @return 下一个标记
	 */
	Token PeekToken();
	
	/**
	 * @brief 预览第n个标记
	 * @param n 要预览的标记位置（从1开始）
	 * @return 第n个标记
	 * @throws std::invalid_argument 如果n小于1
	 */
	Token PeekTokenN(uint32_t n);
	
	/**
	 * @brief 获取并消耗下一个标记
	 * @return 下一个标记
	 */
	Token NextToken();
	
	/**
	 * @brief 匹配并消耗指定类型的标记
	 * @param type 期望的标记类型
	 * @return 匹配的标记
	 * @throws SyntaxError 如果下一个标记不是期望的类型
	 */
	Token MatchToken(TokenType type);

	/**
	 * @brief 创建当前状态的检查点
	 * @return 检查点对象
	 */
	Checkpoint CreateCheckpoint() const {
		return Checkpoint{
			position_,
			peek_position_,
			current_token_,
			peek_token_,
			in_template_,
			in_template_interpolation_,
			template_stack_
		};
	}

	/**
	 * @brief 回溯到指定的检查点
	 * @param checkpoint 之前创建的检查点
	 */
	void RewindToCheckpoint(const Checkpoint& checkpoint) {
		position_ = checkpoint.position;
		peek_position_ = checkpoint.peek_position;
		current_token_ = checkpoint.current_token;
		peek_token_ = checkpoint.peek_token;
		in_template_ = checkpoint.in_template;
		in_template_interpolation_ = checkpoint.in_template_interpolation;
		template_stack_ = checkpoint.template_stack;
	}

	/**
	 * @brief 获取当前源代码位置（跳过空白字符和注释）
	 * @return 源代码位置
	 */
	SourcePos GetSourcePosition() {
		SkipWhitespaceAndComments();
		return position_;
	}

	/**
	 * @brief 获取当前原始源代码位置（不跳过任何字符）
	 * @return 源代码位置
	 */
	SourcePos GetRawSourcePosition() const {
		return position_;
	}

private:
	/**
	 * @brief 读取下一个字符并前进
	 * @return 下一个字符，如果到达文件末尾则返回0
	 */
	char NextChar() noexcept;
	
	/**
	 * @brief 预览下一个字符但不消耗它
	 * @return 下一个字符，如果到达文件末尾则返回0
	 */
	char PeekChar() const noexcept;
	
	/**
	 * @brief 跳过指定数量的字符
	 * @param count 要跳过的字符数
	 */
	void SkipChar(int count) noexcept;
	
	/**
	 * @brief 测试当前位置是否匹配指定的字符串
	 * @param str 要匹配的字符串
	 * @return 如果匹配则返回true，否则返回false
	 */
	bool TestString(std::string_view str) const;
	
	/**
	 * @brief 测试当前位置是否匹配指定的字符
	 * @param c 要匹配的字符
	 * @return 如果匹配则返回true，否则返回false
	 */
	bool TestChar(char c) const;

	/**
	 * @brief 跳过空白字符和注释
	 */
	void SkipWhitespaceAndComments();

	/**
	 * @brief 读取下一个标记
	 * @return 下一个标记
	 */
	Token ReadNextToken();

	/**
	 * @brief 处理反引号字符
	 * @param token 要填充的标记
	 * @return 处理后的标记
	 */
	Token HandleBacktick(Token& token);

	/**
	 * @brief 处理模板字符串插值开始
	 * @param token 要填充的标记
	 * @return 处理后的标记
	 */
	Token HandleTemplateInterpolation(Token& token);

	/**
	 * @brief 处理模板字符串插值结束
	 * @param token 要填充的标记
	 * @return 处理后的标记
	 */
	Token HandleTemplateInterpolationEnd(Token& token);

	/**
	 * @brief 检查当前上下文是否可以开始正则表达式
	 * @return 如果可以开始正则表达式则返回true，否则返回false
	 */
	bool CanStartRegExp() const;

	/**
	 * @brief 处理正则表达式字面量
	 * @param token 要填充的标记
	 * @return 处理后的标记
	 */
	Token HandleRegExp(Token& token);

	/**
	 * @brief 处理运算符
	 * @param token 要填充的标记
	 * @param op_str 运算符字符串
	 * @param initial_type 初始标记类型
	 * @return 处理后的标记
	 */
	Token HandleOperator(Token& token, const std::string& op_str, TokenType initial_type);

	/**
	 * @brief 处理以0开头的数字
	 * @param token 要填充的标记
	 * @return 处理后的标记
	 */
	Token HandleZeroPrefixedNumber(Token& token);

	/**
	 * @brief 处理十六进制数字
	 * @param token 要填充的标记
	 * @param value 已解析的值（包含前缀）
	 * @return 处理后的标记
	 */
	Token HandleHexNumber(Token& token, std::string& value);

	/**
	 * @brief 处理二进制数字
	 * @param token 要填充的标记
	 * @param value 已解析的值（包含前缀）
	 * @return 处理后的标记
	 */
	Token HandleBinaryNumber(Token& token, std::string& value);

	/**
	 * @brief 处理八进制数字
	 * @param token 要填充的标记
	 * @param value 已解析的值（包含前缀）
	 * @return 处理后的标记
	 */
	Token HandleOctalNumber(Token& token, std::string& value);

	/**
	 * @brief 处理十进制数字
	 * @param token 要填充的标记
	 * @param value 已解析的值
	 * @return 处理后的标记
	 */
	Token HandleDecimalNumber(Token& token, std::string& value);

	/**
	 * @brief 处理数字
	 * @param token 要填充的标记
	 * @param first_digit 第一个数字字符
	 * @return 处理后的标记
	 */
	Token HandleNumber(Token& token, char first_digit);

	/**
	 * @brief 处理标识符或关键字
	 * @param token 要填充的标记
	 * @param first_char 第一个字符
	 * @return 处理后的标记
	 */
	Token HandleIdentifierOrKeyword(Token& token, char first_char);

	/**
	 * @brief 读取字符串字面量
	 * @param quote_type 引号类型（单引号或双引号，如果是模板字符串则为0）
	 * @param end_strings 可选的结束字符串列表（用于模板字符串）
	 * @return 解析后的字符串内容
	 */
	std::string ReadString(char quote_type, std::initializer_list<std::string_view> end_strings = {});

	/**
	 * @brief 将Unicode码点编码为UTF-8
	 * @param code_point Unicode码点
	 * @param output 输出字符串
	 */
	void EncodeUTF8(uint32_t code_point, std::string& output);

	/**
	 * @brief 判断字符是否为数字
	 * @param c 要检查的字符
	 * @return 如果是数字则返回true，否则返回false
	 */
	static bool IsDigit(char c) {
		return c >= '0' && c <= '9';
	}

	/**
	 * @brief 判断字符是否为字母
	 * @param c 要检查的字符
	 * @return 如果是字母则返回true，否则返回false
	 */
	static bool IsAlpha(char c) {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	/**
	 * @brief 判断字符是否可以作为标识符的一部分
	 * @param c 要检查的字符
	 * @return 如果可以作为标识符的一部分则返回true，否则返回false
	 */
	static bool IsIdentifierPart(char c) {
		return IsAlpha(c) || IsDigit(c) || c == '_' || c == '$';
	}

private:
	std::string_view source_;       ///< 源代码
	SourcePos position_ = 0;        ///< 当前位置
	SourcePos peek_position_ = 0;   ///< 预览位置
	Token current_token_;           ///< 当前标记
	Token peek_token_;              ///< 预览标记
	bool in_template_ = false;      ///< 是否在模板字符串中
	bool in_template_interpolation_ = false; ///< 是否在模板字符串插值表达式中
	std::stack<bool> template_stack_; ///< 模板字符串的嵌套栈，保存上一个模板字符串的in_template_interpolation_状态
};

} // namespace compiler
} // namespace mjs