#include "lexer.h"

#include <format>
#include <cctype>
#include <algorithm>
#include <stack>

#include <mjs/error.h>

namespace mjs {
namespace compiler {

Lexer::Lexer(std::string_view source)
    : source_(source) {}

char Lexer::NextChar() noexcept {
    if (position_ < source_.size()) {
        return source_[position_++];
    }
    return 0;
}

char Lexer::PeekChar() const noexcept {
    if (position_ < source_.size()) {
        return source_[position_];
    }
    return 0;
}

void Lexer::SkipChar(int count) noexcept {
    position_ += count;
}

bool Lexer::TestString(std::string_view str) const {
    if (position_ + str.size() > source_.size()) {
        return false;
    }
    return source_.compare(position_, str.size(), str) == 0;
}

bool Lexer::TestChar(char c) const {
    return position_ < source_.size() && source_[position_] == c;
}

void Lexer::SkipWhitespaceAndComments() {
    if (in_template_ && !in_template_interpolation_) {
        return;
    }

    while (true) {
        // 跳过空白字符
        while (position_ < source_.size()) {
            char c = source_[position_];
            if (c == ' ' || c == '\t' || c == '\r') {
                ++position_;
            } else if (c == '\n') {
                ++position_;
            } else if (c == '\u2028' || c == '\u2029') {  // 行分隔符和段落分隔符
                ++position_;
            } else {
                break;
            }
        }

        // 检查是否到达源码结束
        if (position_ >= source_.size()) {
            break;
        }

        // 跳过注释
        if (TestString("//")) {
            // 单行注释
            position_ += 2;
            while (position_ < source_.size()) {
                char c = source_[position_];
                ++position_;
                if (c == '\n' || c == '\r' || c == '\u2028' || c == '\u2029') {
                    break;  // 确保跳过行终止符
                }
            }
        } else if (TestString("/*")) {
            // 多行注释
            position_ += 2;
            bool comment_closed = false;
            
            while (position_ < source_.size()) {
                if (source_[position_] == '*' && position_ + 1 < source_.size() && source_[position_ + 1] == '/') {
                    position_ += 2;
                    comment_closed = true;
                    break;
                }
                ++position_;
            }
            
            if (!comment_closed) {
                throw SyntaxError("Unclosed multiline comment");
            }
        } else {
            // 既不是空白也不是注释，退出循环
            break;
        }
    }
}

Token Lexer::PeekToken() {
    if (peek_token_.is(TokenType::kNone)) {
        SourcePos saved_position = position_;
        peek_token_ = ReadNextToken();
        peek_position_ = position_;
        position_ = saved_position;
    }
    return peek_token_;
}

Token Lexer::PeekTokenN(uint32_t n) {
    if (n == 0) {
        throw std::invalid_argument("PeekTokenN: n must be greater than 0");
    }
    
    if (n == 1) {
        return PeekToken();
    }
    
    Checkpoint checkpoint = CreateCheckpoint();
    Token result;
    
    for (uint32_t i = 0; i < n; ++i) {
        result = ReadNextToken();
    }
    
    RewindToCheckpoint(checkpoint);
    return result;
}

Token Lexer::NextToken() {
    if (!peek_token_.is(TokenType::kNone)) {
        Token token = std::move(peek_token_);
        peek_token_ = Token{};
        position_ = peek_position_;
        current_token_ = token;
        return token;
    }
    
    current_token_ = ReadNextToken();
    return current_token_;
}

Token Lexer::MatchToken(TokenType type) {
    Token token = NextToken();
    if (!token.is(type)) {
        throw SyntaxError("Cannot match token, expected token: '{}', actual token: '{}'.",
                         Token::TypeToString(type), Token::TypeToString(token.type()));
    }
    return token;
}

Token Lexer::ReadNextToken() {
    Token token;
    SkipWhitespaceAndComments();

    token.set_pos(GetRawSourcePosition());
    
    // 检查是否到达源码结束
    if (position_ >= source_.size()) {
        token.set_type(TokenType::kEof);
        return token;
    }
    
    char c = NextChar();

    // 处理模板字符串
    if (c == '`') {
        return HandleBacktick(token);
    }
    
    if (in_template_) {
        if (c == '$' && TestChar('{')) {
            return HandleTemplateInterpolation(token);
        } else if (in_template_interpolation_ && c == '}') {
            return HandleTemplateInterpolationEnd(token);
        } else if (!in_template_interpolation_) {
            --position_; // 回退一个字符，以便正确读取模板元素
            token.set_type(TokenType::kTemplateElement);
            token.set_value(ReadString('\0', {"`", "${"}));
            return token;
        }
    }

    // 处理正则表达式字面量
    if (c == '/' && CanStartRegExp()) {
        return HandleRegExp(token);
    }

    // 处理运算符和分隔符
    std::string op_str(1, c);
    auto op_it = Token::operator_map().find(op_str);
    
    // 尝试匹配多字符运算符
    if (op_it != Token::operator_map().end()) {
        return HandleOperator(token, op_str, op_it->second);
    }

    // 处理数字字面量
    if (c == 'N' && TestString("aN")) {
        position_ += 2; // 跳过"aN"
        token.set_type(TokenType::kIdentifier);
        token.set_value("NaN");
        return token;
    } else if (c == 'I' && TestString("nfinity")) {
        position_ += 7; // 跳过"nfinity"
        token.set_type(TokenType::kIdentifier);
        token.set_value("Infinity");
        return token;
    } else if (c == '0') {
        return HandleZeroPrefixedNumber(token);
    } else if (IsDigit(c)) {
        return HandleNumber(token, c);
    }

    // 处理字符串字面量
    if (c == '\'' || c == '\"') {
        token.set_value(ReadString(c));
        token.set_type(TokenType::kString);
        return token;
    }

    // 处理标识符和关键字
    if (c == '_' || IsAlpha(c)) {
        return HandleIdentifierOrKeyword(token, c);
    }

    throw SyntaxError("Cannot parse character: '{}'", c);
}

Token Lexer::HandleBacktick(Token& token) {
    if (in_template_) {
        // 如果已经在模板字符串中，检查是否在插值表达式中
        if (in_template_interpolation_) {
            // 这里理论上应该不需要栈，因为在插值表达式中，才允许嵌套模板字符串，否则就直接结束了

            // 这是嵌套模板的开始
            template_stack_.push(in_template_interpolation_);
            in_template_interpolation_ = false;
        } else {
            // 这是当前模板的结束
            if (!template_stack_.empty()) {
                in_template_interpolation_ = template_stack_.top();
                template_stack_.pop();
            } else {
                in_template_ = false;
            }
        }
    } else {
        // 进入模板字符串模式
        in_template_ = true;
    }
    
    token.set_type(TokenType::kBacktick);
    return token;
}

Token Lexer::HandleTemplateInterpolation(Token& token) {
    SkipChar(1); // 跳过 '{'
    in_template_interpolation_ = true;
    token.set_type(TokenType::kTemplateInterpolationStart);
    return token;
}

Token Lexer::HandleTemplateInterpolationEnd(Token& token) {
    in_template_interpolation_ = false;
    token.set_type(TokenType::kTemplateInterpolationEnd);
    return token;
}

bool Lexer::CanStartRegExp() const {
    return !current_token_.is(TokenType::kIdentifier) && 
           !current_token_.is(TokenType::kInteger) && 
           !current_token_.is(TokenType::kFloat) && 
           !current_token_.is(TokenType::kString) &&
           !current_token_.is(TokenType::kSepRParen) &&
           !current_token_.is(TokenType::kSepRBrack) &&
           !TestChar('/') && !TestChar('*');
}

Token Lexer::HandleRegExp(Token& token) {
    std::string pattern;
    bool in_char_class = false;
    bool escaped = false;
    
    // 读取正则表达式模式
    while (position_ < source_.size()) {
        char next_char = NextChar();
        
        if (next_char == '/' && !escaped && !in_char_class) {
            // 读取标志
            std::string flags;
            while (position_ < source_.size() && 
                   (TestChar('g') || TestChar('i') || 
                    TestChar('m') || TestChar('s') || 
                    TestChar('u') || TestChar('y') || 
                    TestChar('d'))) {
                flags.push_back(NextChar());
            }
            
            token.set_type(TokenType::kRegExp);
            token.set_value(pattern);
            token.set_regex_flags(flags);
            return token;
        }
        
        if (next_char == '[' && !escaped) {
            in_char_class = true;
        } else if (next_char == ']' && !escaped) {
            in_char_class = false;
        } else if (next_char == '\\' && !escaped) {
            escaped = true;
            pattern.push_back(next_char);
            continue;
        } else if (next_char == '\n' || next_char == '\r' || next_char == 0) {
            throw SyntaxError("Unterminated regular expression literal");
        }
        
        pattern.push_back(next_char);
        escaped = false;
    }
    
    throw SyntaxError("Unterminated regular expression literal");
}

Token Lexer::HandleOperator(Token& token, const std::string& op_str, TokenType initial_type) {
    // 尝试匹配更长的运算符
    std::string longer_op = op_str;
    TokenType current_type = initial_type;
    
    while (position_ < source_.size()) {
        longer_op.push_back(source_[position_]);
        auto longer_it = Token::operator_map().find(longer_op);
        
        if (longer_it != Token::operator_map().end()) {
            current_type = longer_it->second;
            ++position_;
        } else {
            longer_op.pop_back(); // 移除最后添加的字符
            break;
        }
    }
    
    token.set_type(current_type);
    return token;
}

Token Lexer::HandleZeroPrefixedNumber(Token& token) {
    token.set_type(TokenType::kInteger);
    std::string value = "0";
    
    if (position_ < source_.size()) {
        char next_char = source_[position_];
        
        if (next_char == 'x' || next_char == 'X') {
            // 十六进制
            return HandleHexNumber(token, value);
        } else if (next_char == 'b' || next_char == 'B') {
            // 二进制
            return HandleBinaryNumber(token, value);
        } else if (next_char == 'o' || next_char == 'O') {
            // 八进制
            return HandleOctalNumber(token, value);
        } else if (IsDigit(next_char) || next_char == '_' || next_char == '.') {
            // 普通数字（以0开头）
            return HandleDecimalNumber(token, value);
        }
    }
    
    token.set_value(value);
    return token;
}

Token Lexer::HandleHexNumber(Token& token, std::string& value) {
    value.push_back(NextChar()); // 添加 'x' 或 'X'
    bool has_digits = false;
    
    while (position_ < source_.size()) {
        if (std::isxdigit(source_[position_])) {
            value.push_back(NextChar());
            has_digits = true;
        } else if (source_[position_] == '_' && has_digits) {
            // 跳过数字分隔符
            NextChar();
        } else {
            break;
        }
    }
    
    if (!has_digits) {
        throw SyntaxError("Invalid hexadecimal number");
    }
    
    // 检查是否为BigInt
    if (TestChar('n')) {
        NextChar();
        token.set_type(TokenType::kBigInt);
    }
    
    token.set_value(std::move(value));
    return token;
}

Token Lexer::HandleBinaryNumber(Token& token, std::string& value) {
    value.push_back(NextChar()); // 添加 'b' 或 'B'
    bool has_digits = false;
    
    while (position_ < source_.size()) {
        if (source_[position_] == '0' || source_[position_] == '1') {
            value.push_back(NextChar());
            has_digits = true;
        } else if (source_[position_] == '_' && has_digits) {
            // 跳过数字分隔符
            NextChar();
        } else {
            break;
        }
    }
    
    if (!has_digits) {
        throw SyntaxError("Invalid binary number");
    }
    
    // 检查是否为BigInt
    if (TestChar('n')) {
        NextChar();
        token.set_type(TokenType::kBigInt);
    }
    
    token.set_value(std::move(value));
    return token;
}

Token Lexer::HandleOctalNumber(Token& token, std::string& value) {
    value.push_back(NextChar()); // 添加 'o' 或 'O'
    bool has_digits = false;
    
    while (position_ < source_.size()) {
        if (source_[position_] >= '0' && source_[position_] <= '7') {
            value.push_back(NextChar());
            has_digits = true;
        } else if (source_[position_] == '_' && has_digits) {
            // 跳过数字分隔符
            NextChar();
        } else {
            break;
        }
    }
    
    if (!has_digits) {
        throw SyntaxError("Invalid octal number");
    }
    
    // 检查是否为BigInt
    if (TestChar('n')) {
        NextChar();
        token.set_type(TokenType::kBigInt);
    }
    
    token.set_value(std::move(value));
    return token;
}

Token Lexer::HandleDecimalNumber(Token& token, std::string& value) {
    bool has_decimal_point = false;
    bool has_exponent = false;
    bool has_digit_after_decimal = false;
    
    while (position_ < source_.size()) {
        if (IsDigit(source_[position_])) {
            value.push_back(NextChar());
            if (has_decimal_point && !has_digit_after_decimal) {
                has_digit_after_decimal = true;
            }
        } else if (source_[position_] == '_' && position_ > 0 && IsDigit(source_[position_ - 1])) {
            // 跳过数字分隔符，但确保前面是数字
            NextChar();
        } else if (source_[position_] == '.' && !has_decimal_point && !has_exponent) {
            has_decimal_point = true;
            token.set_type(TokenType::kFloat);
            value.push_back(NextChar());
        } else if ((source_[position_] == 'e' || source_[position_] == 'E') && !has_exponent) {
            has_exponent = true;
            token.set_type(TokenType::kFloat);
            value.push_back(NextChar());
            
            // 处理指数部分的正负号
            if (position_ < source_.size() && (source_[position_] == '+' || source_[position_] == '-')) {
                value.push_back(NextChar());
            }
            
            // 确保指数部分有数字
            if (position_ >= source_.size() || !IsDigit(source_[position_])) {
                throw SyntaxError("Invalid exponent in number");
            }
        } else {
            break;
        }
    }
    
    // 检查是否为BigInt
    if (TestChar('n')) {
        if (has_decimal_point || has_exponent) {
            throw SyntaxError("BigInt cannot have decimal point or exponent");
        }
        NextChar();
        token.set_type(TokenType::kBigInt);
    }
    
    token.set_value(std::move(value));
    return token;
}

Token Lexer::HandleNumber(Token& token, char first_digit) {
    // 浮点数和整数解析，包括科学计数法
    bool has_decimal_point = false;
    bool has_exponent = false;
    bool has_digit_after_decimal = false;
    token.set_type(TokenType::kInteger);
    std::string value(1, first_digit);
    
    while (position_ < source_.size()) {
        if (IsDigit(source_[position_])) {
            value.push_back(NextChar());
            if (has_decimal_point && !has_digit_after_decimal) {
                has_digit_after_decimal = true;
            }
        } else if (source_[position_] == '_' && position_ > 0 && IsDigit(source_[position_ - 1])) {
            // 跳过数字分隔符，但确保前面是数字
            NextChar();
        } else if (source_[position_] == '.' && !has_decimal_point && !has_exponent) {
            has_decimal_point = true;
            token.set_type(TokenType::kFloat);
            value.push_back(NextChar());
        } else if ((source_[position_] == 'e' || source_[position_] == 'E') && !has_exponent) {
            has_exponent = true;
            token.set_type(TokenType::kFloat);
            value.push_back(NextChar());
            
            // 处理指数部分的正负号
            if (position_ < source_.size() && (source_[position_] == '+' || source_[position_] == '-')) {
                value.push_back(NextChar());
            }
            
            // 确保指数部分有数字
            if (position_ >= source_.size() || !IsDigit(source_[position_])) {
                throw SyntaxError("Invalid exponent in number");
            }
        } else {
            break;
        }
    }
    
    // 检查是否为BigInt
    if (TestChar('n')) {
        if (has_decimal_point || has_exponent) {
            throw SyntaxError("BigInt cannot have decimal point or exponent");
        }
        NextChar();
        token.set_type(TokenType::kBigInt);
    }
    
    token.set_value(std::move(value));
    return token;
}

Token Lexer::HandleIdentifierOrKeyword(Token& token, char first_char) {
    std::string identifier(1, first_char);
    
    while (position_ < source_.size() && IsIdentifierPart(source_[position_])) {
        identifier.push_back(NextChar());
    }

    // 检查是否是关键字
    auto keyword_it = Token::keyword_map().find(identifier);
    if (keyword_it != Token::keyword_map().end()) {
        token.set_type(keyword_it->second);
        return token;
    }

    token.set_type(TokenType::kIdentifier);
    token.set_value(std::move(identifier));
    return token;
}

std::string Lexer::ReadString(char quote_type, std::initializer_list<std::string_view> end_strings) {
    std::string value;
    
    bool is_closed = false;

    while (position_ < source_.size()) {
        // 检查是否匹配结束字符串
        for (const auto& end_string : end_strings) {
            if (!end_string.empty() && TestString(end_string)) {
                return value;
            }
        }
        
        char c = NextChar();
        
        if (c == '\\') {
            // 处理转义字符
            if (position_ >= source_.size()) {
                throw SyntaxError("Incomplete escape sequence in string");
            }
            
            char escaped_char = NextChar();
            switch (escaped_char) {
                case 'n': value.push_back('\n'); break;
                case 't': value.push_back('\t'); break;
                case 'r': value.push_back('\r'); break;
                case 'b': value.push_back('\b'); break;
                case 'f': value.push_back('\f'); break;
                case 'v': value.push_back('\v'); break;
                case '\\': value.push_back('\\'); break;
                case '\"': value.push_back('\"'); break;
                case '\'': value.push_back('\''); break;
                case '`': value.push_back('`'); break;
                case '\n': continue; // 行继续符，忽略
                case 'x': {
                    // 十六进制转义序列 \xXX
                    if (position_ + 2 > source_.size()) {
                        throw SyntaxError("Incomplete hexadecimal escape sequence");
                    }
                    
                    // 解析2位十六进制数字
                    std::string hex_digits;
                    for (int i = 0; i < 2; ++i) {
                        char hex_char = NextChar();
                        if (!std::isxdigit(hex_char)) {
                            throw SyntaxError("Invalid hexadecimal digit in escape sequence");
                        }
                        hex_digits.push_back(hex_char);
                    }
                    
                    // 将十六进制转换为字符
                    unsigned char byte_value = static_cast<unsigned char>(std::stoi(hex_digits, nullptr, 16));
                    value.push_back(static_cast<char>(byte_value));
                    break;
                }
                case 'u': {
                    // Unicode 转义序列
                    if (position_ < source_.size() && source_[position_] == '{') {
                        // 扩展的Unicode转义序列 \u{XXXXX}
                        NextChar(); // 跳过 '{'
                        std::string hex_digits;
                        char hex_char;
                        
                        while (position_ < source_.size() && (hex_char = NextChar()) != '}') {
                            if (!std::isxdigit(hex_char)) {
                                throw SyntaxError("Invalid hexadecimal digit in Unicode escape sequence");
                            }
                            hex_digits.push_back(hex_char);
                        }
                        
                        if (hex_digits.empty() || hex_digits.size() > 6) {
                            throw SyntaxError("Invalid Unicode code point");
                        }
                        
                        // 将十六进制转换为Unicode码点
                        uint32_t code_point = static_cast<uint32_t>(std::stoul(hex_digits, nullptr, 16));
                        
                        if (code_point > 0x10FFFF) {
                            throw SyntaxError("Unicode code point out of range");
                        }
                        
                        // 将Unicode码点转换为UTF-8
                        EncodeUTF8(code_point, value);
                    } else {
                        // 标准的Unicode转义序列 \uXXXX
                        if (position_ + 4 > source_.size()) {
                            throw SyntaxError("Incomplete Unicode escape sequence");
                        }
                        
                        // 解析4位十六进制数字
                        std::string hex_digits;
                        for (int i = 0; i < 4; ++i) {
                            char hex_char = NextChar();
                            if (!std::isxdigit(hex_char)) {
                                throw SyntaxError("Invalid hexadecimal digit in Unicode escape sequence");
                            }
                            hex_digits.push_back(hex_char);
                        }
                        
                        // 将十六进制转换为Unicode码点
                        uint16_t code_unit = static_cast<uint16_t>(std::stoi(hex_digits, nullptr, 16));
                        
                        // 检查是否是代理对的高位部分
                        if (code_unit >= 0xD800 && code_unit <= 0xDBFF) {
                            // 需要一个低位代理
                            if (position_ + 6 <= source_.size() && 
                                source_[position_] == '\\' && 
                                source_[position_ + 1] == 'u') {
                                
                                position_ += 2; // 跳过 '\u'
                                
                                // 解析低位代理
                                std::string low_hex_digits;
                                for (int i = 0; i < 4; ++i) {
                                    char hex_char = NextChar();
                                    if (!std::isxdigit(hex_char)) {
                                        throw SyntaxError("Invalid hexadecimal digit in Unicode surrogate pair");
                                    }
                                    low_hex_digits.push_back(hex_char);
                                }
                                
                                uint16_t low_surrogate = static_cast<uint16_t>(std::stoi(low_hex_digits, nullptr, 16));
                                
                                if (low_surrogate < 0xDC00 || low_surrogate > 0xDFFF) {
                                    throw SyntaxError("Invalid Unicode surrogate pair");
                                }
                                
                                // 计算完整的码点
                                uint32_t code_point = 0x10000 + ((code_unit - 0xD800) << 10) + (low_surrogate - 0xDC00);
                                
                                // 将Unicode码点转换为UTF-8
                                EncodeUTF8(code_point, value);
                            } else {
                                throw SyntaxError("Incomplete Unicode surrogate pair");
                            }
                        } else if (code_unit >= 0xDC00 && code_unit <= 0xDFFF) {
                            throw SyntaxError("Lone Unicode low surrogate");
                        } else {
                            // 单个BMP字符
                            EncodeUTF8(code_unit, value);
                        }
                    }
                    break;
                }
                default:
                    // 其他转义字符直接保留
                    value.push_back(escaped_char);
            }
        } else if (c == quote_type) {
            // 遇到匹配的引号，字符串结束
            is_closed = true;
            break;
        } else if (c == '\0') {
            // 字符串未闭合
            throw SyntaxError("Unterminated string literal");
        } else if ((c == '\n' || c == '\r') && quote_type != 0) {
            // 在非模板字符串中，未转义的换行符是不允许的
            throw SyntaxError("Unterminated string literal");
        } else {
            value.push_back(c); // 普通字符
        }
    }

    if (!is_closed) {
        throw SyntaxError("Unterminated string literal");
    }
    
    return value;
}

void Lexer::EncodeUTF8(uint32_t code_point, std::string& output) {
    if (code_point <= 0x7F) {
        output.push_back(static_cast<char>(code_point));
    } else if (code_point <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | (code_point >> 6)));
        output.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
    } else if (code_point <= 0xFFFF) {
        output.push_back(static_cast<char>(0xE0 | (code_point >> 12)));
        output.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
    } else {
        output.push_back(static_cast<char>(0xF0 | (code_point >> 18)));
        output.push_back(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
    }
}

} // namespace compiler
} // namespace mjs