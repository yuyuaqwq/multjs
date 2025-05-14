#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <format>

#include <mjs/function_def.h>
#include <mjs/object.h>

namespace mjs {

// 提升到堆的变量
class ClosureVar : public ReferenceCounter<ClosureVar> {
public:
	ClosureVar(Value&& value) 
		: value_(std::move(value))
	{
		assert(!value_.IsClosureVar());
	}

	~ClosureVar() = default;

	Value& value() { return value_; }
	const Value& value() const { return value_; }

public:
	// 避免循环引用
	Value value_;
};

// 闭包环境记录，Value: 指向当前环境捕获的闭包变量
// 也可以改成ClosureVar*，手动调用Reference和Dereference，可以节省一些空间
class ClosureEnvironment {
public:
	void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) {
		for (auto& var : closure_var_refs_) {
			callback(context, list, var);
		}
		callback(context, list, lexical_this_);
	}

	const auto& closure_var_refs() const { return closure_var_refs_; }
	auto& closure_var_refs() { return closure_var_refs_; }

	const auto& lexical_this() const { return lexical_this_; }
	void set_lexical_this(Value&& lexical_this) { lexical_this_ = lexical_this; }

private:
	// ClosureVar*
	std::vector<Value> closure_var_refs_;

	// 词法作用域捕获的this
	Value lexical_this_;
};

class FunctionObject : public Object {
public:
	FunctionObject(Context* context, FunctionDef* function_def) noexcept;
	~FunctionObject() = default;

	void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) override {
		Object::GCForEachChild(context, list, callback);
		closure_env_.GCForEachChild(context, list, callback);
	}

	Value ToString(Context* context) override {
		return Value(String::Format("function_object:{}", function_def_->name()));
	}

	FunctionDef& function_def() const { return *function_def_; }

	const auto& closure_env() const { return closure_env_; }
	auto& closure_env() { return closure_env_; }
	
protected:
	FunctionDef* function_def_;

	// 闭包
	ClosureEnvironment closure_env_;
};

} // namespace mjs