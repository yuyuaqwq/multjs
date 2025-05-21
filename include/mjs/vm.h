#pragma once

#include <stdint.h>

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include <mjs/value.h>
#include <mjs/const_pool.h>
#include <mjs/var_def.h>
#include <mjs/stack_frame.h>
#include <mjs/value.h>

namespace mjs {

class Context;
class VM : public noncopyable {
public:
	friend class CodeGener;

public:
	explicit VM(Context* context);

	void ModuleInit(Value* value);

	void BindModuleExportVars(StackFrame* stack_frame);

	template<typename It>
	Value CallFunction(StackFrame* stack_frame, Value func_val, Value this_val, It begin, It end) {
		// 参数正序入栈
		for (It it = begin; it != end; ++it) {
			stack_frame->push(*it);
		}

		CallInternal(stack_frame, std::move(func_val), std::move(this_val), std::distance(begin, end));

		return stack_frame->pop();
	}

private:
	Value& GetVar(StackFrame* stack_frame, VarIndex idx);
	void SetVar(StackFrame* stack_frame, VarIndex idx, Value&& var);

	void Closure(const StackFrame& stack_frame, Value* func_def_val);

	void BindClosureVars(StackFrame* stack_frame);

	// 返回是否需要继续执行字节码
	bool FunctionScheduling(StackFrame* stack_frame, uint32_t par_count);

	void CallInternal(StackFrame* stack_frame, Value func_val, Value this_val, uint32_t param_count);

	void LoadConst(StackFrame* stack_frame, ConstIndex const_idx);

	bool ThrowExecption(StackFrame* stack_frame, std::optional<Value>* error_val);

private:
	void GeneratorSaveContext(StackFrame* stack_frame, GeneratorObject* generator);
	void GeneratorRestoreContext(StackFrame* stack_frame, GeneratorObject* generator);

	Stack& stack();
	FunctionDef* function_def(const Value& func_val) const;

private:
	Context* context_;
	// StackFrame stack_frame_;
};

} // namespace mjs