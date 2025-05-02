#include <mjs/context.h>

#include <iostream>
#include <string_view>

#include <mjs/runtime.h>
#include <mjs/object/module_object.h>

#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/codegener.h"

namespace mjs {

Value Context::Compile(std::string_view script) {
	auto lexer = compiler::Lexer(script.data());

	auto parser = compiler::Parser(&lexer);
	parser.ParseProgram();

	auto codegener = compiler::CodeGener(runtime_, &parser);
	auto module = codegener.Generate();

	// 转换为 module_obj
	vm_.Closure(StackFrame(&runtime_->stack()), &module);

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