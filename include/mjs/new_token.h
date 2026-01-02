#pragma once

#include <memory>
#include <string>

#include <mjs/source_define.h>
#include <mjs/token_base.h>

namespace mjs {
namespace compiler {

/**
 * @class NewToken
 * @brief 新的Token类，使用具体的Token子类
 */
class NewToken {
public:
    /**
     * @brief 默认构造函数
     */
    NewToken() = default;

    /**
     * @brief 构造函数
     * @param token 具体的Token子类
     */
    explicit NewToken(std::unique_ptr<TokenBase> token) : token_(std::move(token)) {}

    /**
     * @brief 检查标记类型是否匹配
     * @param type 要检查的类型
     * @return 如果类型匹配则返回true，否则返回false
     */
    [[nodiscard]] bool is(TokenType type) const noexcept {
        return token_ && token_->type() == type;
    }

    /**
     * @brief 获取标记在源代码中的位置
     * @return 源代码位置
     */
    [[nodiscard]] SourceBytePosition pos() const noexcept {
        return token_ ? token_->pos() : 0;
    }

    /**
     * @brief 设置标记在源代码中的位置
     * @param position 源代码位置
     */
    void set_pos(SourceBytePosition position) {
        if (token_) {
            token_->set_pos(position);
        }
    }

    /**
     * @brief 获取标记类型
     * @return 标记类型
     */
    [[nodiscard]] TokenType type() const noexcept {
        return token_ ? token_->type() : TokenType::kNone;
    }

    /**
     * @brief 设置标记类型
     * @param type 标记类型
     */
    void set_type(TokenType type) {
        if (token_) {
            token_->set_type(type);
        }
    }

    /**
     * @brief 获取标记值
     * @return 标记值的常量引用
     */
    [[nodiscard]] const std::string& value() const noexcept {
        static const std::string empty;
        return token_ ? token_->value() : empty;
    }

    /**
     * @brief 设置标记值
     * @param value 标记值
     */
    void set_value(std::string value) {
        if (token_) {
            token_->set_value(std::move(value));
        }
    }

    /**
     * @brief 获取正则表达式标志
     * @return 正则表达式标志的常量引用
     */
    [[nodiscard]] const std::string& regex_flags() const noexcept {
        static const std::string empty;
        return token_ ? token_->regex_flags() : empty;
    }

    /**
     * @brief 设置正则表达式标志
     * @param flags 正则表达式标志
     */
    void set_regex_flags(std::string flags) {
        if (token_) {
            token_->set_regex_flags(std::move(flags));
        }
    }

    /**
     * @brief 解析标记对应的语句
     * @param parser 解析器实例
     * @return 解析后的语句
     */
    std::unique_ptr<Statement> Parse(Parser& parser) {
        return token_ ? token_->Parse(parser) : nullptr;
    }

    /**
     * @brief 获取底层TokenBase指针
     * @return TokenBase指针
     */
    [[nodiscard]] TokenBase* get() const noexcept {
        return token_.get();
    }

    /**
     * @brief 释放底层TokenBase所有权
     * @return TokenBase指针
     */
    TokenBase* release() noexcept {
        return token_.release();
    }

private:
    std::unique_ptr<TokenBase> token_;
};

} // namespace compiler
} // namespace mjs