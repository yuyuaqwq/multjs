#pragma once

#include <string>
#include <unordered_map>

namespace mjs {

enum class TokenType {
    kNil = 0,      // ��token

    kEof,          // �ļ�������
    kUndefined,    // undefined
    kNull,         // null
    kFalse,        // false
    kTrue,         // true
    //kNan,          // Nan
    //kInfinity,     // Infinity
    kNumber,       // ͨ����������
    kString,       // �ַ���
    kIdentifier,   // [a-zA-Z_][a-zA-Z0-9_]*

    kDecimalLiteral,  // ������
    kIntegerLiteral,  // ����

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
};


class Token {
public:
	bool Is(TokenType type) const noexcept;

	TokenType type() const { return type_; }
	void set_type(TokenType type) { type_ = type; }

	int32_t line() const { return line_; }
	void set_line(int32_t line) { line_ = line; }

	std::string* mutable_str() { return &str_; }
	const std::string& str() const { return str_; }
	void set_str(std::string str) { str_ = std::move(str); }

private:
	int32_t line_ = 0;		// �к�
	TokenType type_ = TokenType::kNil;		// token����
	std::string str_;	// �����Ҫ����Ϣ
};

extern std::unordered_map<std::string, TokenType> g_operators;
extern std::unordered_map<std::string, TokenType> g_keywords;

} // namespace msj