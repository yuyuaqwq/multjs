#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <format>

#include <mjs/function_def.h>
#include <mjs/object/object.h>

namespace mjs {

class FunctionObject : public Object {
public:
	FunctionObject(Context* context, FunctionDef* function_def) noexcept;

	virtual void ForEachChild(intrusive_list<Object>* list, void(*callback)(intrusive_list<Object>* list, const Value& child)) override {
		Object::ForEachChild(list, callback);
		callback(list, parent_function_);
		for (auto& val : closure_value_arr_) {
			callback(list, val);
		}
	}

	virtual Value ToString() override {
		return Value(std::format("function_object:{}", function_def_->name()));
	}

	auto& function_def() const { return *function_def_; }

	const auto& parent_function() const { return parent_function_; }
	auto& parent_function() { return parent_function_; }
	void set_parent_function(Value parent_function) { parent_function_ = std::move(parent_function); }

	const auto& closure_value_arr() const { return closure_value_arr_; }
	auto& closure_value_arr() { return closure_value_arr_; }
	
private:
	FunctionDef* function_def_;

	// 父函数的引用计数占用
	// 用于当前闭包被返回时，延长父函数的closure_value_arr_的生命周期
	Value parent_function_;

	// 当前函数被子函数捕获的值，不在放到栈上，而是提升到堆上(包括UpValue)
	std::vector<Value> closure_value_arr_;
};

} // namespace mjs