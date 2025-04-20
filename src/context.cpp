#include <mjs/context.h>

#include <iostream>
#include <string_view>

#include <mjs/runtime.h>
#include <mjs/object/module_object.h>

#include "lexer.h"
#include "parser.h"
#include "codegener.h"

namespace mjs {

Value Context::Compile(std::string_view script) {
	auto lexer = Lexer(script.data());

	auto parser = Parser(&lexer);
	parser.ParseSource();

	auto codegener = CodeGener(runtime_, &parser);
	auto module = codegener.Generate();

	return module;
}

Value Context::CallModule(Value* value) {
	std::initializer_list<Value> argv = {};
	return CallFunction(value, Value(), argv.begin(), argv.end());
}


Value Context::Eval(std::string_view script) {
	auto module = Compile(script);
	CallModule(&module);
	return module;
}

Value Context::EvalByPath(const char* path) {
	auto module = runtime_->load_module()(this, path);
	return module;
}

} // namespace mjs