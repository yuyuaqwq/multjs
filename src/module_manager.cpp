#include <mjs/module_manager.h>

#include <iostream>
#include <fstream>

#include <mjs/context.h>


namespace mjs {

void ModuleManager::AddCppModule(std::string_view path, CppModuleObject* cpp_module) {
	auto res = cpp_module_cache_.emplace(path, std::move(cpp_module));
	assert(res.second);
}

Value ModuleManager::GetModule(Context* ctx, std::string_view path) {
	auto iter = cpp_module_cache_.find(path);
	if (iter != cpp_module_cache_.end()) {
		return iter->second;
	}

	namespace fs = std::filesystem;

	fs::path absolute_path = fs::absolute(path);
	iter = module_cache_.find(absolute_path);
	if (iter != module_cache_.end()) {
		return iter->second;
	}

	std::ifstream file(absolute_path, std::ios::binary);  // 二进制模式
	if (!file.is_open()) {
		return Value();
	}

	// 读取所有内容（包括 \r\n，不做任何转换）
	std::string content(
		(std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>()
	);

	file.close();


	auto module = ctx->CompileModule(absolute_path.string(), content);

	// 先缓存模块，再调用
	module_cache_.emplace(absolute_path, module);

	ctx->CallModule(&module);

	return module;
}

Value ModuleManager::GetModuleAsync(Context* ctx, std::string_view path) {
	return GetModule(ctx, path);
}

void ModuleManager::ClearModuleCache() {
	module_cache_.clear();
}

} // namespace mjs