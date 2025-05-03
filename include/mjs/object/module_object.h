#pragma once

#include <mjs/module_def.h>
#include <mjs/object/function_object.h>

namespace mjs {

class ExportVar {
public:
public:
	ExportVar() = default;
	ExportVar(Value&& value)
		: value_(std::move(value))
	{
		assert(!value_.IsClosureVar());
	}

	~ExportVar() = default;

	Value& value() { return value_; }
	const Value& value() const { return value_; }

	void set_value(Value value) { value_ = std::move(value); }

private:
    Value value_;
};

class ModuleEnvironment {
public:
	const auto& export_vars() const { return export_vars_; }
	auto& export_vars() { return export_vars_; }

private:
    std::vector<ExportVar> export_vars_;
};

class ModuleObject : public FunctionObject {
public:
    ModuleObject(Context* context, ModuleDef* module_def);

	void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) override {
		FunctionObject::GCForEachChild(context, list, callback);
		for (auto& var : module_env_.export_vars()) {
			callback(context, list, var.value());
		}
	}

	void SetProperty(Context* context, ConstIndex key, Value&& value) override;
	bool GetProperty(Context* context, ConstIndex key, Value* value) override;

    ModuleDef& module_def() const { return static_cast<ModuleDef&>(function_def()); }
	auto& module_env() { return module_env_; }

protected:
    ModuleEnvironment module_env_;
};

} // namespace mjs