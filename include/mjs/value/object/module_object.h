#pragma once

#include <functional>

#include <mjs/value/module_def.h>
#include <mjs/value/object/function_object.h>

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
private:
    ModuleObject(Context* context, ModuleDef* module_def);

public:
	void GCTraverse(Context* context, GCTraverseCallback callback) override {
		// 先调用父类方法遍历属性
		FunctionObject::GCTraverse(context, callback);

		// 遍历导出变量
		for (auto& var : module_env_.export_vars()) {
			callback(context, var.value());
		}
	}

	void SetProperty(Context* context, ConstIndex key, Value&& value) override;

	bool GetProperty(Context* context, ConstIndex key, Value* value) override;

    ModuleDef& module_def() const { return static_cast<ModuleDef&>(*function_def_); }
	auto& module_env() { return module_env_; }

	static ModuleObject* New(Context* context, ModuleDef* module_def) {
		return new ModuleObject(context, module_def);
	}

protected:
    ModuleEnvironment module_env_;
};

} // namespace mjs