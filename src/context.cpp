#include <mjs/context.h>

#include <iostream>
#include <string_view>

#include <mjs/runtime.h>
#include <mjs/error.h>
#include <mjs/object_impl/module_object.h>

#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/codegener.h"

namespace mjs {
	
Context::Context(Runtime* runtime)
	: runtime_(runtime)
	, vm_(this)
	, shape_manager_(this)
	/* , symbol_table_(this)*/ {}

Context::~Context() {
	//assert(runtime_->stack().size() == 0);
	runtime_->stack().clear();
	microtask_queue_.clear();
	local_const_pool_.clear();
	gc_manager_.GC(this);
}

Value Context::CompileModule(std::string module_name, std::string_view script) {
	auto lexer = compiler::Lexer(script.data());
	auto parser = compiler::Parser(&lexer);
	auto codegener = compiler::CodeGener(this, &parser);
	// todo: 未来转为Error对象

	try {
		parser.ParseProgram();
	}
	catch (SyntaxError& e) {
		auto pos = lexer.GetRawSourcePos();
		LineTable line_table;
		line_table.Build(script);
		auto&& [line, column] = line_table.PosToLineAndColumn(pos);
		auto info = std::format("{}: [name:{}, line:{}, column:{}] {}", e.error_name(), module_name, line, column, e.what());
		return Value(String::New(info)).SetException();
	}
	try {
		auto module_def = codegener.Generate(std::move(module_name), script);
		return module_def;
	}
	catch (SyntaxError& e) {
		std::string info;
		if (codegener.cur_module_def()) {
			auto pos = lexer.GetRawSourcePos();
			auto&& [line, column] = codegener.cur_module_def()->line_table().PosToLineAndColumn(pos);
			info = std::format("{}: [name:{}, line:{}, column:{}] {}", e.error_name(), codegener.cur_module_def()->name(), line, column, e.what());
		}
		return Value(String::New(info)).SetException();
	}
}

Value Context::CallModule(Value* value) {
	if (value->IsModuleDef()) {
		vm_.ModuleInit(value);
	}
	std::initializer_list<Value> argv = {};
	return CallFunction(value, Value(), argv.begin(), argv.end());
}


Value Context::Eval(std::string module_name, std::string_view script) {
	auto module = CompileModule(std::move(module_name), script);
	CallModule(&module);
	return module;
}

Value Context::EvalFromFile(std::string_view path) {
	auto module = runtime_->module_manager().GetModule(this, path);
	return module;
}

void Context::ReferenceConstValue(ConstIndex const_index) {
	if (const_index < 0) {
		local_const_pool_.ReferenceConst(const_index);
	}
}
void Context::DereferenceConstValue(ConstIndex const_index) {
	if (const_index < 0) {
		local_const_pool_.DereferenceConst(const_index);
	}
}

ConstIndex Context::FindConstOrInsertToLocal(const Value& value) {
	if (value.const_index() != kConstIndexInvalid) {
		return value.const_index();
	}

	//这里需要保证的是，当前Context运行下，不会出现相等的Value，出现不同的const_index

	// 先查Local，再查Global，最后插入Local，可以保证要么从当前时间开始，使用的都是Global的，要么使用的都是Local的
	// 即使未来Global被插入了相同的Value，也不会使用Global的Value

	auto local_res = local_const_pool_.find(value);
	if (local_res) {
		return *local_res;
	}

	auto global_res = runtime_->const_pool().find(value);
	if (global_res) {
		return *global_res;
	}

	return local_const_pool_.insert(value);
}

// 代码生成用，主要是为了减少重复的内存占用，Global是不会被回收的
ConstIndex Context::FindConstOrInsertToGlobal(const Value& value) {
	if (value.const_index() != kConstIndexInvalid) {
		return value.const_index();
	}

	auto local_res = local_const_pool_.find(value);
	if (local_res) {
		return *local_res;
	}

	return runtime_->const_pool().insert(value);
}

const Value& Context::GetConstValue(ConstIndex const_index) {
	if (const_index < 0) {
		return local_const_pool_[const_index];
	}
	else {
		return runtime_->const_pool()[const_index];
	}
}


} // namespace mjs