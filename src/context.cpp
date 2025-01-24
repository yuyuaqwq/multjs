#include <mjs/context.h>

#include <iostream>
#include <string_view>

#include "lexer.h"
#include "parser.h"
#include "codegener.h"

namespace mjs {

void Context::Eval(std::string_view script) {
	auto lexer = Lexer(script.data());

	auto parser = Parser(&lexer);
	auto src = parser.ParseSource();

	auto codegener = CodeGener(runtime_);
	auto func = codegener.Generate(src.get());

	std::cout << func.function_body()->Disassembly();

	vm_.SetEvalFunction(func);
	vm_.Run();
}

} // namespace mjs