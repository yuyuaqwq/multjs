#pragma once

#include <string>
#include <unordered_map>

namespace mjs {
namespace compiler {

enum class TokenType {
    kNone = 0,      // ��token

    kEof,          // �ļ�������
    kUndefined,    // undefined
    kNull,         // null
    kFalse,        // false
    kTrue,         // true
    //kNan,          // Nan
    //kInfinity,     // Infinity
    //kNumber,       // ͨ����������
    kFloat,  // ������
    kInteger,  // ����
    kString,       // �ַ���
    kIdentifier,   // [a-zA-Z_][a-zA-Z0-9_]*

    // �ָ���
    kSepSemi,         // ;
    kSepComma,        // ,
    kSepDot,          // .
    kSepEllipsis,     // ... �ɱ������⹹�﷨
    kSepColon,        // :
    kSepQuestion,     // ? ���������
    kSepArrow,        // => ��ͷ����

    kSepLParen,       // (
    kSepRParen,       // )
    kSepLBrack,       // [
    kSepRBrack,       // ]
    kSepLCurly,       // {
    kSepRCurly,       // }

    // ���������
    kOpAssign,        // =
    kOpAdd,           // +
    kOpSub,           // -
    kOpMul,           // *
    kOpDiv,           // /
    kOpMod,           // %
    kOpPower,         // ** �������
    kOpInc,           // ++ ����
    kOpDec,           // -- �Լ�

    kOpPrefixInc,     // ǰ׺����
    kOpPrefixDec,     // ǰ׺�Լ�
    kOpSuffixInc,     // ��׺����
    kOpSuffixDec,     // ��׺�Լ�

    // λ�����
    kOpBitNot,        // ~ ��λȡ��
    kOpBitAnd,        // & ��λ��
    kOpBitOr,         // | ��λ��
    kOpBitXor,        // ^ ��λ���
    kOpShiftLeft,     // << ����
    kOpShiftRight,    // >> ����
    kOpUnsignedShiftRight, // >>> �޷�������

    // �߼������
    kOpNot,           // ! �߼���
    kOpAnd,           // && �߼���
    kOpOr,            // || �߼���

    // �Ƚ������
    kOpNe,            // != ������
    kOpEq,            // == ����
    kOpStrictEq,      // === �ϸ����
    kOpStrictNe,      // !== �ϸ񲻵���
    kOpLt,            // < С��
    kOpLe,            // <= С�ڵ���
    kOpGt,            // > ����
    kOpGe,            // >= ���ڵ���

    // �ؼ���
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
    kKwYield,         // yield (����������ʹ��)
    kKwAsync,         // async (���ڶ����첽����)
    kKwAwait,         // await (���ڵȴ��첽���)
    kKwThis,          // this

    // ���������
    kOpNullishCoalescing, // ?? ��ֵ�ϲ������
    kOpOptionalChain,     // ?. ��ѡ�������
    kOpTernary,           // ?: ��Ԫ�����

    // ����
    kUnionType,       // |
};


class Token {
public:
	bool is(TokenType type) const noexcept {
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
	int32_t line_ = 0;		// �к�
	TokenType type_ = TokenType::kNone;		// token����
	std::string str_;	// �����Ҫ����Ϣ
};

extern std::unordered_map<std::string, TokenType> g_operators;
extern std::unordered_map<std::string, TokenType> g_keywords;

} // namespace compiler
} // namespace msj