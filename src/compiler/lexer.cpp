#include "lexer.h"

#include <format>
#include <cctype>
#include <algorithm>

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
    if (in_template_) {
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
            while (position_ < source_.size() && source_[position_] != '\n') {
                ++position_;
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
        in_template_ = !in_template_;
        token.set_type(TokenType::kBacktick);
        return token;
    }
    
    if (in_template_) {
        if (c == '$' && TestChar('{')) {
            if (in_template_interpolation_) {
                throw SyntaxError("Nested interpolation expressions are not allowed");
            }
            SkipChar(1); // 跳过 '{'
            in_template_interpolation_ = true;
            token.set_type(TokenType::kTemplateInterpolationStart);
            return token;
        } else if (in_template_interpolation_ && c == '}') {
            in_template_interpolation_ = false;
            token.set_type(TokenType::kTemplateInterpolationEnd);
            return token;
        } else if (!in_template_interpolation_) {
            --position_; // 回退一个字符，以便正确读取模板元素
            token.set_type(TokenType::kTemplateElement);
            token.set_value(ReadString('\0', {"`", "${"}));
            return token;
        }
    }

    // 处理运算符和分隔符
    std::string op_str(1, c);
    auto op_it = Token::operator_map().find(op_str);
    
    // 尝试匹配多字符运算符
    if (op_it != Token::operator_map().end()) {
        // 尝试匹配更长的运算符
        std::string longer_op = op_str;
        TokenType current_type = op_it->second;
        
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

    // 处理数字字面量
    if (c == 'N' && TestString("aN")) {
        position_ += 2; // 跳过"aN"
        token.set_type(TokenType::kFloat);
        token.set_value("NaN");
        return token;
    } else if (c == 'I' && TestString("nfinity")) {
        position_ += 7; // 跳过"nfinity"
        token.set_type(TokenType::kFloat);
        token.set_value("Infinity");
        return token;
    } else if (c == '0') {
        token.set_type(TokenType::kInteger);
        std::string value(1, c);
        
        if (position_ < source_.size()) {
            char next_char = source_[position_];
            
            if (next_char == 'x' || next_char == 'X') {
                // 十六进制
                value.push_back(NextChar());
                while (position_ < source_.size() && std::isxdigit(source_[position_])) {
                    value.push_back(NextChar());
                }
            } else if (next_char == 'b' || next_char == 'B') {
                // 二进制
                value.push_back(NextChar());
                while (position_ < source_.size() && (source_[position_] == '0' || source_[position_] == '1')) {
                    value.push_back(NextChar());
                }
            } else if (next_char == 'o' || next_char == 'O') {
                // 八进制
                value.push_back(NextChar());
                while (position_ < source_.size() && source_[position_] >= '0' && source_[position_] <= '7') {
                    value.push_back(NextChar());
                }
            } else if (IsDigit(next_char)) {
                // 普通数字（以0开头）
                while (position_ < source_.size() && IsDigit(source_[position_])) {
                    value.push_back(NextChar());
                }
            }
        }
        
        token.set_value(std::move(value));
        return token;
    } else if (IsDigit(c)) {
        // 浮点数和整数解析，包括科学计数法
        bool has_decimal_point = false;
        bool has_exponent = false;
        token.set_type(TokenType::kInteger);
        std::string value(1, c);
        
        while (position_ < source_.size()) {
            char next_char = source_[position_];
            
            if (next_char == '.' && !has_decimal_point && !has_exponent) {
                has_decimal_point = true;
                token.set_type(TokenType::kFloat);
                value.push_back(NextChar());
            } else if ((next_char == 'e' || next_char == 'E') && !has_exponent) {
                has_exponent = true;
                token.set_type(TokenType::kFloat);
                value.push_back(NextChar());
                
                // 处理指数部分的正负号
                if (position_ < source_.size() && (source_[position_] == '+' || source_[position_] == '-')) {
                    value.push_back(NextChar());
                }
            } else if (IsDigit(next_char)) {
                value.push_back(NextChar());
            } else {
                break;
            }
        }
        
        token.set_value(std::move(value));
        return token;
    }

    // 处理字符串字面量
    if (c == '\'' || c == '\"') {
        token.set_value(ReadString(c));
        token.set_type(TokenType::kString);
        return token;
    }

    // 处理标识符和关键字
    if (c == '_' || IsAlpha(c)) {
        std::string identifier(1, c);
        
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

    throw SyntaxError("Cannot parse character: '{}'", c);
}

std::string Lexer::ReadString(char quote_type, std::initializer_list<std::string_view> end_strings) {
    std::string value;
    
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
                case '\\': value.push_back('\\'); break;
                case '\"': value.push_back('\"'); break;
                case '\'': value.push_back('\''); break;
                case '`': value.push_back('`'); break;
                case '\n': continue; // 行继续符，忽略
                case 'u': {
                    // Unicode 转义序列
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
                    char16_t code_point = static_cast<char16_t>(std::stoi(hex_digits, nullptr, 16));
                    
                    // 简单处理：将Unicode码点转换为UTF-8
                    if (code_point <= 0x7F) {
                        value.push_back(static_cast<char>(code_point));
                    } else if (code_point <= 0x7FF) {
                        value.push_back(static_cast<char>(0xC0 | (code_point >> 6)));
                        value.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
                    } else {
                        value.push_back(static_cast<char>(0xE0 | (code_point >> 12)));
                        value.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
                        value.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
                    }
                    break;
                }
                default:
                    throw SyntaxError("Invalid escape character: '{}'", escaped_char);
            }
        } else if (c == quote_type) {
            // 遇到匹配的引号，字符串结束
            break;
        } else if (c == '\0') {
            // 字符串未闭合
            throw SyntaxError("Unterminated string literal");
        } else {
            value.push_back(c); // 普通字符
        }
    }
    
    return value;
}

} // namespace compiler
} // namespace mjs