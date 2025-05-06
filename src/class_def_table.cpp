#include <mjs/class_def_table.h>

#include <mjs/runtime.h>
#include <mjs/class_def_impl/symbol_class_def.h>
#include <mjs/class_def_impl/array_object_class_def.h>
#include <mjs/class_def_impl/object_class_def.h>
#include <mjs/class_def_impl/generator_object_class_def.h>
#include <mjs/class_def_impl/promise_object_class_def.h>

namespace mjs {

ClassDefTable::ClassDefTable(Runtime* runtime) {
	Register(std::make_unique<ClassDef>(runtime, ClassId::kNumber, "Number"));
	Register(std::make_unique<ClassDef>(runtime, ClassId::kString, "String"));
	Register(std::make_unique<SymbolClassDef>(runtime));
	Register(std::make_unique<ObjectClassDef>(runtime));
	Register(std::make_unique<ArrayObjectClassDef>(runtime));
	Register(std::make_unique<GeneratorObjectClassDef>(runtime));
	Register(std::make_unique<PromiseObjectClassDef>(runtime));
}

void ClassDefTable::Register(ClassDefUnique class_def) {
	auto id = class_def->id();
	std::string_view name = class_def->name();
	auto idx = insert(std::move(class_def));
	if (idx != static_cast<uint32_t>(id)) {
		// 必须按枚举定义顺序插入，以确保高效查找
		throw std::runtime_error("Class id mismatch.");
	}
	class_def_map_.emplace(name, class_def_arr_[idx].get());
}

} // namespace mjs