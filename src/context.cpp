#include <mjs/context.h>

#include <iostream>
#include <string_view>

#include <mjs/runtime.h>
#include <mjs/object/module_object.h>

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

	// auto module = Value(new ModuleObject(this, &func.function_def()));

	std::initializer_list<Value> argv = {};
	Call(&func, Value(), argv.begin(), argv.end());

	return func;
}

} // namespace mjs