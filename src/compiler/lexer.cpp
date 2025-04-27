#include "lexer.h"

#include <format>

namespace mjs {
namespace compiler {

Lexer::Lexer(const char* src)
    : src_(src) {}

Lexer::~Lexer() noexcept = default;

// 获取下一字符
char Lexer::NextChar() noexcept {
    if (pos_ < src_.size()) {
        return src_[pos_++];
    }
    return 0;
}

char Lexer::PeekChar() noexcept {
    if (pos_ < src_.size()) {
        return src_[pos_];
    }
    return 0;
}

// 跳过指定字符数
void Lexer::SkipChar(int count) noexcept {
    pos_ += count;
}

bool Lexer::TestStr(const std::string& str) {
    return !src_.compare(pos_, str.size(), str);
}

bool Lexer::TestChar(char c) {
    return src_[pos_] == c;
}


// 前瞻Token
Token Lexer::PeekToken() {
    // 如果没有前瞻过
    if (peek_.is(TokenType::kNone)) {
        peek_ = NextToken();
    }
    return peek_;
}

// 前瞻自此往后第N个Token
Token Lexer::PeekTokenN(uint32_t n) {
    auto idx = pos_;
    auto line = line_;

    if (n == 1) {
        return PeekToken();
    }
    else if (n > 1 && !peek_.is(TokenType::kNone)) {
        n -= 1;
    }
    
    for (uint32_t i = 0; i < n; ++i) {
        if (i + 1 == n) {
            ReadNextToken();
            pos_ = idx;
            line_ = line;
            return cur_token_;
        }
        ReadNextToken();
    }
    throw std::invalid_argument("n error.");
}

inline static bool IsDigit(char c) {
    return c >= '0' && c <= '9';
}
inline static bool IsAlpha(char c) {
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

// 获取下一Token
Token Lexer::NextToken() {
    if (!peek_.is(TokenType::kNone)) {
        // 如果有前瞻保存的token，返回前瞻的结果
        Token token = std::move(peek_);
        peek_.set_type(TokenType::kNone);
        return token;
    }
    cur_token_ = ReadNextToken();
    return cur_token_;
}

// 匹配下一Token
Token Lexer::MatchToken(TokenType type) {
    auto token = NextToken();
    if (token.is(type)) {
        return token;
    }
    throw LexerException("cannot match token");
}

Token Lexer::ReadNextToken() {
    Token token;
    char c;
    do {
        // 跳过空格和换行
        while (c = NextChar()) {
            if (c == ' ' || c == '\t') {
                continue;
            }
            else if (c == '\n') {
                line_++;
            }
            else {
                break;
            }
        }

        // 跳过注释
        if (c == '/' && TestChar('/')) {
            while ((c = NextChar()) && c != '\n');
        }
        else if (c == '/' && TestChar('*')) {
            // 多行注释
            SkipChar(1);
            bool end = false;
            while (c = NextChar()) {
                if (c == '\n') {
                    line_++;
                }
                else if (c == '*' && TestChar('/')) {
                    SkipChar(1);
                    end = true;
                    break;
                }
            }
            if (!end) {
                // 多行注释未闭合
                throw LexerException("Unfinished multiline comments");
            }
        }
        else {
            break;
        }
    } while (true);

    token.set_line(line_);

    if (c == 0) {
        token.set_type(TokenType::kEof);
        return token;
    }

    // 根据符号返回对应类型的Token
    // 目前没有一个字符找不到但是两个能找到的类型
    auto c_str = std::string(1, c);
    decltype(g_operators)::iterator op_it = g_operators.end();
    do {
        auto it = g_operators.find(c_str);
        if (it == g_operators.end()) {
            break;
        }
        if (c_str.size() > 1) {
            SkipChar(1);
        }
        auto next_c = PeekChar();
        c_str = c_str + std::string(1, next_c);
        op_it = it;

    } while (true);
    if (op_it != g_operators.end()) {
        token.set_type(op_it->second);
        return token;
    }

    // Number
    if (c == 'N' && TestStr("aN")) {
        token.set_type(TokenType::kFloat);
        token.set_str("NaN");
        return token;
    }
    else if (c == 'I' && TestStr("nfinity")) {
        token.set_type(TokenType::kFloat);
        token.set_str("Infinity");
        return token;
    }
    else if (c == '0') {
        token.set_type(TokenType::kInteger);
        token.mutable_str()->push_back(c);
        char next_char = PeekChar();
        if (next_char == 'x' || next_char == 'X') {
            // Hexadecimal
            token.mutable_str()->push_back(NextChar());
            while (std::isxdigit(c = NextChar())) {
                token.mutable_str()->push_back(c);
            }
            SkipChar(-1);
        }
        else if (next_char == 'b' || next_char == 'B') {
            // Binary
            token.mutable_str()->push_back(NextChar());
            while ((c = NextChar()) == '0' || c == '1') {
                token.mutable_str()->push_back(c);
            }
            SkipChar(-1);
        }
        else if (next_char == 'o' || next_char == 'O') {
            // Octal
            token.mutable_str()->push_back(NextChar());
            while ((c = NextChar()) >= '0' && c <= '7') {
                token.mutable_str()->push_back(c);
            }
            SkipChar(-1);
        }
        else {
            // Normal number starting with 0
            while (IsDigit(c = NextChar())) {
                token.mutable_str()->push_back(c);
            }
            SkipChar(-1);
        }
        return token;
    }
    else if (IsDigit(c)) {
        // Float and integer parsing, including scientific notation
        bool point = false, exp = false;
        token.set_type(TokenType::kInteger);
        token.mutable_str()->push_back(c);
        while (c = NextChar()) {
            if (c == '.' && !point && !exp) {
                point = true;
            }
            else if ((c == 'e' || c == 'E') && !exp) {
                exp = true;
                point = true; // After 'e' no more decimal point allowed
                token.mutable_str()->push_back(c);
                char next_char = PeekChar();
                if (next_char == '+' || next_char == '-') {
                    token.mutable_str()->push_back(NextChar());
                }
                continue;
            }
            else if (!IsDigit(c)) {
                SkipChar(-1);
                break;
            }
            token.mutable_str()->push_back(c);
        }
        if (point == true || exp == true) {
            token.set_type(TokenType::kFloat);
        }
        return token;
    }

    // String
    if (c == '\'' || c == '\"') {
        char quote_type = c; // 记录引号类型（单引号或双引号）
        size_t begin_pos = pos_; // 记录字符串开始位置
        std::string str_value;
        while (true) {
            c = NextChar();
            if (c == '\\') {
                // 处理转义字符
                char escapedChar = NextChar();
                switch (escapedChar) {
                case 'n': str_value.push_back('\n'); break;
                case 't': str_value.push_back('\t'); break;
                case 'r': str_value.push_back('\r'); break;
                case '\\': str_value.push_back('\\'); break;
                case '\"': str_value.push_back('\"'); break;
                case '\'': str_value.push_back('\''); break;
                case '\n': continue; // 换行续行
                default:
                    throw LexerException("Invalid escape character");
                }
            }
            else if (c == quote_type) {
                // 遇到匹配的引号，字符串结束
                break;
            }
            else if (c == '\0') {
                // 字符串未闭合
                throw LexerException("Unterminated string literal");
            }
            else {
                str_value.push_back(c); // 普通字符
            }
        }

        token.set_str(str_value);
        token.set_type(TokenType::kString);
        return token;
    }

    // 标识符或关键字
    if (c == '_' || IsAlpha(c)) {
        std::string ident;
        ident.push_back(c);
        while (true) {
            char next_char = PeekChar();
            if (next_char == '_' || IsAlpha(next_char) || IsDigit(next_char)) {
                ident.push_back(NextChar());
            }
            else {
                break;
            }
        }

        if (cur_token_.is(TokenType::kSepDot)) {
            token.set_type(TokenType::kIdentifier);
            token.set_str(ident);
            return token;
        }

        // 是否是关键字
        auto keyword_it = g_keywords.find(ident);
        if (keyword_it != g_keywords.end()) {
            token.set_type(keyword_it->second);
            return token;
        }

        token.set_type(TokenType::kIdentifier);
        token.set_str(ident);
        return token;
        
    }

    throw LexerException("cannot parse token");
}

} // namespace compiler
} // namespace msj