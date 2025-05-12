#include <mjs/context.h>

#include <iostream>
#include <string_view>

#include <mjs/runtime.h>
#include <mjs/object_impl/module_object.h>

#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/codegener.h"

namespace mjs {

Value Context::Compile(std::string module_name, std::string_view script) {
	auto lexer = compiler::Lexer(script.data());

	auto parser = compiler::Parser(&lexer);
	parser.ParseProgram();

	auto codegener = compiler::CodeGener(runtime_, &parser);
	auto module = codegener.Generate(std::move(module_name));

	vm_.ModuleInit(&module);

	return module;
}

Value Context::CallModule(Value* value) {
	std::initializer_list<Value> argv = {};
	return CallFunction(value, Value(), argv.begin(), argv.end());
}


Value Context::Eval(std::string module_name, std::string_view script) {
	auto module = Compile(std::move(module_name), script);
	CallModule(&module);
	return module;
}

Value Context::EvalByPath(std::string_view path) {
	auto module = runtime_->module_mgr().GetModule(this, path);
	return module;
}

ConstIndex Context::InsertConst(const Value& value) {
	if (value.const_index() != kConstIndexInvalid) {
		return value.const_index();
	}

	//������Ҫ��֤���ǣ���ǰContext�����£����������ȵ�Value�����ֲ�ͬ��const_index

	// �Ȳ�Local���ٲ�Global��������Local�����Ա�֤Ҫô�ӵ�ǰʱ�俪ʼ��ʹ�õĶ���Global�ģ�Ҫôʹ�õĶ���Local��
	// ��ʹδ��Global����������ͬ��Value��Ҳ����ʹ��Global��Value

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

} // namespace mjs