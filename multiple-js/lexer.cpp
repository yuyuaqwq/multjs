#include "lexer.h"

#include <format>

namespace mjs {

Lexer::Lexer(const char* src)
    : src_(src)
    , look_{ 0, TokenType::kNil } {}

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


// 前瞻下一Token
Token Lexer::LookAHead() {
    if (look_.type == TokenType::kNil) {        // 如果没有前瞻过
        look_ = NextToken();       // 获取
    }
    return look_;
}


inline static bool IsDigit(char c) {
    return c >= '0' && c <= '9';
}
inline static bool IsAlpha(char c) {
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

// 获取下一Token
Token Lexer::NextToken() {
    Token token;
    if (!look_.Is(TokenType::kNil)) {        // 如果有前瞻保存的token
        // 返回前瞻的结果
        token = look_;
        look_.type = TokenType::kNil;
        return token;
    }

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

    

    token.line = line_;

    if (c == 0) {
        token.type = TokenType::kEof;
        return token;
    }

    // 根据字符返回对应类型的Token
    switch (c) {
    case ';':
        token.type = TokenType::kSepSemi;
        return token;
    case ',':
        token.type = TokenType::kSepComma;
        return token;
    case ':':
        if (TestChar('=')) {
            SkipChar(1);
            token.type = TokenType::kOpNewVar;
            return token;
        }
        token.type = TokenType::kSepColon;
        return token;
    case '(':
        token.type = TokenType::kSepLParen;
        return token;
    case ')':
        token.type = TokenType::kSepRParen;
        return token;
    case '[':
        token.type = TokenType::kSepLBrack;
        return token;
    case ']':
        token.type = TokenType::kSepRBrack;
        return token;
    case '{':
        token.type = TokenType::kSepLCurly;
        return token;
    case '}':
        token.type = TokenType::kSepRCurly;
        return token;

    case '+':
        token.type = TokenType::kOpAdd;
        return token;
    case '-':
        token.type = TokenType::kOpSub;
        return token;
    case '*':
        token.type = TokenType::kOpMul;
        return token;
    case '/':
        token.type = TokenType::kOpDiv;
        return token;
    case '!':
        if (TestChar('=')) {
            SkipChar(1);
            token.type = TokenType::kOpNe;
            return token;
        }
        break;
    case '=':
        if (TestChar('=')) {
            SkipChar(1);
            token.type = TokenType::kOpEq;
            return token;
        }
        token.type = TokenType::kOpAssign;
        return token;
    case '<':
        if (TestChar('=')) {
            SkipChar(1);
            token.type = TokenType::kOpLe;
            return token;
        }
        token.type = TokenType::kOpLt;
        return token;

    case '>':
        if (TestChar('=')) {
            SkipChar(1);
            token.type = TokenType::kOpGe;
            return token;
        }
        token.type = TokenType::kOpGt;
        return token;

    }

    
    if (c == 'n' && TestStr("ull")) {
        SkipChar(3);
        token.type = TokenType::kNull;
    }
    if (c == 'f' && TestStr("alse")) {
        SkipChar(4);
        token.type = TokenType::kFalse;
    }
    if (c == 't' && TestStr("rue")) {
        SkipChar(3);
        token.type = TokenType::kFalse;
    }

    // Number
    if (IsDigit(c)) {
        token.type = TokenType::kNumber;
        token.str.push_back(c);
        while (c = NextChar()) {
            if (IsDigit(c)) {
                token.str.push_back(c);
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

        token.str = src_.substr(beginPos, endPos - beginPos);
        token.type = TokenType::kString;
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
            token.type = keyword->second;
        }
        else {
            token.type = TokenType::kIdentifier;
            token.str = ident;
        }
        return token;

    }
    throw LexerException("cannot parse token");
}


// 匹配下一Token
Token Lexer::MatchToken(TokenType type) {
    auto token = NextToken();
    if (token.Is(type)) {
        return token;
    }
    throw LexerException("cannot match token");
}

} // namespace msj