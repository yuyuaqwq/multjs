#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include <mjs/source_define.h>

namespace mjs {
namespace compiler {

/**
 * @enum TokenType
 * @brief 词法标记的类型枚举
 */
enum class TokenType {
    kNone = 0,      // 空标记

    kEof,           // 文件结束符
    kUndefined,     // undefined
    kNull,          // null
    kFalse,         // false
    kTrue,          // true
    //kNan,          // Nan
    //kInfinity,     // Infinity
    //kNumber,       // 通用数字类型
    kFloat,         // 浮点数
    kInteger,       // 整数
    kBigInt,        // BigInt (ES2020)
    kString,        // 字符串
    kRegExp,        // 正则表达式
    kBacktick,      // `
    kTemplateElement, // 模板字符串元素
    kTemplateInterpolationStart, // ${
    kTemplateInterpolationEnd,   // }

    kIdentifier,    // 标识符: [a-zA-Z_][a-zA-Z0-9_]*

    // 分隔符
    kSepSemi,         // ;
    kSepComma,        // ,
    kSepDot,          // .
    kSepEllipsis,     // ... 可变参数或解构语法
    kSepColon,        // :
    kSepQuestion,     // ? 条件运算符
    kSepArrow,        // => 箭头函数

    kSepLParen,       // (
    kSepRParen,       // )
    kSepLBrack,       // [
    kSepRBrack,       // ]
    kSepLCurly,       // {
    kSepRCurly,       // }

    // 基本运算符
    kOpAssign,        // =
    kOpAdd,           // +
    kOpSub,           // -
    kOpMul,           // *
    kOpDiv,           // /
    kOpMod,           // %
    kOpPower,         // ** 幂运算符
    kOpInc,           // ++ 自增
    kOpDec,           // -- 自减

    kOpPrefixInc,     // 前缀自增
    kOpPrefixDec,     // 前缀自减
    kOpSuffixInc,     // 后缀自增
    kOpSuffixDec,     // 后缀自减

    // 复合赋值运算符
    kOpAddAssign,     // +=
    kOpSubAssign,     // -=
    kOpMulAssign,     // *=
    kOpDivAssign,     // /=
    kOpModAssign,     // %=
    kOpPowerAssign,   // **=
    kOpBitAndAssign,  // &=
    kOpBitOrAssign,   // |=
    kOpBitXorAssign,  // ^=
    kOpShiftLeftAssign,       // <<=
    kOpShiftRightAssign,      // >>=
    kOpUnsignedShiftRightAssign, // >>>=

    // 位运算符
    kOpBitNot,        // ~ 按位取反
    kOpBitAnd,        // & 按位与
    kOpBitOr,         // | 按位或
    kOpBitXor,        // ^ 按位异或
    kOpShiftLeft,     // << 左移
    kOpShiftRight,    // >> 右移
    kOpUnsignedShiftRight, // >>> 无符号右移

    // 逻辑运算符
    kOpNot,           // ! 逻辑非
    kOpAnd,           // && 逻辑与
    kOpOr,            // || 逻辑或

    // 比较运算符
    kOpNe,            // != 不等于
    kOpEq,            // == 等于
    kOpStrictEq,      // === 严格等于
    kOpStrictNe,      // !== 严格不等于
    kOpLt,            // < 小于
    kOpLe,            // <= 小于等于
    kOpGt,            // > 大于
    kOpGe,            // >= 大于等于

    // 关键字
    kKwFunction,      // function
    kKwIf,            // if
    kKwElse,          // else
    kKwWhile,         // while
    kKwFor,           // for
    kKwContinue,      // continue
    kKwBreak,         // break
    kKwReturn,        // return
    // kKwVar,           // var，当前编译器不支持var
    kKwLet,           // let
    kKwConst,         // const
    kKwImport,        // import
    kKwAs,            // as
    kKwExport,        // export
    kKwFrom,          // from
    kKwClass,         // class
    kKwNew,           // new
    kKwDelete,        // delete
    kKwTry,           // try
    kKwCatch,         // catch
    kKwFinally,       // finally
    kKwThrow,         // throw
    kKwSwitch,        // switch
    kKwCase,          // case
    kKwDefault,       // default
    kKwTypeof,        // typeof
    kKwInstanceof,    // instanceof
    kKwVoid,          // void
    kKwIn,            // in
    kKwWith,          // with
    kKwYield,         // yield (在生成器中使用)
    kKwAsync,         // async (用于定义异步函数)
    kKwAwait,         // await (用于等待异步结果)
    kKwThis,          // this
    kKwExtends,       // extends
    kKwSuper,         // super
    kKwStatic,        // static
    kKwGet,           // get
    kKwSet,           // set

    // 其他运算符
    kOpNullishCoalescing, // ?? 空值合并运算符
    kOpOptionalChain,     // ?. 可选链运算符
    kOpTernary,           // ?: 三元运算符

    // 类型
    kUnionType = kOpBitOr,       // |
};

/**
 * @class Token
 * @brief 表示词法分析产生的标记
 */
class Token {
public:
    /**
     * @brief 检查标记类型是否匹配
     * @param type 要检查的类型
     * @return 如果类型匹配则返回true，否则返回false
     */
    [[nodiscard]] bool is(TokenType type) const noexcept {
        return type_ == type;
    }

    /**
     * @brief 获取标记在源代码中的位置
     * @return 源代码位置
     */
    [[nodiscard]] SourcePosition pos() const noexcept { 
        return position_; 
    }
    
    /**
     * @brief 设置标记在源代码中的位置
     * @param position 源代码位置
     */
    void set_pos(SourcePosition position) { 
        position_ = position; 
    }

    /**
     * @brief 获取标记类型
     * @return 标记类型
     */
    [[nodiscard]] TokenType type() const noexcept { 
        return type_; 
    }
    
    /**
     * @brief 设置标记类型
     * @param type 标记类型
     */
    void set_type(TokenType type) { 
        type_ = type; 
    }

    /**
     * @brief 获取可修改的标记值
     * @return 指向标记值的指针
     */
    std::string* mutable_value() { 
        return &value_; 
    }
    
    /**
     * @brief 获取标记值
     * @return 标记值的常量引用
     */
    [[nodiscard]] const std::string& value() const noexcept { 
        return value_; 
    }
    
    /**
     * @brief 设置标记值
     * @param value 标记值
     */
    void set_value(std::string value) { 
        value_ = std::move(value); 
    }

    /**
     * @brief 获取正则表达式标志
     * @return 正则表达式标志的常量引用
     */
    [[nodiscard]] const std::string& regex_flags() const noexcept {
        return regex_flags_;
    }

    /**
     * @brief 设置正则表达式标志
     * @param flags 正则表达式标志
     */
    void set_regex_flags(std::string flags) {
        regex_flags_ = std::move(flags);
    }

    /**
     * @brief 将标记类型转换为字符串表示
     * @param type 标记类型
     * @return 标记类型的字符串表示
     */
    static std::string TypeToString(TokenType type);

    static const std::unordered_map<std::string, TokenType>& operator_map();
    static const std::unordered_map<std::string, TokenType>& keyword_map();

    static constexpr size_t kOperatorMaxSize = 4;

private:
    SourcePosition position_ = 0;           ///< 标记在源代码中的位置
    TokenType type_ = TokenType::kNone; ///< 标记类型
    std::string value_;                ///< 标记值（如标识符名称、字符串内容等）
    std::string regex_flags_;          ///< 正则表达式标志（如果是正则表达式标记）
};

} // namespace compiler
} // namespace mjs