#include <mjs/runtime.h>

#include <fstream>

#include <mjs/context.h>
#include <mjs/class_def_impl/symbol_class_def.h>
#include <mjs/class_def_impl/array_object_class_def.h>
#include <mjs/class_def_impl/object_class_def.h>
#include <mjs/class_def_impl/generator_object_class_def.h>
#include <mjs/class_def_impl/promise_object_class_def.h>

namespace mjs {

Runtime::Runtime() {
	class_def_table_.Register(std::make_unique<ClassDef>(this, ClassId::kNumber, "Number"));
	class_def_table_.Register(std::make_unique<ClassDef>(this, ClassId::kString, "String"));
	class_def_table_.Register(std::make_unique<SymbolClassDef>(this));
	class_def_table_.Register(std::make_unique<ObjectClassDef>(this));
	class_def_table_.Register(std::make_unique<ArrayObjectClassDef>(this));
	class_def_table_.Register(std::make_unique<GeneratorObjectClassDef>(this));
	class_def_table_.Register(std::make_unique<PromiseObjectClassDef>(this));

	auto load_module = [](Context* ctx, const char* path) -> Value {
		namespace fs = std::filesystem;
		// 缓存key相对路径转绝对路径
		// 如果模块已被缓存，那么就直接返回缓存的模块

		// 要改成缓存到context里
		auto& module_cache = ctx->runtime().module_cache();

		fs::path absolute_path = fs::absolute(path);

		auto iter = module_cache.find(absolute_path);
		if (iter != module_cache.end()) {
			return iter->second;
		}

		std::fstream file;
		file.open(absolute_path);
		auto content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());;
		file.close();

		auto module = ctx->Compile(absolute_path.string(), content);

		// 先缓存模块，再调用
		module_cache.emplace(absolute_path, module);

		ctx->CallModule(&module);
		return module;
	};

	set_load_module(load_module);
	set_load_module_async(load_module);
}

} // namespace mjs