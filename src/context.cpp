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

	Call(func, Value(), {});

	return func;
}

Value Context::Call(Value func_val, Value this_val, const std::vector<Value>& argv) {
	return vm_.CallFunction(std::move(func_val), std::move(this_val), argv);
}

} // namespace mjs