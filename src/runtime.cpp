#include <mjs/runtime.h>

#include <fstream>

#include <mjs/context.h>
#include <mjs/class_def/generator_class_def.h>
#include <mjs/class_def/promise_class_def.h>

namespace mjs {

Runtime::Runtime() {
	class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kBase, "Object"));
	class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kNumber, "Number"));
	class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kString, "String"));
	class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kArray, "Array"));
	class_def_table_.Register(std::make_unique<GeneratorClassDef>());
	class_def_table_.Register(std::make_unique<PromiseClassDef>());

	auto load_module = [](Context* ctx, const char* path) -> Value {
		namespace fs = std::filesystem;
		// ����key���·��ת����·��
		// ���ģ���ѱ����棬��ô��ֱ�ӷ��ػ����ģ��

		// Ҫ�ĳɻ��浽context��
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

		auto module = ctx->Compile(content);

		// �Ȼ���ģ�飬�ٵ���
		module_cache.emplace(absolute_path, module);

		ctx->CallModule(&module);
		return module;
	};

	set_load_module(load_module);
	set_load_module_async(load_module);
}

} // namespace mjs