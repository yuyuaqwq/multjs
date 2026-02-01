#include <mjs/class_def_table.h>

#include <mjs/runtime.h>
#include <mjs/class_def/string_object_class_def.h>
#include <mjs/class_def/symbol_class_def.h>
#include <mjs/class_def/array_object_class_def.h>
#include <mjs/class_def/object_class_def.h>
#include <mjs/class_def/function_object_class_def.h>
#include <mjs/class_def/generator_object_class_def.h>
#include <mjs/class_def/promise_object_class_def.h>

namespace mjs {

ClassDefTable::ClassDefTable(Runtime* runtime) {}

void ClassDefTable::Initialize(Runtime* runtime) {
	// 注册所有内置类定义，顺序重要，先Object，再Function，再是其他对象
	Register(std::make_unique<ObjectClassDef>(runtime));
	Register(std::make_unique<FunctionObjectClassDef>(runtime));
	Register(std::make_unique<ClassDef>(runtime, ClassId::kNumberObject, "Number"));
	Register(std::make_unique<StringObjectClassDef>(runtime));
	Register(std::make_unique<ArrayObjectClassDef>(runtime));
	Register(std::make_unique<GeneratorObjectClassDef>(runtime));
	Register(std::make_unique<PromiseObjectClassDef>(runtime));
	Register(std::make_unique<ClassDef>(runtime, ClassId::kAsyncObject, "Async"));
	Register(std::make_unique<ClassDef>(runtime, ClassId::kModuleObject, "Module"));
	Register(std::make_unique<ClassDef>(runtime, ClassId::kCppModuleObject, "CppModule"));
	Register(std::make_unique<SymbolClassDef>(runtime));
}

void ClassDefTable::Register(ClassDefUnique class_def) {
	auto id = class_def->id();
	auto idx = Insert(std::move(class_def));
	if (idx != static_cast<uint32_t>(id)) {
		// 必须按枚举定义顺序插入，以确保高效查找
		throw InternalError("Class id mismatch.");
	}
}

void ClassDefTable::Clear() {
	class_def_arr_.clear();
}

} // namespace mjs