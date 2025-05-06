#pragma once

#include <filesystem>

#include <mjs/noncopyable.h>
#include <mjs/value.h>

namespace mjs {

class ModuleMgr : public noncopyable {
public:
	virtual void AddCppModule(std::string_view path, mjs::CppModuleObject* cpp_module_object);

	virtual Value GetModule(Context* ctx, std::string_view path);

	virtual Value GetModuleAsync(Context* ctx, std::string_view path);

	virtual void ClearModuleCache();

private:
	std::unordered_map<std::filesystem::path, Value> cpp_module_cache_;
	std::unordered_map<std::filesystem::path, Value> module_cache_;
};

} // namespace mjs