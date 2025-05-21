#pragma once

#include <filesystem>

#include <mjs/noncopyable.h>
#include <mjs/value.h>

namespace mjs {

class ModuleManagerBase : public noncopyable {
public:
	virtual void AddCppModule(std::string_view path, CppModuleObject* cpp_module_object) = 0;
	virtual Value GetModule(Context* ctx, std::string_view path) = 0;
	virtual Value GetModuleAsync(Context* ctx, std::string_view path) = 0;
	virtual void ClearModuleCache() = 0;
};

class ModuleManager : public ModuleManagerBase {
public:
	void AddCppModule(std::string_view path, CppModuleObject* cpp_module_object) override;
	Value GetModule(Context* ctx, std::string_view path) override;
	Value GetModuleAsync(Context* ctx, std::string_view path) override;
	void ClearModuleCache() override;

protected:
	std::unordered_map<std::filesystem::path, Value> cpp_module_cache_;
	std::unordered_map<std::filesystem::path, Value> module_cache_;
};

} // namespace mjs