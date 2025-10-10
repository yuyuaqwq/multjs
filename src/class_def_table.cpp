#include <mjs/class_def_table.h>

#include <mjs/runtime.h>
#include <mjs/class_def_impl/string_object_class_def.h>
#include <mjs/class_def_impl/symbol_class_def.h>
#include <mjs/class_def_impl/array_object_class_def.h>
#include <mjs/class_def_impl/object_class_def.h>
#include <mjs/class_def_impl/generator_object_class_def.h>
#include <mjs/class_def_impl/promise_object_class_def.h>

namespace mjs {

ClassDefTable::ClassDefTable(Runtime* runtime) {
	// 注册所有内置类定义
	Register(std::make_unique<SymbolClassDef>(runtime));
	Register(std::make_unique<ObjectClassDef>(runtime));
	Register(std::make_unique<ClassDef>(runtime, ClassId::kNumberObject, "Number"));
	Register(std::make_unique<StringObjectClassDef>(runtime));
	Register(std::make_unique<ArrayObjectClassDef>(runtime));
	Register(std::make_unique<ClassDef>(runtime, ClassId::kFunctionObject, "Function"));
	Register(std::make_unique<GeneratorObjectClassDef>(runtime));
	Register(std::make_unique<PromiseObjectClassDef>(runtime));
	Register(std::make_unique<ClassDef>(runtime, ClassId::kAsyncObject, "Async"));
	Register(std::make_unique<ClassDef>(runtime, ClassId::kModuleObject, "Module"));
	Register(std::make_unique<ClassDef>(runtime, ClassId::kConstructorObject, "Constructor"));
	Register(std::make_unique<ClassDef>(runtime, ClassId::kCppModuleObject, "CppModule"));
}

void ClassDefTable::Register(ClassDefUnique class_def) {
	auto id = class_def->id();
	auto idx = insert(std::move(class_def));
	if (idx != static_cast<uint32_t>(id)) {
		// 必须按枚举定义顺序插入，以确保高效查找
		throw std::runtime_error("Class id mismatch.");
	}
}

void ClassDefTable::Clear() {
	class_def_arr_.clear();
}

} // namespace mjs