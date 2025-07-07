#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <format>

#include <mjs/function_def.h>
#include <mjs/object.h>

namespace mjs {

class FunctionObject : public Object {
protected:
	FunctionObject(Context* context, FunctionDefBase* function_def) noexcept;
	FunctionObject(Context* context, FunctionDefBase* function_def, ClassId class_id) noexcept;

public:
	~FunctionObject() = default;

	void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) override {
		Object::GCForEachChild(context, list, callback);
		closure_env_.GCForEachChild(context, list, callback);
	}

	Value ToString(Context* context) override {
		return Value(String::Format("function_object:{}", function_def_->name()));
	}

	FunctionDef& function_def() const { return static_cast<FunctionDef&>(*function_def_); }

	const auto& closure_env() const { return closure_env_; }
	auto& closure_env() { return closure_env_; }
	
	static FunctionObject* New(Context* context, FunctionDef* function_def) {
        return new FunctionObject(context, function_def);
	}

protected:
	FunctionDefBase* function_def_;

	// 闭包
	ClosureEnvironment closure_env_;
};

} // namespace mjs