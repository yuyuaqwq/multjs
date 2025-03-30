#include <mjs/context.h>

#include <iostream>
#include <string_view>

#include "lexer.h"
#include "parser.h"
#include "codegener.h"

namespace mjs {

Value Context::Eval(std::string_view script) {
	auto lexer = Lexer(script.data());

	auto parser = Parser(&lexer);
	auto src = parser.ParseSource();

	auto codegener = CodeGener(runtime_);
	auto func = codegener.Generate(src.get());

	Call(func);

	return func;
}

void Context::Call(const Value& func) {
	std::cout << func.function_def().Disassembly(this);

	vm_.EvalFunction(func);
}



} // namespace mjs