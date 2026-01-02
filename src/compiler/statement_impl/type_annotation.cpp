#include "type_annotation.h"

#include <mjs/error.h>

#include "../code_generator.h"
#include "named_type.h"

namespace mjs {
namespace compiler {

void TypeAnnotation::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 类型注解在运行时不需要生成代码
    // 它们只在编译时用于类型检查
}

std::unique_ptr<TypeAnnotation> TypeAnnotation::TryParseTypeAnnotation(Lexer* lexer) {
	if (!lexer->PeekToken().is(TokenType::kSepColon)) {
		return nullptr;
	}

	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kSepColon);

	// 解析类型
	std::unique_ptr<Type> type;

	if (lexer->PeekToken().is(TokenType::kIdentifier)) {
		// 命名类型
		auto type_start = lexer->GetSourcePosition();
		auto type_name = lexer->NextToken().value();
		auto type_end = lexer->GetRawSourcePosition();

		type = std::make_unique<NamedType>(type_start, type_end, std::move(type_name));
	} else if (lexer->PeekToken().is(TokenType::kSepLParen)) {
		// 联合类型
		type = UnionType::ParseUnionType(lexer);
	} else {
		throw SyntaxError("Invalid type annotation");
	}

	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<TypeAnnotation>(start, end, std::move(type));
}

} // namespace compiler
} // namespace mjs