#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <format>
#include <functional>

#include <mjs/value/function_def.h>
#include <mjs/value/object/object.h>

namespace mjs {

class FunctionObject : public Object {
protected:
	FunctionObject(Context* context, FunctionDefBase* function_def) noexcept;

	FunctionObject(Context* context, FunctionDefBase* function_def, ClassId class_id) noexcept;

public:
	~FunctionObject() = default;

	void GCTraverse(Context* context, GCTraverseCallback callback) override;

	Value ToString(Context* context) override {
		return Value(String::Format("function_object:{}", function_def_->name()));
	}

	FunctionDef& function_def() const { return static_cast<FunctionDef&>(*function_def_); }

	const auto& closure_env() const { return closure_env_; }

	auto& closure_env() { return closure_env_; }

protected:
	friend class GCManager;

	void InitPrototypeProperty(Context* context);

	FunctionDefBase* function_def_;

	ClosureEnvironment closure_env_;		///< 闭包
};

} // namespace mjs