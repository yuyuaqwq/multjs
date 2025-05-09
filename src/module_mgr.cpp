#include <mjs/module_mgr.h>

#include <fstream>

#include <mjs/context.h>


namespace mjs {

void ModuleMgr::AddCppModule(std::string_view path, CppModuleObject* cpp_module) {
	auto res = module_cache_.emplace(path, std::move(cpp_module));
	assert(res.second);
}

Value ModuleMgr::GetModule(Context* ctx, std::string_view path) {
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

	std::fstream file;
	file.open(absolute_path);
	auto content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());;
	file.close();

	auto module = ctx->Compile(absolute_path.string(), content);

	// 先缓存模块，再调用
	module_cache_.emplace(absolute_path, module);

	ctx->CallModule(&module);

	return module;
}

Value ModuleMgr::GetModuleAsync(Context* ctx, std::string_view path) {
	return GetModule(ctx, path);
}

void ModuleMgr::ClearModuleCache() {
	module_cache_.clear();
}

} // namespace mjs