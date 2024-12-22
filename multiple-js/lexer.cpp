#include "lexer.h"

#include <format>

namespace mjs {

Lexer::Lexer(const char* src)
    : src_(src) {}

Lexer::~Lexer() noexcept = default;

// 获取下一字符
char Lexer::NextChar() noexcept {
    if (idx_ < src_.size()) {
        return src_[idx_++];
    }
    return 0;
}

// 跳过指定字符数
void Lexer::SkipChar(int count) noexcept {
    idx_ += count;
}

bool Lexer::TestStr(const std::string& str) {
    return !src_.compare(idx_, str.size(), str);
}

bool Lexer::TestChar(char c) {
    return src_[idx_] == c;
}


// 前瞻Token
Token Lexer::PeekToken() {
    // 如果没有前瞻过
    if (peek_.Is(TokenType::kNil)) {
        peek_ = NextToken();
    }
    return peek_;
}

// 前瞻自此往后第N个Token
Token Lexer::PeekTokenN(uint32_t n) {
    auto idx = idx_;
    auto line = line_;

    if (n == 1) {
        return PeekToken();
    }
    else if (n > 1 && !peek_.Is(TokenType::kNil)) {
        n -= 1;
    }
    
    for (uint32_t i = 0; i < n; ++i) {
        if (i + 1 == n) {
            auto token = ReadNextToken();
            idx_ = idx;
            line_ = line;
            return token;
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
    if (!peek_.Is(TokenType::kNil)) {
        // 如果有前瞻保存的token，返回前瞻的结果
        Token token = std::move(peek_);
        peek_.set_type(TokenType::kNil);
        return token;
    }
    return ReadNextToken();
}

// 匹配下一Token
Token Lexer::MatchToken(TokenType type) {
    auto token = NextToken();
    if (token.Is(type)) {
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
        else {
            break;
        }

    } while (true);

    token.set_line(line_);

    if (c == 0) {
        token.set_type(TokenType::kEof);
        return token;
    }

    // 根据字符返回对应类型的Token
    switch (c) {
    case ';':
        token.set_type(TokenType::kSepSemi);
        return token;
    case ':':
        token.set_type(TokenType::kSepColon);
        return token;
    case ',':
        token.set_type(TokenType::kSepComma);
        return token;
    case '(':
        token.set_type(TokenType::kSepLParen);
        return token;
    case ')':
        token.set_type(TokenType::kSepRParen);
        return token;
    case '[':
        token.set_type(TokenType::kSepLBrack);
        return token;
    case ']':
        token.set_type(TokenType::kSepRBrack);
        return token;
    case '{':
        token.set_type(TokenType::kSepLCurly);
        return token;
    case '}':
        token.set_type(TokenType::kSepRCurly);
        return token;

    case '+':
        token.set_type(TokenType::kOpAdd);
        return token;
    case '-':
        token.set_type(TokenType::kOpSub);
        return token;
    case '*':
        token.set_type(TokenType::kOpMul);
        return token;
    case '/':
        token.set_type(TokenType::kOpDiv);
        return token;
    case '!':
        if (TestChar('=')) {
            SkipChar(1);
            token.set_type(TokenType::kOpNe);
            return token;
        }
        break;
    case '=':
        if (TestChar('=')) {
            SkipChar(1);
            token.set_type(TokenType::kOpEq);
            return token;
        }
        token.set_type(TokenType::kOpAssign);
        return token;
    case '<':
        if (TestChar('=')) {
            SkipChar(1);
            token.set_type(TokenType::kOpLe);
            return token;
        }
        token.set_type(TokenType::kOpLt);
        return token;

    case '>':
        if (TestChar('=')) {
            SkipChar(1);
            token.set_type(TokenType::kOpGe);
            return token;
        }
        token.set_type(TokenType::kOpGt);
        return token;
    }

    if (c == 'n' && TestStr("ull")) {
        SkipChar(3);
        token.set_type(TokenType::kNull);
    }
    if (c == 'f' && TestStr("alse")) {
        SkipChar(4);
        token.set_type(TokenType::kFalse);
    }
    if (c == 't' && TestStr("rue")) {
        SkipChar(3);
        token.set_type(TokenType::kFalse);
    }

    // Number
    if (IsDigit(c)) {
        token.set_type(TokenType::kNumber);
        token.mutable_str()->push_back(c);
        while (c = NextChar()) {
            if (IsDigit(c)) {
                token.mutable_str()->push_back(c);
            }
            else {
                SkipChar(-1);
                break;
            }
        }
        return token;
    }

    // String
    if (c == '\"') {
        size_t beginPos = idx_;
        size_t endPos = -1;
        if (c == '\'') {
            endPos = src_.find('\'', idx_);
        }
        else if (c == '\"') {
            endPos = src_.find('\"', idx_);
        }

        if (endPos == -1) {
            throw LexerException("incorrect short string");
        }

        idx_ = endPos + 1;

        token.set_str(src_.substr(beginPos, endPos - beginPos));
        token.set_type(TokenType::kString);
        return token;
    }

    // 标识符或关键字
    if (c == '_' || IsAlpha(c)) {
        std::string ident;
        size_t beginPos = idx_ - 1;
        char c = NextChar();
        while (c && (c == '_' || IsAlpha(c) || IsDigit(c))) {
            c = NextChar();
        }
        idx_--;

        size_t endPos = idx_;

        ident = src_.substr(beginPos, endPos - beginPos);

        auto keyword = g_keywords.find(ident);
        if (keyword != g_keywords.end()) {
            token.set_type(keyword->second);
        }
        else {
            token.set_type(TokenType::kIdentifier);
            token.set_str(ident);
        }
        return token;
    }
    throw LexerException("cannot parse token");
}

} // namespace msj