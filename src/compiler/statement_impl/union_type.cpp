#include "src/compiler/statement_impl/union_type.h"

#include <mjs/error.h>

#include "src/compiler/statement_impl/named_type.h"
#include "src/compiler/lexer.h"

namespace mjs {
namespace compiler {

std::unique_ptr<UnionType> UnionType::ParseUnionType(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();

	// 解析联合类型的成员
	std::vector<std::unique_ptr<Type>> types;

	// 第一个类型
	if (lexer->PeekToken().is(TokenType::kIdentifier)) {
		auto type_start = lexer->GetSourcePosition();
		auto type_name = lexer->NextToken().value();
		auto type_end = lexer->GetRawSourcePosition();

		types.push_back(std::make_unique<NamedType>(type_start, type_end, std::move(type_name)));
	} else {
		throw SyntaxError("Expected type name");
	}

	// 后续类型
	while (lexer->PeekToken().is(TokenType::kOpBitOr)) {
		lexer->NextToken(); // 消耗 |

		if (lexer->PeekToken().is(TokenType::kIdentifier)) {
			auto type_start = lexer->GetSourcePosition();
			auto type_name = lexer->NextToken().value();
			auto type_end = lexer->GetRawSourcePosition();

			types.push_back(std::make_unique<NamedType>(type_start, type_end, std::move(type_name)));
		} else {
			throw SyntaxError("Expected type name after |");
		}
	}

	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<UnionType>(start, end, std::move(types));
}

} // namespace compiler
} // namespace mjs