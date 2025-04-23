#pragma once

#include <string>
#include <unordered_map>

namespace mjs {
namespace compiler {

enum class TokenType {
    kNil = 0,      // 空token

    kEof,          // 文件结束符
    kUndefined,    // undefined
    kNull,         // null
    kFalse,        // false
    kTrue,         // true
    //kNan,          // Nan
    //kInfinity,     // Infinity
    //kNumber,       // 通用数字类型
    kString,       // 字符串
    kIdentifier,   // [a-zA-Z_][a-zA-Z0-9_]*

    kFloatLiteral,  // 浮点数
    kIntLiteral,  // 整数

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
    // kKwVar,           // var
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

    // 其他运算符
    kOpNullishCoalescing, // ?? 空值合并运算符
    kOpOptionalChain,     // ?. 可选链运算符
    kOpTernary,           // ?: 三元运算符
};


class Token {
public:
	bool Is(TokenType type) const noexcept {
        return type_ == type;
    }

	TokenType type() const { return type_; }
	void set_type(TokenType type) { type_ = type; }

	int32_t line() const { return line_; }
	void set_line(int32_t line) { line_ = line; }

	std::string* mutable_str() { return &str_; }
	const std::string& str() const { return str_; }
	void set_str(std::string str) { str_ = std::move(str); }

private:
	int32_t line_ = 0;		// 行号
	TokenType type_ = TokenType::kNil;		// token类型
	std::string str_;	// 保存必要的信息
};

extern std::unordered_map<std::string, TokenType> g_operators;
extern std::unordered_map<std::string, TokenType> g_keywords;

} // namespace compiler
} // namespace msj